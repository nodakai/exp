#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <time.h>

static uint32_t myAtoiN(const void *rawPtr, unsigned n)
{
    const char *ptr = (const char *)rawPtr;
    uint32_t i, ret = 0;
    for (i = 0; i < n; ++i) {
        const char c = ptr[i];
        if ('0' <= c && c <= '9')
            ret = 10*ret + (c - '0');
    }
    return ret;
}

static double diffInNsec(const struct timespec *ts0, const struct timespec *ts1)
{
    return 1e9 * (ts1->tv_sec - ts0->tv_sec) + (ts1->tv_nsec - ts0->tv_nsec);
}

int main(int argc, char *argv[])
{
    int i, N = 100000;
    uint64_t f = 0;
    struct timespec ts0, ts1;
    const char *arg, defArg[] = { ' ', '1', '2', '3' };
    size_t len;
    double nsec;

    if (argc > 1) {
        arg = argv[1];
        len = strlen(argv[1]);
    } else {
        arg = defArg;
        len = sizeof defArg;
    }

    clock_gettime(CLOCK_REALTIME, &ts0);
    for (i = 0; i < N; ++i) {
        f += myAtoiN(arg, len);
    }
    clock_gettime(CLOCK_REALTIME, &ts1);
    nsec = diffInNsec(&ts0, &ts1);
    printf("%f nsec / call\n", nsec / N);
    printf("%lu\n", f / N);

    return f;
}
