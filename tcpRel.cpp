#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <cstdlib>

using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>

enum { RC_OK = 0, RC_NG = -1, INVALID_SOCK = -1 };

const short SERVER_PORT = 23432;

const size_t BUFLEN = 2048;

static bool s_lingerZero = false;

static void myPerror(const char *msg) {
    ::perror(msg);
    ::exit(30);
}

static void myMicroSleep(long microSec) {
    const long mega = 1000 * 1000;
    ::timespec sleepDur = { microSec / mega, 1000 * (microSec % mega) }, rest;
    errno = 0;
    while (::nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
    if (errno)
        myPerror("nanosleep(2)");
}

static void printUsage() {
    cout << "tcpRel [-S|-C] [-l]" << endl;
    cout << "    -C: client mode (default)" << endl;
    cout << "    -S: server mode" << endl;
    cout << "    -l: use SO_LINGER" << endl;
    ::exit(10);
}

string toString(const ::timespec &ts) {
    ::tm theTm;
    ::localtime_r(&ts.tv_sec, &theTm);
    char buf[BUFLEN];
    ::strftime(buf, sizeof buf, "%H:%M:%S.", &theTm);
    ostringstream oss;
    oss << buf << std::setw(9) << std::setfill('0') << ts.tv_nsec ;
    return oss.str();
}

static void doClient(size_t len, const string &serverAddr) {
    cout << __PRETTY_FUNCTION__ << endl;

    const int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCK == sock)
        myPerror("socket(2)");

    const int opt = 1;
    if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt))
        myPerror("setsockopt(2) with SO_REUSEADDR");

    ::sockaddr_in sin = { };
    sin.sin_family = AF_INET;

    if (s_lingerZero) {
        const ::linger l = { 1, 0 };
        if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_LINGER, &l, sizeof l))
            myPerror("setsockopt(2) with SO_LINGER");

        sin.sin_port = htons(23333);
        if (1 != ::inet_pton(AF_INET, "0.0.0.0", &sin.sin_addr)) {
            cerr << "inet_pton(3) with \"0.0.0.0\"" << endl;
            ::exit(20);
        }

        if (RC_NG == ::bind(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
            myPerror("bind(2)");
    }

    sin.sin_port = htons(SERVER_PORT);
    ::inet_pton(AF_INET, serverAddr.c_str(), &sin.sin_addr);

    if (RC_NG == ::connect(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
        myPerror("connect(2)");

    string data(len, 'A');
    for (int i = 0; i < 1; ++i) {
        if (0 < i)
            myMicroSleep(500*1000);

        const ssize_t nSent = ::send(sock, data.data(), data.size(), 0);
        if (data.size() != nSent)
            myPerror("send(2)");
        /*
        ::timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        cout << toString(ts) << ": " << i << ": sent [" << data << "]" << endl;
        */
    }
    ::close(sock);
}

static void doServer() {
    cout << __PRETTY_FUNCTION__ << endl;

    const int svrSock = ::socket(AF_INET, SOCK_STREAM, 0);

    const int opt = 1;
    if (RC_NG == ::setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt))
        myPerror("setsockopt(2) with SO_REUSEADDR");

    ::sockaddr_in sin = { };
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    ::inet_pton(AF_INET, "0.0.0.0", &sin.sin_addr);
    if (RC_NG == ::bind(svrSock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
        myPerror("bind(2)");

    if (RC_NG == ::listen(svrSock, 1))
        myPerror("listen(2)");

    const int cliSock = ::accept(svrSock, NULL, NULL);
    if (INVALID_SOCK == cliSock)
        myPerror("accept(2)");

    /*
    const char welcome[] = "WELCOME";
    if (sizeof welcome - 1 != ::send(cliSock, welcome, sizeof welcome - 1, 0))
        myPerror("send(2)");
    */

    myMicroSleep(200 * 1000);
    for (int i = 0; i < 3; ++i) {
        if (0 < i)
            myMicroSleep(500*1000);

        char buf[100*BUFLEN];
        const ssize_t nRecv = ::recv(cliSock, buf, sizeof buf, 0);
        if (0 >= nRecv)
            myPerror("recv(2)");

        ::timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        cout << toString(ts) << ": " << i << ": received [";
        cout.write(buf, nRecv);
        cout << "]" << endl;
    }
}

enum EnumMode { EnumModeServer, EnumModeClient };

int main(int argc, char *argv[]) {
    EnumMode mode = EnumModeClient;
    string serverAddr("127.0.0.1");
    size_t len = BUFLEN;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "SC:lL:h"))) {
        switch (optchr) {
            case 'S': mode = EnumModeServer; break;
            case 'C': mode = EnumModeClient; serverAddr.assign(::optarg); break;

            case 'l': s_lingerZero = true; break;

            case 'L': len = ::strtoull(::optarg, NULL, 0); break;

            case 'h':
            default: printUsage();
        }
    }

    if (EnumModeClient == mode)
        doClient(len, serverAddr);
    else
        doServer();

    myMicroSleep(500 * 1000);
}
