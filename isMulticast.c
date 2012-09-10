#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h> /* in_addr_t */
#include <netdb.h> /* getaddrinfo */

enum { RC_OK = 0, RC_NG = -1 };

static void check(const char *addr)
{
    int rc;
    struct addrinfo hints = { 0 }, *res;
    in_addr_t iat, masked;

    hints.ai_family = AF_INET;
    if (RC_NG == (rc = getaddrinfo(addr, NULL, &hints, &res))) {
        fprintf(stderr, "getaddrinfo(\"%s\"): %s", addr, gai_strerror(rc));
        freeaddrinfo(res);
        exit(20);
    }

    iat = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
    masked = iat & 0xF0;
    printf("iat == 0x%08X;  iat & (0xF0) == 0x%02X;  [%3.3s]\n", iat, masked, 0xE0 == masked ? "Yes" : "No");

    freeaddrinfo(res);
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; ++i) {
        if (i > 1)
            printf("\n");
        printf("Tests if \"%.20s\" is an IPv4 multicast address or not...\n", argv[i]);
        check(argv[i]);
    }
    return EXIT_SUCCESS;
}
