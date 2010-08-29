#include <cstdio>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

struct C0 { int _x; };

struct C1 { int _y; };

struct D : public C0, C1 { int _z; };

void cast_and_print(D *dp)
{
    C0 *cp0 = dp;
    C1 *cp1 = dp;

    printf("    dp == 0x%" PRIxPTR "\n", reinterpret_cast<uintptr_t>(dp));
    printf("(C0)dp == 0x%" PRIxPTR "\n", reinterpret_cast<uintptr_t>(cp0));
    printf("(C1)dp == 0x%" PRIxPTR "\n", reinterpret_cast<uintptr_t>(cp1));
}

int main(void)
{
    D *dp = new D;
    cast_and_print(dp);
    delete dp;

    cast_and_print(0);

    return 0;
}
