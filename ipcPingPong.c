#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strcpy */
#include <time.h> /* clock_gettime */

#include <netdb.h> /* getaddrinfo */
#include <sys/socket.h> /* socket */
#include <sys/un.h> /* sockaddr_un */
#include <unistd.h> /* close, unlink */
#include <getopt.h> /* getopt */

#define BUFLEN 1024
char g_errbuf[BUFLEN];

enum EnumMode {
    EnumModeClient,
    EnumModeServer
};

enum EnumSocketType {
    EnumSocketTypeStream,
    EnumSocketTypeDgram
};

enum EnumIpcMethod {
    EnumIpcMethodSocketInet,
    EnumIpcMethodSocketUnix
};

const long GIGA = 1000 * 1000 * 1000;

static long myAtoi(char *arg)
{
    char *end;
    const long ret = strtol(arg, &end, 10);
    if ('\0' == *end)
        return ret;
    sprintf(g_errbuf, "strtol(\"%.20s\")", arg);
    perror(g_errbuf);
    exit(21);
}

static long diffTimespec(const struct timespec *t0, const struct timespec *t1)
{
    return (t1->tv_nsec - t0->tv_nsec)
        + GIGA * (t1->tv_sec - t0->tv_sec);
}

struct Message {
    int m_length;
    char m_body[];
};

const char g_host[] = "localhost";
const char g_port[] = "32301";

static void setSockOpts(int serverSock)
{
    int opt;
    struct linger l = { 0 };

    opt = 1;
    if (-1 == setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt)) {
        perror("setsockopt(2)");
        exit(47);
    }

    l.l_onoff = 1;
    l.l_linger = 0;
    if (-1 == setsockopt(serverSock, SOL_SOCKET, SO_LINGER, &l, sizeof l)) {
        perror("setsockopt(2)");
        exit(48);
    }
}

static void testSocketInetDgramServer(int nRep)
{
    int serverSock;
    int i, rc;
    struct addrinfo hints = { 0 }, *res;

    printf("Starting the INET DGRAM socket server...  ");
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if (0 != (rc = getaddrinfo(NULL, g_port, &hints, &res))) {
        fprintf(stderr, "%s", gai_strerror(rc));
        exit(24);
    }
    if (-1 == (serverSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol))) {
        perror("socket(2)");
        exit(29);
    }
    setSockOpts(serverSock);
    if (-1 == bind(serverSock, res->ai_addr, res->ai_addrlen)) {
        perror("bind(2)");
        exit(34);
    }
    printf("Done.\nBenchmarking...  ");

    for (i = 0; i < nRep; ++i) {
        int ri;
        struct sockaddr_storage ss = { 0 };
        socklen_t len = sizeof ss, dlen = sizeof ri;
        if (recvfrom(serverSock, &ri, dlen, 0, (struct sockaddr *)&ss, &len) < dlen) break;
        if (sendto(serverSock, &ri, dlen, 0, (struct sockaddr *)&ss, len) < dlen) break;
    }
    if (i != nRep)
        fprintf(stderr, "i==%d vs %d==nRep\n", i, nRep);

    printf("Done.\nQuitting...\n");
    if (-1 == close(serverSock)) {
        perror("close(serverSock)");
        exit(28);
    }
}

static void testSocketInetStreamServer(int nRep)
{
    int i, rc;
    int serverSock, connSock;
    struct addrinfo hints = { 0 }, *res;

    printf("Starting the INET STREAM socket server...  ");
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (0 != (rc = getaddrinfo(NULL, g_port, &hints, &res))) {
        fprintf(stderr, "%s", gai_strerror(rc));
        exit(41);
    }
    if (-1 == (serverSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol))) {
        perror("socket(2)");
        exit(42);
    }
    setSockOpts(serverSock);
    if (-1 == bind(serverSock, res->ai_addr, res->ai_addrlen)) {
        perror("bind(2)");
        exit(43);
    }
    if (-1 == listen(serverSock, 5)) {
        perror("listen(2)");
        exit(44);
    }
    printf("Done.\nWaiting for an incominc connection...  ");
    if (-1 == (connSock = accept(serverSock, NULL, NULL))) {
        perror("accept(2)");
        exit(45);
    }
    printf("Connected.\nBenchmarking...  ");

    for (i = 0; i < nRep; ++i) {
        int ri;
        if (0 >= recv(connSock, &ri, sizeof ri, 0)) break;
        if (0 >= send(connSock, &ri, sizeof ri, 0)) break;
    }
    if (i != nRep)
        fprintf(stderr, "i==%d vs %d==nRep\n", i, nRep);

    printf("Done.\nQuitting...\n");
    if (-1 == close(connSock)) {
        perror("close(serverSock)");
        exit(46);
    }
    if (-1 == close(serverSock)) {
        perror("close(serverSock)");
        exit(47);
    }
}

