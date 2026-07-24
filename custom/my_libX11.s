.include "./wrapper.inc"

.section .text

WRAP_BIG_FUNC XCreateWindow

WRAP_MED_FUNC XSetStandardProperties
WRAP_MED_FUNC XChangeProperty

WRAP_FUNC XInternAtom
WRAP_FUNC XOpenDisplay
WRAP_FUNC_VOID XCloseDisplay
WRAP_FUNC XFree
WRAP_FUNC_VOID XMapWindow
WRAP_FUNC_VOID XDestroyWindow
WRAP_FUNC_VOID XSetNormalHints
WRAP_FUNC XParseGeometry
WRAP_FUNC XCreateColormap
WRAP_FUNC XSetErrorHandler
WRAP_FUNC_VOID XGetErrorText
WRAP_FUNC XLookupString
WRAP_FUNC XLookupKeysym
WRAP_FUNC XPending
WRAP_FUNC_VOID XNextEvent
