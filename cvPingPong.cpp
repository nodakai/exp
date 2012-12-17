#define _GNU_SOURCE 1

#include <iostream>
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

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <linux/futex.h>
#include <sys/syscall.h>

#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };
static const size_t BUFLEN = 2048;

static size_t g_nRep = 100, g_msgLen = 300;
static int g_sleepDurUsec = 10 * 1000;

template <unsigned N>
inline bool mySprintf(char (&buf)[N], uint64_t n, char fill = ' ')
{ return mySprintf(buf, N, n, fill); }

bool mySprintf(char *buf, unsigned N, uint64_t n, char fill = ' ') {
    int i;
    for (i = N - 1; i >= 0; --i) {
        const int ith = n % 10;
        buf[i] = '0' + ith;
        n /= 10;
        if (0 == n) // if n==0 was given, we need to write only single '0'
            break;
    }

    if (0 > i && n) // not enough width to represent the input integer!
        return false;

    while (--i >= 0)
        buf[i] = fill;
    return true;
}

long myAtoi(const char *str)
{
    if ('\0' == str[0]) {
        cerr << __PRETTY_FUNCTION__ << ": ERROR: empty input." << endl;
        ::exit(22);
    }

    char *end;
    const long ret = ::strtol(str, &end, 0);
    if ('\0' == *end)
        return ret;

    cerr << __PRETTY_FUNCTION__ << ": ERROR: invalid input \"" << str << "\"" << endl;
    ::exit(21);
}

uint64_t myAtoiN(const void *rawPtr, unsigned n) {
    const auto ptr = reinterpret_cast<const char *>(rawPtr);
    uint64_t ret = 0;
    unsigned i = 0;
    while (' ' == ptr[i])
        ++i;
    for ( ; i < n; ++i) {
        const char c = ptr[i];
        if ('0' <= c && c <= '9')
            ret = 10*ret + (c - '0');
    }
    return ret;
}

template <typename T, unsigned N>
inline uint64_t myAtoiN(const T (&buf)[N]) { return myAtoiN(buf, N); }


static void printUsage()
{
    cout << "cvPingPong [-n nRep] [-s sleepDur] [-h]" << endl;
    ::exit(5);
}

struct Channel {
    typedef std::unique_lock<std::mutex> Lock;

#if 1
    std::atomic_bool m_check;
    size_t m_msgLen;
    char m_buf[BUFLEN];

    Channel() : m_check(false) { }

    void send(const std::string &buf) {
//      mySprintf(&m_buf[0], 8, buf.size() + 1);
        m_msgLen = buf.size() + 1;
        ::memcpy(&m_buf[8], &buf[0], buf.size());
        m_buf[8 + buf.size()] = ';';
        __builtin_ia32_sfence();
        m_check = true;
        // __atomic_thread_fence(__ATOMIC_RELEASE);
    }

    void recv(std::string &buf) {
        while ( ! m_check)
            ;
        __builtin_ia32_lfence();
//      const size_t msgLen = myAtoiN(m_buf, 8);
        const size_t msgLen = m_msgLen;
        if (';' != m_buf[8+msgLen - 1]) {
            cout << "[" ;
            cout.write(m_buf, 8+msgLen+1);
            cout << "]" << endl;
            abort();
        }
        buf.assign(&m_buf[8], msgLen - 1);
        __builtin_ia32_mfence();
        m_check = false;
    }


#elif 0
    int m_ftxVar;

    int futex(int op, int val) {
        return ::syscall(SYS_futex, &m_ftxVar, op, val, NULL, NULL, 0);
    }

    void send(const std::string &buf) {
        futex(FUTEX_WAKE, 1);
    }

    void recv(const std::string &buf) {
        int var;
        do {
            var = m_ftxVar;
        } while (futex(FUTEX_WAIT, var));
    }

#elif 0
    bool m_check;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    char m_buf[BUFLEN];

