#include <iostream>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

boost::mutex s_mtx;

void kernelNoLock(int tid, int nRep)
{
    const string msg = "Hi there";

    for (int i = 0; i < nRep; ++i) {
        cout << boost::posix_time::microsec_clock::local_time() <<
            " [" << tid << "] " << msg << endl;
    }
}

void kernelScopedLock(int tid, int nRep)
{
    const string msg = "Hi there";

    for (int i = 0; i < nRep; ++i) {
        boost::mutex::scoped_lock lk(s_mtx);
        cout << boost::posix_time::microsec_clock::local_time() <<
            " [" << tid << "] " << msg << endl;
    }
}

boost::mutex::scoped_lock autolk()
{
    return boost::mutex::scoped_lock(s_mtx);
}

void kernelCommaLock(int tid, int nRep)
{
    const string msg = "Hi there";

#define LOCKED_COUT0 (boost::mutex::scoped_lock(s_mtx)), cout
#define LOCKED_COUT autolk(), cout
    for (int i = 0; i < nRep; ++i) {
        LOCKED_COUT << boost::posix_time::microsec_clock::local_time() <<
            " [" << tid << "] " << msg << endl;
    }
}

typedef void (*Kernel) (int, int);

int main(int argc, char *argv[])
{
    enum Mode {
        ModeNoLock,
        ModeScopedLock,
        ModeCommaLock
    };

    int nThreads = 2, nRep = 100, mode = ModeNoLock;
    if (argc > 1)
        nThreads = boost::lexical_cast<int>(argv[1]);
    if (argc > 2)
        nRep = boost::lexical_cast<int>(argv[2]);
    if (argc > 3)
        mode = boost::lexical_cast<int>(argv[3]);

    Kernel kernel;
    switch (mode) {
        case ModeNoLock:
            kernel = kernelNoLock;
            cout << "No lock" << endl;
            break;
        case ModeScopedLock:
            kernel = kernelScopedLock;
            cout << "Scoped lock" << endl;
            break;
        case ModeCommaLock:
            kernel = kernelCommaLock;
            cout << "Comma lock" << endl;
            break;
    }

    boost::thread_group tgr;
    for (int i = 0; i < nThreads; ++i)
        tgr.create_thread(boost::bind(kernel, i, nRep));

    tgr.join_all();
}
