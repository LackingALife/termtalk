#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define IP "localhost"
#define PORT "50607"
#define N 10

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
        printf(RED_T "%s\n" RESET_T, gai_strerror(ret));
        return -1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sock == -1) {
        perror(RED_T "call to socket() failed.\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }

    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror(RED_T "call to setsockopt() failed." RESET_T);
        return -1;
    } 

    ret = bind(sock, res->ai_addr, res->ai_addrlen);
    if (ret == -1) {
        perror(RED_T "call to bind() failed.\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }

    printf("listening on port %s..\n", PORT);
    ret = listen(sock, N);
    if (ret == -1) {
        perror(RED_T "call to listen() failed.\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }

    struct sockaddr_storage new_st;
    struct sockaddr new_sa = *(struct sockaddr*) &new_st;

    socklen_t len = sizeof(new_st);

    int new_fd;
    new_fd = accept(sock, &new_sa, &len);
    if (new_fd == -1) {
        perror(RED_T "call to accept() failed.\n" RESET_T);
        freeaddrinfo(res);
        return -1;
    }

    struct sockaddr_in new_sai = *(struct sockaddr_in *) &new_sa;
    char ip_out[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &new_sai.sin_addr, ip_out, INET_ADDRSTRLEN)) {
        printf(GREEN_T "connected to %s!\n" RESET_T, ip_out);
    }

    while (1) {
        char buf[1024];
        int b = recv(new_fd, buf, sizeof(buf), 0);

        if (b <= 0) {
            printf("%s disconnected!", ip_out);
            break;
        }
        if (strcmp(buf, "/stop") == 0) {
            printf("%s ordered a /stop!", ip_out);
            break;
        }

        printf("%s\n", buf);
    }

    freeaddrinfo(res);

    return 0;
}
