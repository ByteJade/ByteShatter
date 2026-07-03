#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (i < argc-1 && strcmp(argv[i], "-framerate") == 0) {
            printf("FRAMERATE\n");
        }
        else {
            printf("NOT");
        }
    }
    return 0;
}