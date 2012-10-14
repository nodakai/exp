#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <tr1/unordered_set>
#include <stdexcept>
#include <algorithm>

using namespace std;

#include <getopt.h>
#include <time.h>

static long myAtoi(const char *str) {
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' != *end)
        throw std::invalid_argument(string("myAtoi(") + str + ")");
    return ret;
}

static int myRand(int * const state) {
    const unsigned mask = ~0U >> 1;
    *state = 1103515245 * (*state) + 12345;
    return *state & mask;
}

static void printUsage() {
    cout << "vecScanPerf (-V|-S|-H) [-n nRep] [-k nKey]" << endl;
    cout << "    -V: use std::vector as a container" << endl;
    cout << "    -S: use std::set as a container" << endl;
    cout << "    -H: use std::tr1::unordered_set as a container" << endl;
    cout << "    -n nRep (int): number of repetitions" << endl;
    cout << "    -k nKey (int): number of keys in a container" << endl;
}

enum Container {
    ContainerVector,
    ContainerSet,
    ContainerHashset,
};

static int s_nKey = 5;
static long s_nRep = 1000;

template <class T>
static void doTest(const char *name, T &theSet)
{
    int okCnt = 0, ngCnt = 0;
    int randState = 123;

    ::timespec ts0, ts1;
    ::clock_gettime(CLOCK_REALTIME, &ts0);
    const typename T::const_iterator b(theSet.begin()), e(theSet.end());
    for (int i = 0; i < s_nRep; ++i) {
        const int v = (myRand(&randState) % 7) * 100;
        if (e == std::find(b, e, v))
            ++ngCnt;
        else
            ++okCnt;
    }
    ::clock_gettime(CLOCK_REALTIME, &ts1);

    cout << "okCnt == " << okCnt << ";  ngCnt == " << ngCnt << endl;
    const long diffNsec = (ts1.tv_sec - ts0.tv_sec) * 1000 * 1000 * 1000 +
        (ts1.tv_nsec - ts0.tv_nsec);
    cout << name << ": elapsed time == " << (diffNsec / 1000 / 1000) << " msec;  " <<
        (static_cast<double>(diffNsec) / s_nRep) << " nsec / call " << endl;
}

static void doTestSet() {
    set<int> theSet;
    for (int k = 0; k < s_nKey; ++k)
        theSet.insert(100 * (k+1));

    doTest(__func__, theSet);
}

static void doTestHashset() {
    tr1::unordered_set<int> theSet;
    for (int k = 0; k < s_nKey; ++k)
        theSet.insert(100 * (k+1));

    doTest(__func__, theSet);
}

static void doTestVector() {
    vector<int> theSet;
    for (int k = 0; k < s_nKey; ++k)
        theSet.push_back(100 * (k+1));

    doTest(__func__, theSet);
}

int main(int argc, char *argv[])
{
    Container mode = ContainerSet;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "VSHn:k:"))) {
        switch (optchr) {
            case 'V': mode = ContainerVector; break;
            case 'S': mode = ContainerSet; break;
            case 'H': mode = ContainerHashset; break;

            case 'n': s_nRep = myAtoi(::optarg); break;
            case 'k': s_nKey = myAtoi(::optarg); break;

            default: printUsage();
        }
    }

    if (ContainerSet == mode)
        doTestSet();
    else if (ContainerHashset == mode)
        doTestHashset();
    else
        doTestVector();
}
