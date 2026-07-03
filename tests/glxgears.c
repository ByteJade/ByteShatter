#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static GLboolean fullscreen = GL_FALSE;	/* Create a single fullscreen window */
static GLboolean stereo = GL_FALSE;	/* Enable stereo.  */
static GLint samples = 0;               /* Choose visual with at least N samples. */
static GLboolean animate = GL_TRUE;	/* Animation */
static GLfloat eyesep = 5.0;		/* Eye separation. */
static GLfloat fix_point = 40.0;	/* Fixation point distance.  */
static GLfloat left, right, asp;	/* Stereo frustum params.  */

static int framerate = -1; /* Framerate limit if > 0 */

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
    unsigned int winWidth = 300, winHeight = 300;
    int x = 0, y = 0;
    Display *dpy;
    Window win;
    GLXContext ctx;
    char *dpyName = NULL;
    GLboolean printInfo = GL_FALSE;
    VisualID visId;
    int i;

    for (i = 1; i < argc; i++) {
        if (i < argc-1 && strcmp(argv[i], "-display") == 0) {
            dpyName = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i], "-info") == 0) {
            printInfo = GL_TRUE;
        }
        else if (strcmp(argv[i], "-stereo") == 0) {
            stereo = GL_TRUE;
        }
        else if (i < argc-1 && strcmp(argv[i], "-samples") == 0) {
            samples = strtod(argv[i+1], NULL );
            i++;
        }
        else if (strcmp(argv[i], "-fullscreen") == 0) {
            fullscreen = GL_TRUE;
        }
        else if (i < argc-1 && strcmp(argv[i], "-geometry") == 0) {
            XParseGeometry(argv[i+1], &x, &y, &winWidth, &winHeight);
            i++;
        }
        else if (i < argc-1 && strcmp(argv[i], "-framerate") == 0) {
            framerate = strtod(argv[i+1], NULL );
            i++;
        }
        else {
            usage();
            return -1;
        }
    }
    return 0;
}