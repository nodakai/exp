#define _POSIX_C_SOURCE 200112L

#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sys/time.h>

#include <stdint.h>

#include <getopt.h>

using namespace std;

static double s_FREQ = 3.4;

static inline uint64_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi) : : "%ecx" );
    // return lo;
    return lo | (static_cast<uint64_t>(hi) << 32);
}

static double myAtof(const char *str)
{
    char *end;
    const long ret = strtod(str, &end);
    if ('\0' == *end)
        return ret;
    fprintf(stderr, "myAtof: invalid argument [%s]", str);
    exit(20);
}

int Round(int x)
{
    uint64_t nth = 1;
    int x0 = x;
    while (x0 > 0) {
        nth *= 10;
        x0 /= 10;
    }

    // nth /= 100;
    const int up = nth - 1;
    if (0 < nth)
        return x;
    else
        return ((x + up) / nth) * nth;
}

static void printUsage(void)
{
    printf("gettimeofdaySpin [-f freq]\n");
    printf("    -f freq (double): CPU frequency in GHz (defaults to 3.4)\n");
}

int main(int argc, char *argv[])
{
    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "f:"))) {
        switch (optchr) {
            case 'f': s_FREQ = myAtof(::optarg); break;

            default: printUsage();
        }
    }

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
#elif 0
       struct timespec a, b;
       clock_gettime(CLOCK_REALTIME, &a);
       clock_gettime(CLOCK_REALTIME, &b);
       if (a.tv_nsec != b.tv_nsec)
           arr.push_back(b.tv_nsec - a.tv_nsec);
#else
       struct timeval a, b;
       gettimeofday(&a, NULL);
       gettimeofday(&b, NULL);
       if (a.tv_usec != b.tv_usec)
           arr.push_back(b.tv_usec - a.tv_usec);
#endif
    }

    typedef map<int, int> Histo;
    Histo histo;
    for (int i = 0; i < arr.size(); ++i) {
        histo[Round(arr[i])] += 1;
    }

    for (Histo::const_iterator it(histo.begin()), itEnd(histo.end()); it != itEnd; ++it) {
        cout << it->first
#if 0
            << ", " << it->first / s_FREQ
#endif
            << ", " << it->second << endl;
    }
}
