#define _GNU_SOURCE 1

#include <iostream>
#include <iomanip>

using namespace std;

#include <time.h>
#include <sched.h>
#include <sys/syscall.h>
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };

static void myPerror(const char *msg, int rc)
{
    ::perror(msg);
    ::exit(rc);
}

int gettid()
{
    return ::syscall(SYS_gettid);
}

static void mySetAffinity(int coreNo)
{
    ::cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(coreNo, &cpuSet);
    if (RC_OK != ::sched_setaffinity(gettid(), sizeof cpuSet, &cpuSet))
        myPerror("sched_setaffinity(2)", 30);
}

int main(int argc, char *argv[])
{
    struct timespec tsArr[40];

    for (int i = 0; i < 40; ++i) {
        mySetAffinity(i % 4);
        sched_yield();
    }

    for (int i = 0; i < 40; ++i) {
        mySetAffinity(i % 4);
        sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &tsArr[i]);
    }

    const long base = tsArr[0].tv_nsec;
    for (int i = 0; i < 10; ++i) {
        cout << std::setw(6) << tsArr[i+0].tv_nsec - base ;
        cout << std::setw(6) << tsArr[i+1].tv_nsec - base ;
        cout << std::setw(6) << tsArr[i+2].tv_nsec - base ;
        cout << std::setw(6) << tsArr[i+3].tv_nsec - base ;
        cout << endl;
    }
}
