#include <stdio.h>

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; ++i) {
        int x = 123456;
        const int rc = sscanf(argv[i], "%d", &x);
        printf("\"%s\" => %d [%d]\n", argv[i], x, rc);
    }
    return 0;
}
