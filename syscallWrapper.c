#define _GNU_SOURCE 1
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

int main(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    syscall(SYS_gettimeofday, &tv, NULL);
    return 0;
}
