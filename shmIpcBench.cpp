#define _GNU_SOURCE 1

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <csignal>

#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1, RC_SHUT = -2, RC_TIMEO = -3 };
const int INVALID_SOCK = -1;
static const size_t BUFLEN = 2048;

static std::atomic_int g_running = ATOMIC_VAR_INIT(1);
static void sigIntHandler(int)
{
    g_running = 0;
    const char msg[] = "Caught SIGINT;  Set g_running = false;\n";
    auto ign = ::write(STDOUT_FILENO, msg, sizeof msg - 1);
    (void)ign;
}

static void myPerror(const char *msg, int rc)
{
    ::perror(msg);
    ::exit(rc);
}

static const unsigned IDLE_MSEC = 700, PING_MSEC = 500, TIMEO_MSEC = 100;
static const unsigned LOOP_MAX = 10 * 1024 * 1024; // 100 clk * 10 M / 3.3 GHz =~ 0.3 sec

static uint64_t myTime()
{
    ::timeval tv;
    ::gettimeofday(&tv, NULL);
    return (static_cast<uint64_t>(tv.tv_sec) << 32) | static_cast<uint64_t>(tv.tv_usec);
}

static void myMicroSleep(long usec)
{
    const int mega = 1024 * 1024;
    ::timespec sleepDur = { usec / mega, 1000 * (usec % mega) }, rest;
    while (::nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
}

static void mySetAffinity(int coreNo)
{
    ::cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(coreNo, &cpuSet);
    const pid_t myTid = ::syscall(SYS_gettid);
    if (RC_OK != ::sched_setaffinity(myTid, sizeof cpuSet, &cpuSet))
        myPerror("sched_setaffinity(2)", 30);
}

static const char *formatTv(const ::timeval &tv, char *buf)
{
    char buf1[BUFLEN];
    ::tm theTm;
    ::localtime_r(&tv.tv_sec, &theTm);
    ::strftime(buf1, BUFLEN, "%H:%M:%S", &theTm);
    ::snprintf(buf, BUFLEN, "%s.%06ld", buf1, tv.tv_usec);
    return buf;
}

class SimpChanBase {
protected:

    enum {
        EMPTY = 0x1A2B3C4D,
        OCCUPIED = 0x1D2C3B4A,
        TERMINATOR = ';',
        HB_SIG = SIZE_MAX
    };

    std::atomic_int m_check;
    std::atomic_uint_fast64_t m_time;
    size_t m_msgLen;
    char m_buf[BUFLEN];

protected:
    SimpChanBase() = delete;

    bool available(int ch) {
        if (EMPTY != ch && OCCUPIED != ch) {
            cout << __func__ << "(): found an uninitialized chunk in the shared memory." << endl;
            return true;
        }

        const uint64_t now = myTime(), t0 = m_time, elapsedMsec = now - t0;
        if (IDLE_MSEC < elapsedMsec) {
            myMicroSleep(1000 * IDLE_MSEC);
            const uint64_t t1 = m_time;
            if (t0 != t1) {
                cout << __func__ << "(): detected a fresh incoming HeartBeat;  giving up this shm..." << endl;
                return false;
            }
        }
        cout << __func__ << "(): HeartBeat was too old;  Will use this shm..." << endl;
        return true;
    }

    void init(int &rcOut) {
        int chk = m_check;
        if ( ! available(chk)) {
            rcOut = RC_NG;
            return;
        }

        if ( ! m_check.compare_exchange_strong(chk, EMPTY)) {
            rcOut = RC_NG;
            return;
        }
        m_time = myTime();
        m_msgLen = 0;
        std::fill(m_buf, m_buf + sizeof(m_buf), '\0');
        rcOut = RC_OK;
    }

    inline bool validateTerminator(size_t msgLen) {
        const bool ret = (TERMINATOR == m_buf[msgLen]) ;
        if (ret) {
            m_buf[msgLen] = '\0'; // for this terminator not to hide a bug in the next invocation
        } else {
            cout << "Input string w/o the terminator symbol: [" ;
            cout.write(m_buf, msgLen + 1);
            cout << "]" << endl;
        }
        return ret;
    }
};

class SimpChanBlocking;

class SimpChanBusyLoop : public SimpChanBase {
private:
    SimpChanBlocking *m_oth;

private:
    SimpChanBusyLoop() = delete;

public:
    void init(SimpChanBlocking * other, int &rcOut) {
        SimpChanBase::init(rcOut);
        if (RC_OK != rcOut)
            return;

        m_oth = other;
    }

    void send(const void *in, size_t len, int &rcOut) {
        for (unsigned i = 0; EMPTY != m_check.load(std::memory_order_acquire); ++i) {
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            } else if (LOOP_MAX <= i) {
                rcOut = RC_TIMEO;
                return;
            }
//          myMicroSleep(1);
        }

        m_msgLen = len;
        ::memcpy(&m_buf[0], in, len);
        m_buf[len] = TERMINATOR;
        m_time = myTime();
        m_check.store(OCCUPIED, std::memory_order_release);
        rcOut = RC_OK;
    }

    void sendHB() {
        if (EMPTY == m_check.load(std::memory_order_acquire))
            return;

        m_msgLen = HB_SIG;
        m_time = myTime();
        m_check.store(OCCUPIED, std::memory_order_release);
    }

    void recv(std::string &out, int &rcOut) {
RETRY:
        for (unsigned i = 0; OCCUPIED != m_check.load(std::memory_order_acquire); ++i) {
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            } else if (LOOP_MAX <= i) {
                rcOut = RC_TIMEO;
                return;
            }
        }

        const size_t msgLen = m_msgLen;
        if (HB_SIG == msgLen)
            goto RETRY;
        if ( ! validateTerminator(msgLen)) {
            rcOut = RC_NG;
            return;
        }

        out.assign(&m_buf[0], msgLen);
        m_check.store(EMPTY, std::memory_order_release);
        rcOut = RC_OK;
    }
};

