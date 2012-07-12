#include <stdlib.h>

struct Foo {
    char arr[1024];
};

int main(void)
{
    struct Foo foo;
    foo.arr = malloc(1024);
    free(foo.arr);
    return 0;
}
