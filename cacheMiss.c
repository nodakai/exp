#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static const uint64_t kilo = 1000ULL;
static const uint64_t giga = 1000ULL * 1000 * 1000;

static uint64_t toNsec(struct timespec * const restrict pTs) {
    return pTs->tv_nsec + giga * pTs->tv_sec;
};

static int myrand(int * const restrict state)
{
    const unsigned mask = ~0U >> 1;
    *state = 1103515245 * (*state) + 12345;
    return *state & mask;
}

static unsigned NLOOP = 100 * 1000;

static int test01()
{
    int i;
    for (i = 0; i < NLOOP; ++i) {
        ;
    }
    return i;
}

static int test02()
{
    int i, r = 2525105;
    for (i = 0; i < NLOOP; ++i) {
        myrand(&r);
    }
    return r;
}

static size_t BUFLEN = 100 * 1000;
static int *buf;

static int test03()
{
    int i, r = 2525105;
    for (i = 0; i < NLOOP; ++i) {
        myrand(&r);
        const int idx = i % BUFLEN;
        buf[idx] = buf[idx] + r;
    }
    return buf[1];
}

static int test04()
{
    int i, r = 2525105;
    for (i = 0; i < NLOOP; ++i) {
        const int idx = myrand(&r) % BUFLEN;
        buf[idx] = buf[idx] + r;
    }
    return buf[1];
}




int main(int argc, char *argv[])
{
    struct timespec tsArr[100];
    int i, j, dummy = 0;

    if (argc > 1) {
        BUFLEN = atoi(argv[1]);
    }
    buf = (int*)calloc(BUFLEN, sizeof(*buf));
    if (! buf) perror("malloc() to buf");

    if (argc > 2) {
        NLOOP = atoi(argv[2]);
    }

    fprintf(stderr, "Never forget to execute me under the taskset(1) command!\n");
    fprintf(stderr, "sizeof(buf) == %.1f KB\n", sizeof(*buf) * BUFLEN / 1024.0);
    fprintf(stderr, "NLOOP == %u\n", NLOOP);

    i = 0;
    /* warming up */
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);

    /* OK let's get started */
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test03();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);
    dummy += test04();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsArr[i++]);


    for (j = 0; j < i - 1; ++j) {
        double res = toNsec(&tsArr[j+1]) - toNsec(&tsArr[j]);
#ifdef DEBUG
        printf("rawRes == %f\n", res);
#else
        if (getenv("DEBUG")) printf("rawRes == %f\n", res);
        const char * fmt;
        if (res < 1e3) {
            fmt = "%.0f ns";
        } else if (res < 1e6) {
            res /= 1e3;
            fmt = "%.2f us";
        } else if (res < 1e9) {
            res /= 1e6;
            fmt = "%.2f ms";
        } else {
            res /= 1e9;
            if (res < 1e3) {
                fmt = "%.2f s";
            } else {
                fmt = "%.2e s";
            }
        }

        printf("delta[%d] == ", j);
        printf(fmt, res);
        printf("\n");
#endif
    }

    if (getenv("DEBUG")) {
        FILE *fp = fopen("/proc/self/maps", "r");
        for (;;) {
            char line[1024];
            char * const ret = fgets(line, 1024, fp);
            if (ret == NULL) break;
            line[1023] = '\0';
            printf("%s", line);
        }
        fclose(fp);
    }

    return dummy << 30;
}
