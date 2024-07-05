#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
	READ, WRITE
} PIPE;

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

int check_addr(char *ip, char *port, struct addrinfo hints, struct addrinfo *target, char *target_name) {
	int ret;	
	if (!target_name) target_name = "Unknown";
	ret = getaddrinfo(ip, port, &hints, &target);
	if (ret) {
		fprintf(stderr, RED_T "getaddrinfo() to %s failed: %s\n" RESET_T, target_name, gai_strerror(ret));
		return -1;
	}
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
		ret = connect(sock, target->ai_addr, target->ai_addrlen);
		if (ret == -1) {
			fprintf(stderr, RED_T "connect() failed\n" RESET_T);
			return -1;
		}
	} else {
		fprintf(stderr, RED_T "setup_socket() failed\n" RESET_T);
		return -1;
	}

	return sock;
}

void *recieve_thread(void *arg){
	thread_args *args = (thread_args *) arg;		

	return 0;
}

void *send_thread(struct addrinfo *target, int sock){

	return 0;
}

int main(int argc, char **argv){
	int ret;
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
	hints.ai_flags = AI_PASSIVE;

	//Custom function
	if (check_addr(NULL, port, hints, local, "local") == -1) return -1;

	if (check_addr(ip, port, hints, peer, "peer") == -1) return -1;

	int recieve_sock, send_sock;

	if ((recieve_sock = setup_socket(BIND, local)) == -1) return -1;

	if ((send_sock = setup_socket(CONNECT, peer)) == -1) return -1;
	
	pthread_t thread_id[2];
	thread_args args[2];
	
	//Pipes are not needed yet
	/*
	int pipes[2][2];
	
	for (int i = 0; i < 2; i++) {
		pipe(pipes[i]);
	}
	*/


	args[RECIEVE]->addrinfo = local;
	args[RECIEVE]->sock = recieve_sock;

	args[SEND]->addrinfo = peer;
	args[SEND]->sock = send_sock;
	
	static int END = 0;

	pthread_create()

	return 0;
}
