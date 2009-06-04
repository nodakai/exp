#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

typedef enum { FALSE, TRUE, } BOOL;
typedef enum { PLUS, MINUS, } sign;

typedef unsigned char Byte;
typedef unsigned int uint;
typedef union {
    float floatval;
    uint intval;
    Byte peek[4];
} floatlook;

#define SIGN(x)     ( ((x).peek[3] & 0x80) >> 7 )
#define EXPONENT(x) ( ((x).peek[3] & 0x7f) << 1  |  ((x).peek[2] & 0x80) >> 7 )
#define MANTISSA(x) ( ((x).peek[2] & 0x7f) << 16  |  ((x).peek[1] << 8) | (x).peek[0] )

#define SET_SIGN(x, v)     ( (x).peek[3] = ((x).peek[3] & 0x7f) | (v & 0x01) << 7 )
#define SET_EXPONENT(x, v) ( (x).peek[3] = ((x).peek[3] & 0x80) | (v & 0xfe) >> 1, \
                             (x).peek[2] = ((x).peek[2] & 0x7f) | (v & 0x01) << 7 )
#define SET_MANTISSA(x, v) ( (x).peek[2] = (Byte)(((x).peek[2] & 0x80) | (v & 0x7f0000) >> 16), \
                             (x).peek[1] = (Byte)((v & 0xff00) >> 8), \
                             (x).peek[0] = (Byte)(v & 0xff) )

void binprint_float(floatlook *ptr)     /*{{{*/
{
    int i;
    printf("0x%08x = ", *((int *)&(ptr->floatval)) );
    for (i = 31; i >= 0; --i) {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0) {
            putchar(' ');
        }
        else if (32 - i == 1 || 32 - i == 9) {
            printf("   ");
        }
    }

    printf(" = %88.47f\n", ptr->floatval);
}
/*}}}*/
void binprint_int(int theint)    /*{{{*/
{
    int i;
    Byte *peek = (Byte *)&theint;

    printf("%12d  =  ", theint);
    for (i = 31; i >= 0; --i) {
        printf("%d", (peek[i/8] >> (i%8)) & 0x01);

        if (i % 8 == 0) {
            putchar(' ');
        }
    }
    putchar('\n');
}       /*}}}*/
BOOL float_eq(floatlook *x, floatlook *y)/*{{{*/
{
    return x->peek[3] == y->peek[3] && x->peek[2] == y->peek[2] &&
        x->peek[1] == y->peek[1] && x->peek[0] == y->peek[0];
}
/*}}}*/
BOOL float_gt(floatlook *x, floatlook *y)/*{{{*/
{
    if (SIGN(*x) < SIGN(*y)) {
        return TRUE;
    }
    else if (SIGN(*x) > SIGN(*y)) {
        return FALSE;
    }
    else {
        BOOL abs_x_is_larger = SIGN(*x) == PLUS ? TRUE : FALSE;

        if (EXPONENT(*x) > EXPONENT(*y)) {
            return abs_x_is_larger;
        }
        else if (EXPONENT(*x) < EXPONENT(*y)) {
            return ! abs_x_is_larger;
        }
        else {
            if (MANTISSA(*x) > MANTISSA(*y)) {
                return abs_x_is_larger;
            }
            else {
                return ! abs_x_is_larger;
            }
        }
    }
}/*}}}*/
BOOL float_lt(floatlook *x, floatlook *y) /*{{{*/
{
    return !(float_gt(x, y) || float_eq(x, y));
}
/*}}}*/
BOOL float_ge(floatlook *x, floatlook *y) /*{{{*/
{
    return !float_lt(x, y);
}
/*}}}*/
BOOL float_le(floatlook *x, floatlook *y) /*{{{*/
{
    return !float_gt(x, y);
}
/*}}}*/
#define IS_ZERO(x) (EXPONENT(x) == 0x00 && MANTISSA(x) == 0x000000)
#define IS_PZERO(x) (IS_ZERO(x) && SIGN(x) == PLUS)
#define IS_MZERO(x) (IS_ZERO(x) && SIGN(x) == MINUS)
#define IS_INF(x) (EXPONENT(x) == 0xff && MANTISSA(x) == 0x000000)
#define IS_PINF(x) (IS_INF(x) && SIGN(x) == PLUS)
#define IS_MINF(x) (IS_INF(x) && SIGN(x) == MINUS)
#define IS_NaN(x) (EXPONENT(x) == 0xff && MANTISSA(x) != 0x000000)
#define IS_SNaN(x) (IS_NaN(x) && (MANTISSA(x) >> 22) == 0x00)
#define IS_QNaN(x) (IS_NaN(x) && (MANTISSA(x) >> 22) == 0x01)
#define IS_DENORMALIZED(x) (EXPONENT(x) == 0x00 && MANTISSA(x) != 0x000000)
#define IS_NORMALIZED(x) ( ! IS_DENORMALIZED(x))

