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

static void no_border( Display *dpy, Window w)
{
    static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
    static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

    typedef struct
    {
        unsigned long       flags;
        unsigned long       functions;
        unsigned long       decorations;
        long                inputMode;
        unsigned long       status;
    } PropMotifWmHints;

    PropMotifWmHints motif_hints;
    Atom prop, proptype;
    unsigned long flags = 0;

    /* setup the property */
    motif_hints.flags = MWM_HINTS_DECORATIONS;
    motif_hints.decorations = flags;

    /* get the atom for the property */
    prop = XInternAtom( dpy, "_MOTIF_WM_HINTS", True );
    if (!prop) {
        /* something went wrong! */
        return;
    }

    /* not sure this is correct, seems to work, XA_WM_HINTS didn't work */
    proptype = prop;

    XChangeProperty( dpy, w,                         /* display, window */
                        prop, proptype,                 /* property, type */
                        32,                             /* format: 32-bit datums */
                        PropModeReplace,                /* mode */
                        (unsigned char *) &motif_hints, /* data */
                        PROP_MOTIF_WM_HINTS_ELEMENTS    /* nelements */
                    );
}

static void
make_window( Display *dpy, const char *name,
             int x, int y, int width, int height,
             Window *winRet, GLXContext *ctxRet, VisualID *visRet)
{
    int attribs[64];
    int i = 0;

    int scrnum;
    XSetWindowAttributes attr;
    unsigned long mask;
    Window root;
    Window win;
    GLXContext ctx;
    XVisualInfo *visinfo;

    /* Singleton attributes. */
    attribs[i++] = GLX_RGBA;
    attribs[i++] = GLX_DOUBLEBUFFER;
    if (stereo)
        attribs[i++] = GLX_STEREO;

    /* Key/value attributes. */
    attribs[i++] = GLX_RED_SIZE;
    attribs[i++] = 1;
    attribs[i++] = GLX_GREEN_SIZE;
    attribs[i++] = 1;
    attribs[i++] = GLX_BLUE_SIZE;
    attribs[i++] = 1;
    attribs[i++] = GLX_DEPTH_SIZE;
    attribs[i++] = 1;
    if (samples > 0) {
        attribs[i++] = GLX_SAMPLE_BUFFERS;
        attribs[i++] = 1;
        attribs[i++] = GLX_SAMPLES;
        attribs[i++] = samples;
    }

    attribs[i++] = None;

    scrnum = DefaultScreen( dpy );
    root = RootWindow( dpy, scrnum );

    visinfo = glXChooseVisual(dpy, scrnum, attribs);
    if (!visinfo) {
        printf("Error: couldn't get an RGB, Double-buffered");
        if (stereo)
            printf(", Stereo");
        if (samples > 0)
            printf(", Multisample");
        printf(" visual\n");
        exit(1);
    }

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    /* XXX this is a bad way to get a borderless window! */
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow( dpy, root, x, y, width, height,
                    0, visinfo->depth, InputOutput,
                    visinfo->visual, mask, &attr );

    if (fullscreen)
        no_border(dpy, win);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = x;
        sizehints.y = y;
        sizehints.width  = width;
        sizehints.height = height;
        sizehints.flags = USSize | USPosition;
        XSetNormalHints(dpy, win, &sizehints);
        XSetStandardProperties(dpy, win, name, name,
                                None, (char **)NULL, 0, &sizehints);
    }

    ctx = glXCreateContext( dpy, visinfo, NULL, True );
    if (!ctx) {
        printf("Error: glXCreateContext failed\n");
        exit(1);
    }

    *winRet = win;
    *ctxRet = ctx;
    *visRet = visinfo->visualid;

    XFree(visinfo);
}

static int is_glx_extension_supported(Display *dpy, const char *query)
{
    const int scrnum = DefaultScreen(dpy);
    const char *glx_extensions = NULL;
    const size_t len = strlen(query);
    const char *ptr;

    if (glx_extensions == NULL) {
        glx_extensions = glXQueryExtensionsString(dpy, scrnum);
    }

    ptr = strstr(glx_extensions, query);
    return ((ptr != NULL) && ((ptr[len] == ' ') || (ptr[len] == '\0')));
}
static void query_vsync(Display *dpy, GLXDrawable drawable)
{
    int interval = 0;
#if defined(GLX_EXT_swap_control)
    if (is_glx_extension_supported(dpy, "GLX_EXT_swap_control")) {
        unsigned int tmp = -1;
        glXQueryDrawable(dpy, drawable, GLX_SWAP_INTERVAL_EXT, &tmp);
        interval = tmp;
    } else
#endif
    if (is_glx_extension_supported(dpy, "GLX_MESA_swap_control")) {
        PFNGLXGETSWAPINTERVALMESAPROC pglXGetSwapIntervalMESA =
            (PFNGLXGETSWAPINTERVALMESAPROC)
            glXGetProcAddressARB((const GLubyte *) "glXGetSwapIntervalMESA");

        interval = (*pglXGetSwapIntervalMESA)();
    } else if (is_glx_extension_supported(dpy, "GLX_SGI_swap_control")) {
        interval = 1;
    }


    if (interval > 0) {
        printf("Running synchronized to the vertical refresh.  The framerate should be\n");
        if (interval == 1) {
            printf("approximately the same as the monitor refresh rate.\n");
        } else if (interval > 1) {
            printf("approximately 1/%d the monitor refresh rate.\n",
                    interval);
        }
    }
}

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
            samples = strtol(argv[i+1], NULL, 10);
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
            framerate = strtol(argv[i+1], NULL, 10);
            i++;
        }
        else {
            usage();
            return -1;
        }
    }
    dpy = XOpenDisplay(dpyName);
    if (!dpy) {
        printf("Error: couldn't open display %s\n",
            dpyName ? dpyName : getenv("DISPLAY"));
        return -1;
    }
    if (fullscreen) {
        int scrnum = DefaultScreen(dpy);

        x = 0; y = 0;
        winWidth = DisplayWidth(dpy, scrnum);
        winHeight = DisplayHeight(dpy, scrnum);
    }
    make_window(dpy, "glxgears", x, y, winWidth, winHeight, &win, &ctx, &visId);
    XMapWindow(dpy, win);
    glXMakeCurrent(dpy, win, ctx);

    if (framerate <=  0) {
        query_vsync(dpy, win);
    }
    else {
        printf("Limiting framerate to ~%d frames/sec\n", framerate);
    }

    if (printInfo) {
        printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
        printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
        printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
        printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
        printf("VisualID %d, 0x%x\n", (int) visId, (int) visId);
    }

    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, ctx);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}