static void cgt(struct timespec *pTs)
{
    if (clock_gettime(CLOCK_REALTIME, pTs)) {
        perror("clock_gettime(CLOCK_REALTIME, &t0)");
        exit(50);
    }
}

static void testClientBody(int sock, int nRep)
{
    int i;
    struct timespec t0, t1;
    double elapsedTime;

    cgt(&t0);

    for (i = 0; i < nRep; ++i) {
        int ri;
        if (sizeof i != send(sock, &i, sizeof i, 0)) abort();
        if (sizeof ri != recv(sock, &ri, sizeof ri, 0)) abort();
        if (i != ri) abort();
    }

    cgt(&t1);
    elapsedTime = diffTimespec(&t0, &t1) * 1e-9;
    printf("Done.\nClosing the connection...  ");
    if (-1 == close(sock)) {
        perror("close(2)");
        exit(23);
    }
    printf("Closed.\nElapsed time: %4.2f sec (RTT %4.2f usec == 2 * %4.2f usec).\n",
        elapsedTime, 1e6 * elapsedTime / nRep, 1e6 * elapsedTime / nRep / 2);
    printf("Quitting...\n");
}

static void testSocketInetClient(int nRep, int socktype)
{
    int sock, rc;
    struct addrinfo hints = { 0 }, *res;

    puts("Connecting to the server...  ");
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    if (0 != (rc = getaddrinfo(g_host, g_port, &hints, &res))) {
        fprintf(stderr, "%s", gai_strerror(rc));
        exit(22);
    }
    if (-1 == (sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol))) {
        perror("socket(2)");
        exit(20);
    }
    if (-1 == connect(sock, res->ai_addr, res->ai_addrlen)) {
        perror("connect(2)");
        exit(21);
    }
    freeaddrinfo(res);
    puts("Connected.\nNow benchmarking...  ");
    testClientBody(sock, nRep);
}

const char g_unixServerSockPath[] = "/tmp/ipcPingPong-UnixDomainServerSock";
const char g_unixClientSockPath[] = "/tmp/ipcPingPong-UnixDomainClietnSock";

static void testSocketUnixDgramServer(int nRep)
{
    int i, serverSock;
    struct sockaddr_un sun = { 0 };

    printf("Starting the UNIX domain DGRAM socket server...  ");
    if (-1 == (serverSock = socket(AF_UNIX, SOCK_DGRAM, 0))) {
        perror("socket(2)");
        exit(30);
    }
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, g_unixServerSockPath);
    unlink(g_unixServerSockPath);
    if (-1 == bind(serverSock, (const struct sockaddr *)&sun, sizeof sun)) {
        perror("bind(2)");
        exit(31);
    }
    printf("Done.\nBenchmarking...  ");

    for (i = 0; i < nRep; ++i) {
        int ri;
        struct sockaddr_storage ss = { 0 };
        socklen_t len = sizeof ss, dlen = sizeof ri;
        if (recvfrom(serverSock, &ri, dlen, 0, (struct sockaddr *)&ss, &len) < dlen) break;
        if (sendto(serverSock, &ri, dlen, 0, (const struct sockaddr *)&ss, len) < dlen) break;
    }

    printf("Done.\nQuitting...\n");
    if (-1 == close(serverSock)) {
        perror("close(serverSock)");
        exit(28);
    }
}