floatlook float_plus(floatlook *x, floatlook *y)    /*{{{*/
{
    const int xe = EXPONENT(*x), ye = EXPONENT(*y);
    const int xm = MANTISSA(*x), ym = MANTISSA(*y);
    const int xs = SIGN(*x), ys = SIGN(*y);
    int e, m;
    unsigned int mrs; /* [ ...mantissa..., round, sticky] */
    floatlook result;
    result.intval = 0xdeadbeef;

    if (IS_NaN(*x)) {   /*{{{ NaN check */
        return *x;  /* NaN propagates */
    }
    if (IS_NaN(*y)) {
        return *y;
    }    /*}}}*/

    if (IS_ZERO(*x)) {   /*{{{ when zero is found, return the other */
        return *y;
    }
    if (IS_ZERO(*y)) {
        return *x;
    }    /*}}}*/

/*{{{  handling plus/minus infinities */
#define RETURN_NaN do {                 \
  SET_EXPONENT(result, 0xff);           \
  SET_MANTISSA(result, 0x700000);       \
  return result;                        \
} while (0)
    if (IS_PINF(*x)) {
        if (!IS_MINF(*y)) {
            return *x;
        }
        else {
            RETURN_NaN;
        }
    }
    else if (IS_MINF(*x)) {
        if (!IS_PINF(*y)) {
            return *x;
        }
        else {
            RETURN_NaN;
        }
    }
    if (IS_PINF(*y)) {
        if (!IS_MINF(*x)) {
            return *y;
        }
        else {
            RETURN_NaN;
        }
    }
    else if (IS_MINF(*y)) {
        if (!IS_PINF(*x)) {
            return *y;
        }
        else {
            RETURN_NaN;
        }
    }    /*}}}*/

    assert(xs == ys);
    if (xs == ys) {
        if (IS_NORMALIZED(*y)) {
            if (IS_NORMALIZED(*x)) {   /*{{{  both are normalized */
                if (xe > ye) {   /*{{{  2.4E4 + 4.7E2 == (2.4 + 0.047)E4 */
                    int cnt;
                    e = xe;
                    mrs = (ym + 0x800000) << 2; /* 0x800000 == 2**23 */
                    for (cnt = 0; cnt < xe - ye; ++cnt) {
                        if ((mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02) {
                            mrs >>= 1;
                            mrs |= 0x01;  /* sticky bit */
                        }
                        else {
                            mrs >>= 1;
                        }
                    }
                    mrs += (xm + 0x800000) << 2;
                }
                else {
                    int cnt;
                    e = ye;
                    mrs = (xm + 0x800000) << 2;
                    for (cnt = 0; cnt < ye - xe; ++cnt) {
                        if ((mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02) {
                            mrs >>= 1;
                            mrs |= 0x01;
                        }
                        else {
                            mrs >>= 1;
                        }
                    }
                    mrs += (ym + 0x800000) << 2;
                }    /*}}}*/

                /* printf("mrgs>>3="); tmp.intval = mrgs >> 3; binprint_int(&tmp); */
                if ( (mrs >> 2) > 0x1000000 ) {   /*{{{  9.2E2 + 3.3E2 == 12.5E2 == 1.25E3 */
                    ++e;
                    if ( (mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02 ) {
                        mrs >>= 1;
                        mrs |= 0x01;
                    }
                    else {
                        mrs >>= 1;
                    }
                }    /*}}}*/

                m = (mrs >> 2);    /*{{{  rounding mrs yields m */
                if ( (mrs & 0x03) == 0x03       /* xxx.1_1 = xxx.1yyy...yy1 (using sticky bit) */
                     || (mrs & 0x07) == 0x06 )  /* xxx1.1_0 (round to even) */
                {
                    ++m;
                }    /*}}}*/

                SET_SIGN(result, xs);
                SET_EXPONENT(result, e);
                SET_MANTISSA(result, m);
                return result;
            }    /*}}}*/
            else {   /*{{{  only x is denormalized */
                /* 0.093E0 + 1.2E2 == (0.00093 + 1.2)E2 */
                int cnt;
                mrs = xm << 2;
                for (cnt = 0; cnt < ye - (-127); ++cnt) {
                    if ( (mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02 ) {
                        mrs >>= 1;
                        mrs |= 0x01;  /* sticky bit */
                    }
                    else {
                        mrs >>= 1;
                    }
                }
                mrs += (ym + 0x800000) << 2;
                e = ye;

                if ( (mrs >> 2) > 0x1000000 ) {   /*{{{  9.2E2 + 3.3E2 == 12.5E2 == 1.25E3 */
                    ++e;
                    if ( (mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02 ) {
                        mrs >>= 1;
                        mrs |= 0x01;
                    }
                    else {
                        mrs >>= 1;
                    }
                }    /*}}}*/

                /* intlook tmp; tmp.intval = m; binprint_int(&tmp); */
                m = mrs << 2;
                if ( (mrs & 0x03) == 0x03 || (mrs & 0x07) == 0x06 ) {
                    ++m;
                }

                SET_SIGN(result, xs);
                SET_EXPONENT(result, e);
                SET_MANTISSA(result, m);
                return result;
            }    /*}}}*/
        }
        else
        {
            if (IS_NORMALIZED(*x)) {   /*{{{  only y is denormalized */
                int cnt;
                mrs = ym << 2;
                for (cnt = 0; cnt < xe - (-127); ++cnt) {
                    if ( (mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02 ) {
                        mrs >>= 1;
                        mrs |= 0x01;  /* sticky bit */
                    }
                    else {
                        mrs >>= 1;
                    }
                }
                mrs += (xm + 0x800000) << 2;
                e = xe;

                if ( (mrs >> 2) > 0x1000000 ) {   /*{{{  9.2E2 + 3.3E2 == 12.5E2 == 1.25E3 */
                    ++e;
                    if ( (mrs & 0x01) == 0x01 || (mrs & 0x02) == 0x02 ) {
                        mrs >>= 1;
                        mrs |= 0x01;
                    }
                    else {
                        mrs >>= 1;
                    }
                }    /*}}}*/

                /* intlook tmp; tmp.intval = m; binprint_int(&tmp); */
                m = mrs << 2;
                if ( (mrs & 0x03) == 0x03 || (mrs & 0x07) == 0x06 ) {
                    ++m;
                }
                /* ++e; */

                SET_SIGN(result, ys);
                SET_EXPONENT(result, e);
                SET_MANTISSA(result, m);
                return result;
            }    /*}}}*/
            else    /*{{{  both are denormalized */
            {
                fprintf(stderr, "Should not come here.");
                exit(1);
            }    /*}}}*/
        }
    }
    else
    {
        return result;
    }
}
/*}}}*/

int
main(void)
{
    int cnt;
    floatlook x, y, z, w;

/*  binprint_float(&z);
    putchar('\n');
    SET_SIGN(z, MINUS);   binprint_float(&z);
    SET_SIGN(z, MINUS);   binprint_float(&z);
    SET_SIGN(z, PLUS);    binprint_float(&z);
    SET_SIGN(z, PLUS);    binprint_float(&z);
    SET_SIGN(z, MINUS);   binprint_float(&z);
    putchar('\n');
    SET_EXPONENT(z, 0x01);    binprint_float(&z);
    SET_EXPONENT(z, 0x07);    binprint_float(&z);
    SET_EXPONENT(z, 0x25);    binprint_float(&z);
    SET_SIGN(&z, PLUS);
    SET_EXPONENT(z, 0x03);    binprint_float(&z);
    SET_EXPONENT(z, 0xbb);    binprint_float(&z);
    putchar('\n');
    SET_MANTISSA(z, 0x030303);    binprint_float(&z);
    SET_MANTISSA(z, 0x730ff0);    binprint_float(&z);
    SET_SIGN(z, PLUS);
    SET_EXPONENT(z, 0x9a);    binprint_float(&z); */

    x.floatval = sqrtf(3); y.floatval = sqrtf(5); z.floatval = sqrtf(3);
#define EQ_TEST(xxx, yyy) assert( (xxx.floatval == yyy.floatval) == float_eq(&xxx, &yyy) )
    EQ_TEST(x, y); EQ_TEST(x, z);
#define GT_TEST(xxx, yyy) assert( (xxx.floatval > yyy.floatval) == float_gt(&xxx, &yyy) )
    GT_TEST(x, y); GT_TEST(x, z); GT_TEST(y, x);

    x.floatval = 4.3f;                      printf("x="); binprint_float(&x);
    y.floatval = 2.3f;                      printf("y="); binprint_float(&y);
    z.floatval = x.floatval + y.floatval;   printf("z="); binprint_float(&z);
    w = float_plus(&x, &y);                 printf("w="); binprint_float(&w);
    assert(z.floatval == w.floatval);
    putchar('\n');

    srand((uint)time(NULL));
    for (cnt = 0; cnt < 100000; ++cnt) {
        int sign = (1 - 2 * (rand() & 0x01));
        x.floatval = (float)(sign * ((double)rand() / RAND_MAX) * pow(10, (rand() % 75) - 37));
        /* printf("x="); binprint_float(&x); */
        y.floatval = (float)(sign * ((double)rand() / RAND_MAX) * pow(10, (rand() % 75) - 37));
        /* printf("y="); binprint_float(&y); */
        z.floatval = x.floatval + y.floatval;
        /* printf("z="); binprint_float(&z); */
        w = float_plus(&x, &y);
        /* printf("w="); binprint_float(&w); */
        /* assert(z.floatval == w.floatval); */
        if (z.floatval != w.floatval) {
            printf("x="); binprint_float(&x);
            printf("y="); binprint_float(&y);
            printf("z="); binprint_float(&z);
            printf("w="); binprint_float(&w);
            abort();
        }
    }

    printf("Conguratulations!\n");

    return 0;
}
