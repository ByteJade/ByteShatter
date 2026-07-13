#include <stdio.h>

int main() {
    const char* s1 = "max";
    const char* s2 = "min";
    const char* s3 = "neg";
    const char* s4 = "pos";
    int x1 = 2147483647;
    int x2 = -2147483648;
    int x3 = -1;
    int x4 = 1;
    printf("%s %i %s %i %s %i %s %i\n",
                s1, x1, s2, x2, s3, x3, s4, x4);
}