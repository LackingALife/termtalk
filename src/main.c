#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT_STRLEN 6

#define GREEN_T "\033[1;32m"
#define RED_T   "\033[1;31m"
#define RESET_T "\033[0m"

typedef enum {
    IP, PORT
} OPTIONS;


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

int check_addr(char *ip, char *port, struct addrinfo hints, struct addrinfo *target) {
	int ret;	
	ret = getaddrinfo(ip, port, &hints, &target);
    if (ret) {
        fprintf(stderr, RED_T "getaddrinfo (Peer) failed: %s\n" RESET_T, gai_strerror(ret));
		return -1;
    }
	return 0;
}

int setup_socket(struct addrinfo *target){

	int sock = socket(target->ai_family, target->ai_socktype, target->ai_protocol);

	if (sock == -1) {
        fprintf(stderr, RED_T "socket failed\n" RESET_T);
		return -1;
	}
	
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
	if (check_addr(ip, port, hints, local) == -1) return -1;

	if (check_addr(ip, port, hints, peer) == -1) return -1;

	

    return 0;
}
