#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFLEN 1024

int main(int argc, char *argv[])
{
    printf("DO NOT FORGET TO USE ipcrm -m <id>\n");
    int id;
    if (argc == 1) {
        id = shmget(IPC_PRIVATE, BUFLEN, IPC_CREAT|0666);
        printf("Initialized ID is %d\n", id);
    }
    else {
        sscanf(argv[1], "%d", &id);
        printf("Given ID is %d\n", id);
    }
    char *shmptr = shmat(id, 0, 0);
    memset(shmptr, 0, BUFLEN);
    char buf[BUFLEN], line[BUFLEN];
    for (;;) {
        fgets(line, BUFLEN, stdin);
        sscanf(line, "%s", buf);
        if (strcmp(buf, "quit") == 0) {
            shmdt(shmptr);
            return 0;
        }
        printf("before:[%s]\n", shmptr);
        strncpy(shmptr, buf, BUFLEN);
        printf("after: [%s]\n", shmptr);
    }
    return 0;
}
