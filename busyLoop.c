#include <stdint.h>
#include <limits.h>

static void busyKernel() {
    uint64_t i, sum = 0;
    for (i = 0; i < UINT64_MAX; ++i)
        sum += i * i;
}

int main(int argc, char *argv[]) {
    busyKernel();
    return 123 == sum;
}
