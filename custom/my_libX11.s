.include "./wrapper.inc"

.section .text

WRAP_BIG_FUNC XCreateWindow

WRAP_MED_FUNC XSetStandardProperties
WRAP_MED_FUNC XChangeProperty

WRAP_FUNC XInternAtom
WRAP_FUNC XOpenDisplay
WRAP_FUNC XCloseDisplay
WRAP_FUNC XFree
WRAP_FUNC XMapWindow
WRAP_FUNC XDestroyWindow
WRAP_FUNC XSetNormalHints
WRAP_FUNC XParseGeometry
WRAP_FUNC XCreateColormap
WRAP_FUNC XSetErrorHandler
