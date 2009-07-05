#include <spu_intrinsics.h>
#include <iostream>
#include "vect.h"

using namespace std;

typedef vec_float4 * const __restrict__ fp_t;
void comp_kernel(fp_t dst, const fp_t src, int n)
{
    const int m(6);
    typedef Vect<vec_float4,m> Float;
    int i=0;
    for (; i+m-1<n; i+=m) {
        // s_0=v_src[i+0]; s_1=v_src[i+1]; ...
        Float s = src[i];
        // d_0=2*s_0*s_0-100; d_1=2*s_1*s_1-100; ...
        // Float d = 2.f*s*s - 100.f;
        Float d = spu_splats(2.f)*s*s + spu_splats(1.f);
        // v_dst[i+0]=d_0; v_dst[i+1]=d_1=; ...
        dst[i] ^= d;
    }
    for (; i<n; ++i) {
        vec_float4 s = src[i];
        vec_float4 d = spu_splats(2.f)*s*s + spu_splats(1.f);
        dst[i] = d;
    }
}

void comp_kernel0(fp_t dst, const fp_t src, int n)
{
    const int m(6);
    typedef Vect<vec_float4,m> Float;
    int i=0;
    for (; i+m-1<n; i+=m) {
        int i_0 = i, i_1 = i+1, i_2 = i+2, i_3 = i+3, i_4 = i+4, i_5 = i+5;
        vec_float4 s_0 = src[i_0];
        vec_float4 s_1 = src[i_1];
        vec_float4 s_2 = src[i_2];
        vec_float4 s_3 = src[i_3];
        vec_float4 s_4 = src[i_4];
        vec_float4 s_5 = src[i_5];
        // d_0=2*s_0*s_0-100; d_1=2*s_1*s_1-100; ...
        // Float d = 2.f*s*s - 100.f;
        vec_float4 d_0 = spu_splats(2.f)*s_0*s_0 + spu_splats(1.f);
        vec_float4 d_1 = spu_splats(2.f)*s_1*s_1 + spu_splats(1.f);
        vec_float4 d_2 = spu_splats(2.f)*s_2*s_2 + spu_splats(1.f);
        vec_float4 d_3 = spu_splats(2.f)*s_3*s_3 + spu_splats(1.f);
        vec_float4 d_4 = spu_splats(2.f)*s_4*s_4 + spu_splats(1.f);
        vec_float4 d_5 = spu_splats(2.f)*s_5*s_5 + spu_splats(1.f);
        // v_dst[i+0]=d_0; v_dst[i+1]=d_1=; ...
        dst[i_0] = d_0;
        dst[i_1] = d_1;
        dst[i_2] = d_2;
        dst[i_3] = d_3;
        dst[i_4] = d_4;
        dst[i_5] = d_5;
    }
    for (; i<n; ++i) {
        vec_float4 s = src[i];
        vec_float4 d = spu_splats(2.f)*s*s + spu_splats(1.f);
        dst[i] = d;
    }
}

const int N(1024);

int main(void)
{
    float a[N], b[N];
    for (int i=0; i<N; ++i) {
        a[i] = static_cast<float>(i);
    }
    comp_kernel(reinterpret_cast<vec_float4 *>(b), reinterpret_cast<vec_float4 *>(a), N / 4);
    for (int i=0; i<N; ++i) {
        if (i>0) {
            cout << ", " ;
        }
        cout << b[i];
    }
    cout << endl;
    return 0;
}
