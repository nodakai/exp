#include <stdio.h>

int main()
{
    {
        float f = -1.75;
        int fi = *((int *)&f);
        float f2 = *((float *)&fi);
        int fi2;
        float f3;
        *((float *)&fi2) = f2;
        *((int *)&f3) = fi2;
        printf("%f %08x %f, %08x, %f\n", f, fi, f2, fi2, f3);
    }
    {
        double d = -1.75;
        int di0 = *((int *)&d);
        /* int di1_ = *(((int *)&d)+1); */
        int di1 = *((int *)&d+1);
        double d2;
        *((int *)&d2) = di0;
        *((int *)&d2+1) = di1;
        printf("%f %08x %08x %f\n", d, di0, di1, d2);
    }
    return 0;
}
