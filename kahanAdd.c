#include <stdio.h>

float ordinarySum(float nums[], int n)
{
    int i;
    float sum=0;
    for (i=0; i<n; ++i) {
        sum += nums[i];
    }
    return sum;
}

float kahanSum(float nums[], int n)
{
    int i;
    float sum = nums[0];
    volatile float c = 0.0f;
    for (i=1; i<n; ++i) {
        volatile float y = nums[i] - c;
        volatile float t = sum + y;
        c = (t-sum) - y;
        sum = t;
    }
    return sum;
}

double doubleSum(float nums[], int n)
{
    int i;
    double sum=0;
    for (i=0; i<n; ++i) {
        sum += nums[i];
    }
    return sum;
}

int main(void)
{
    float nums[] = {
        1e0f, 3.14159265e-7f, 2.71828183e-7f,
    };
    int n = sizeof(nums)/sizeof(float);
    float sum = ordinarySum(nums, n);
    float kSum = kahanSum(nums, n);
    double dSum = doubleSum(nums, n);
    printf("%8.8f vs. %8.8f (%8.8f)\n", sum, kSum, dSum);
    return 0;
}
