#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <stdint.h>
#include <unistd.h> /* close */
#include <sys/types.h> /* ftruncate */
#include <sys/mman.h> /* shm_open */
#include <sys/stat.h> /* shm_open */
#include <fcntl.h> /* shm_open */
#include <signal.h>
#include <pthread.h>
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };

static const char shmName[] = "robustFutexTest";
static const size_t SHM_SIZE = 4*1024;

static void myPerror(const char *msg, int rc)
{
    perror(msg);
    exit(rc);
}

struct ShmData {
    int lastOp;
    pthread_mutex_t mtx;
    char marker[4];
};

int main(int argc, char *argv[])
{
    int shmFd, rc;
    void * p;
    volatile struct ShmData * shmData;

    shmFd = shm_open(shmName, O_RDWR | O_CREAT, 0644);
    if (RC_NG == shmFd)
        myPerror("shm_open(3)", 30);

    if (RC_OK != ftruncate(shmFd, SHM_SIZE))
        myPerror("ftruncate(2)", 33);

    p = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    shmData = (volatile struct ShmData *)p;
    memcpy(shmData->marker, "0000", 4);

    if (1 >= argc || 0 == strcmp(argv[1], "0")) {
        printf("Operation %s\n", argv[1]);
        shmData->lastOp == 0;
        __asm__ __volatile__("mfence" : : : "memory");

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
        if (RC_OK != (rc = pthread_mutex_init(&shmData->mtx, &attr))) {
            printf("%d: rc==%d\n", __LINE__, rc);
            errno = rc;
            myPerror("pthread_mutex_init(3)", 36);
        }
        __asm__ __volatile__("mfence" : : : "memory");
    } else if (0 == strcmp(argv[1], "1") ){
        printf("Operation %s\n", argv[1]);
        shmData->lastOp == 1;
        __asm__ __volatile__("mfence" : : : "memory");

        if (RC_OK != (rc = pthread_mutex_lock(&shmData->mtx))) {
            printf("%d: rc==%d\n", __LINE__, rc);
            errno = rc;
            myPerror("pthread_mutex_init(3)", 36);
        }
        __asm__ __volatile__("mfence" : : : "memory");
        printf("Lock OK\n");

        printf("Hit Enter: ");
        getchar();

        raise(SIGSEGV);
    } else if (0 == strcmp(argv[1], "2") ){
        printf("Operation %s\n", argv[1]);
        shmData->lastOp == 2;
        __asm__ __volatile__("mfence" : : : "memory");

        if (RC_OK != (rc = pthread_mutex_trylock(&shmData->mtx))) {
            printf("%d: rc==%d\n", __LINE__, rc);
            errno = rc;
            myPerror("pthread_mutex_init(3)", 36);
        }
        __asm__ __volatile__("mfence" : : : "memory");
    } else {
        printf("Unknown operation: [%s]\n", argv[1]);
    }

    return 0;
}
