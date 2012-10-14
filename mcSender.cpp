#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

#include <cstdio>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <netdb.h>
#include <time.h>

enum { RC_OK = 0, RC_NG = -1 };

int g_buflen = 0;

static long myAtoi(const char *str)
{
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' == *end)
        return ret;
    else
        throw std::invalid_argument(string("myAtoi: invalid argument [") + str + "]");
}

static void myPerror(const char *msg)
{
    ::perror(msg);
    throw std::runtime_error(msg);
}

static void myMicroSleep(const long microSec)
{
    ::timespec sleepDur = { microSec / 1000 / 1000, 1000 * (microSec % (1000 * 1000)) }, rest;
    while (RC_NG == ::nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
}

static void split(const string &orig, string &ipAddr, string &port)
{
    const auto commaPos = orig.find_first_of(',');
    if (string::npos == commaPos || (orig.size() - 1) == commaPos || 0 == commaPos)
        throw std::invalid_argument(string("split: invalid argument [") + orig + "]");

    ipAddr = orig.substr(0, commaPos);
    port = orig.substr(commaPos + 1);
}

static int setupSocket(const string &ipAddr, const string &port)
{
    cout << "setupSocket(" << ipAddr << ", " << port << ");" << endl;

    ::addrinfo hints = { }, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    const int rc = ::getaddrinfo(ipAddr.c_str(), port.c_str(), &hints, &res);
    if (RC_OK != rc) {
        ::freeaddrinfo(res);
        cerr << ::gai_strerror(rc) << endl;
        throw std::runtime_error(::gai_strerror(rc));
    }

    const int sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (RC_NG == sock)
        myPerror("socket(2)");

    int reuse = 1;
    if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse))
        myPerror("setsockopt(2) with SO_REUSEADDR");

    ::sockaddr_in sin = *reinterpret_cast<const ::sockaddr_in *>(res->ai_addr);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (RC_NG == ::bind(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
        myPerror("bind(2)");

    char loop = 1;
    if (RC_NG == ::setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof loop))
        myPerror("setsockopt(2) with IP_MULTICAST_LOOP");

    char ttl = 32;
    if (RC_NG == ::setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof ttl))
        myPerror("setsockopt(2) with IP_MULTICAST_TTL");

    int buflen;
    socklen_t len = sizeof buflen;
    if (RC_NG == ::getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, &len) || sizeof buflen != len)
        myPerror("getsockopt(2) with SO_SNDBUF");
    cout << "SO_SNDBUF => " << buflen << " (" << (buflen / 1024. / 1024.) << " MB); len == " << len << endl;
    buflen = (g_buflen ? g_buflen : 600 * 1024);
    if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof buflen))
        myPerror("getsockopt(2) with SO_SNDBUF");
    if (RC_NG == ::getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buflen, &len) || sizeof buflen != len)
        myPerror("getsockopt(2) with SO_SNDBUF");
    cout << "SO_SNDBUF => " << buflen << " (" << (buflen / 1024. / 1024.) << " MB)" << endl;

    if (RC_NG == ::connect(sock, res->ai_addr, res->ai_addrlen))
        myPerror("connect(2)");

    ::freeaddrinfo(res);

    return sock;
}

int main(int argc, char *argv[])
{
    int nMsg = 10, lMsg = 200, sleepDurMs = 100;

    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "n:l:s:b:"))) {
        switch (optchr) {
            case 'n': nMsg = myAtoi(::optarg); break;
            case 'l': lMsg = myAtoi(::optarg); break;

            case 's': sleepDurMs = myAtoi(::optarg); break;

            case 'b': g_buflen = myAtoi(::optarg); break;

            default: ::exit(10);
        }
    }

    vector<string> ipAddrs, ports;
    for (int i = ::optind; i < argc; ++i) {
        ipAddrs.push_back(string());
        ports.push_back(string());
        split(argv[i], ipAddrs.back(), ports.back());
    }

    vector<int> sockets;
    for (size_t i = 0, iEnd = ipAddrs.size(); i < iEnd; ++i)
        sockets.push_back(setupSocket(ipAddrs[i], ports[i]));

    ::timespec ts0, ts1;
    ::clock_gettime(CLOCK_REALTIME, &ts0);

    string s(lMsg, ' ');

    for (int k = 0; k < nMsg; ++k) {
        /*
        for (size_t i = 0, iEnd = ipAddrs.size(); i < iEnd; ++i)
            ::send(sockets[i], &s[0], s.size(), 0);
        */
        s[4] = '0' + k % 10;
        s[3] = '0' + (k / 10) % 10;
        s[2] = '0' + (k / 100) % 10;
        s[1] = '0' + (k / 1000) % 10;
        s[0] = '0' + (k / 10000) % 10;
        while (s.size() != ::send(sockets[0], &s[0], s.size(), 0))
            ;

        if (0 < sleepDurMs) {
            cout << "Sent [" << s << "]" << endl;
            myMicroSleep(1000 * sleepDurMs);
        }
    }
    ::clock_gettime(CLOCK_REALTIME, &ts1);

    const long diffMsec = (ts1.tv_sec - ts0.tv_sec) * 1000 + (ts1.tv_nsec - ts0.tv_nsec) / 1000 / 1000;
    cout << 1e-3 * diffMsec << " sec in total;  " << (1e3 * diffMsec / nMsg) << " us/msg; " << (s.size() * nMsg / (diffMsec / 1e3) / 1024. / 1024.) << " MB/s; " << (s.size() * nMsg / 1024. / 1024.) << "  MB in total." << endl;

    return 0;
}
