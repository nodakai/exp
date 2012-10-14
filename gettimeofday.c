#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <stdint.h>

#include <getopt.h>

static double s_FREQ = 3.4;

static inline uint64_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi) : : "%ecx" );
    return lo | (((uint64_t)hi) << 32);
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

static void printUsage(void)
{
    printf("gettimeofday [-f freq]\n");
    printf("    -f freq (double): CPU frequency in GHz (defaults to 3.4)\n");
}

int main(int argc, char *argv[])
{
    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "f:"))) {
        switch (optchr) {
            case 'f': s_FREQ = myAtof(optarg); break;

            default: printUsage();
        }
    }

    {
        int i;
        for (i = 0; i < 20000000; ++i)
            free(malloc(i));
    }

    {
        struct timeval tv0, tv1;
        gettimeofday(&tv0, NULL);
        printf("sizeof(timeval) == {%u, %u}\n", (unsigned)sizeof(tv0.tv_sec), (unsigned)sizeof(tv0.tv_usec));
        gettimeofday(&tv1, NULL);
        printf("gettimeofday(): (%lu, %lu) - (%lu, %lu) = %lu usec\n", tv1.tv_sec, tv1.tv_usec, tv0.tv_sec, tv0.tv_usec, (tv1.tv_sec - tv0.tv_sec) * 1000000UL + (tv1.tv_usec - tv0.tv_usec));
    }
    printf("\n");

    {
        size_t i;
        clockid_t ids[] = {
            CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID
            /* , CLOCK_REALTIME_HR, CLOCK_MONOTONIC_HR */
        };
        for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
            struct timespec ts0, ts1, res;
            clock_getres(ids[i], &res);
            clock_gettime(ids[i], &ts0);
            printf("sizeof(timespec) == {%u, %u}\n", (unsigned)sizeof(ts0.tv_sec), (unsigned)sizeof(ts0.tv_nsec));
            clock_gettime(ids[i], &ts1);
            printf("clock_getres(%d) => (%lu, %lu)\n", ids[i], res.tv_sec, res.tv_nsec);
            printf("clock_gettime(%d): (%lu, %lu) - (%lu, %lu) = %lu nsec\n", ids[i], ts1.tv_sec, ts1.tv_nsec, ts0.tv_sec, ts0.tv_nsec, (ts1.tv_sec - ts0.tv_sec) * 1000000000UL + (ts1.tv_nsec - ts0.tv_nsec));
            printf("\n");
        }
    }

    {
        const clock_t cl = clock();
        printf("CPU time spent so far is %lu units == %f secs\n", cl, cl / (double)CLOCKS_PER_SEC);
        printf("sizeof(clock_t) == %u\n", (unsigned)sizeof(cl));
    }
    printf("\n");

    {
        time_t tm;
        time(&tm);
        printf("%lu secs from the Epoch\n", tm);
        printf("sizeof(time_t) => %u\n", (unsigned)sizeof(tm));
    }
    printf("\n");

    {
        uint64_t t0, t1, t2, diff;
        t0 = rdtsc();
        t1 = rdtsc();
        diff = t1 - t0;
        printf("%lu => %f\n", diff, diff / s_FREQ);
        t2 = rdtsc();
        diff = t2 - t1;
        printf("%lu - %lu == %lu [cycles] => %f [ns]\n", t1, t0, diff, diff / s_FREQ);
    }

    return 0;
}
