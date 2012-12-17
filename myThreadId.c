#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h> /* get*id */
#include <unistd.h> /* get*id */
#include <sys/syscall.h> /* gettid */

typedef unsigned long UL;

static pid_t gettid() {
    return syscall(SYS_gettid);
}

int main(void) {
    pid_t pid, tid;
    pthread_t pthId;

    pid = getpid();
    tid = gettid();
    pthId = pthread_self();

    printf("pid == %lu, tid == %lu, pthread ID == %lu\n", (UL)pid, (UL)tid, (UL)pthId);

    printf("Hit Enter to quit...");
    getchar();

    return 0;
}
