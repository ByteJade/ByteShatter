#include <GL/glew.h>
#include "wrapper.h"

WRAP_NORETURN(GLenum, glewInit, void);
WRAP_NORETURN(GLboolean, glewIsSupported, const char *name);
WRAP_NORETURN(int, glewGetExtension, const char *name);
WRAP_NORETURN(int, glewGetError, void);
WRAP_NORETURN(const char*, glewGetString, GLenum name);
WRAP_NORETURN(void, glewExperimental, void);