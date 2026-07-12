#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "wrapper.h"
#include <stdio.h>
#include <stdlib.h>

WRAP(Atom, XInternAtom, Display *display, char *atom_name, Bool only_if_exists);
WRAP(int, DefaultScreen, Display *display);
WRAP(Window, RootWindow, Display *display, int screen_number);
//WRAP(Display*, XOpenDisplay, char *display_name);
WRAP(int, XCloseDisplay, Display *display);
WRAP(int, XFree, void *data);
WRAP(int, XMapWindow, Display *display, Window w);
WRAP(int, XDestroyWindow, Display *display, Window w);
WRAP(void, XSetNormalHints, Display *display, Window w, XSizeHints *hints);
WRAP(int, XParseGeometry, const char *parsestring, int *x_return, int *y_return, unsigned int *width_return, unsigned int *height_return);

WRAP(Colormap, XCreateColormap, Display *display, Window w, Visual *visual, int alloc);

Display* my_XOpenDisplay(char *display_name) {

    printf("DISPLAY: %s\n", display_name);
    Display* result;
    asm volatile(
        "bl XOpenDisplay\n"
        "mov %0, x0\n"
        : "=r" (result)
    );
    printf("RETURN: %p\n", result);
    return result;
}
int my_XSetStandardProperties(
    Window w, char *window_name,
    char *icon_name, Pixmap icon_pixmap,
    char **argv, int argc,
    XSizeHints *hints) {
    POP8;
    int result;
    asm volatile(
        "bl XChangeProperty\n"
        "mov %w0, w0\n"
        : "=r" (result)
    );
    return result;
}
int my_XChangeProperty(
    Display *display, Window w,
    Atom property, Atom type,
    int format, int mode,
    unsigned char *data, int nelements) {
    POP8;
    int result;
    asm volatile(
        "bl XChangeProperty\n"
        "mov %w0, w0\n"
        : "=r" (result)
    );
    return result;
}
Window my_XCreateWindow(
    Display *display, Window parent,
    int x, int y,
    unsigned int width, unsigned int height,
    unsigned int border_width,
    int depth, unsigned int class,
    Visual *visual,
    unsigned long valuemask,
    XSetWindowAttributes *attribute
) {
    POP8;
    Window ret;
    asm volatile(
        "bl XCreateWindow\n"
	"mov %0, x0"
	: "=r" (ret)
    );
    return ret;
}

