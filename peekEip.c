#include <stdio.h>

#define EIP_PEEK(reg)           \
    __asm__ (                   \
        "call 1f\n\t"        \
        "1: pop %0"        \
        : "=r"(reg)             \
    )

int foo(int x)
{
    void *p1;
    EIP_PEEK(p1);
    printf("%p\n", p1);
}

int main(void)
{
    void *p0, *p2;
    EIP_PEEK(p0);
    printf("%p\n", p0);
    EIP_PEEK(p2);
    printf("%p\n", p2);
    return 0;
}
