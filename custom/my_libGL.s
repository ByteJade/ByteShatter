.include "./wrapper.inc"

.section .text

WRAP_FUNC(glGetString)

WRAP_FUNC(glXMakeCurrent)
WRAP_FUNC(glXGetProcAddressARB)
WRAP_FUNC(glXDestroyContext)
WRAP_FUNC(glXCreateContext)
WRAP_FUNC(glXQueryDrawable)
WRAP_FUNC(glXChooseVisual)
WRAP_FUNC(glXQueryExtensionsString)
