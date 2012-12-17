#define __USE_XOPEN
#include <stdio.h>
#include <stdint.h>
#include <float.h>

typedef uint8_t Byte;

#define INTperBYTE (sizeof(int)/sizeof(Byte))
#define UINTperBYTE (sizeof(unsigned int)/sizeof(Byte))
#define DOUBLEperBYTE (sizeof(double)/sizeof(Byte))
#define FLOATperBYTE (sizeof(float)/sizeof(Byte))

/*
typedef union {
    int intval;
    Byte peek[INTperBYTE];
} intlook;

typedef union {
    float floatval;
    Byte peek[FLOATperBYTE];
} floatlook;
*/

/*
void
binprint_int(intlook *ptr)
{
    printf("%12d  =  ", ptr->theint);

    int i;
    for (i = 8 * INTperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
    putchar('\n');
}
*/

int
binprint_int(int theint)
{
    int i;
    Byte *peek = (Byte *)&theint;

    printf("%12d  =  ", theint);

    for (i = 8 * INTperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (peek[i/8] >> (i%8)) & 0x01);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
    putchar('\n');

    return 0;
}

/*
void
binprint_uint(uintlook *ptr)
{
    printf("%12u  =  ", ptr->utheint);

    int i;
    for (i = 8 * UINTperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
    }
    putchar('\n');
}
*/

/*
void
binprint_float(floatlook *ptr)
{
    int i;
    for (i = 8 * FLOATperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATperBYTE * 8 - i == 1 || FLOATperBYTE * 8 - i == 9)
        {
            printf("   ");
        }
    }

    printf(" = %88.47f\n", ptr->floatval);
}
*/

