#include <cstdio>
#include <cstring>

#define NTOHL1(mcl)               \
  (( (mcl)        & 0xffU) << 24) | \
  ((((mcl) >>  8) & 0xffU) << 16) | \
  ((((mcl) >> 16) & 0xffU) <<  8) | \
   (((mcl) >> 24) & 0xffU)

#define NTOHL(mcl)                \
  (((mcl) << 24) & (0xffU << 24)) | \
  (((mcl) <<  8) & (0xffU << 16)) | \
  (((mcl) >>  8) & (0xffU <<  8)) | \
  (((mcl) >> 24) &  0xffU)

int main()
{
    const char *str = "helo";
    const int x = *reinterpret_cast<const int*>(str);
    const int y = 'helo';
    const int z = NTOHL('helo');

    printf("%08x\n%08x\n%08x\n", x, y, z);

    printf("%s\n", 0 == memcmp(str, &z, sizeof z) ? "equal" : "not equal");
}
