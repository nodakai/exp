#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <cstdio> // perror
#include <cstdlib>

using namespace std;

#include <stdint.h>

#include <getopt.h>
#include <sys/time.h> // {get,set}timeofday
#include <sys/types.h> // socket
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h> // close
#include <netdb.h> // getaddrinfo
#include <errno.h>

enum { RC_OK = 0, RC_NG = -1 };
enum EnumMode { EnumModeServer, EnumModeClient };
const size_t BUFLEN = 2048;
const long MEGA = 1000 * 1000;
const int maxN = 100;

static long myAtoi(const char *str)
{
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' != *end)
        throw invalid_argument(string("myAtoi: got an invalid argument \"") + str + "\"");
    return ret;
}

static void myPerror(const char *msg)
{
    ::perror(msg);
    throw runtime_error(msg);
}

static long diffTvInUs(const ::timeval &a, const ::timeval &b)
{
    return (a.tv_sec - b.tv_sec) * MEGA + (a.tv_usec - b.tv_usec);
}

static void addToTv(::timeval &lhs, long us)
{
    lhs.tv_sec += us / MEGA;
    lhs.tv_usec += us % MEGA;

    if (MEGA <= lhs.tv_usec) {
        lhs.tv_sec += lhs.tv_usec / MEGA;
        lhs.tv_usec %= MEGA;
    }
}

const int SERVICE_PORT = 43234;
const char SERVICE_PORT_STR[] = "43234";

static ssize_t recvAll(int sock, void *rawBuf, ssize_t len)
{
    char * const buf = reinterpret_cast<char *>(rawBuf);

    ssize_t nRecvTot = 0;
    while (nRecvTot < len) {
        const ssize_t nRecv = ::recv(sock, buf + nRecvTot, len - nRecvTot, 0);
        if (RC_NG == nRecv) {
            const int eno = errno;
            if (EINTR == eno || EAGAIN == eno)
                continue;
            else
                return RC_NG;
        } else if (0 == nRecv)
            return 0;
        else
            nRecvTot += nRecv;
    }
    return nRecvTot;
}

static ssize_t sendAll(int sock, const void *rawBuf, ssize_t len)
{
    const char * const buf = reinterpret_cast<const char *>(rawBuf);

    ssize_t nSentTot = 0;
    while (nSentTot < len) {
        const ssize_t nSent = ::send(sock, buf + nSentTot, len - nSentTot, 0);
        if (RC_NG == nSent) {
            const int eno = errno;
            if (EINTR == eno || EAGAIN == eno)
                continue;
            else
                return RC_NG;
        } else if (0 == nSent)
            return 0;
        else
            nSentTot += nSent;
    }
    return nSentTot;
}

static void mySleepUsec(long usec)
{
    ::timespec sleepDur = { usec / MEGA, 1000 * (usec % MEGA) }, rest;
    while (RC_NG == ::nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
}

static void connHandler(int connSock)
{
    if (RC_NG == connSock) {
        const int eno = errno;
        if (EINTR == eno || EAGAIN == eno)
            return;
        else myPerror("accept(2)");
    }

    struct ScopeExit {
        const int m_sock;
        ScopeExit(int sock) : m_sock(sock) { }
        ~ScopeExit() { ::close(m_sock); }
    } so(connSock);

    int n;
    ssize_t rc = recvAll(connSock, &n, sizeof n);
    if (sizeof n != rc || 0 > n || maxN < n) {
        cerr << "recvAll@" << __LINE__ << " failed (" << rc << ")" << endl;
        throw runtime_error("recvAll");
    }

    for (int i = 0; i < n; ++i) {
        ::timeval tv;
        const ssize_t recvRc = recvAll(connSock, &tv, sizeof tv);
        const ssize_t sentRc = sendAll(connSock, &tv, sizeof tv);
        if (sizeof tv != recvRc) {
            cerr << "recvAll@" << __LINE__ << " failed (" << rc << ")" << endl;
            throw runtime_error("recvAll");
        }
        if (sizeof tv != sentRc) {
            cerr << "sendAll@" << __LINE__ << " failed (" << rc << ")" << endl;
            throw runtime_error("sendAll");
        }
    }

    mySleepUsec(10 * 1000);

    ::timeval tv;
    ::gettimeofday(&tv, NULL);
    rc = sendAll(connSock, &tv, sizeof tv);
    if (sizeof tv != rc) {
        cerr << "sendAll@" << __LINE__ << " failed (" << rc << ")" << endl;
        throw runtime_error("recvAll");
    }

    ::shutdown(connSock, SHUT_RDWR);
}

static void doServer()
{
    ::addrinfo hints = { }, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const int rc = ::getaddrinfo("0.0.0.0", SERVICE_PORT_STR, &hints, &res);
    if (rc) {
        cerr << ::gai_strerror(rc) << endl;
        ::freeaddrinfo(res);
        ::exit(20);
    }

    const int svrSock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (RC_NG == svrSock)
        myPerror("socket(2)");

    const int opt = 1;
    if (RC_NG == ::setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt))
        myPerror("setsockopt(2) with SO_REUSEADDR");

    if (RC_NG == ::setsockopt(svrSock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt))
        myPerror("setsockopt(2) with TCP_NODELAY");

    const ::timeval accTimeout = { 0, 876543 };
    if (RC_NG == ::setsockopt(svrSock, SOL_SOCKET, SO_RCVTIMEO, &accTimeout, sizeof accTimeout))
        myPerror("setsockopt(2) with SO_RCVTIMEO");

    if (RC_NG == ::bind(svrSock, res->ai_addr, res->ai_addrlen))
        myPerror("bind(2)");

    ::freeaddrinfo(res);

    if (RC_NG == ::listen(svrSock, 5))
        myPerror("listen(2)");

    for (;;)
        connHandler(::accept(svrSock, NULL, NULL));
}

