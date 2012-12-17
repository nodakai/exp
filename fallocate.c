#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h> /* strtol */
#include <string.h>
#include <errno.h>

#include <fcntl.h> /* posix_fallocate */
#include <linux/falloc.h> /* fallocate */
#include <getopt.h>

enum { RC_OK = 0, RC_NG = -1 };

#define BUFLEN 2048

static void printUsage()
{
    printf("fallocate (-p|-l|-L) [-h] path [len [off]]");
    exit(10);
}

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

enum EnumMode {
    EnumModePosix = 'p',
    EnumModeLinux = 'l',
    EnumModeLinuxWithFlag = 'L'
};

int main(int argc, char *argv[])
{
    int i;
    FILE *fp;
    int fd;
    char *path = NULL;
    size_t off = 0, len = 0;
    char optchr;
    enum EnumMode mode = EnumModePosix;

    while (-1 != (optchr = getopt(argc, argv, "plLh"))) {
        switch (optchr) {
            case EnumModePosix:
            case EnumModeLinux:
            case EnumModeLinuxWithFlag:
                mode = optchr;
                break;

            case 'h':
            default:
                printUsage();
        }
    }

    for (i = optind; i < argc; ++i) {
        if (NULL == path)
            path = argv[i];
        else if ( ! len)
            len = myAtoi(argv[i]);
        else if ( ! off)
            off = myAtoi(argv[i]);
    }

    if (NULL == path)
        printUsage();

    if (0 == len)
        len = (size_t)100 * 1024 * 1024;

    fp = fopen(path, "w");
    if (NULL == fp)
        myPerror("fopen", 42);
    fd = fileno(fp);

    if (EnumModePosix == mode) {
        const int rc = posix_fallocate(fd, off, len);
        if (RC_OK != rc) {
            char buf[BUFLEN];
            fprintf(stderr, "posix_fallocate: %s", strerror_r(rc, buf, sizeof buf));
            exit(40);
        }
    } else {
        const int flag = (EnumModeLinuxWithFlag == mode ? FALLOC_FL_KEEP_SIZE : 0);
        if (RC_NG == fallocate(fd, flag, off, len))
            myPerror("fallocate", 44);
    }

    fclose(fp);

    return 0;
}
