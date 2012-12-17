#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

#include <stdint.h>

#include <getopt.h>
#include <time.h>
#include <errno.h>

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


static void printUsage()
{
    cout << "memmovePerf [-n nRep] [-s size]" << endl;
    ::exit(5);
}

static void doBench(size_t len, size_t nRep, int sleepDur)
{
    cout << "len==" << len << ", nRep==" << nRep << ", sleepDur==" << sleepDur << endl;

    string s(len + 200, 'A');

    string winnieThePoo(100 * 1024*1024, 'W');
    string donaldDuck(100 * 1024*1024, 'D');

    double elapsedUsec = 0;
    ::timespec ts0, ts1;
    for (size_t i = 0; i < nRep; ++i) {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ::memmove(&s[8], &s[0], len);
        ::clock_gettime(CLOCK_REALTIME, &ts1);

        elapsedUsec += 1e6 * (ts1.tv_sec - ts0.tv_sec) + 1e-3 * (ts1.tv_nsec - ts0.tv_nsec);

        if (i % 2)
            winnieThePoo.assign(donaldDuck);
        else
            donaldDuck.assign(winnieThePoo);
        ::usleep(sleepDur);
    }

    cout << "Elapsed time is " << elapsedUsec << " usec (" << (1e3 * elapsedUsec / nRep) << " nsec per trial)" << endl;
    cout << winnieThePoo[0] << donaldDuck[0] << endl;
}

int main(int argc, char *argv[])
{
    size_t nRep = 1000, len = 300;
    int sleepDur = 1;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "n:l:s:hv:"))) {
        switch (optchr) {
            case 'n': nRep = myAtoi(::optarg); break;
            case 'l': len = myAtoi(::optarg); break;
            case 's': sleepDur = myAtoi(::optarg); break;

            case 'h':
            default:
                printUsage();
        }
    }

    doBench(len, nRep, sleepDur);
}
