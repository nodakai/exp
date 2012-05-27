#define _POSIX_C_SOURCE 200112L

#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sys/time.h>

#include <stdint.h>

using namespace std;

static inline uint32_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return lo;
    // return lo | (((uint64_t)hi) << 32);
}

int Round(int x)
{
    uint64_t nth = 1;
    int x0 = x;
    while (x0 > 0) {
        nth *= 10;
        x0 /= 10;
    }

    nth /= 100;
    const int up = nth - 1;
    return ((x + up)/ nth) * nth;
}

int main(int argc, char *argv[])
{
    long rep = 10000;
    if (argc > 1)
        rep = strtol(argv[1], NULL, 10);

    vector<uint32_t> arr(2 * rep);
    arr.resize(0);
    while (arr.size() < rep) {
#if 0
       const uint32_t a = rdtsc(), b = rdtsc();
       if (a != b)
           arr.push_back(b - a);
#else
       struct timespec a, b;
       clock_gettime(CLOCK_REALTIME, &a);
       clock_gettime(CLOCK_REALTIME, &b);
       if (a.tv_nsec != b.tv_nsec)
           arr.push_back(b.tv_nsec - a.tv_nsec);
#endif
    }

    typedef map<int, int> Histo;
    Histo histo;
    for (int i = 0; i < arr.size(); ++i) {
        histo[Round(arr[i])] += 1;
    }

    for (Histo::const_iterator it(histo.begin()), itEnd(histo.end()); it != itEnd; ++it) {
        cout << it->first << ", " << it->first / 2.93 << ", " << it->second << endl;
    }
}
