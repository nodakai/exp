#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static void myPerror(const char *msg, int rc)
{
    perror(msg);
    exit(rc);
}

static long myAtoi(const char *str)
{
    char *end;
    long ret;
    if ('\0' == *str) {
        fprintf(stderr, "myAtoi: empty string");
        exit(30);
    }
    errno = 0;
    ret = strtol(str, &end, 10);
    if (errno)
        myPerror("strtol", 31);
    if ('\0' != *end) {
        fprintf(stderr, "myAtoi: erroneous input \"%s\"", str);
        exit(32);
    }
    return ret;
}

int main(int argc, char *argv[])
{
    char *p;
    long len = 10L * 1024 * 1024 * 1024;
    if (1 < argc)
        len = myAtoi(argv[1]);
    p = (char *)calloc(len, 1);
    if (NULL == p) {
        fprintf(stderr, "calloc(3) failed.\n");
        return 1;
    }
    printf("Hit Enter to exit...");
    getchar();
    free(p);
    return 0;
}
