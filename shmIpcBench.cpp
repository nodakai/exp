#define _GNU_SOURCE 1

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
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
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1, RC_SHUT = -2 };
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

static const unsigned IDLE_MSEC = 700, PING_MSEC = 500;

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

static const char *tvFormat(const ::timeval &tv, char *buf)
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
        OCCUPIED = 0x1D2C3B4A
    };

    std::atomic_int m_check;
    std::atomic_uint_fast64_t m_time;
    size_t m_msgLen;
    char m_buf[BUFLEN];

protected:
    SimpChanBase() = delete;

    void init() {
        m_check = EMPTY;
        m_time = myTime();
        m_msgLen = 0;
        std::fill(m_buf, m_buf + sizeof(m_buf), '\0');
    }

public:
    bool held() {
        const int ch = m_check;
        if (EMPTY != ch && OCCUPIED != ch) {
            cout << __func__ << "(): no sign of occupation." << endl;
            init();
            return false;
        }

        const uint64_t now = myTime(), t0 = m_time, elapsedMsec = now - t0;
        if (IDLE_MSEC < elapsedMsec) {
            myMicroSleep(1000 * IDLE_MSEC);
            const uint64_t t1 = m_time;
            if (t0 != t1) {
                cout << __func__ << "(): detected a fresh incoming HeartBeat;  giving up this shm..." << endl;
                return true;
            }
        }
        cout << __func__ << "(): HeartBeat was too old;  Will use this shm..." << endl;
        init();
        return false;
    }
};

class SimpChanBusyLoop : public SimpChanBase {
private:
    SimpChanBusyLoop() = delete;

public:
    void init() {
        SimpChanBase::init();
    }

    void send(const std::string &in, int &rcOut) {
        while (EMPTY != m_check.load(std::memory_order_acquire)) {
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            }
            myMicroSleep(1);
        }

        // std::atomic_thread_fence(std::memory_order_acquire);
        // __builtin_ia32_lfence();
        m_msgLen = in.size();
        ::memcpy(&m_buf[0], &in[0], in.size());
        m_buf[in.size()] = ';';
        m_time = myTime();
        // __builtin_ia32_sfence();
        m_check.store(OCCUPIED, std::memory_order_release);
        rcOut = RC_OK;
    }

    void recv(std::string &out, int &rcOut) {
        while (OCCUPIED != m_check.load(std::memory_order_acquire)) {
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            }
        }

        // __builtin_ia32_lfence();
        const size_t msgLen = m_msgLen;
        if (';' != m_buf[msgLen]) {
            cout << "[" ;
            cout.write(m_buf, msgLen);
            cout << "]" << endl;
            ::abort();
        }
        out.assign(&m_buf[0], msgLen);
//      __builtin_ia32_mfence();
        m_check.store(EMPTY, std::memory_order_release);
        rcOut = RC_OK;
    }
};

class SimpChanBlocking : public SimpChanBase {
private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    typedef std::unique_lock<std::mutex> Lock;

    static const std::chrono::milliseconds tmo;

private:
    SimpChanBlocking() = delete;

public:
    void init() {
        SimpChanBase::init();

        static_assert(0 < _POSIX_THREAD_PROCESS_SHARED,
            "_POSIX_THREAD_PROCESS_SHARED is not supported.");

        ::pthread_mutexattr_t ma;
        ::pthread_mutexattr_init(&ma);
        ::pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
        ::pthread_mutex_init(m_mtx.native_handle(), &ma);
        ::pthread_mutexattr_destroy(&ma);

        ::pthread_condattr_t ca;
        ::pthread_condattr_init(&ca);
        ::pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
        ::pthread_cond_init(m_cv.native_handle(), &ca);
        ::pthread_condattr_destroy(&ca);
    }

    void send(const std::string &in, int &rcOut) {
        Lock lk(m_mtx);
        if ( ! g_running) {
            rcOut = RC_SHUT;
            return;
        }

        while (EMPTY != m_check) {
            m_cv.wait_for(lk, tmo);
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            }
        }

