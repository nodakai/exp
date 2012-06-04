#include <stdio.h>
#include <stdlib.h>

#ifdef GETOPT_LONG
#define _GNU_SOURCE
#include <unistd.h>

void testMain(int argc, char *argv[])
{
}
#else /* default, plain getopt(2) */
#include <getopt.h>

void testMain(int argc, char *argv[], const char *optspec)
{
    char optchr;
    int i;

    printf("optspec==\"%s\"\n", optspec);
    for (i = 0; i < argc; ++i) {
        printf("argv[%2d]: \"%s\"\n", i, argv[i]);
    }

    optind=1;
    while (-1 != (optchr = getopt(argc, argv, optspec))) {
        printf("optchr=='%c'; optarg==\"%s\"\n", optchr, optarg);
    }
    for (i = optind; i < argc; ++i) {
        printf("non-option args[%2d]==\"%s\"\n", i, argv[i]);
    }
}
#endif

#define ARGC(arr) (sizeof(arr) / sizeof(arr[0]) - 1)

int main(void)
{
    {
        char *myArgv[] = { "testMain", "-a", "-b", "c", "d", NULL };
        const int myArgc = ARGC(myArgv);
        testMain(myArgc, myArgv, "ab");
    }
    putchar('\n');
    {
        char *myArgv[] = { "testMain", "-a", "c", "d", "-b", NULL };
        const int myArgc = ARGC(myArgv);
        testMain(myArgc, myArgv, "ab");
    }
    putchar('\n');
    {
        char *myArgv[] = { "testMain", "-a", "-b", "c", "d", NULL };
        const int myArgc = ARGC(myArgv);
        testMain(myArgc, myArgv, "ab:");
    }
    putchar('\n');
    {
        char *myArgv[] = { "testMain", "-a", "-b", "c", "d", NULL };
        const int myArgc = ARGC(myArgv);
        testMain(myArgc, myArgv, "a:b:");
    }
    putchar('\n');
    {
        char *myArgv[] = { "testMain", "-a", "-b", "c", "d", NULL };
        const int myArgc = ARGC(myArgv);
        testMain(myArgc, myArgv, "av");
    }

    return 0;
}
