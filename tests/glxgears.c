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
    if (strcmp(argv[argc-1], "-display") == 0) {
        printf("DISPLAY\n");
    }
    return 0;
}