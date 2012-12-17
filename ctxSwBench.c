#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h> /* usleep */
#include <sys/syscall.h>

static void myPerror(const char *msg, int rc)
{
    perror(msg);
    exit(rc);
}

static long myAtoi(const char *str)
{
    char *end;
    long ret;
    if ('\0' == str[0]) {
        fprintf(stderr, "myAtoi: empty string");
        exit(30);
    }
    errno = 0;
    ret = strtol(str, &end, 0);
    if (errno)
        myPerror("strtol", 31);
    if ('\0' != *end) {
        fprintf(stderr, "myAtoi: erroneous input \"%s\"", str);
        exit(32);
    }
    return ret;
}

#define FUNC_UNDER_TEST() getpid()

enum EnumMode {
    EnumModeSched,
    EnumModePid
};

int main(int argc, char *argv[])
{
    size_t i, nRep = 1000, nPrevRep = 0, sleepDurUsec = 0;
    double elapsedUsec;
    struct timespec ts0, ts1;
    enum EnumMode mode = EnumModeSched;

    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "n:N:s:PSh"))) {
        switch (optchr) {
            case 'n': nRep = myAtoi(optarg); break;
            case 'N': nPrevRep = myAtoi(optarg); break;
            case 's': sleepDurUsec = myAtoi(optarg); break;

            case 'P': mode = EnumModePid; break;
            case 'S': mode = EnumModeSched; break;

            default:
                exit(10);
        }
    }

    switch (mode) {
        case EnumModeSched:
            {
                for (i = 0; i < nPrevRep; ++i) {
                    clock_gettime(CLOCK_MONOTONIC, &ts0);
                    syscall(SYS_sched_yield);
                }

                if (0 < sleepDurUsec)
                    usleep(sleepDurUsec);

                clock_gettime(CLOCK_MONOTONIC, &ts0);
                for (i = 0; i < nRep; ++i)
                    syscall(SYS_sched_yield);
                clock_gettime(CLOCK_MONOTONIC, &ts1);
            }
            break;

        case EnumModePid:
            {
                for (i = 0; i < nPrevRep; ++i) {
                    clock_gettime(CLOCK_MONOTONIC, &ts0);
                    syscall(SYS_getpid);
                }

                if (0 < sleepDurUsec)
                    usleep(sleepDurUsec);

                clock_gettime(CLOCK_MONOTONIC, &ts0);
                for (i = 0; i < nRep; ++i)
                    syscall(SYS_getpid);
                clock_gettime(CLOCK_MONOTONIC, &ts1);
            }
            break;
    }

    elapsedUsec = 1e6 * (ts1.tv_sec - ts0.tv_sec) + 1e-3 * (ts1.tv_nsec - ts0.tv_nsec);
    printf("Elapsed time is %f usec (%f nsec per call.)\n", elapsedUsec, 1e3 * elapsedUsec / nRep);

    return 0;
}
