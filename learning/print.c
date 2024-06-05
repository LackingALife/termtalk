#include <stdio.h>
#include <arpa/inet.h>

int main(void) {
    char *l_in = "127.0.0.1";
    char l_out[INET_ADDRSTRLEN];

    struct sockaddr_in me;
    if (inet_pton(AF_INET, l_in, &(me.sin_addr)) != 1) return -1;

    if (!inet_ntop(AF_INET, &(me.sin_addr), l_out, INET_ADDRSTRLEN)) return -1;
    printf("%s", l_out);

    return 0;
}
