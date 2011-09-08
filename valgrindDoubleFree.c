#include <stdlib.h>

void main(void)
{
    char *p = (char *)malloc(10);
    free(p);
    free(p);
}
