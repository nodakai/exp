#include <stdio.h>

union {
    struct {
        float f;
        int i;
    };
    unsigned char dump[8];
} u0;

int main()
{
    int i;

    u0.f = 0.1;
    u0.i = 10;

    for (i=0; i<8; ++i)
        printf("%02x ", u0.dump[i]);
    printf("\n");

    return 0;
}
