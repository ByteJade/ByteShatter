#include "wrapper.h"

WRAP_BIG_FUNC(XCreateWindow)
WRAP_BIG_FUNC(XSetWMProperties)

WRAP_MED_FUNC(XSetStandardProperties)
WRAP_MED_FUNC(XChangeProperty)

WRAP_FUNC(XScreenNumberOfScreen)
WRAP_FUNC(XSetWMProtocols)
WRAP_FUNC(XInternAtom)
WRAP_FUNC(XOpenDisplay)
WRAP_FUNC(XDefaultScreenOfDisplay)
WRAP_FUNC(XFree)
WRAP_FUNC(XParseGeometry)
WRAP_FUNC(XCreateColormap)
WRAP_FUNC(XSetErrorHandler)
WRAP_FUNC(XLookupString)
WRAP_FUNC(XLookupKeysym)
WRAP_FUNC(XPending)
WRAP_FUNC(XGetVisualInfo)
WRAP_FUNC(XGetErrorText)

WRAP_FUNC_VOID(XStoreName)
WRAP_FUNC_VOID(XMapWindow)
WRAP_FUNC_VOID(XDestroyWindow)
WRAP_FUNC_VOID(XSetNormalHints)
WRAP_FUNC_VOID(XNextEvent)
WRAP_FUNC_VOID(XCloseDisplay)