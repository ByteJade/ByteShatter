#include <stdio.h>

int main() {
    const char* s1 = "neg";
    const char* s2 = "pos";
    const char* s3 = "max";
    const char* s4 = "min";
    int x1 = -1;
    int x2 = 1;
    int x3 = 2147483647;
    int x4 = -2147483648;
    printf("%s %x %s %x %s %x %s %x\n",
                s1, x1, s2, x2, s3, x3, s4, x4);
}