        m_msgLen = in.size();
        ::memcpy(&m_buf[0], &in[0], in.size());
        m_buf[in.size()] = ';';
        m_check = OCCUPIED;
        m_time = myTime();
        m_cv.notify_one();
        rcOut = RC_NG;
    }

    void recv(std::string &out, int &rcOut) {
        Lock lk(m_mtx);
        if ( ! g_running) {
            rcOut = RC_SHUT;
            return;
        }

        while (OCCUPIED != m_check) {
            m_cv.wait_for(lk, tmo);
            if ( ! g_running) {
                rcOut = RC_SHUT;
                return;
            }
        }

        if (';' != m_buf[m_msgLen]) {
            cout << "[" ;
            cout.write(m_buf, m_msgLen);
            cout << "]" << endl;
            abort();
        }
        out.assign(&m_buf[0], m_msgLen);
        m_check = EMPTY;
        rcOut = RC_OK;
    }
};

const std::chrono::milliseconds SimpChanBlocking::tmo(IDLE_MSEC);

class DuplChanBase {
protected:

    static const size_t MMAP_LEN = 1024*1024;

    struct SharedMemFmt {
        SimpChanBusyLoop fastCh;
        SimpChanBlocking ch;
    };

    SharedMemFmt * m_p;

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
    void open(const string &id) {
        const int fd = ::shm_open(id.c_str(), O_CREAT | O_RDWR, 0644);
        if (RC_NG == fd)
            myPerror("open(2)", 30);
        if (RC_NG == ::ftruncate(fd, MMAP_LEN))
            myPerror("ftruncate(2)", 33);

        void * const p = ::mmap(NULL, MMAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (MAP_FAILED == p)
            myPerror("mmap(2)", 36);

        m_p = reinterpret_cast<SharedMemFmt *>(p);
        cout << "p==" << m_p << endl;

        if (RC_OK != ::close(fd))
            myPerror("close(2)", 39);
    }
};

class DuplChanFastSend : public DuplChanBase {
public:
    inline void send(const std::string &buf, int &rcOut) { m_p->fastCh.send(buf, rcOut); }
    inline void recv(std::string &buf, int &rcOut) { m_p->ch.recv(buf, rcOut); }

    bool held() const { return m_p->fastCh.held(); }

    void open(const string &id) {
        DuplChanBase::open(id);
        if (m_p->fastCh.held())
            abort();
        else
            m_p->fastCh.init();
    }
};

class DuplChanFastRecv : public DuplChanBase {
public:
    inline void send(const std::string &buf, int &rcOut) { m_p->ch.send(buf, rcOut); }
    inline void recv(std::string &buf, int &rcOut) { m_p->fastCh.recv(buf, rcOut); }

    bool held() const { return m_p->ch.held(); }

    void open(const string &id) {
        DuplChanBase::open(id);
        if (m_p->ch.held())
            abort();
        else
            m_p->ch.init();
    }
};

//----------------------------------------------------------------------------//

static const string id("shmIpcBench");

static void serverSenderKernel(DuplChanFastRecv *chan)
{
}

static void serverReceiverKernel(DuplChanFastRecv *chan)
{
}

static void doServer()
{
    DuplChanFastRecv chan{};
    chan.open(id);

    std::thread senderThread(serverSenderKernel, &chan);
    std::thread receiverThread(serverReceiverKernel, &chan);

    if (senderThread.joinable())
        senderThread.join();
    if (receiverThread.joinable())
        receiverThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doClient()
{
    DuplChanFastSend chan{};
    chan.open(id);

    const int svrSock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCK == svrSock)
        myPerror("socket(2)", 40);
    ::sockaddr_in sin{};

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
        oss << "[" << __func__ << ": " << tvFormat(tv, buf) << "]" ;
        const auto &msg = oss.str();
        chan->send(msg, rc /* out */ );
        if (RC_OK != rc)
            break;
        cout << msg << endl;
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
        cout << __func__ << ": " << tvFormat(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doSimpleServer()
{
    DuplChanFastRecv chan{};
    chan.open(id);

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
        oss << "[" << __func__ << ": " << tvFormat(tv, buf) << "]" ;
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
        cout << __func__ << ": " << tvFormat(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doSimpleClient()
{
    DuplChanFastSend chan{};
    chan.open(id);

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

    bool simple = false;

    enum EnumMode {
        EnumModeServer,
        EnumModeClient
    };
    enum EnumMode mode = EnumModeServer;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "sCS"))) {
        switch (optchr) {
            case 's': simple = true; break;

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
            doClient();
        else
            doServer();
    }
}
