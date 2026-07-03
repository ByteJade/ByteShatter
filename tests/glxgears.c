#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char* test = "-display";
    printf("arg: %s; test: %s\n", argv[argc-1], test);
    if (strcmp(argv[argc-1], test) == 0) {
        printf("DISPLAY\n");
    } else {
        printf("NOT DISPLAY\n");
    }
    return 0;
}