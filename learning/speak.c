#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "50607"
#define IP "localhost"

#define GREEN_T "\033[1;32m"
#define RED_T   "\033[1;31m"
#define RESET_T "\033[0m"


int main(void) {

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;
    int ret = getaddrinfo(IP, PORT, &hints, &res);

    if (ret) {
        perror(RED_T "call to getaddrinfo() failed.\n" RESET_T);
        printf("%s\n", gai_strerror(ret));
        return -1;
    }

    char l_out[INET_ADDRSTRLEN];

    struct sockaddr *a = res->ai_addr;
    struct sockaddr_in *a_b = (struct sockaddr_in *) a;

    if (!inet_ntop(res->ai_family, &(a_b->sin_addr), l_out, INET_ADDRSTRLEN)) {
        freeaddrinfo(res);
        return -1;
    }

    printf("found possible connection at %s:%hu\n", l_out, (ushort) ntohs(a_b->sin_port));

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sock == -1) {
        perror(RED_T "call to socket() failed\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }

    ret = connect(sock, res->ai_addr, res->ai_addrlen);
    if (ret == -1) {
        perror(RED_T "call to connect() failed.\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }
	printf("You can now message freely!\n");
	while (0 != 1) {
		char msg[1024];
		fgets(msg, 1024, stdin);
		int b = send(sock, msg, sizeof(msg), 0);
		printf(GREEN_T "sent %d out of %d bytes!\n" RESET_T, b, (int) sizeof(msg));
		if (strcmp(msg, "/stop") == 0) break;
	}
    freeaddrinfo(res);

    return 0;
}
