#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv)
{
    int i;
    char *p = NULL, *q = NULL, *t;
    for (i=0; i<argc; ++i) {
        t = (char *)realloc(p, 0);
        p = (char *)realloc(q, 0);
        q = t;
        printf("p==<%p>; q==<%p>\n", p, q);
    }
    free(p);
    free(q);
}
