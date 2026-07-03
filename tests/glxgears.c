#include <stdio.h>
#include <string.h>

static void usage()
{
    printf("Usage:\n");
    printf("  -display <displayname>  set the display to run on\n");
    printf("  -stereo                 run in stereo mode\n");
    printf("  -samples N              run in multisample mode with at least N samples\n");
    printf("  -fullscreen             run in fullscreen mode\n");
    printf("  -info                   display OpenGL renderer info\n");
    printf("  -geometry WxH+X+Y       window geometry\n");
    printf("  -framerate N            limit framerate to N frams/sec\n");
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-display") == 0) {
            printf("DISPLAY\n");
        }
        else if (strcmp(argv[i], "-info") == 0) {
            printf("INFO\n");
        }
        else if (strcmp(argv[i], "-stereo") == 0) {
            printf("STEREO\n");
        }
        else if (i < argc-1 && strcmp(argv[i], "-samples") == 0) {
            printf("SAMPLES\n");
        }
        else if (strcmp(argv[i], "-fullscreen") == 0) {
            printf("FULLSCREEN\n");
        }
        else if (i < argc-1 && strcmp(argv[i], "-geometry") == 0) {
            printf("GEOMETRY\n");
        }
        else if (i < argc-1 && strcmp(argv[i], "-framerate") == 0) {
            printf("FRAMERATE\n");
        }
        else {
            usage();
            return -1;
        }
    }
    return 0;
}