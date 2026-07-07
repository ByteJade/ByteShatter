#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "wrapper.h"

WRAP_NORETURN(Atom, XInternAtom, Display *display, char *atom_name, Bool only_if_exists);
WRAP_NORETURN(int, DefaultScreen, Display *display);
WRAP_NORETURN(Window, RootWindow, Display *display, int screen_number);

WRAP_NORETURN(Colormap, XCreateColormap, Display *display, Window w, Visual *visual, int alloc);

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
	"mov %x0, x0"
	: "=r" (ret)
    );
    return ret
}

