#include "wrapper.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

WRAP(void, glMatrixMode, GLenum mode);
WRAP(void, glPushMatrix, void);
WRAP(void, glPopMatrix, void);
WRAP(void, glLoadIdentity, void);
WRAP(void, glLoadMatrixf, const GLfloat *m);
WRAP(void, glMultMatrixf, const GLfloat *m);

WRAP(void, glTranslatef, GLfloat x, GLfloat y, GLfloat z);
WRAP(void, glTranslated, GLdouble x, GLdouble y, GLdouble z);
WRAP(void, glRotatef, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
WRAP(void, glRotated, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
WRAP(void, glScalef, GLfloat x, GLfloat y, GLfloat z);
WRAP(void, glScaled, GLdouble x, GLdouble y, GLdouble z);

WRAP(void, glCallList, GLuint list);
WRAP(void, glCallLists, GLsizei n, GLenum type, const GLvoid *lists);
WRAP(GLuint, glGenLists, GLsizei range);
WRAP(void, glNewList, GLuint list, GLenum mode);
WRAP(void, glEndList, void);
WRAP(void, glDeleteLists, GLuint list, GLsizei range);

WRAP(void, glClear, GLbitfield mask);
WRAP(void, glClearColor, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
WRAP(void, glClearDepth, GLclampd depth);
WRAP(void, glClearStencil, GLint s);

WRAP(void, glDrawBuffer, GLenum mode);
WRAP(void, glReadBuffer, GLenum mode);

WRAP(void, glShadeModel, GLenum mode);
WRAP(void, glEnable, GLenum cap);
WRAP(void, glDisable, GLenum cap);
WRAP(void, glColorMaterial, GLenum face, GLenum mode);

WRAP(void, glNormal3f, GLfloat nx, GLfloat ny, GLfloat nz);
WRAP(void, glNormal3d, GLdouble nx, GLdouble ny, GLdouble nz);
WRAP(void, glNormal3fv, const GLfloat *v);
WRAP(void, glVertex3f, GLfloat x, GLfloat y, GLfloat z);
WRAP(void, glVertex3d, GLdouble x, GLdouble y, GLdouble z);
WRAP(void, glVertex3fv, const GLfloat *v);
WRAP(void, glBegin, GLenum mode);
WRAP(void, glEnd, void);

WRAP(void, glColor3f, GLfloat red, GLfloat green, GLfloat blue);
WRAP(void, glColor3d, GLdouble red, GLdouble green, GLdouble blue);
WRAP(void, glColor3fv, const GLfloat *v);
WRAP(void, glColor4f, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

WRAP(void, glMaterialf, GLenum face, GLenum pname, GLfloat param);
WRAP(void, glMaterialfv, GLenum face, GLenum pname, const GLfloat *params);
WRAP(void, glLightf, GLenum light, GLenum pname, GLfloat param);
WRAP(void, glLightfv, GLenum light, GLenum pname, const GLfloat *params);
WRAP(void, glLightModeli, GLenum pname, GLint param);

WRAP(void, glTexImage2D, GLenum target, GLint level, GLint internalFormat,
              GLsizei width, GLsizei height, GLint border, GLenum format,
              GLenum type, const GLvoid *pixels);
WRAP(void, glTexParameteri, GLenum target, GLenum pname, GLint param);
WRAP(void, glTexParameterf, GLenum target, GLenum pname, GLfloat param);
WRAP(void, glBindTexture, GLenum target, GLuint texture);
WRAP(void, glDeleteTextures, GLsizei n, const GLuint *textures);
WRAP(void, glGenTextures, GLsizei n, GLuint *textures);

WRAP(void, glViewport, GLint x, GLint y, GLsizei width, GLsizei height);
WRAP(void, glOrtho, GLdouble left, GLdouble right, GLdouble bottom,
              GLdouble top, GLdouble nearVal, GLdouble farVal);
WRAP(void, glFrustum, GLdouble left, GLdouble right, GLdouble bottom,
              GLdouble top, GLdouble nearVal, GLdouble farVal);
//WRAP(void, gluLookAt, GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
//              GLdouble centerX, GLdouble centerY, GLdouble centerZ,
//              GLdouble upX, GLdouble upY, GLdouble upZ);

WRAP(void, glRenderMode, GLenum mode);
WRAP(void, glSelectBuffer, GLsizei size, GLuint *buffer);
WRAP(void, glInitNames, void);
WRAP(void, glPushName, GLuint name);
WRAP(void, glPopName, void);
WRAP(void, glLoadName, GLuint name);

WRAP(void, glGenBuffers, GLsizei n, GLuint *buffers);
WRAP(void, glDeleteBuffers, GLsizei n, const GLuint *buffers);
WRAP(void, glBindBuffer, GLenum target, GLuint buffer);
WRAP(void, glBufferData, GLenum target, GLsizeiptr size,
              const GLvoid *data, GLenum usage);
WRAP(void, glBufferSubData, GLenum target, GLintptr offset,
              GLsizeiptr size, const GLvoid *data);
WRAP(void, glVertexPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP(void, glNormalPointer, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP(void, glColorPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP(void, glTexCoordPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP(void, glEnableClientState, GLenum array);
WRAP(void, glDisableClientState, GLenum array);
WRAP(void, glDrawArrays, GLenum mode, GLint first, GLsizei count);
WRAP(void, glDrawElements, GLenum mode, GLsizei count, GLenum type,
              const GLvoid *indices);

WRAP(GLenum, glGetError, void);
WRAP(void, glGetIntegerv, GLenum pname, GLint *params);
WRAP(void, glGetFloatv, GLenum pname, GLfloat *params);
WRAP(void, glGetDoublev, GLenum pname, GLdouble *params);
WRAP(void, glGetBooleanv, GLenum pname, GLboolean *params);

WRAP(const GLubyte*, glGetString, GLenum name);
WRAP(void, glGetPointerv, GLenum pname, GLvoid **params);

WRAP(void, glStencilFunc, GLenum func, GLint ref, GLuint mask);
WRAP(void, glStencilOp, GLenum fail, GLenum zfail, GLenum zpass);
WRAP(void, glDepthFunc, GLenum func);
WRAP(void, glDepthMask, GLboolean flag);

WRAP(void, glBlendFunc, GLenum sfactor, GLenum dfactor);
WRAP(void, glBlendEquation, GLenum mode);

WRAP(void, glPolygonMode, GLenum face, GLenum mode);
WRAP(void, glCullFace, GLenum mode);
WRAP(void, glFrontFace, GLenum mode);

WRAP(void, glFlush, void);
WRAP(void, glFinish, void);
WRAP(void, glXSwapBuffers, void);

WRAP(XVisualInfo*, glXChooseVisual, Display *dpy, int screen, int *attribList);
WRAP(void, glXDestroyContext, Display *dpy, GLXContext ctx);
WRAP(void, glXGetProcAddressARB, const GLubyte *procName);
WRAP(int, glXQueryDrawable, Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
WRAP(Bool, glXMakeCurrent, Display *dpy, GLXDrawable drawable, GLXContext ctx);
WRAP(GLXContext, glXCreateContext, Display *dpy, XVisualInfo* vis, GLXContext shareList, Bool direct);