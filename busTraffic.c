#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>

struct SpinnerArg {
    const int ** samplePtrWatcher;
};

static void *spinner(void * rawPArg)
{
    size_t cnt = 0;
    uintptr_t ret;
    struct SpinnerArg * const pArg = (struct SpinnerArg *)rawPArg;
    const int ** const samplePtrWatcher = pArg->samplePtrWatcher;

    const int * const origPtr = *samplePtrWatcher;

    for (;;) {
        __builtin_ia32_sfence();
        if (origPtr != *samplePtrWatcher) break;
        ++cnt;
    }

    ret = cnt; /* **samplePtrWatcher; */
    return (void*)ret;
}

int main(int argc, char *argv[])
{
    pthread_t *thrArr;
    struct SpinnerArg *spArgArr;
    void **retArr;
    struct timespec ts0, ts1;
    double dt;
    int i, numSpinners = 2, rep = 1000 * 1000, *buf;
    size_t bufLen = 1024 * 1024;
    const int dat0 = 100, dat1 = 200, *samplePtr = &dat0;

    for (i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--numSpinners")) {
            if (++i == argc) perror("--numSpinners");
            numSpinners = atoi(argv[i]);
        } else if (0 == strcmp(argv[i], "--rep")) {
            if (++i == argc) perror("--rep");
            rep = atoi(argv[i]);
        } else if (0 == strcmp(argv[i], "--bufSizeKB")) {
            if (++i == argc) perror("--bufSizeKB");
            bufLen = atoi(argv[i]) * 1024ULL;
        }
    }

    buf = (int*)calloc(sizeof(*buf), bufLen);
    if ( ! buf)
        perror("calloc() for buf");

    spArgArr = (struct SpinnerArg *)malloc(sizeof(*spArgArr) * numSpinners);
    for (i = 0; i < numSpinners; ++i) {
        spArgArr[i].samplePtrWatcher = &samplePtr;
    }
    thrArr = (pthread_t *)malloc(sizeof(*thrArr) * numSpinners);
    retArr = (void **)malloc(sizeof(*retArr) * numSpinners);

    for (i = 0; i < numSpinners; ++i) {
        pthread_create(&thrArr[i], NULL, spinner, (void*)&spArgArr[i]);
    }

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts0);
    for (i = 0; i < rep; ++i) {
        const int idx = i % bufLen;
        buf[idx] = 2 * buf[idx] + 1;
    }
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts1);
    samplePtr = &dat1;
    printf("samplePtr updated\n");

    for (i = 0; i < numSpinners; ++i) {
        pthread_join(thrArr[i], &retArr[i]);
    }

    for (i = 0; i < numSpinners; ++i) {
        printf("retArr[%d] == %lu;  \n", i, (uintptr_t)retArr[i]);
    }

    dt = (ts1.tv_sec - ts0.tv_sec) + 1e-9 * (ts1.tv_nsec - ts0.tv_nsec);
    printf("dt == %f sec\n", dt);

    return 0;
}
