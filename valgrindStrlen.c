#include <stdlib.h>
#include <string.h>

void main(void)
{
    int len;
    char *s = (char *)malloc(10);
    s[0] = '\0';
    len = strlen(s);
    free(s);
}
