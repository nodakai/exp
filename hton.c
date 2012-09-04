#include <arpa/inet.h>

int fl(int x)
{
    return htonl(x);
}

int fs(int x)
{
    return htons(x);
}

int flc(int x)
{
    return htonl(0x12345678);
}

int fsc(int x)
{
    return htons(0x9876);
}
