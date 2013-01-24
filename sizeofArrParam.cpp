#include <stdio.h>

#define BUFLEN 2048

static void foo(char arr[BUFLEN]) {
    printf("sizeof arr == %ld\n", sizeof(arr));
}

static void bar(char (&arr)[BUFLEN]) {
    printf("sizeof arr == %ld\n", sizeof(arr));
}

int main(int argc, char *argv[]) {
    char arr[BUFLEN];
    char arr1[BUFLEN + 1];
    foo(arr);
    foo(arr1);
    bar(arr);
//  bar(arr1);
}
