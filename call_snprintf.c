#include <stdio.h>
#include <limits.h>
#include <string.h>

#define _BSD_VA_LIST_ void*

int
nk_vsnprintf_ss(char *sbuf, size_t slen, const char *fmt0, _BSD_VA_LIST_ ap);

int main(void)
{
    char buf[1024];

    int x = -10;
    unsigned long long ull = INT_MAX + 1ULL;
    double d = 10;

    snprintf(buf, 1024, "x=%d, ull=%llu, d=%f", x, ull, d);
    puts(buf);
    memset(buf, 0, sizeof(buf));

    char args[1024];
    int off = 0;
    memcpy(args + off, &x, sizeof(x)); off += sizeof(x);
    memcpy(args + off, &ull, sizeof(ull)); off += sizeof(ull);
    memcpy(args + off, &d, sizeof(d)); off += sizeof(d);
    nk_vsnprintf_ss(buf, 1024, "x=%d, ull=%llu, d=%f", args);
    puts(buf);

    return 0;
}
