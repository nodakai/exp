#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/prctl.h>

enum { RC_OK = 0, RC_NG = -1 };

static void myNanoSleep(uint64_t nsec)
{
    int giga = 1000 * 1000 * 1000;
    struct timespec sleepDur = { nsec / giga, nsec % giga }, rest;
    while (nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
}

static uint64_t tsDiffNsec(const struct timespec *lhs, const struct timespec *rhs)
{
    return (lhs->tv_sec - rhs->tv_sec) * (uint64_t)1000 * 1000 * 1000 + (lhs->tv_nsec - rhs->tv_nsec);
}

int main(void)
{
    int i, j, durations[20] = { 1, 10, 100, 500, 1000, 5*1000, 10*1000, 100*1000, 1000*1000, 10*1000*1000, -1 };
    struct timespec ts[21], res;

    printf("prctl() => %d\n", prctl(PR_GET_TIMERSLACK));
    prctl(PR_SET_TIMERSLACK, 100);
    printf("prctl() => %d\n", prctl(PR_GET_TIMERSLACK));

    for (j = 0; j < 5; ++j) {
        for (i = 0; 0 < durations[i]; ++i) {
            if (RC_OK != clock_gettime(CLOCK_MONOTONIC, &ts[i])) {
                perror("clock_gettime");
                exit(10);
            }
            myNanoSleep(durations[i]);
        }
        if (RC_OK != clock_gettime(CLOCK_MONOTONIC, &ts[i])) {
            perror("clock_gettime");
            exit(20);
        }

        for (i = 0; 0 < durations[i]; ++i)
            printf("%8d ns => %8lu ns\n", durations[i], tsDiffNsec(&ts[i+1], &ts[i]));
    }

    if (RC_OK != clock_getres(CLOCK_MONOTONIC, &res)) {
        perror("clock_getres");
        exit(40);
    }
    printf("clock_getres(CLOCK_MONOTONIC) => {%lu, %lu}\n", res.tv_sec, res.tv_nsec);

    return 0;
}