class SimpChanBlocking : public SimpChanBase {
private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    SimpChanBusyLoop *m_oth;

    typedef std::unique_lock<std::mutex> Lock;
    typedef std::chrono::milliseconds MSec;

private:
    SimpChanBlocking() = delete;

public:
    void init(SimpChanBusyLoop * other, int &rcOut) {
        SimpChanBase::init(rcOut);
        if (RC_OK != rcOut)
            return;

        m_oth = other;

        static_assert(0 < _POSIX_THREAD_PROCESS_SHARED,
            "_POSIX_THREAD_PROCESS_SHARED is not supported.");

        ::pthread_mutexattr_t ma;
        ::pthread_mutexattr_init(&ma);
        ::pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
        ::pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
        ::pthread_mutex_init(m_mtx.native_handle(), &ma);
        ::pthread_mutexattr_destroy(&ma);

        ::pthread_condattr_t ca;
        ::pthread_condattr_init(&ca);
        ::pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
        ::pthread_cond_init(m_cv.native_handle(), &ca);
        ::pthread_condattr_destroy(&ca);
    }

    void send(const void *in, size_t len, int &rcOut) {
        const MSec tmo(IDLE_MSEC);
        Lock lk(m_mtx);
        if ( ! g_running) {
            rcOut = RC_SHUT;
            return;
        }

        const std::chrono::system_clock::time_point deadln =
            std::chrono::system_clock::now() + tmo;
        while (EMPTY != m_check) {
            if (std::cv_status::timeout == m_cv.wait_until(lk, deadln)) {
                rcOut = RC_TIMEO;
                return;
            }
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            }
        }

        m_msgLen = len;
        ::memcpy(&m_buf[0], in, len);
        m_buf[len] = TERMINATOR;
        m_check = OCCUPIED;
        m_time = myTime();
        m_cv.notify_one();
        rcOut = RC_NG;
    }

    void sendHB() {
        const MSec tmo(IDLE_MSEC);
        Lock lk(m_mtx);
        if ( ! g_running) {
            return;
        }

        const std::chrono::system_clock::time_point deadln =
            std::chrono::system_clock::now() + tmo;
        while (EMPTY != m_check) {
            if (std::cv_status::timeout == m_cv.wait_until(lk, deadln)) {
                return;
            }
            if ( ! g_running) {
                return;
            }
        }

        m_msgLen = HB_SIG;
        m_time = myTime();
        m_check.store(OCCUPIED, std::memory_order_release);
    }

    void recv(std::string &out, int &rcOut) {
RETRY:
        Lock lk(m_mtx);
        if ( ! g_running) {
            rcOut = RC_SHUT;
            return;
        }

        const int split = 4;
        const MSec tmo(IDLE_MSEC / split);
        for (int i = 0; i < split; ++i) {
            const std::chrono::system_clock::time_point deadln =
                 std::chrono::system_clock::now() + tmo;
            while (OCCUPIED != m_check) {
                if (std::cv_status::timeout == m_cv.wait_until(lk, deadln) && split-1 == i) {
                    rcOut = RC_TIMEO;
                    return;
                }
                if ( ! g_running) {
                    rcOut = RC_SHUT;
                    return;
                }
            }
        }

        const size_t msgLen = m_msgLen;
        if (HB_SIG == msgLen)
            goto RETRY;
        if ( ! validateTerminator(msgLen)) {
            rcOut = RC_NG;
            return;
        }

        out.assign(&m_buf[0], m_msgLen);
        m_check = EMPTY;
        rcOut = RC_OK;
    }
};

