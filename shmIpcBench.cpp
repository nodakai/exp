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
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };
static const size_t BUFLEN = 2048;

static std::atomic_int g_running = ATOMIC_VAR_INIT(1);
static void sigIntHandler(int) {
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

static void myMicroSleep(uint64_t usec)
{
    const uint64_t mega = 1024 * 1024;
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
        cout << __func__ << "(): HeartBeat was obsolete;  Will use this shm..." << endl;
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

    void send(const std::string &in) {
        while (g_running && EMPTY != m_check)
            ;
        if ( ! g_running)
            return;

        __builtin_ia32_lfence();
        m_msgLen = in.size();
        ::memcpy(&m_buf[0], &in[0], in.size());
        m_buf[in.size()] = ';';
        __builtin_ia32_sfence();
        m_check = OCCUPIED;
        m_time = myTime();
    }

    void recv(std::string &out) {
        while (g_running && OCCUPIED != m_check)
            ;
        if ( ! g_running)
            return;

        __builtin_ia32_lfence();
        const size_t msgLen = m_msgLen;
        if (';' != m_buf[msgLen]) {
            cout << "[" ;
            cout.write(m_buf, msgLen);
            cout << "]" << endl;
            ::abort();
        }
        out.assign(&m_buf[0], msgLen);
        __builtin_ia32_mfence();
        m_check = EMPTY;
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

    void send(const std::string &in) {
        Lock lk(m_mtx);
        if ( ! g_running)
            return;

        while (EMPTY != m_check && std::cv_status::timeout == m_cv.wait_for(lk, tmo))
            if ( ! g_running)
                return;

        m_msgLen = in.size();
        ::memcpy(&m_buf[0], &in[0], in.size());
        m_buf[in.size()] = ';';
        m_check = OCCUPIED;
        m_time = myTime();
        m_cv.notify_one();
    }

    void recv(std::string &out) {
        Lock lk(m_mtx);
        if ( ! g_running)
            return;

        while (OCCUPIED != m_check && std::cv_status::timeout == m_cv.wait_for(lk, tmo))
            if ( ! g_running)
                return;

        if (';' != m_buf[m_msgLen]) {
            cout << "[" ;
            cout.write(m_buf, m_msgLen);
            cout << "]" << endl;
            abort();
        }
        out.assign(&m_buf[0], m_msgLen);
        m_check = EMPTY;
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
    inline void send(const std::string &buf) { m_p->fastCh.send(buf); }
    inline void recv(std::string &buf) { m_p->ch.recv(buf); }

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
    inline void send(const std::string &buf) { m_p->ch.send(buf); }
    inline void recv(std::string &buf) { m_p->fastCh.recv(buf); }

    bool held() const { return m_p->ch.held(); }

    void open(const string &id) {
        DuplChanBase::open(id);
        if (m_p->ch.held())
            abort();
        else
            m_p->ch.init();
    }
};

static const string id("shmIpcBench");

static void serverSenderKernel(DuplChanFastRecv *shm)
{
    char buf[BUFLEN];
    ::timeval tv;
    std::ostringstream oss;

    for (;;) {
        myMicroSleep(1000 * PING_MSEC);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);

        oss.str("");
        oss << "[" << __func__ << ": " << tvFormat(tv, buf) << "]" ;
        const auto &msg = oss.str();
        shm->send(msg);
        if ( ! g_running)
            break;
        cout << msg << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void serverReceiverKernel(DuplChanFastRecv *shm)
{
    string buf;
    ::timeval tv;
    char timeBuf[BUFLEN];

    for (;;) {
        shm->recv(buf);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);
        cout << __func__ << ": " << tvFormat(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doServer()
{
    DuplChanFastRecv shm;
    shm.open(id);

    std::thread senderThread(serverSenderKernel, &shm);
    std::thread receiverThread(serverReceiverKernel, &shm);

    getchar();
    if (senderThread.joinable())
        senderThread.join();
    if (receiverThread.joinable())
        receiverThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void clientSenderKernel(DuplChanFastSend *shm)
{
    char buf[BUFLEN];
    ::timeval tv;
    std::ostringstream oss;

    for (;;) {
        myMicroSleep(1000 * PING_MSEC);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);

        oss.str("");
        oss << "[" << __func__ << ": " << tvFormat(tv, buf) << "]" ;
        const auto &msg = oss.str();
        shm->send(msg);
        if ( ! g_running)
            break;
        cout << msg << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void clientReceiverKernel(DuplChanFastSend *shm)
{
    string buf;
    ::timeval tv;
    char timeBuf[BUFLEN];

    for (;;) {
        shm->recv(buf);
        if ( ! g_running)
            break;
        ::gettimeofday(&tv, NULL);
        cout << __func__ << ": " << tvFormat(tv, timeBuf) << ": [" << buf << "]" << endl;
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}


static void doClient()
{
    DuplChanFastSend shm;
    shm.open(id);

    std::thread senderThread(clientSenderKernel, &shm);
    std::thread receiverThread(clientReceiverKernel, &shm);

    getchar();
    if (senderThread.joinable())
        senderThread.join();
    if (receiverThread.joinable())
        receiverThread.join();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

enum EnumMode {
    EnumModeServer,
    EnumModeClient
};

int main(int argc, char *argv[])
{
    {
        struct ::sigaction act = { };
        act.sa_handler = sigIntHandler;
        if (RC_NG == ::sigemptyset(&act.sa_mask))
            myPerror("sigemptyset(3)", 4);
        if (RC_NG == ::sigaction(SIGINT, &act, NULL))
            myPerror("sigaction(2)", 7);
    }

    enum EnumMode mode = EnumModeServer;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "CS"))) {
        switch (optchr) {
            case 'C': mode = EnumModeClient; break;
            case 'S': mode = EnumModeServer; break;

            default:
                ::exit(5);
        }
    }

    if (EnumModeClient == mode) {
        doClient();
    } else {
        doServer();
    }
}
