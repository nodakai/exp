#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };

static long myAtoi(const char *str)
{
    char *end;
    const long ret = strtol(str, &end, 10);
    if ('\0' == *end)
        return ret;
    fprintf(stderr, "myAtoi: invalid argument [%s]", str);
    exit(20);
}

static void myPerror(const char *msg)
{
    perror(msg);
    exit(10);
}

int main(int argc, char *argv[])
{
    int i, sock, specifiedBuflen = 0, len = 200, N = 100000;
    long elapsedUs;
    struct sockaddr_in sin;
    struct timespec ts0, ts1;
    char *buf, optchr;

    while (-1 != (optchr = getopt(argc, argv, "b:n:l:"))) {
        switch (optchr) {
            case 'b': specifiedBuflen = myAtoi(optarg); break;
            case 'n': N = myAtoi(optarg); break;
            case 'l': len = myAtoi(optarg); break;
        }
    }

    buf = malloc(len);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    {
        int buflen;
        socklen_t len = sizeof buflen;
        if (RC_NG == getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, &len) || sizeof buflen != len)
            myPerror("getsockopt(2) with SO_SNDBUF");
        printf("buflen == %d (%f MB)\n", buflen, buflen / 1024. / 1024.);

        buflen = (specifiedBuflen ? specifiedBuflen : 600 * 1024);
        if (RC_NG == setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof buflen))
            myPerror("setsockopt(2) with SO_SNDBUF");

        if (RC_NG == getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, &len) || sizeof buflen != len)
            myPerror("getsockopt(2) with SO_SNDBUF");
        printf("buflen == %d (%f MB)\n", buflen, buflen / 1024. / 1024.);
    }

    sin.sin_family = AF_INET;
    if (1 != inet_pton(AF_INET, argv[optind++], &sin.sin_addr))
        myPerror("inet_pton(3)");
    sin.sin_port = htons(atoi(argv[optind++]));

    memset(buf, '_', len);

    clock_gettime(CLOCK_REALTIME, &ts0);
    for (i = 0; i < N; ++i) {
        buf[5] = '0' + i % 10;
        buf[4] = '0' + (i / 10) % 10;
        buf[3] = '0' + (i / 100) % 10;
        buf[2] = '0' + (i / 1000) % 10;
        buf[1] = '0' + (i / 10000) % 10;
        buf[0] = '0' + (i / 100000) % 10;

        sendto(sock, buf, len, 0, (const struct sockaddr *)&sin, sizeof sin);
    }
    clock_gettime(CLOCK_REALTIME, &ts1);

    close(sock);

    free(buf);

    elapsedUs = (ts1.tv_sec - ts0.tv_sec) * 1000 * 1000 + (ts1.tv_nsec - ts0.tv_nsec) / 1000;
    printf("%ld us;  %f MB/s;  %f MB in total;\n", elapsedUs, len * N / 1024. / 1024. / elapsedUs * 1e6, len * N / 1024. / 1024.);

    return 0;
}
