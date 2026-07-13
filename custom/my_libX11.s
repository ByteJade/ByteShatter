.include "./wrapper.inc"

.section .text


WRAP_BIG_FUNC XSetStandardProperties
WRAP_BIG_FUNC XChangeProperty
WRAP_BIG_FUNC XCreateWindow

WRAP XInternAtom
WRAP DefaultScreen
WRAP RootWindow
WRAP XOpenDisplay
WRAP XCloseDisplay
WRAP XFree
WRAP XMapWindow
WRAP XDestroyWindow
WRAP XSetNormalHints
WRAP XParseGeometry
WRAP XCreateColormap