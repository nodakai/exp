#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <getopt.h>

static void getAndShowAffinity(int simple)
{
    int i, n, nOn;
    cpu_set_t mask;
    short on[CPU_SETSIZE];

    if (0 > sched_getaffinity(0, sizeof mask, &mask)) {
        perror("sched_getaffinity");
        exit(20);
    }

    /* n = CPU_COUNT(&mask); */
    n = CPU_SETSIZE;
    for (i = 0, nOn = 0; i < n; ++i) {
        const int isSet = CPU_ISSET(i, &mask);

        if (simple)
            printf("[%4d:%c]%s", i, isSet ? 'O' : ' ', (0 == (i + 1) % 10) ? "\n" : "");
        else {
            if (isSet)
                on[nOn++] = i;
        }
    }

    if ( ! simple) {
        const char *intFmts[] = { "[%2d]", "[%3d]", "[%4d]" };
        const char *blankFmts[] = { "[  ]", "[   ]", "[    ]" };

        const int fmt = on[nOn-1] < 100 ? 0 : (on[nOn-1] < 100 ? 1 : 2);

        for (i = 0; i < nOn; ) {
            int cur = on[i];
            printf(intFmts[fmt], cur);
            if (++i < nOn) {
                while (++cur < on[i])
                    printf(blankFmts[fmt]);
            }
        }
    }
    printf("\n");
}

static void setAffinity(long long maskSpec)
{
    size_t i;
    cpu_set_t mask;

    printf("mask spec == 0x%08llx\n", maskSpec);

    CPU_ZERO(&mask);
    for (i = 0; i < 8 * sizeof maskSpec; ++i) {
        if (((unsigned long long)maskSpec >> i) & 0x1)
            CPU_SET(i, &mask);
    }

    if (0 > sched_setaffinity(0, sizeof mask, &mask)) {
        perror("sched_setaffinity");
        exit(30);
    }
}

enum EnumMode {
    EnumModeGet,
    EnumModeSet
};

int main(int argc, char *argv[])
{
    int simple = 0;
    enum EnumMode mode = EnumModeGet;
    long long maskSpec = -1;

    char optchr;
    while (-1 != (optchr = getopt(argc, argv, "gs:S"))) {
        switch (optchr) {
            case 'g': mode = EnumModeGet; break;
            case 's':
                mode = EnumModeSet;
                maskSpec = strtoll(optarg, NULL, (0 == strncasecmp(optarg, "0x", 2) ? 16 : 10));
                break;
            case 'S': simple = 1; break;
            default: exit(10);
        }
    }

    if (EnumModeSet == mode) {
        printf("Set mode\n");
        getAndShowAffinity(simple);
        setAffinity(maskSpec);
        getAndShowAffinity(simple);
    } else {
        printf("Get mode\n");
        getAndShowAffinity(simple);
    }
    return 0;
}
