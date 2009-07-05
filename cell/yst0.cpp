#include <stdio.h>
#include <string.h>
#include <spu_intrinsics.h>


#define DECL(N) vec_float4 o_ ## N ;
#define LOAD_EXP(N)  o_ ## N = spu_madd(spu_splats(2.f), spu_mul(in[ N ], in[ N ]), spu_splats(1.f))
#define STORE(N)  out[ N ] = o_ ## N ;


#define UNROLL(X) \
    X(0);  X(1);  X(2);  X(3); \
    X(4);  X(5);  X(6);  X(7); \
    X(8);  X(9); X(10); X(11); \
   X(12); X(13); X(14); X(15); \


void func(vec_float4 * in, vec_float4 * out)
{
    int i;

    for (i = 0; i < 256; i += 16)
    {
        UNROLL(DECL)
        UNROLL(LOAD_EXP)
        UNROLL(STORE)
    }
}

template <typename S, unsigned N>
class Vect;

template <typename S, unsigned N>
void operator^=(S &f, const Vect<S,N> &vf)
{
    f = vf.f_;
    *(&f+1) ^= vf.r_;
}

template <typename S>
void operator^=(S &f, const Vect<S,1> &vf)
{
    f = vf.f_;
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
const Vect<S,1> operator*(const S &f, const Vect<S,1> &vf)
{
    S f0(f * vf.f_);
    Vect<S,1> ret(f0);
    return ret;
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
const Vect<S,1> operator+(const S &f, const Vect<S,1> &vf)
{
    S f0(f + vf.f_);
    Vect<S,1> ret(f0);
    return ret;
}

template<typename S, unsigned N>
class Vect
{
public:
    Vect (const S &f)
        : f_(f), r_(*(&f+1)) { }
    Vect (const S &f, const Vect<S,N-1> &r)
        : f_(f), r_(r) { }
    Vect (const Vect<S,N> &o)
        : f_(o.f_), r_(o.r_) { }
    Vect<S,N> & operator=(const Vect<S,N> &o) { f_ = o.f_; r_ = o.r_; };
    friend void operator^=<>(S &, const Vect<S,N> &);
    friend const Vect<S,N> operator*<>(const S &, const Vect<S,N> &);
    const Vect<S,N> operator*(const Vect<S,N> &) const;
    const Vect<S,N> operator*(const S &) const;
    friend const Vect<S,N> operator+<>(const S &, const Vect<S,N> &);
    const Vect<S,N> operator+(const Vect<S,N> &) const;
    const Vect<S,N> operator+(const S &) const;
protected:
    const S f_;
    Vect<S, N-1> r_;
};

template<typename S>
class Vect<S,1>
{
public:
    Vect (const S &f)
        : f_(f) { };
    Vect (const Vect<S,1> &o)
        : f_(o.f_) { };
    void operator=(const Vect<S,1> &o) { f_ = o.f_; };
    friend void operator^=<>(S &, const Vect<S,1> &);
    friend const Vect<S,1> operator*<>(const S &, const Vect<S,1> &);
    const Vect<S,1> operator*(const Vect<S,1> &) const;
    const Vect<S,1> operator*(const S &) const;
    friend const Vect<S,1> operator+<>(const S &, const Vect<S,1> &);
    const Vect<S,1> operator+(const Vect<S,1> &) const;
    const Vect<S,1> operator+(const S &) const;
protected:
    const S f_;
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
const Vect<S,1> Vect<S,1>::operator*(const Vect<S,1> &o) const
{
    S f0(f_ * o.f_);
    Vect<S,1> ret(f0);
    return ret;
}

template <typename S, unsigned N>
const Vect<S,N> Vect<S,N>::operator*(const S &f) const
{
    S f0(f_ * f);
    Vect<S,N-1> r0(r_ * f);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,1> Vect<S,1>::operator*(const S &f) const
{
    S f0(f_ * f);
    Vect<S,1> ret(f0);
    return ret;
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
const Vect<S,1> Vect<S,1>::operator+(const Vect<S,1> &o) const
{
    S f0(f_ + o.f_);
    Vect<S,1> ret(f0);
    return ret;
}

template <typename S, unsigned N>
const Vect<S,N> Vect<S,N>::operator+(const S &f) const
{
    S f0(f_ + f);
    Vect<S,N-1> r0(r_ + f);
    Vect<S,N> ret(f0, r0);
    return ret;
}

template <typename S>
const Vect<S,1> Vect<S,1>::operator+(const S &f) const
{
    S f0(f_ + f);
    Vect<S,1> ret(f0);
    return ret;
}

void func1(vec_float4 * in, vec_float4 * out)
{
    int i;

    const int m(16);
    typedef Vect<vec_float4,m> Float;
    for (i = 0; i+m-1 < 256; i += m)
    {
        Float in_f = in[i];
        Float out_f = spu_splats(2.f) * in_f * in_f + spu_splats(1.f);
        out[i] ^= out_f;
    }
}