static string tvToString(const ::timeval &tv)
{
    char buf[BUFLEN];
    ::tm theTm;
    ::localtime_r(&tv.tv_sec, &theTm);
    ::strftime(buf, sizeof buf, "%F %T", &theTm);
    buf[sizeof buf - 1] = '\0';

    ostringstream oss;
    oss << buf << '.' << setfill('0') << setw(6) << tv.tv_usec ;
    return oss.str();
}

static void doClient(int n, const string &addr, bool doit)
{
    ::addrinfo *res;
    {
        ::addrinfo hints = { };
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        const int rc = ::getaddrinfo(addr.c_str(), SERVICE_PORT_STR, &hints, &res);
        if (rc) {
            cerr << ::gai_strerror(rc) << endl;
            ::freeaddrinfo(res);
            ::exit(20);
        }
    }
    struct ScopeExitGai {
        ::addrinfo *m_res;
        ScopeExitGai(::addrinfo *res) : m_res(res) { }
        ~ScopeExitGai() { ::freeaddrinfo(m_res); }
    } seGai(res);

    const int sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (RC_NG == sock)
        myPerror("socket(2)");
    struct ScopeExitSock {
        int m_sock;
        ScopeExitSock(int sock) : m_sock(sock) { }
        ~ScopeExitSock() { ::close(m_sock); }
    } seSock(sock);

    const int opt = 1;
    if (RC_NG == ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt))
        myPerror("setsockopt(2) with TCP_NODELAY");

    if (RC_NG == ::connect(sock, res->ai_addr, res->ai_addrlen))
        myPerror("connect(2)");

    ssize_t rc = sendAll(sock, &n, sizeof n);
    if (RC_NG == rc) {
        cerr << "sendAll@" << __LINE__ << " failed (" << rc << ")" << endl;
        throw runtime_error("sendAll");
    }

    vector<long> latenciesUsec;
    latenciesUsec.reserve(n);

    for (int i = 0; i < n; ++i) {
        ::timeval sentTv = { i, i }, recvTv, tv0, tv1;
        mySleepUsec(10 * 1000);

        ::gettimeofday(&tv0, NULL);
        const ssize_t sendRc = sendAll(sock, &sentTv, sizeof sentTv);
        const ssize_t recvRc = recvAll(sock, &recvTv, sizeof recvTv);
        ::gettimeofday(&tv1, NULL);

        if (sizeof sentTv != sendRc) {
            cerr << "sendAll@" << __LINE__ << " failed (" << sendRc << ")" << endl;
            throw runtime_error("sendAll");
        }
        if (sizeof recvTv != recvRc) {
            cerr << "recvAll@" << __LINE__ << " failed (" << recvRc << ")" << endl;
            throw runtime_error("recvAll");
        }
        if ( ! (recvTv.tv_sec == i && recvTv.tv_usec == i)) {
            cerr << "i == " << i << " != {" << recvTv.tv_sec << ',' << recvTv.tv_usec << "} == recvTv" << endl;
            throw runtime_error("i != recvTv");
        }

        latenciesUsec.push_back(diffTvInUs(tv1, tv0) / 2);
    }

    std::sort(latenciesUsec.begin(), latenciesUsec.end());
    const long latencyUsec = latenciesUsec[latenciesUsec.size() / 2];

    ::timeval theirTime;
    rc = recvAll(sock, &theirTime, sizeof theirTime);

    if (sizeof theirTime != rc) {
        cerr << "recvAll@" << __LINE__ << " failed (" << rc << ")" << endl;
        throw runtime_error("recvAll");
    }

    addToTv(theirTime, latencyUsec);

    if (doit && RC_NG == ::settimeofday(&theirTime, NULL))
        myPerror("settimeofday(2)");

    cout << "The min/mean/max latencies were " << (1e-3 * latenciesUsec.front()) << "/" <<
        (1e-3 * latencyUsec) << "/" << (1e-3 * latenciesUsec.back()) << " ms." << endl;
    cout << "Now [" << tvToString(theirTime) << "]  (doit==" << (doit ? "YES" : " no") <<
        ")" << endl;
}

int main(int argc, char *argv[])
{
    int n = 3;
    EnumMode mode = EnumModeClient;
    string addr;
    bool doit = false;

    char optchr;
    while (RC_NG != (optchr = ::getopt(argc, argv, "n:CSd"))) {
        switch (optchr) {
            case 'n': n = myAtoi(::optarg); break;

            case 'C': mode = EnumModeClient; break;
            case 'd': doit = true; break;

            case 'S': mode = EnumModeServer; break;

            default: ::exit(10);
        }
    }

    for (int i = ::optind; i < argc; ++i) {
        addr.assign(argv[i]);
        break;
    }

    if (EnumModeServer == mode)
        doServer();
    else {
        if (addr.empty())
            throw invalid_argument("Please specify addr and port in the cmd line");

        if (0 > n || maxN < n)
            throw invalid_argument("-n out of large");

        doClient(n, addr, doit);
    }
}
