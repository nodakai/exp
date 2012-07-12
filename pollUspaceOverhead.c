#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <poll.h>
#include <sys/select.h>
#include <getopt.h>

static uint64_t rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__ ( "rdtscp" : "=a"(lo), "=d"(hi) : : "%ecx" );
    return lo | (((uint64_t)hi) << 32);
}

static int testSelectOverhead(int n)
{
    int i;
    uint64_t t0, t1;

    fd_set readFds, errorFds;
    int readableFd = -1, errorFd = -1;

    FD_ZERO(&readFds);
    FD_ZERO(&errorFds);

    if (n == 12345) {
        FD_SET(123, &readFds);
        FD_SET(123, &errorFds);
    } else {
        FD_SET(n - 1, &readFds);
    }

    t0 = rdtsc();

    for (i = 0; i < n; ++i) {
        if (FD_ISSET(i, &readFds)) {
            readableFd = i;
            break;
        } else if (FD_ISSET(i, &errorFds)) {
            errorFd = i;
            break;
        }
    }

    if (readableFd >= 0) {
        t1 = rdtsc();
    } else if (errorFd >= 0) {
        t1 = rdtsc();
    }

    return t1 - t0;
}

static int testPollOverhead(int n)
{
    int i;
    uint64_t t0, t1;
    struct pollfd * const pollfdArr = (struct pollfd *)calloc(sizeof *pollfdArr, n);
    int readableFd = -1, errorFd = -1;

    if (n == 12345) {
        pollfdArr[0].revents |= POLLIN;
        pollfdArr[0].revents |= POLLERR;
    } else {
        pollfdArr[n - 1].revents |= POLLIN;
    }

    t0 = rdtsc();

    for (i = 0; i < n; ++i) {
        if (pollfdArr[i].revents & POLLIN) {
            readableFd = i;
            break;
        } else if (pollfdArr[i].revents & POLLERR) {
            errorFd = i;
            break;
        }
    }

    if (readableFd >= 0) {
        t1 = rdtsc();
    } else if (errorFd >= 0) {
        t1 = rdtsc();
    }

    free(pollfdArr);

    return t1 - t0;
}

int main(int argc, char *argv[])
{
    char optchr;
    int n = -1;
    int cycles = 0;
    while (-1 != (optchr = getopt(argc, argv, "s:p:"))) {
        switch (optchr) {
            case 's': n = atoi(optarg); cycles = testSelectOverhead(n); break;
            case 'p': n = atoi(optarg); cycles = testPollOverhead(n); break;
            default: exit(10);
        }
    }

    if (cycles > 0) {
        printf("%6d clks == %6.2f ns => %6.2f ns/trial\n", cycles, cycles/2.93, cycles/2.93 / n);
    }

    return 0;
}
