#include <GL/glew.h>
#include "wrapper.h"

WRAP(GLenum, glewInit, void);
WRAP(GLboolean, glewIsSupported, const char *name);
WRAP(int, glewGetExtension, const char *name);
WRAP(int, glewGetError, void);
WRAP(const char*, glewGetString, GLenum name);
WRAP(void, glewExperimental, void);