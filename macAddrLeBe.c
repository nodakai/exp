#include <stdio.h>
#include <stdint.h>


union U {
    uint64_t m_l[2];
    uint32_t m_i[4];
    uint8_t m_c[12];
};

int main(void)
{
    int i;
    union U uni;

    uni.m_l[0] = uni.m_l[1] = -1;

    uni.m_c[ 0] = 0x12;
    uni.m_c[ 1] = 0x34;
    uni.m_c[ 2] = 0x56;
    uni.m_c[ 3] = 0x78;
    uni.m_c[ 4] = 0x9a;
    uni.m_c[ 5] = 0xbc;

    uni.m_c[ 6] = 0xd1;
    uni.m_c[ 7] = 0xe2;
    uni.m_c[ 8] = 0xf3;
    uni.m_c[ 9] = 0x04;
    uni.m_c[10] = 0x15;
    uni.m_c[11] = 0x26;

    printf("%08x%08x %08x%08x\n", uni.m_i[0], uni.m_i[1], uni.m_i[2], uni.m_i[3]);
    const uint64_t mask = (1ULL << 48) - 1;
    printf("%016llx %016llx %016llx\n%016llx %016llx %016llx\n", uni.m_l[0], uni.m_l[1], ((uni.m_l[0] & ~mask) >> 48) | (uni.m_l[1] << 16), mask, mask, mask);
    for (i = 0; i < 16; ++i) {
        printf("%02x", (uint8_t)uni.m_c[i]);
        if (0 == (i+1) % 8)
            printf(" ");
    }
    printf("\n");

    return 0;
}
