#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <sys/time.h>
#include <getopt.h>

static uint64_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi) : : "%ecx" );
    return lo | (((uint64_t)hi) << 32);
}

long myAtoi(const char *str)
{
    if ('\0' == str[0]) {
        fprintf(stderr, "%s: ERROR: empty input.\n", __PRETTY_FUNCTION__);
        exit(22);
    } else {
        char *end;
        const long ret = strtol(str, &end, 0);
        if ('\0' == *end)
            return ret;

        fprintf(stderr, "%s: ERROR: invalid input \"%s\".\n", __PRETTY_FUNCTION__, str);
        exit(21);
    }
}

enum EnumMode {
    EnumModeClockGetTime,
    EnumModeGetTimeOfDay,
    EnumModeTime
};

#define GETTIME(var)    var = rdtsc()

int main(int argc, char *argv[])
{
    enum EnumMode mode = EnumModeClockGetTime;
    size_t i, nRep = 1000 * 1000, nPreRep = 10;
    struct timespec ts;
    struct timeval tv;
    double elapsedUsec;
    uint64_t start, stop;

    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "CGTn:N:"))) {
        switch (optchr) {
            case 'C': mode = EnumModeClockGetTime; break;
            case 'G': mode = EnumModeGetTimeOfDay; break;
            case 'T': mode = EnumModeTime; break;

            case 'n': nRep = myAtoi(optarg); break;
            case 'N': nPreRep = myAtoi(optarg); break;

            default:
                exit(4);
        }
    }

    if (EnumModeClockGetTime == mode) {
        puts("clock_gettime(2)");

        for (i = 0; i < nPreRep; ++i) {
            GETTIME(start);
            clock_gettime(CLOCK_MONOTONIC, &ts);
            GETTIME(stop);
        }

        GETTIME(start);
        for (i = 0; i < nRep; ++i)
            clock_gettime(CLOCK_MONOTONIC, &ts);
        GETTIME(stop);
    } else if (EnumModeGetTimeOfDay == mode) {
        puts("gettimeofday(2)");

        for (i = 0; i < nPreRep; ++i) {
            GETTIME(start);
            gettimeofday(&tv, NULL);
            GETTIME(stop);
        }

        GETTIME(start);
        for (i = 0; i < nRep; ++i)
            gettimeofday(&tv, NULL);
        GETTIME(stop);
    } else if (EnumModeTime == mode) {
        puts("time(2)");

        for (i = 0; i < nPreRep; ++i) {
            GETTIME(start);
            time(NULL);
            GETTIME(stop);
        }

        GETTIME(start);
        for (i = 0; i < nRep; ++i)
            time(NULL);
        GETTIME(stop);
    } else {
        fprintf(stderr, "Should not get here!\n");
        exit(8);
    }

    elapsedUsec = (stop - start) / 3.4e3; /* 1e6 * (stop.tv_sec - start.tv_sec) + 1e-3 * (stop.tv_nsec - start.tv_nsec); */
    printf("Elapsed time is %f usec (%f nsec per call)\n", elapsedUsec, 1e3 * elapsedUsec / nRep);

    return 0;
}
