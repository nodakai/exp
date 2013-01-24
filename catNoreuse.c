#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h> /* open */
#include <sys/stat.h> /* open */
#include <unistd.h> /* read */
#include <fcntl.h> /* posix_fadvise */

enum { RC_OK = 0, RC_NG = -1 };
static const ssize_t BUFLEN = (ssize_t)2 * 1024 * 1024 * 1024;

int main(int argc, char *argv[])
{
    ssize_t nRead, nWrite;
    void *pBuf;
    int fd, rc;
    if (1 >= argc) {
        fprintf(stderr, "USAGE: catNoreuse filepath\n");
        exit(8);
    }

    fd = open(argv[1], O_DIRECT | O_SYNC);
    if (0 > fd) {
        perror("open(2)");
        exit(10);
    }

    rc = posix_memalign(&pBuf, 512, BUFLEN);
    /* pBuf = (uint8_t *)malloc(BUFLEN); */
    /* fprintf(stderr, "pBuf == %p\n", pBuf); */
    if (rc) {
        fprintf(stderr, "malloc(%lu) failed.\n", BUFLEN);
        exit(11);
    }

    nRead = read(fd, pBuf, BUFLEN);
    if (RC_NG == nRead) {
        perror("read(2)");
        exit(13);
    } else if (BUFLEN == nRead) {
        fprintf(stderr, "The file was too large to fit in a buffer.");
        exit(15);
    }

    nWrite = write(STDOUT_FILENO, pBuf, nRead);
    if (nWrite != nRead) {
        fprintf(stderr, "nWrite==%ld vs %ld==nRead\n", nWrite, nRead);
    }

    free(pBuf);

    /*
    if (RC_OK != posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED)) {
        perror("posix_fadvise(3)");
        exit(13);
    }
    */

    /* fprintf(stderr, "Quitting...\n"); */
    return 0;
}
