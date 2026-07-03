#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (strcmp(argv[argc-1], "-display") == 0) {
        printf("DISPLAY\n");
    }
    return 0;
}