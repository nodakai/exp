#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <getopt.h>
#include <arpa/inet.h>
#include <linux/un.h>
#define __USE_POSIX 1
#include <netdb.h>

char errbuf[1024];
char tmp0[16], tmp1[16], tmp2[16], tmp3[INET_ADDRSTRLEN];

typedef void (*testFun_t)(char **names, int ni);

const char *addrtypeStr(int typ, char *buf, size_t buflen)
{

#define CASE(id)                        \
    case id:                            \
        strncpy(buf, #id, buflen);      \
        break

    switch (typ) {
        CASE(AF_INET);
        CASE(AF_INET6);
        default:
            snprintf(buf, buflen, "Unknown[%d]", typ);
            break;
    }
    return buf;
}

const char *socktypeStr(int typ, char *buf, size_t buflen)
{
    switch (typ) {
        CASE(SOCK_STREAM);
        CASE(SOCK_DGRAM);
        CASE(SOCK_RAW);
        CASE(SOCK_RDM);
        CASE(SOCK_SEQPACKET);
        CASE(SOCK_PACKET);
        default:
            snprintf(buf, buflen, "Unknown[%d]", typ);
            break;
    }
    return buf;
}

const char *protoStr(int typ, char *buf, size_t buflen)
{
    switch (typ) {
        CASE(IPPROTO_IP);
        CASE(IPPROTO_ICMP);
        CASE(IPPROTO_IGMP);
        CASE(IPPROTO_IPIP);
        CASE(IPPROTO_TCP);
        CASE(IPPROTO_EGP);
        CASE(IPPROTO_PUP);
        CASE(IPPROTO_UDP);
        CASE(IPPROTO_IDP);
        CASE(IPPROTO_TP);
        CASE(IPPROTO_IPV6);
        CASE(IPPROTO_ROUTING);
        CASE(IPPROTO_FRAGMENT);
        CASE(IPPROTO_RSVP);
        CASE(IPPROTO_GRE);
        CASE(IPPROTO_ESP);
        CASE(IPPROTO_AH);
        CASE(IPPROTO_ICMPV6);
        CASE(IPPROTO_NONE);
        CASE(IPPROTO_DSTOPTS);
        CASE(IPPROTO_MTP);
        CASE(IPPROTO_ENCAP);
        CASE(IPPROTO_PIM);
        CASE(IPPROTO_COMP);
        CASE(IPPROTO_SCTP);
        CASE(IPPROTO_RAW);
        default:
            snprintf(buf, buflen, "Unknown[%d]", typ);
            break;
    }
    return buf;
}

const char *sockaddrStr(const struct sockaddr *sa, int af, char *buf, size_t buflen)
{
    switch (af) {
        case AF_INET:
        case AF_INET6:
            if (NULL == inet_ntop(af, &((struct sockaddr_in *)sa)->sin_addr, buf, buflen)) {
                perror("inet_ntop()");
                exit(20);
            }
            break;
        case AF_UNIX:
            strncpy(buf, ((struct sockaddr_un *)sa)->sun_path, buflen);
            break;
        default:
            strncpy(buf, "Unknown", buflen);
    }
    return buf;
}

void testOld(char **names, int ni)
{
    const char *name;
    for (name = names[ni]; name; name = names[++ni]) {
        int i, j;
        char *str;
        struct hostent *pHost;

        printf("gethostbyname(\"%s\")\n", name);
        if (NULL == (pHost = gethostbyname(name))) {
            snprintf(errbuf, sizeof errbuf, "gethostbyname(\"%s\")", name);
            perror(errbuf);
            exit(10);
        }

        printf("struct hostent {\n"
            "  char *h_name == \"%s\";\n  char **h_aliases == {", pHost->h_name);
        for (i = 0, str = pHost->h_aliases[i]; str; str = pHost->h_aliases[++i]) {
            if (i == 0) {
                printf(" ");
            } else {
                printf(", ");
            }
            printf("\"%s\"", str);
        }

        if (i > 0)
            printf(" ");
        printf("};\n"
            "  int h_addrtype == %s;\n  int h_length == %d;\n  char **h_addr_list == {",
            addrtypeStr(pHost->h_addrtype, tmp0, sizeof tmp0), pHost->h_length);

        for (i = 0, str = pHost->h_addr_list[i]; str; str = pHost->h_addr_list[++i]) {
            if (i == 0) {
                printf(" ");
            } else {
                printf(", ");
            }
            for (j = 0; j < pHost->h_length; ++j) {
                if (j > 0)
                    printf(".");
                printf("%3d", str[j]&0xff);
            }
        }

        if (i > 0)
            printf(" ");
        printf("};\n};\n");
    }
}

void testNew(char **names, int ni)
{
    char *name;
    struct addrinfo hints, *pRes, *pAddr;
    int rc;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;

    for (name = names[ni]; name; name = names[++ni]) {
        printf("getaddrinfo(\"%s\")\n", name);
        if ((rc = getaddrinfo(name, NULL, &hints, &pRes))) {
            fprintf(stderr, "%2d-th getaddrinfo(\"%s\") => \"%s\"\n", ni, name, gai_strerror(rc));
            exit(10);
        }

        for (pAddr = pRes; pAddr; pAddr = pAddr->ai_next) {
            printf("struct addrinfo {\n"
                "  int ai_flags = %d\n  int ai_family = %s\n"
                "  int ai_socktype = %s\n  int ai_protocol = %s\n"
                "  size_t ai_addrlen = %lu\n  struct sockaddr *ai_addr = %s;\n"
                "  char *ai_canonname = \"%s\";\n  struct addrinfo *ai_next = %p\n};\n",
                pAddr->ai_flags, addrtypeStr(pAddr->ai_family, tmp0, sizeof tmp0),
                socktypeStr(pAddr->ai_socktype, tmp1, sizeof tmp1),
                protoStr(pAddr->ai_protocol, tmp2, sizeof tmp2),
                (unsigned long)pAddr->ai_addrlen,
                sockaddrStr(pAddr->ai_addr, pAddr->ai_family, tmp3, sizeof tmp3),
                pAddr->ai_canonname, pAddr->ai_next);
        }

        freeaddrinfo(pRes);
    }
}

int main(int argc, char *argv[])
{
    char optchr;
    testFun_t testFun = testOld;

    while (-1 != (optchr = getopt(argc, argv, "on"))) {
        switch (optchr) {
            case 'o':
                testFun = testOld;
                break;
            case 'n':
                testFun = testNew;
                break;
            default:
                perror("getopt()");
                exit(50);
        }
    }

    if (optind < argc) {
        testFun(argv, optind);
    } else {
        char *names[] = { "localhost", "google.com", "google.jp", NULL };
        testFun(names, 0);
    }

    return 0;
}
