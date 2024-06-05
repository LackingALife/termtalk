#include <netinet/in.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(void) {
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    int ret = getaddrinfo("localhost", "25565", &hints, &res);

    if (ret != 0) {
        printf("%s", gai_strerror(ret));
        return -1;
    }

    char l_out[INET_ADDRSTRLEN];

    struct sockaddr *a = res->ai_addr;
    struct sockaddr_in *a_b = (struct sockaddr_in *) a;

    inet_ntop(res->ai_family, &(a_b->sin_addr), l_out, INET_ADDRSTRLEN);
    printf("%s:%hi", l_out, ntohs(a_b->sin_port));

    return 0;
}