class DuplChanBase {
protected:

    static const size_t MMAP_LEN = 1024*1024;

    // on-disk format of the communication channel
    struct SharedMemFmt {
        SimpChanBusyLoop fastCh;
        SimpChanBlocking ch;
    };

    SharedMemFmt * m_p;
    std::atomic_int m_isConnected;

protected:
    DuplChanBase() : m_p(NULL) { }
    ~DuplChanBase() { close(); }

    void close() {
        if (m_p) {
            if (RC_OK != ::munmap(m_p, MMAP_LEN))
                myPerror("munmap(2)", 42);
            m_p = NULL;
        }
    }

public:
    void open(const string &id, int &rcOut) {
        m_isConnected = false;

        const int fd = ::shm_open(id.c_str(), O_CREAT | O_RDWR, 0644);
        if (RC_NG == fd) {
            cout << "Error in shm_open(3)" << endl;
            rcOut = errno;
            return;
        }
        if (RC_NG == ::ftruncate(fd, MMAP_LEN)) {
            cout << "Error in ftruncate(2)" << endl;
            rcOut = errno;
            return;
        }

        void * const p = ::mmap(NULL, MMAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (MAP_FAILED == p) {
            cout << "Error in mmap(2)" << endl;
            rcOut = errno;
            return;
        }

        m_p = reinterpret_cast<SharedMemFmt *>(p);
        cout << "p==" << m_p << endl;

        if (RC_OK != ::close(fd)) {
            cout << "Error in close(2)" << endl;
            rcOut = errno;
            return;
        }
    }
};

class DuplChanFastSend : public DuplChanBase {
public:
    inline void send(const void *buf, size_t len, int &rcOut) {
        while (g_running && ! m_isConnected)
            myMicroSleep(1);
        m_p->fastCh.send(buf, len, rcOut);
    }
    inline void send(const std::string &buf, int &rcOut) { send(buf.data(), buf.size(), rcOut); }

    inline void recv(std::string &buf, int &rcOut) {
        while (g_running && ! m_isConnected)
            myMicroSleep(1);
        m_p->ch.recv(buf, rcOut);
        if (RC_OK != rcOut)
            m_isConnected = false;
    }

    void open(const string &id, int &rcOut) {
        DuplChanBase::open(id, rcOut);
        if (RC_OK != rcOut)
            return;

        m_p->fastCh.init(&m_p->ch, rcOut);
    }
};

class DuplChanFastRecv : public DuplChanBase {
public:
    inline void send(const void *buf, size_t len, int &rcOut) {
        while (g_running && ! m_isConnected)
            myMicroSleep(1);
        m_p->ch.send(buf, len, rcOut);
    }
    inline void send(const std::string &buf, int &rcOut) { send(buf.data(), buf.size(), rcOut); }

    inline void recv(std::string &buf, int &rcOut) {
        while (g_running && ! m_isConnected)
            myMicroSleep(1);
        m_p->fastCh.recv(buf, rcOut);
        if (RC_OK != rcOut)
            m_isConnected = false;
    }

