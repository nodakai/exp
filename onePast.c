#include <stdlib.h>
#include <string.h>

void main(void)
{
    char *x = (char *)malloc(10);
    memset(x, 0, 10);
    char c = x[10];
    x[10] = 'A';
}