void
binprint_float(float thefloat)
{
    int i;
    Byte *peek = (Byte *)&thefloat;

    printf("%88.47f  =  ", thefloat);

    for (i = 8 * FLOATperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (peek[i/8] >> (i%8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATperBYTE * 8 - i == 1 || FLOATperBYTE * 8 - i == 9)
        {
            printf("   ");
        }
    }
    putchar('\n');
}

/* void
binprint_float_wide(floatlook *ptr, int width, int dec)
{
    int i;
    for (i = 8 * FLOATperBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATperBYTE * 8 - i == 1 || FLOATperBYTE * 8 - i == 9)
        {
            printf("   ");
        }
    }

    char fmt[15];
    sprintf(fmt, " = %%%d.%df\n", width, dec);
    printf(fmt, ptr->floatval);
} */

void test_int(void)
{
    int i, theint;
    theint = 0x1;
    binprint_int(theint);

    for (i = 0; i < 32; ++i)
    {
        theint <<= 1;
        binprint_int(theint);
    }

    theint = -0x1;
    binprint_int(theint);

    for (i = 0; i < 32; ++i)
    {
        theint <<= 1;
        binprint_int(theint);
    }

    theint = ~0;
    binprint_int(theint);

    for (i = 0; i < 32; ++i)
    {
        theint >>= 1;
        binprint_int(theint);
    }

    theint = ~0;
    binprint_int(theint);

    for (i = 0; i < 32; ++i)
    {
        theint = (unsigned int)theint >> 1;
        binprint_int(theint);
    }
}

void test_float()
{
    int i;
    float thefloat;
    Byte peek[4];

    thefloat = 2.0;
    binprint_float(thefloat);
    thefloat *= 2.0;
    binprint_float(thefloat);
    thefloat *= 2.0;
    binprint_float(thefloat);
    printf("                                       --- snip ---\n");
    for (i = 0; i < 122; ++i)
    {
        thefloat *= 2.0;
        /* binprint_float(thefloat); */
    }
    thefloat *= 2.0;
    binprint_float(thefloat);
    thefloat *= 2.0;
    binprint_float(thefloat);
    thefloat *= 2.0;
    binprint_float(thefloat);

    putchar('\n');

    thefloat = 2.0;
    binprint_float(thefloat);
    thefloat /= 2.0;
    binprint_float(thefloat);
    thefloat /= 2.0;
    binprint_float(thefloat);
    printf("                                       --- snip ---\n");
    for (i = 0; i < 122; ++i)
    {
        thefloat /= 2.0;
        /* binprint_float(thefloat); */
    }
    thefloat /= 2.0;
    binprint_float(thefloat);
    thefloat /= 2.0;
    binprint_float(thefloat);
    thefloat /= 2.0;
    binprint_float(thefloat);

    /* thefloat = -1.0;
       binprint_float(thefloat);

       for (i = 0; i < EXPMAX; ++i)
       {
           thefloat *= 2.0;
           binprint_float(thefloat);
       } */

    putchar('\n');
    putchar('\n');

    peek[3] = 0x00;
    peek[2] = 0xff;
    peek[1] = 0xff;
    peek[0] = 0xff;
    /* memcpy(&thefloat, &peek, FLOATperBYTE); */
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0xff;
    peek[1] = 0xff;
    peek[0] = 0xfe;
    binprint_float(*((float *)peek));

    printf("                                       --- snip ---\n");

    peek[3] = 0x00;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x02;
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x01;
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x00;
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0x7f;
    peek[1] = 0xff;
    peek[0] = 0xff;
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0x7f;
    peek[1] = 0xff;
    peek[0] = 0xfe;
    binprint_float(*((float *)peek));

    printf("                                       --- snip ---\n");

    peek[3] = 0x00;
    peek[2] = 0x00;
    peek[1] = 0x00;
    peek[0] = 0x02;
    binprint_float(*((float *)peek));

    peek[3] = 0x00;
    peek[2] = 0x00;
    peek[1] = 0x00;
    peek[0] = 0x01;
    binprint_float(*((float *)peek));

    putchar('\n');

    thefloat = 0.0;
    binprint_float(thefloat);

    thefloat = -0.0;
    binprint_float(thefloat);

    peek[3] = 0x7f;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x00;
    binprint_float(*((float *)peek));

    peek[3] = 0xff;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x00;
    binprint_float(*((float *)peek));

    peek[3] = 0x7f;
    peek[2] = 0x80;
    peek[1] = 0x00;
    peek[0] = 0x01;
    binprint_float(*((float *)peek));

    peek[3] = 0x7f;
    peek[2] = 0xc0;
    peek[1] = 0x00;
    peek[0] = 0x01;
    binprint_float(*((float *)peek));

    putchar('\n');

    peek[3] = 0x7f;
    peek[2] = 0x7f;
    peek[1] = 0xff;
    peek[0] = 0xff;
    binprint_float(*((float *)peek));
}

void binprint_double(double thedouble)
{
    int i, j;
    union {
        double d;
        Byte b[DOUBLEperBYTE];
    } u;

    u.d = thedouble;

    for (i = 0; i < DOUBLEperBYTE; ++i) {
        for (j = 0; j < 8; ++j)
            printf("%d", (u.b[DOUBLEperBYTE - 1 - i] >> (7 - j)) & 0x01);
        putchar(' ');
    }
    putchar('\n');
}

void test_float01()
{
    int i;

    binprint_double(1);
    binprint_double(2);
    binprint_double(4);
    binprint_double(10);

    binprint_double(0.1);

    binprint_double(DBL_MAX);
    binprint_double(-DBL_MAX);

    binprint_double(DBL_EPSILON);

    binprint_double(1.0 + DBL_EPSILON);

    putchar('\n');

    const double onetenth = 0.1;
    for (i = 0; i <= 10; ++i) {
        volatile double a = i * onetenth;
        volatile double b = 10 * a;
        printf("%f %s %f\n", b, (b == i ? "==" : "!="), (double)i);
    }

}

int main(void)
{
    /* puts("test_int()\n"); test_int(); */

    /* puts("test_float()\n"); test_float(); */
    test_float01();

    return 0;
}
