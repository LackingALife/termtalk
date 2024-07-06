#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <pthread.h>

#define PORT_STRLEN 6
#define TIME_OUT 5
#define NUM_OF_MAX_CONNECT 1

#define GREEN_T "\033[1;32m"
#define RED_T   "\033[1;31m"
#define RESET_T "\033[0m"

typedef enum {
    IP, PORT
} OPTIONS;

typedef enum {
	BIND, CONNECT
} MODE;

typedef enum {
	RECEIVE, SEND
} THREAD;

typedef struct {
	struct addrinfo *target;
	int sock;
} thread_args;

void usage(char *name) {
	printf("Usage: %s --ip=<ip> --port=<port>\n", name);
}


int process_opts(char *ip, char *port, int argc, char **argv) {
	int opt;
	struct option options[2] = {0}; // ip and port
	options[0].name = "ip";
	options[0].has_arg = 1;
	options[0].val = IP;

	options[1].name = "port";
	options[1].has_arg = 1;
	options[1].val = PORT;

	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case IP:
				strcpy(ip, optarg);
				break;
			case PORT:
				strcpy(port, optarg);
				break;
			case '?':
			default:
				return -1;
		}
	}

	return optind;
}

int check_addr(char *ip, char *port, struct addrinfo hints, struct addrinfo **target, char *target_name) {
	int ret;	
	if (!target_name) target_name = "Unknown";
	ret = getaddrinfo(ip, port, &hints, *(&target));
	if (ret) {
		fprintf(stderr, RED_T "getaddrinfo() to %s failed: %s\n" RESET_T, target_name, gai_strerror(ret));
		return -1;
	}
	int i = 0; 
	for (struct addrinfo *helper = *target; helper; helper = helper->ai_next){
		i++;
	}
	printf("Found %d possible peers!\n", i);
	return 0;
}

int diff_time(struct timespec time1, struct timespec time2) {

	int ret;
	ret = time1.tv_sec - time2.tv_sec;
	return ret;
}

int get_local_ip(char *local_ip) {

	struct ifaddrs *addrs, *tmp;

	if (getifaddrs(&addrs) == -1) { 
		return -1;
	}
	tmp = addrs;

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
			if (strcmp("127.0.0.1", inet_ntoa(pAddr->sin_addr)) == 0 ) { 
				tmp = tmp->ifa_next;
				continue;
			}
			strcpy(local_ip, inet_ntoa(pAddr->sin_addr));
			break;
		}

		tmp = tmp->ifa_next;
	}


	freeifaddrs(addrs);


	return 0;

}

int setup_socket(MODE mode, struct addrinfo *target){

	int ret;

	int sock = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (sock == -1) {
		fprintf(stderr, RED_T "socket() failed\n" RESET_T);
		return -1;
	}
	if (mode == BIND) {
		ret = bind(sock, target->ai_addr, target->ai_addrlen);
		if (ret == -1) {
			fprintf(stderr, RED_T "bind() failed\n" RESET_T);
			return -1;
		}
	} else if (mode == CONNECT) {
		struct timespec initial_time, current_time;
		clock_gettime(CLOCK_MONOTONIC, &initial_time);
		while (1) {
			//printf("%d\n", sock);
			ret = connect(sock, target->ai_addr, target->ai_addrlen);
			//printf("%d\n", sock);
			if (ret != -1) {
				printf("Connected to peer!\n");
				return sock;
			}
			clock_gettime(CLOCK_MONOTONIC, &current_time);
			if (diff_time(current_time, initial_time) > TIME_OUT){
				fprintf(stderr, RED_T "connect() timed out!\n" RESET_T);
				return -1;
			}
		}
	} else {
		fprintf(stderr, RED_T "setup_socket() failed\n" RESET_T);
		return -1;
	}

	return sock;
}

void print_addr(struct addrinfo *target) {

	if (!target) return;

    char l_out[INET_ADDRSTRLEN];

    struct sockaddr *a = target->ai_addr;
    struct sockaddr_in *a_b = (struct sockaddr_in *) a;

    if (!inet_ntop(target->ai_family, &(a_b->sin_addr), l_out, INET_ADDRSTRLEN)) {
		fprintf(stderr, RED_T "inet_ntop() failed in print_addr()!\n" RESET_T);
        return;
    }

    printf("%s:%hu\n", l_out, ntohs(a_b->sin_port));

}

