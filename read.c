#include <stdio.h>
#include <stdlib.h> /* exit */
#define __USE_GNU /* O_DIRECT */
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* open, read */
#include <stdint.h> /* uintptr_t */

#define BUFLEN (1024 * 1024)
#define ALIGN ((uintptr_t) 4 * 1024)

int main(int argc, char *argv[])
{
    int ofd, cnt = 0;
    ssize_t nbytes;
    size_t tot = 0;
    char buf_raw[BUFLEN + ALIGN], *buf;

    buf = (char*)(((uintptr_t)buf_raw + (ALIGN - 1)) & ~(ALIGN - 1));

    ofd = open(argv[1], O_RDONLY | O_DIRECT | O_SYNC );
    if (ofd < 0) {
        perror("open");
        exit(1);
    }

    for (;;) {
        nbytes = read(ofd, buf, BUFLEN);
        if (nbytes < 0) {
            perror("read");
            close(ofd);
            exit(10);
        }

        tot += nbytes;
        if (++cnt % 100 == 0 || nbytes != BUFLEN) {
            printf("%llu bytes (%.0f MB == %.2f GB) read.\n", tot, tot / 1024.0 / 1000, tot / 1024.0 / 1000 / 1000);
        }
        if (nbytes != BUFLEN) break;
    }

    close(ofd);
    return 0;
}
