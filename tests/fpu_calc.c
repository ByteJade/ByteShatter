#include <math.h>
#include <stdio.h>

int main() {
    float a = 8.44f;
    float b = 2.f;
    float c = 3.f;

    float a_div_b = a / b;
    float mul_c = a_div_b * c;
    printf("a: %f, b: %f, c: %f\n", a, b, c);
    printf("a/b = %f (4.220000)\n", a_div_b);
    printf("(a/b)*c = %f (12.660000)\n", mul_c);

    float cos_r = cos(mul_c);
    printf("cos: %f (0.995620)\n", cos_r);
    return 0;
}