    Channel() : m_check(false) {
        static_assert(0 < _POSIX_THREAD_PROCESS_SHARED, "_POSIX_THREAD_PROCESS_SHARED is not supported.");

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

    void send(const string &str) {
        mySprintf(&m_buf[0], 8, str.size() + 1);
        ::memcpy(&m_buf[8], &str[0], str.size());
        m_buf[8 + str.size()] = ';';
//      cout << __PRETTY_FUNCTION__ << ": " << this << endl;
//      hexdump();
        Lock lk(m_mtx);
        m_check = true;
        m_cv.notify_one();
    }

    void recv(string &str) {
//      cout << __PRETTY_FUNCTION__ << ": " << this << endl;
//      hexdump();
        Lock lk(m_mtx);
        while ( ! m_check) {
//          hexdump();
            m_cv.wait(lk);
        }
        const size_t msgLen = myAtoiN(m_buf, 8);
        if (';' != m_buf[8+msgLen - 1]) {
            cout << "[" ;
            cout.write(m_buf, 8+msgLen+1);
            cout << "]" << endl;
            abort();
        }
        str.assign(&m_buf[8], msgLen - 1);
        m_check = false;
    }

#elif 0
    std::mutex m_mtx;

    Channel() { m_mtx.lock(); }

    void notify() {
        m_mtx.unlock();
    }

    void wait() {
        m_mtx.lock();
    }
#endif

    void hexdump() const {
        const char *p = reinterpret_cast<const char *>(this);
        size_t i;
        for (i = 0; i < sizeof(*this); ++i) {
            cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned)(unsigned char)p[i] << ' ';
            if (0 == (i+1)%16)
                cout << endl;
        }
        if (0 != i%16)
            cout << endl;
        cout << std::dec;
    }
};

static void setAffinity(unsigned affMask) {
    if (0 < affMask)
        return;

    ::pthread_t tid = ::pthread_self();

    ::cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (size_t i = 0, nProc = ::sysconf(_SC_NPROCESSORS_CONF); i < nProc; ++i) {
        if ((affMask >> i) & 0x01)
            CPU_SET(i, &cpuSet);
    }

    if (RC_OK != ::pthread_setaffinity_np(tid, sizeof cpuSet, &cpuSet)) {
        ::perror("pthread_setaffinity_np(3)");
        ::exit(8);
    }
}

static void showStatus(const char *name, unsigned affMask, const string *id = NULL)
{
    cout << name << ": nRep==" << g_nRep << ", sleepDurUsec==" << g_sleepDurUsec <<
        ", affMask==0x" << std::setw(8) << std::setfill('0') << std::hex << affMask << std::setfill(' ') << std::dec;
    if (id)
        cout << ", id==" << *id ;
    cout << endl;
}