static int TO_END = 0;

void *receive_thread(void *arg){
	thread_args *args = (thread_args *) arg;		
	struct addrinfo *ip = args->target;
	int sock = args->sock;
	char buf[1024];
    while (TO_END == 0) {
        int b = recv(sock, buf, sizeof(buf), 0);

        if (b <= 0) {
			print_addr(ip);
            printf(" disconnected!\n");
			close(sock);
            break;
        }
        if (strcmp(buf, "/stop\n") == 0) {
			print_addr(ip);
            printf(" ended the chat!\n Press ENTER to leave\n");
			close(sock);
	        break;
        }
        printf("%s", buf);
    }
	TO_END = 1;	
	fflush(stdin);
	return 0;
}

void *send_thread(void *arg){
	thread_args *args = (thread_args *) arg;		
	int sock = args->sock;
	char msg[1024];
	while (TO_END == 0) {
		fgets(msg, 1024, stdin);
		int b = send(sock, msg, sizeof(msg), 0);
		if (strcmp(msg, "/stop\n") == 0) {
			printf("Stopping chat!\n");

			close(sock);
			break;
		}
		//printf(GREEN_T "sent %d out of %d bytes!\n" RESET_T, b, (int) sizeof(msg));
	}
	TO_END = 1;
	return 0;
}


int main(int argc, char **argv){ int ret;
	char ip[INET_ADDRSTRLEN] = {0};
	char port[PORT_STRLEN] = {0};

	ret = process_opts(ip, port, argc, argv);
	if (ret != 3) {
		printf("%s:%s\n", ip, port);

		usage(*argv);
		return -1;
	}

	struct addrinfo hints, *local, *peer;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	//Custom function
	//if (check_addr("localhost", port, hints, &local, "local") == -1) return -1;

	if (check_addr(ip, port, hints, &peer, "peer") == -1) return -1;

	//print_addr(local);
	print_addr(peer);

	int local_sock, peer_sock;

	printf("Trying to connect!\n");	
	peer_sock = setup_socket(CONNECT, peer);

	if (peer_sock == -1) {
		printf("Couldn't connect to peer. Opening Socket.\n");

		char local_ip[INET_ADDRSTRLEN];
		if (get_local_ip(local_ip) == -1) {
			fprintf(stderr, RED_T "Failed to get_local_ip()\n" RESET_T);
			return -1;
		}

		if (check_addr(local_ip, port, hints, &local, "local") == -1) return -1;
		print_addr(local);
		local_sock = setup_socket(BIND, local);		

		ret = listen(local_sock, NUM_OF_MAX_CONNECT);
		if (ret == -1) {
			fprintf(stderr, RED_T "call to listen() failed.\n" RESET_T);
			return -1;
		}
		struct sockaddr_storage new_st;
		struct sockaddr new_sa = *(struct sockaddr*) &new_st;

		socklen_t len = sizeof(new_st);

		peer_sock = accept(local_sock, &new_sa, &len);
		if (peer_sock == -1) {
			fprintf(stderr, RED_T "call to accept() failed.\n" RESET_T);
			return -1;
		}

		struct sockaddr_in new_sai = *(struct sockaddr_in *) &new_sa;
		char peer_ip[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &new_sai.sin_addr, peer_ip, INET_ADDRSTRLEN)) {
			fprintf(stderr, GREEN_T "connected to %s!\n" RESET_T, peer_ip);
		}
	} 

	pthread_t thread_id[2];
	thread_args arg;
	
	arg.sock = peer_sock;
	arg.target = peer;

	pthread_create(&thread_id[RECEIVE], NULL, receive_thread, &arg);

	pthread_create(&thread_id[SEND], NULL, send_thread, &arg);

	for (int i = 0; i < 2; i++) {
		pthread_join(thread_id[i], NULL);
	}
		
	fflush(stdin);
	return 0;
}
