#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT_STRLEN 6

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
	return 0;
}

int diff_time(struct timespec time1, struct timespec time2) {

	int ret;
	ret = time1.tv_sec - time2.tv_sec;
	return ret;
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
		while(1) {
			ret = connect(sock, target->ai_addr, target->ai_addrlen);
			if (ret != -1) {
				break;
			}
			clock_gettime(CLOCK_MONOTONIC, &current_time);
			if (diff_time(current_time, initial_time) > 5) {
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

void *receive_thread(void *arg){
	thread_args *args = (thread_args *) arg;		

	
	return 0;
}

void *send_thread(void *arg){
	thread_args *args = (thread_args *) arg;		

	return 0;
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

    printf("%s:%hi\n", l_out, ntohs(a_b->sin_port));

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
	if (check_addr("localhost", port, hints, &local, "local") == -1) return -1;

	if (check_addr(ip, port, hints, &peer, "peer") == -1) return -1;

	print_addr(local);
	print_addr(peer);

	int receive_sock, send_sock;
	receive_sock = setup_socket(BIND, local);
	if (receive_sock == -1) return -1;

	send_sock = setup_socket(CONNECT, peer);
	if (send_sock == -1) return -1;
	
	pthread_t thread_id[2];
	thread_args args[2];

	args[RECEIVE].target = local;
	args[RECEIVE].sock = receive_sock;

	args[SEND].target = peer;
	args[SEND].sock = send_sock;
	
	static int END = 0;

	pthread_create(&thread_id[RECEIVE], NULL, receive_thread, &args[RECEIVE]);

	pthread_create(&thread_id[SEND], NULL, send_thread, &args[SEND]);

	for (int i = 0; i < 2; i++) {
		pthread_join(thread_id[i], NULL);
	}
	
	return 0;
}
