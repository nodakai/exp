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

#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>

#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };
static const size_t BUFLEN = 2048;

static void myPerror(const char *msg, int rc)
{
    ::perror(msg);
    ::exit(rc);
}

uint64_t myTime()
{
    ::timeval tv;
    ::gettimeofday(&tv, NULL);
    return (static_cast<uint64_t>(tv.tv_sec) << 32) | static_cast<uint64_t>(tv.tv_usec);
}

class BusyLoopChannel {
public:
    typedef std::unique_lock<std::mutex> Lock;

    enum {
        EMPTY = 0x1A2B3C4D,
        OCCUPIED = 0xD9C8B7A6
    };

private:
    std::atomic_int m_check;
    size_t m_msgLen;
    char m_buf[BUFLEN];

public:
    BusyLoopChannel() : m_check(EMPTY), m_msgLen(0) {
        std::fill(m_buf, m_buf + BUFLEN, '\0');
    }

    void send(const std::string &buf) {
        while (EMPTY != m_check)
            ;
        __builtin_ia32_lfence();
        m_msgLen = buf.size() + 1;
        ::memcpy(&m_buf[8], &buf[0], buf.size());
        m_buf[8 + buf.size()] = ';';
        __builtin_ia32_sfence();
        m_check = OCCUPIED;
    }

    void recv(std::string &buf) {
        while (OCCUPIED != m_check)
            ;
        __builtin_ia32_lfence();
        const size_t msgLen = m_msgLen;
        if (';' != m_buf[8+msgLen - 1]) {
            cout << "[" ;
            cout.write(m_buf, 8+msgLen+1);
            cout << "]" << endl;
            ::abort();
        }
        buf.assign(&m_buf[8], msgLen - 1);
        __builtin_ia32_mfence();
        m_check = EMPTY;
    }

};

class BlockingChannel {
public:
    typedef std::unique_lock<std::mutex> Lock;

    enum {
        EMPTY = 0x1A2B3C4D,
        OCCUPIED = 0xD9C8B7A6
    };

private:
    std::atomic_int m_check;
    size_t m_msgLen;
    char m_buf[BUFLEN];

public:
    BlockingChannel() : m_check(EMPTY), m_msgLen(0) {
        std::fill(m_buf, m_buf + BUFLEN, '\0');
    }

    void send(const std::string &buf) {
        while (EMPTY != m_check)
            ;
        __builtin_ia32_lfence();
        m_msgLen = buf.size() + 1;
        ::memcpy(&m_buf[8], &buf[0], buf.size());
        m_buf[8 + buf.size()] = ';';
        __builtin_ia32_sfence();
        m_check = OCCUPIED;
    }

    void recv(std::string &buf) {
        while (OCCUPIED != m_check)
            ;
        __builtin_ia32_lfence();
        const size_t msgLen = m_msgLen;
        if (';' != m_buf[8+msgLen - 1]) {
            cout << "[" ;
            cout.write(m_buf, 8+msgLen+1);
            cout << "]" << endl;
            ::abort();
        }
        buf.assign(&m_buf[8], msgLen - 1);
        __builtin_ia32_mfence();
        m_check = EMPTY;
    }

};

class SharedMemBase {
private:
    char * m_p;

protected:

    const size_t MMAP_LEN = 1024*1024;

    BusyLoopChannel *m_fastCh;
    BlockingChannel *m_ch;

protected:
    SharedMemBase() : m_p(NULL), m_fastCh(NULL), m_ch(NULL) { }
    ~SharedMemBase() { close(); }

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
        if (RC_OK != ::close(fd))
            myPerror("close(2)", 39);

        m_p = reinterpret_cast<char *>(p);
        cout << "p==" << (void*)m_p << endl;

        m_fastCh = reinterpret_cast<BusyLoopChannel *>(m_p);
        m_ch = reinterpret_cast<BlockingChannel*>(m_p + sizeof(*m_fastCh));
    }

};

class SharedMemUpward : public SharedMemBase {
public:
    inline void send(const std::string &buf) { m_fastCh->send(buf); }
    inline void recv(std::string &buf) { m_ch->recv(buf); }
};

class SharedMemDownward : public SharedMemBase {
public:
    inline void send(const std::string &buf) { m_ch->send(buf); }
    inline void recv(std::string &buf) { m_fastCh->recv(buf); }
};

static const string id("shmIpcBench");

static void doServer()
{
    SharedMemDownward shm;
    shm.open(id + "0");

    getchar();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

static void doClient()
{
    SharedMemUpward shm;
    shm.open(id + "0");

    getchar();

    cout << "Returning from " << __PRETTY_FUNCTION__ << endl;
}

enum EnumMode {
    EnumModeServer,
    EnumModeClient
};

int main(int argc, char *argv[])
{
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