static void pingerKernel(Channel *chA, Channel *chB, unsigned affMask)
{
    showStatus(__PRETTY_FUNCTION__, affMask);

    setAffinity(affMask);
    const int sleepDurUsec = g_sleepDurUsec;
    ::timespec ts0, ts1;
    string buf(BUFLEN, 'A'), buf1(BUFLEN, 'A');
    buf.resize(g_msgLen);
    buf1.assign(buf);

    chA->send(buf);
    chB->recv(buf);

    double elapsedUsec = 0;
    for (size_t i = 0, nRep = g_nRep; i < nRep; ++i) {
        if (0 < sleepDurUsec)
            ::usleep(sleepDurUsec);

        ::clock_gettime(CLOCK_MONOTONIC, &ts0);
        chA->send(buf);
        chB->recv(buf);
        ::clock_gettime(CLOCK_MONOTONIC, &ts1);

        elapsedUsec += 1e6 * (ts1.tv_sec - ts0.tv_sec) + 1e-3 * (ts1.tv_nsec - ts0.tv_nsec);
        if (buf != buf1)
            cerr << "[" << buf << "] vs [" << buf1 << "]" << endl;
    }

    const double rtt = elapsedUsec / g_nRep;
    cout << "Elapsed time is " << elapsedUsec << " usec (RTT " << rtt << " usec/call == 2 * " << (rtt / 2.0) << " usec/call)" << endl;
    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void pongerKernel(Channel *chA, Channel *chB, unsigned affMask)
{
    showStatus(__PRETTY_FUNCTION__, affMask);

    setAffinity(affMask);

    string buf(BUFLEN, 'A');
    for (size_t i = 0, nRep = 1 + g_nRep; i < nRep; ++i) {
        chA->recv(buf);
        chB->send(buf);
    }

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doThreaded(unsigned aff1, unsigned aff2)
{
    Channel chA, chB;

    std::thread ponger(pongerKernel, &chA, &chB, aff2);
    ::usleep(1);
    std::thread pinger(pingerKernel, &chA, &chB, aff1);

    pinger.join();
    ponger.join();
}

const size_t MMAP_LEN = 8*1024;

static void doProcessServer(unsigned affMask, const string &id)
{
    showStatus(__PRETTY_FUNCTION__, affMask, &id);

    const int fd = ::shm_open(id.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (RC_NG == fd) {
        ::perror("shm_open(3)");
        ::exit(30);
    }
    if (RC_NG == ::ftruncate(fd, MMAP_LEN)) {
        ::perror("ftruncate(2)");
        ::exit(31);
    }
    void * const p = ::mmap(NULL, MMAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == p) {
        ::perror("mmap(2)");
        ::exit(32);
    }
    if (RC_OK != ::close(fd)) {
        ::perror("close(2)");
        ::exit(33);
    }
    cout << "p==" << p << endl;

    string buf(BUFLEN, 'A');
//  if (RC_OK != ::mlockall(MCL_CURRENT)) {
//      ::perror("mlockall(2)");
//      ::exit(34);
//  }

    auto const chA = reinterpret_cast<Channel *>(p), chB = chA + 1;
//  auto const chB = reinterpret_cast<Channel *>(((uintptr_t)p) + 128);
    new (chA) Channel;
    new (chB) Channel;

    for (size_t i = 0, nRep = g_nRep; i < nRep; ++i) {
        chA->recv(buf);
        chB->send(buf);
    }

    if (RC_OK != ::munmap(p, MMAP_LEN)) {
        ::perror("munmap(2)");
        ::exit(38);
    }
}

static void doProcessClient(unsigned affMask, const string &id)
{
    showStatus(__PRETTY_FUNCTION__, affMask, &id);

    const int fd = ::shm_open(id.c_str(), O_RDWR, 0);
    if (RC_NG == fd) {
        ::perror("shm_open(3)");
        ::exit(30);
    }
    void * const p = ::mmap(NULL, MMAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == p) {
        ::perror("mmap(2)");
        ::exit(32);
    }
    if (RC_OK != ::close(fd)) {
        ::perror("close(2)");
        ::exit(33);
    }
    cout << "p==" << p << endl;

    string buf(BUFLEN, 'A');
    buf.resize(g_msgLen);
//  if (RC_OK != ::mlockall(MCL_CURRENT)) {
//      ::perror("mlockall(2)");
//      ::exit(34);
//  }

    auto const chA = reinterpret_cast<Channel *>(p), chB = chA + 1;
//  auto const chB = reinterpret_cast<Channel *>(((uintptr_t)p) + 128);

    const int sleepDurUsec = g_sleepDurUsec;
    ::timespec ts0, ts1;

    double elapsedUsec = 0;
    for (size_t i = 0, nRep = g_nRep; i < nRep; ++i) {
        ::usleep(sleepDurUsec);

        ::clock_gettime(CLOCK_MONOTONIC, &ts0);
        chA->send(buf);
        chB->recv(buf);
        ::clock_gettime(CLOCK_MONOTONIC, &ts1);

        elapsedUsec += 1e6 * (ts1.tv_sec - ts0.tv_sec) + 1e-3 * (ts1.tv_nsec - ts0.tv_nsec);
    }

    const double rtt = elapsedUsec / g_nRep;
    cout << "Elapsed time is " << elapsedUsec << " usec (RTT " << rtt << " usec/call == 2 * " << (rtt / 2.0) << " usec/call)" << endl;
    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;

    if (RC_OK != ::munmap(p, MMAP_LEN)) {
        ::perror("munmap(2)");
        ::exit(38);
    }
}

enum EnumMode {
    EnumModeThread,
    EnumModeProcessServer,
    EnumModeProcessClient
};

int main(int argc, char *argv[])
{
    enum EnumMode mode = EnumModeThread;
    unsigned aff1 = -1, aff2 = -1;
    string id;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "TS:C:n:s:l:1:2:h"))) {
        switch (optchr) {
            case 'T': mode = EnumModeThread; break;
            case 'S': mode = EnumModeProcessServer; id.assign(::optarg); break;
            case 'C': mode = EnumModeProcessClient; id.assign(::optarg); break;

            case 'n': g_nRep = myAtoi(::optarg); break;
            case 's': g_sleepDurUsec = myAtoi(::optarg); break;
            case 'l': g_msgLen = myAtoi(::optarg); break;

            case '1': aff1 = myAtoi(::optarg); break;
            case '2': aff2 = myAtoi(::optarg); break;

            case 'h':
            default:
                printUsage();
        }
    }

    if (EnumModeThread == mode)
        doThreaded(aff1, aff2);
    else if (EnumModeProcessServer == mode)
        doProcessServer(aff1, id);
    else if (EnumModeProcessClient == mode)
        doProcessClient(aff1, id);
    else {
        cerr << "Unknown mode==\"" << mode << "\"" << endl;
        ::exit(9);
    }
}
