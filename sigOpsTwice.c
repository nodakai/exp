#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

void myPerror(const char *msg)
{
    perror(msg);
    exit(10);
}

int main(void)
{
    sigset_t ss;
    if (sigemptyset(&ss))
        myPerror("sigemptyset(3)");
    if (sigaddset(&ss, SIGINT))
        myPerror("sigaddset(3)");
    if (sigaddset(&ss, SIGINT))
        myPerror("sigaddset(3)");
    if (sigdelset(&ss, SIGINT))
        myPerror("sigdelset(3)");
    if (sigdelset(&ss, SIGINT))
        myPerror("sigdelset(3)");

    printf("All API calls were OK.\n");
    return 0;
}
