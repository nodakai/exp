#include <spu_intrinsics.h>
#include "vect.h"

void test_simd_pow4(vector float * input_buffer, vector float * output_buffer)
{
    vector float temp0;
    vector float temp1;
    vector float temp2;

    for (int i = 0; i < 256; ++i)
    {
        temp0 = input_buffer[i];
        temp1 = spu_mul(temp0, temp0);
        temp2 = spu_mul(temp1, temp1);
        output_buffer[i] = temp2;
    }
}

void test_simd_pow4_1(vector float * input_buffer, vector float * output_buffer)
{
    const int m(8);
    typedef Vect<vec_float4,m> Float;

    for (int i = 0; i+m-1 < 256; i+=m)
    {
        Float temp0 = input_buffer[i];
        Float temp1 = temp0 * temp0;
        output_buffer[i] ^= temp1 * temp1;
    }
}

void test_simd_pow4_1_1(vector float * input_buffer, vector float * output_buffer)
{
    const int m(8);
    typedef Vect<vec_float4,m> Float;

    for (int i = 0; i+m-1 < 256; i+=m)
    {
        Float temp0 = input_buffer[i];
        output_buffer[i] ^= (temp0 * temp0) * (temp0 * temp0);
    }
}

void test_simd_pow4_2(vector float * input_buffer, vector float * output_buffer)
{
    const int m(8);
    typedef Vect<vec_float4,m> Float;

    int i = 0;
    Float temp0 = input_buffer[i];
    Float temp1 = temp0 * temp0;
    Float temp2 = temp1 * temp1;
    i += m;
    temp0 = input_buffer[i];
    for (; i+m-1 +m < 256; i+=m)
    {
        output_buffer[i - m] ^= temp2;
        temp1 = temp0 * temp0;
        temp2 = temp1 * temp1;
        temp0 = input_buffer[i];
    }
}

#include <stdio.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>

#define MAX_BUFSIZE (64)

vec_float4 in_spe[MAX_BUFSIZE]  __attribute__((aligned(16)));
vec_float4 out_spe[MAX_BUFSIZE] __attribute__((aligned(16)));

typedef struct {
    unsigned long long  ea_in;
    unsigned long long  ea_out;
    unsigned int        size;
    int                 pad[3];
} abs_params_t;

abs_params_t abs_params __attribute__((aligned(16)));

int main(unsigned long long spe, unsigned long long argp, unsigned long long envp)
{
    int i;
    int tag = 1;

    /* DMA Transfer 1 : GET input/output parameters */
    spu_mfcdma64(&abs_params, mfc_ea2h(argp), mfc_ea2l(argp),
                 sizeof(abs_params_t), tag, MFC_GET_CMD);
    spu_writech(MFC_WrTagMask, 1 << tag);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);

    /* DMA Transfer 2 : GET input data */
    spu_mfcdma64(in_spe, mfc_ea2h(abs_params.ea_in), mfc_ea2l(abs_params.ea_in),
                 abs_params.size * sizeof(float), tag, MFC_GET_CMD);
    spu_writech(MFC_WrTagMask, 1 << tag);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);

    /* Calculate absolute values */
    test_simd_pow4(in_spe, out_spe);

    /* DMA Transfer 3 : PUT output data */
    spu_mfcdma64(out_spe, mfc_ea2h(abs_params.ea_out), mfc_ea2l(abs_params.ea_out),
                 abs_params.size * sizeof(float), tag, MFC_PUT_CMD);
    spu_writech(MFC_WrTagMask, 1 << tag);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);

    return 0;
}