    void open(const string &id, int &rcOut) {
        DuplChanBase::open(id, rcOut);
        if (RC_OK != rcOut)
            return;

        m_p->ch.init(&m_p->fastCh, rcOut);
    }
};

//----------------------------------------------------------------------------//

static const string id("shmIpcBench");

static void udp2ipcKernel(DuplChanFastSend *chan)
{
    mySetAffinity(1);

    ::timeval tv;
    char buf[BUFLEN], buf1[BUFLEN];

    const int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    ::sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(33442);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (RC_OK != ::bind(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
        myPerror("bind(2)", 46);

    const int epFd = ::epoll_create(1);
    if (INVALID_SOCK == epFd)
        myPerror("epoll_create(2)", 49);
    ::epoll_event evt{};
    evt.events = EPOLLIN;
    evt.data.fd = sock;
    if (RC_OK != ::epoll_ctl(epFd, EPOLL_CTL_ADD, sock, &evt))
        myPerror("epoll_ctl(2)", 50);

    while (g_running) {
        const int rc = ::epoll_wait(epFd, &evt, 1, 0);
        if (0 > rc) {
            if (EINTR == errno)
                continue;
            else
                myPerror("epoll_wait(2)", 55);
        } else if (0 == rc) {
            continue;
        } else {
            const ssize_t nRecv = ::recv(sock, buf, sizeof buf, 0);
            ::gettimeofday(&tv, NULL);

            int rc;
            chan->send(buf, nRecv, rc);

            cout << ">>" << formatTv(tv, buf1) << " >> " << __func__ << " {" ;
            cout.write(buf, nRecv);
            cout << "} >>" << endl;
        }
    }

    ::close(epFd);
    ::close(sock);

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doServer()
{
    int rc;
    DuplChanFastSend chan{};
    chan.open(id, rc /* out */ );

    std::thread udp2ipcThread(udp2ipcKernel, &chan);

    char buf1[BUFLEN];
    string buf;
    ::timeval tv;

    while (g_running) {
        chan.recv(buf, rc);
        if (RC_OK != rc)
            break;
        else {
            ::gettimeofday(&tv, NULL);
            cout << ">>" << formatTv(tv, buf1) << " >> " << __func__ << " {" << buf << "}" << endl;
        }
    }

    if (udp2ipcThread.joinable())
        udp2ipcThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void tcp2ipcThreadKernel(int sock, DuplChanFastRecv *chan)
{
    std::ostringstream oss;
    ::timeval tv;
    char buf[BUFLEN], buf1[BUFLEN];

    while (g_running) {
        oss.str("");
        ::gettimeofday(&tv, NULL);
        oss << "<<" << formatTv(tv, buf) << " << " ;
        for (int i = 0; i < 4; ++i)
            oss << __func__ ;
        oss << "<<" ;
        const auto &msg = oss.str();
        cout << "<<" << formatTv(tv, buf) << " {" << msg << "}" << endl;

        const ssize_t nSend = ::send(sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if (RC_OK >= nSend) {
            cout << "Failed to send(2) => " << nSend << endl;
            break;
        }

        for (const uint64_t t0 = myTime(); myTime() - t0 < 1000 * PING_MSEC;  ) {
            const ssize_t nRecv = ::recv(sock, buf, sizeof buf, 0);
            if (0 > nRecv) {
                const int eno = errno;
                if (EINTR == eno || EAGAIN == eno)
                    continue;
                else
                    myPerror("recv(2)", 47);
            } else if (0 == nRecv) {
                cout << "The TCP connection was disconnected." << endl;
                goto BREAK;
            } else {
                int rc;
                chan->send(buf, nRecv, rc /* out */ );
                ::gettimeofday(&tv, NULL);

                cout << ">>" << formatTv(tv, buf1) << " >> " << __func__ << " {" ;
                cout.write(buf, nRecv);
                cout << "} >>" << endl;
            }
        }
    }

BREAK:
    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doClient(bool connect)
{
    int rc;
    DuplChanFastRecv chan{};
    chan.open(id, rc /* out */ );

    int sock = INVALID_SOCK;
    std::thread tcp2ipcThread;
    if (connect) {
        sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (INVALID_SOCK == sock)
            myPerror("socket(2)", 40);

        const ::timeval timeo{ TIMEO_MSEC / 1000, 1000 * (TIMEO_MSEC % 1000) };
        if (RC_NG == ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof timeo))
            myPerror("setsockopt(2) with SO_RCVTIMEO", 41);
        const int opt = 1;
        if (RC_NG == ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof opt))
            myPerror("setsockopt(2) with TCP_NODELAY", 41);

        ::sockaddr_in sin{};
        sin.sin_family = AF_INET;
        sin.sin_port = htons(44332);
        if (0 == ::inet_aton("192.168.2.80", &sin.sin_addr)) {
            cout << __func__ << "(): error in inet_aton(3)" << endl;
            return;
        }
        if (RC_NG == ::connect(sock, reinterpret_cast<const ::sockaddr *>(&sin), sizeof sin))
            myPerror("connect(2)", 43);

        tcp2ipcThread = std::thread(tcp2ipcThreadKernel, sock, &chan);
    }

    string buf;
    std::stringstream oss;
    char buf1[BUFLEN], buf2[BUFLEN];
    ::timeval tv0, tv1, tv2;

    mySetAffinity(2);

    while (g_running) {
        chan.recv(buf, rc /* out */ );
        ::gettimeofday(&tv0, NULL);
        if (RC_OK != rc) {
            cout << __func__ << ": rc==" << rc << endl;
            break;
        }
        if ( ! g_running)
            break;

        if (connect) {
#if 0
            oss.str("");
            oss << "[" ;
            for (int i = 0; i < 8; ++i)
                oss << "Order" ;
            oss << buf << "]" ;
            const auto &msg = oss.str();
            ::gettimeofday(&tv1, NULL);
            const ssize_t nSent = ::send(sock, &msg[0], msg.size(), MSG_NOSIGNAL);
            ::gettimeofday(&tv2, NULL);
#else
            const ssize_t nSent = ::send(sock, buf.data(), buf.size(), MSG_NOSIGNAL);
            ::gettimeofday(&tv1, NULL);
#endif
            if (0 >= nSent) // ignore EINTR for now...
                break;
            else
                cout << "<<" << __func__ << " << (" << formatTv(tv0, buf1) << "," << formatTv(tv1, buf2) /* << "," << formatTv(tv2, buf1) */ << ") << {" << buf /* msg */ << "} <<" << endl;
        }
    }

    if (tcp2ipcThread.joinable())
        tcp2ipcThread.join();
    if (INVALID_SOCK != sock)
        ::close(sock);
    while (g_running)
        myMicroSleep(IDLE_MSEC * 1000);

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

//----------------------------------------------------------------------------//

static void simpleServerSenderKernel(DuplChanFastRecv *chan)
{
    char buf[BUFLEN];
    ::timeval tv;
    std::ostringstream oss;
    int rc;

    for (;;) {
        myMicroSleep(1000 * PING_MSEC);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);

        oss.str("");
        oss << "[" << __func__ << ": " << formatTv(tv, buf) << "]" ;
        const auto &msg = oss.str();
        chan->send(msg, rc /* out */ );
        if (RC_OK != rc)
            break;
        cout << __func__ << "() << {" << msg << "}" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void simpleServerReceiverKernel(DuplChanFastRecv *chan)
{
    string buf;
    ::timeval tv;
    char timeBuf[BUFLEN];
    int rc;

    for (;;) {
        chan->recv(buf, rc /* out */ );
        if (RC_OK != rc)
            break;
        ::gettimeofday(&tv, NULL);
        cout << __func__ << ": " << formatTv(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doSimpleServer()
{
    int rc;
    DuplChanFastRecv chan{};
    chan.open(id, rc /* out */ );

    std::thread senderThread(simpleServerSenderKernel, &chan);
    std::thread receiverThread(simpleServerReceiverKernel, &chan);

    if (senderThread.joinable())
        senderThread.join();
    if (receiverThread.joinable())
        receiverThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void simpleClientSenderKernel(DuplChanFastSend *chan)
{
    char buf[BUFLEN];
    ::timeval tv;
    std::ostringstream oss;
    int rc;

    for (;;) {
        myMicroSleep(1000 * PING_MSEC);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);

        oss.str("");
        oss << "[" << __func__ << ": " << formatTv(tv, buf) << "]" ;
        const auto &msg = oss.str();
        chan->send(msg, rc /* out */ );
        if (RC_OK != rc)
            break;
        cout << msg << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void simpleClientReceiverKernel(DuplChanFastSend *chan)
{
    string buf;
    ::timeval tv;
    char timeBuf[BUFLEN];
    int rc;

    for (;;) {
        chan->recv(buf, rc /* out */ );
        if (RC_OK != rc)
            break;
        ::gettimeofday(&tv, NULL);
        cout << __func__ << ": " << formatTv(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doSimpleClient()
{
    int rc;
    DuplChanFastSend chan{};
    chan.open(id, rc /* out */ );

    std::thread senderThread(simpleClientSenderKernel, &chan);
    std::thread receiverThread(simpleClientReceiverKernel, &chan);

    if (senderThread.joinable())
        senderThread.join();
    if (receiverThread.joinable())
        receiverThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

//----------------------------------------------------------------------------//

int main(int argc, char *argv[])
{
    {
        struct ::sigaction act{};
        act.sa_handler = sigIntHandler;
        if (RC_NG == ::sigemptyset(&act.sa_mask))
            myPerror("sigemptyset(3)", 4);
        if (RC_NG == ::sigaction(SIGINT, &act, NULL))
            myPerror("sigaction(2)", 7);
    }

    bool simple = false, connect = false;

    enum EnumMode {
        EnumModeServer,
        EnumModeClient
    };
    enum EnumMode mode = EnumModeServer;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "scCS"))) {
        switch (optchr) {
            case 's': simple = true; break;
            case 'c': connect = true; break;

            case 'C': mode = EnumModeClient; break;
            case 'S': mode = EnumModeServer; break;

            default:
                ::exit(5);
        }
    }

    if (simple) {
        if (EnumModeClient == mode)
            doSimpleClient();
        else
            doSimpleServer();
    } else {
        if (EnumModeClient == mode)
            doClient(connect);
        else
            doServer();
    }
}
