.include "./wrapper.inc"

.section .text

WRAP_FUNC glGetString
WRAP_FUNC glTranslated
WRAP_FUNC glNormal3f
WRAP_FUNC glEnd
WRAP_FUNC glBegin
WRAP_FUNC glEnable
WRAP_FUNC glMatrixMode
WRAP_FUNC glClear
WRAP_FUNC glGenLists
WRAP_FUNC glDeleteLists
WRAP_FUNC glPushMatrix
WRAP_FUNC glPopMatrix
WRAP_FUNC glCallList
WRAP_FUNC glViewport

WRAP_FUNC glXMakeCurrent
WRAP_FUNC glXGetProcAddressARB
WRAP_FUNC glXDestroyContext
WRAP_FUNC glXCreateContext
WRAP_FUNC glXQueryDrawable
WRAP_FUNC glXChooseVisual
WRAP_FUNC glXQueryExtensionsString
