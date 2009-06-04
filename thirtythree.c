#include <stdio.h>

typedef enum { FALSE, TRUE, } BOOL;

typedef enum { PLUS, MINUS, } sign;

typedef unsigned char Byte;

#define INTparBYTE (sizeof(int)/sizeof(Byte))
#define UINTparBYTE (sizeof(unsigned int)/sizeof(Byte))
#define DOUBLEparBYTE (sizeof(double)/sizeof(Byte))
#define FLOATparBYTE (sizeof(float)/sizeof(Byte))

/*
typedef union {
    int intval;
    Byte peek[INTparBYTE];
} intlook;

typedef union {
    float floatval;
    Byte peek[FLOATparBYTE];
} floatlook;
*/

/*
void
binprint_int(intlook *ptr)
{
    printf("%12d  =  ", ptr->theint);

    int i;
    for (i = 8 * INTparBYTE - 1; i >= 0; --i)
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

    for (i = 8 * INTparBYTE - 1; i >= 0; --i)
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
    for (i = 8 * UINTparBYTE - 1; i >= 0; --i)
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
    for (i = 8 * FLOATparBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATparBYTE * 8 - i == 1 || FLOATparBYTE * 8 - i == 9)
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

    for (i = 8 * FLOATparBYTE - 1; i >= 0; --i)
    {
        printf("%d", (peek[i/8] >> (i%8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATparBYTE * 8 - i == 1 || FLOATparBYTE * 8 - i == 9)
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
    for (i = 8 * FLOATparBYTE - 1; i >= 0; --i)
    {
        printf("%d", (ptr->peek[i / 8] >> (i % 8)) & 0x1);

        if (i % 8 == 0)
        {
            putchar(' ');
        }
        else if (FLOATparBYTE * 8 - i == 1 || FLOATparBYTE * 8 - i == 9)
        {
            printf("   ");
        }
    }

    char fmt[15];
    sprintf(fmt, " = %%%d.%df\n", width, dec);
    printf(fmt, ptr->floatval);
} */

int
main(void)
{
    int i, theint;
    float thefloat;
    Byte peek[4];

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

    putchar('\n');
    putchar('\n');

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
    /* memcpy(&thefloat, &peek, FLOATparBYTE); */
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

    return 0;
}
