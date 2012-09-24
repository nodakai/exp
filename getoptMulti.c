#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
    int i, k;
    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "abc:d:"))) {
        switch (optchr) {
            case 'a':
                printf("Got '%c'\n", optchr);
                break;
            case 'b':
                printf("Got '%c'\n", optchr);
                break;
            case 'c':
                printf("Got '%c' => [%s]\n", optchr, optarg);
                break;
            case 'd':
                printf("Got '%c' => [%s]\n", optchr, optarg);
                break;

            default:
                exit(10);
        }
    }

    for (i = optind, k = 0; i < argc; ++i, ++k) {
        printf("%02d-th pos-arg: [%s]\n", k, argv[i]);
    }

    printf("Done.\n");

    return 0;
}
