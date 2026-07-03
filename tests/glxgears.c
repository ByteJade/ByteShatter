#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("arg: %s\n", argv[argc-1]);
    if (strcmp(argv[argc-1], "-display") == 0) {
        printf("DISPLAY\n");
    }
    return 0;
}