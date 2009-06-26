#include <spu_intrinsics.h>
#include <iostream>

using namespace std;

template <typename S, unsigned N>
class Vect;

template <typename S, unsigned N>
const Vect<S,N> & operator^=(S &f, const Vect<S,N> &vf)
{
    f = vf.f_;
    *(&f+1) ^= vf.r_;
    return vf;
}

template <typename S>
const Vect<S,0> & operator^=(S &f, const Vect<S,0> &vf)
{
    return vf;
}

template <typename S, unsigned N>
const Vect<S,N> operator*(const S &f, const Vect<S,N> &vf)
{
    S f0(f * vf.f_);
    Vect<S,N-1> r0(f * vf.r_);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,0> operator*(const S &f, const Vect<S,0> &vf)
{
    return vf;
}

template <typename S, unsigned N>
const Vect<S,N> operator+(const S &f, const Vect<S,N> &vf)
{
    S f0(f + vf.f_);
    Vect<S,N-1> r0(f + vf.r_);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,0> operator+(const S &f, const Vect<S,0> &vf)
{
    return vf;
}

template<typename S, unsigned N>
class Vect
{
public:
    Vect (const S &f)
        : f_(f), r_(*((&f)+1)) { }
    Vect (const S &f, const Vect<S,N-1> &r)
        : f_(f), r_(r) { }
    Vect (const Vect<S,N> &o)
        : f_(o.f_), r_(o.r_) { }
    Vect<S,N> & operator=(const Vect<S,N> &o) { f_ = o.f_; r_ = o.r_; };
    friend const Vect<S,N> & operator^=<>(S &, const Vect<S,N> &);
    friend const Vect<S,N> operator*<>(const S &, const Vect<S,N> &);
    const Vect<S,N> operator*(const Vect<S,N> &) const;
    friend const Vect<S,N> operator+<>(const S &, const Vect<S,N> &);
    const Vect<S,N> operator+(const Vect<S,N> &) const;
protected:
    const S f_;
    Vect<S, N-1> r_;
};

template<typename S>
class Vect<S,0>
{
public:
    Vect (const S &) { };
    Vect (const Vect<S,0> &) { };
    Vect<S,0> & operator=(const Vect<S,0> &) { };
    const Vect<S,0> operator*(const Vect<S,0> &) const;
    const Vect<S,0> operator+(const Vect<S,0> &) const;
};

template <typename S, unsigned N>
const Vect<S,N> Vect<S,N>::operator*(const Vect<S,N> &o) const
{
    S f0(f_ * o.f_);
    Vect<S,N-1> r0(r_ * o.r_);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,0> Vect<S,0>::operator*(const Vect<S,0> &o) const
{
    return o;
}

template <typename S, unsigned N>
const Vect<S,N> Vect<S,N>::operator+(const Vect<S,N> &o) const
{
    S f0(f_ + o.f_);
    Vect<S,N-1> r0(r_ + o.r_);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,0> Vect<S,0>::operator+(const Vect<S,0> &o) const
{
    return o;
}

typedef vec_float4 * const __restrict__ fp_t;
void comp_kernel(fp_t dst, const fp_t src, int n)
{
    const int m(6);
    typedef Vect<vec_float4,m> Float;
    int i;
    for (i=0; i+m-1<n; i+=m) {
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

#if 0
const int N(20);

int main(void)
{
    float a[N], b[N];
    for (int i=0; i<N; ++i) {
        a[i] = static_cast<float>(i);
    }
    comp_kernel(b, a, N);
    for (int i=0; i<N; ++i) {
        if (i>0) {
            cout << ", " ;
        }
        cout << b[i];
    }
    cout << endl;
    return 0;
}
#endif
