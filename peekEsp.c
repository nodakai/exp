#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PEEK_ESP(reg)          \
    __asm__ __volatile__ (      \
        "movq %%rsp, %0"        \
        : "=r"(reg)             \
    )

int g0;

int main(void)
{
    void *p0;
    PEEK_ESP(p0);
    {
        char buf[4 * 1024];
        void *p1;
        PEEK_ESP(p1);
        {
            int y = 0;
            void *p2;
            PEEK_ESP(p2);
            g0 = ((char*)p2 - (char*)&y);
            printf("%p\n%p\n%p\n", p0, p1, p2);
        }
        void *p3;
        PEEK_ESP(p3);
        printf("%p\n", p3);
    }
    return 0;
}