static void testSocketUnixStreamServer(int nRep)
{
    int i, serverSock, connSock;
    struct sockaddr_un sun = { 0 };

    printf("Starting the UNIX domain STREAM socket server...  ");
    if (-1 == (serverSock = socket(AF_UNIX, SOCK_STREAM, 0))) {
        perror("socket(2)");
        exit(30);
    }
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, g_unixServerSockPath);
    unlink(g_unixServerSockPath);
    if (-1 == bind(serverSock, (const struct sockaddr *)&sun, sizeof sun)) {
        perror("bind(2)");
        exit(31);
    }
    if (-1 == listen(serverSock, 5)) {
        perror("listen(2)");
        exit(32);
    }
    printf("Done.\nWaiting for ain incomming connection...  ");
    if (-1 == (connSock = accept(serverSock, NULL, NULL))) {
        perror("accept(2)");
        exit(33);
    }
    printf("Connected.\nBenchmarking...  ");

    for (i = 0; i < nRep; ++i) {
        int ri;
        if (0 >= recv(connSock, &ri, sizeof ri, 0)) break;
        if (0 >= send(connSock, &ri, sizeof ri, 0)) break;
    }
    if (i != nRep)
        fprintf(stderr, "i==%d vs %d==nRep", i, nRep);

    printf("Done.\nQuitting...\n");
    if (-1 == close(serverSock)) {
        perror("close(serverSock)");
        exit(28);
    }
}

static void testSocketUnixClient(int nRep, int socktype)
{
    int sock;
    struct sockaddr_un sun = { 0 };

    puts("Connecting to the server...  ");
    if (-1 == (sock = socket(AF_UNIX, socktype, 0))) {
        perror("socket(AF_UNIX)");
        exit(32);
    }
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, g_unixClientSockPath);
    unlink(g_unixClientSockPath);
    if (-1 == bind(sock, (const struct sockaddr *)&sun, sizeof sun)) {
        perror("connect(2)");
        exit(45);
    }

    strcpy(sun.sun_path, g_unixServerSockPath);
    if (-1 == connect(sock, (const struct sockaddr *)&sun, sizeof sun)) {
        perror("connect(2)");
        exit(33);
    }
    puts("Connected.\nNow benchmarking...  ");
    testClientBody(sock, nRep);
}

void createIpcId(const char *pfx, char *outbuf)
{
    char timebuf[BUFLEN];
    struct timespec now_ts;
    clock_gettime(CLOCK_REALTIME, &now_ts);
    struct tm now_tm;
    localtime_r(&now_ts.tv_sec, &now_tm);
    strftime(timebuf, sizeof timebuf, "%Y%m%d%H%M%S", &now_tm);
    sprintf(outbuf, "%s%s%06ld", pfx, timebuf, now_ts.tv_nsec / 1000);
}

int main(int argc, char *argv[])
{
    enum EnumMode mode = EnumModeServer;
    enum EnumSocketType sockType = EnumSocketTypeStream;
    enum EnumIpcMethod ipcMethod = EnumIpcMethodSocketInet;
    int nRep = 10000;
    char ipcId[BUFLEN] = { 0 };

    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "n:sdiuCS"))) {
        switch (optchr) {
            case 'n': nRep = myAtoi(optarg); break;

            case 's': sockType = EnumSocketTypeStream; break;
            case 'd': sockType = EnumSocketTypeDgram; break;

            case 'i': ipcMethod = EnumIpcMethodSocketInet; break;
            case 'u': ipcMethod = EnumIpcMethodSocketUnix; break;

            case 'C': mode = EnumModeClient; break;
            case 'S': mode = EnumModeServer; break;

            default:
                exit(40);
        }
    }

    /* printf("ipcId == \"%s\"\n", ipcId); */

    switch (ipcMethod) {
        case EnumIpcMethodSocketInet:
            if (EnumModeServer == mode) {
                if (EnumSocketTypeDgram == sockType)
                    testSocketInetDgramServer(nRep);
                else
                    testSocketInetStreamServer(nRep);
            } else
                testSocketInetClient(nRep, (EnumSocketTypeDgram == sockType ? SOCK_DGRAM : SOCK_STREAM));
            break;

        case EnumIpcMethodSocketUnix:
            if (EnumModeServer == mode) {
                if (EnumSocketTypeDgram == sockType)
                    testSocketUnixDgramServer(nRep);
                else
                    testSocketUnixStreamServer(nRep);
            } else
                testSocketUnixClient(nRep, (EnumSocketTypeDgram == sockType) ? SOCK_DGRAM : SOCK_STREAM);
            break;
    }

    return 0;
}
