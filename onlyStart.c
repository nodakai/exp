/*
void exit(int x)
{
    __asm__ __volatile__ (
        "movl $1, %%eax\n\t"
        "movl %0, %%ebx\n\t"
        "int $0x80"
        :
        : "r"(x)
        : "%eax", "%ebx"
    );
}
*/

int _start(void)
{
    /* exit(42); */
    __asm__ __volatile__ (
        "movl $1, %%eax\n\t"
        "movl $42, %%ebx\n\t"
        "int $0x80"
        :
        :
        : "%eax", "%ebx"
    );
}
