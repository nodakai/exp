#include <stdio.h>
#include <stdlib.h> /* exit */
#define __USE_GNU /* O_DIRECT */
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* open, read */
#include <stdint.h> /* uintptr_t */

#define ALIGN ((uintptr_t) 4 * 1024)

int main(int argc, char *argv[])
{
    int ofd, cnt = 0;
    ssize_t nbytes;
    size_t tot = 0, ptot = 0, buflen = 1024 * 1024;
    char *buf_raw, *buf;

    if (argc < 2) {
        fprintf(stderr, "USAGE: directRead file [buflen]\n");
        exit(30);
    }

    if (argc > 2) {
        char *p;
        buflen = strtol(argv[2], &p, 10);
        if (*p != '\0') {
            perror("strtol(argv[2])");
            exit(20);
        }
        if (buflen > SIZE_MAX / 4) {
            fprintf(stderr, "Invalid buflen == %llu (== %lld ?)\n", buflen, buflen);
            exit(40);
        }
        printf("buflen == %llu bytes (%.2f KB == %.2f MB)\n", buflen, buflen / 1024.0, buflen / 1024e3);
    }
    buf_raw = (char*)malloc(buflen);
    buf = (char*)(((uintptr_t)buf_raw + (ALIGN - 1)) & ~(ALIGN - 1));

    ofd = open(argv[1], O_RDONLY | O_DIRECT | O_SYNC );
    if (ofd < 0) {
        perror("open");
        exit(1);
    }

    for (;;) {
        nbytes = read(ofd, buf, buflen);
        if (nbytes < 0) {
            perror("read");
            close(ofd);
            exit(10);
        }

        tot += nbytes;
        if (tot - ptot >= 100 * 1024 * 1000 || nbytes != buflen) {
            printf("%llu bytes (%.0f MB == %.2f GB) read.\n", tot, tot / 1024e3, tot / 1024e6);
            ptot = tot;
        }
        if (nbytes != buflen) break;
    }

    close(ofd);
    return 0;
}
