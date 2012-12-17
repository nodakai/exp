#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <stdexcept> // runtime_error

#include <cstdlib>
#include <cstdio> // perror

#include <sys/types.h> // socket
#include <sys/socket.h> // socket
#include <arpa/inet.h> // inet_*
#include <sys/epoll.h> // epoll
#include <signal.h>
#include <unistd.h> // write
#include <errno.h>
#include <getopt.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

enum { RC_OK = 0, RC_NG = -1 };
const static int MAX_EVENTS = 10;
const static int BUFLEN = 2048;

bool g_running = true;

static void myPrror(const char *msg) {
    ::perror(msg);
    throw std::runtime_error(msg);
}

static long myAtoi(const char *str) {
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' != *end) throw std::invalid_argument(string("myAtoi(") + str + ")");
    return ret;
}

static void sigIntHandler(int) {
    g_running = false;
    const char msg[] = "Caught SIGINT;  Set g_running = false;\n";
    auto ign = ::write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)ign;
}

static int setupSocket(const char *mcAddr, const char *mcPort)
{
    const int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (RC_NG == sock)
        myPrror("socket(2)");

    ::sockaddr_in sin = { };
    sin.sin_family = AF_INET;

    int opt = 1;
    if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt))
        myPrror("setsockopt(2) with SO_REUSEADDR");

    {
        uint16_t intPort;
        std::istringstream iss(mcPort);
        iss >> intPort;
        if ( ! iss) {
            std::ostringstream oss;
            oss << "Invalid port == [" << mcPort << "]" << endl;
            throw std::invalid_argument(oss.str());
        }
        sin.sin_port = htons(intPort);
    }
    if (1 != ::inet_pton(AF_INET, mcAddr, &sin.sin_addr))
        throw std::invalid_argument(string("Invalid address == [") + mcAddr + "]");
    if (RC_NG == ::bind(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
        myPrror("bind(2)");

    ::ip_mreq mreq = { };
    if (1 != ::inet_pton(AF_INET, mcAddr, &mreq.imr_multiaddr))
        throw std::invalid_argument(string("Invalid address == [") + mcAddr + "]");
    if (1 != ::inet_pton(AF_INET, "0.0.0.0", &mreq.imr_interface))
        throw std::runtime_error("Couldn't convert 0.0.0.0 to ::inet_addr");
    if (RC_NG == ::setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof mreq))
        myPrror("setsockopt(2) with IP_ADD_MEMBERSHIP");

    int buflen;
    socklen_t len = sizeof buflen;
    if (RC_NG == ::getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buflen, &len))
        myPrror("getsockopt(2) with SO_RCVBUF");
    cout << "SO_RCVBUF => " << buflen << " (" << (buflen / 1024. / 1024.) << " MB);  len==" << len << endl;
    buflen = 64 * 1024 * 1024;
    if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buflen, sizeof buflen))
        myPrror("setsockopt(2) with SO_RCVBUF");
    if (RC_NG == ::getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buflen, &len))
        myPrror("getsockopt(2) with SO_RCVBUF");
    cout << "SO_RCVBUF => " << buflen << " (" << (buflen / 1024. / 1024.) << " MB)" << endl;

    return sock;
}

enum PrintMode {
    PRINT_NONE,
    PRINT_FULL,
    PRINT_DOT
};

static void printUsage() {
    cout << "mcListener [-n nMsg] (-F|-D|-N) addr0 port0 [addr1 port1 ...]" << endl;
    cout << "    -n nMsg (int): " << endl;
    cout << "    -F: fully print received datagrams in hex" << endl;
    cout << "    -D: print a dot for each received datagram" << endl;
    cout << "    -N: print nothing" << endl;

    ::exit(20);
}

int main(int argc, char *argv[])
{
    long nMsg = -1;
    PrintMode mode = PRINT_FULL;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "n:FDN"))) {
        switch (optchr) {
            case 'n': nMsg = myAtoi(::optarg); break;

            case 'F': mode = PRINT_FULL; break;
            case 'D': mode = PRINT_DOT; break;
            case 'N': mode = PRINT_NONE; break;

            default: ::exit(10);
        }
    }

    std::vector<int> sockets;
    for (int i = ::optind; i + 2 <= argc; i += 2) {
        const auto mcAddr = argv[i], mcPort = argv[i + 1];
        cout << "Listening to (" << mcAddr << ", " << mcPort << ")" << endl;
        sockets.push_back(setupSocket(mcAddr, mcPort));
    }

    if (sockets.empty()) {
        cout << "No addr / port to listen to was given!" << endl << endl;
        printUsage();
    }

    {
        struct ::sigaction act = { };
        act.sa_handler = sigIntHandler;
        if (RC_NG == ::sigemptyset(&act.sa_mask))
            myPrror("sigemptyset(3)");
        if (RC_NG == ::sigaction(SIGINT, &act, NULL))
            myPrror("sigaction(2)");
    }

    const int epFd = ::epoll_create(1);
    if (RC_NG == epFd)
        myPrror("epoll_create(2)");

    for (auto it(sockets.begin()), itEnd(sockets.end()); it != itEnd; ++it) {
        ::epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = *it;
        if (RC_NG == ::epoll_ctl(epFd, EPOLL_CTL_ADD, *it, &ev))
            myPrror("epoll_ctl(2) with EPOLL_CTL_ADD");
    }

    int cnt = 0;
    while (g_running && (0 > nMsg || cnt < nMsg++)) {
        ::epoll_event events[MAX_EVENTS];
        const int nEvts = ::epoll_wait(epFd, events, MAX_EVENTS, -1 ); // no time out
        if (RC_NG == nEvts) {
            const int eno = errno;
            if (EAGAIN == eno || EINTR == eno)
                continue;
            else
                myPrror("epoll_wait(2)");
        }
        for (int ei = 0; ei < nEvts; ++ei, ++cnt) {
            char buf[BUFLEN];
            const auto nRecv = ::recv(events[ei].data.fd, buf, (PRINT_FULL == mode ? BUFLEN : 0), 0);
            if (0 > nRecv)
                myPrror("recv(2)");

            if (PRINT_FULL == mode) {
                int i;
                for (i = 0; i < nRecv; ++i) {
                    if (0 == i % 16)
                        cout << std::hex << std::setw(6) << std::setfill('0') << i << std::dec << " [" ;
                    const auto c = buf[i];
                    if (0x11 == c)
                        cout << '#' ;
                    else if (0x12 == c)
                        cout << '$' ;
                    else if (0x13 == c)
                        cout << '@' ;
                    else if ( ! ::isprint(c))
                        cout << '.' ;
                    else
                        cout << c ;
                    if (0 == (i+1) % 16)
                        cout << ']' << endl;
                    // else if (0 == (i+1) % 8) cout << ' ';
                }
                if (0 != i % 16)
                    cout << ']' << endl;
            } else if (PRINT_DOT == mode)
                cout << '.' << std::flush;
        }
    }

    cout << cnt << " packets received." << endl;
}
