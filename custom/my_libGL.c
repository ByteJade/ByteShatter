#include "wrapper.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

WRAP_NORETURN(void, glMatrixMode, GLenum mode);
WRAP_NORETURN(void, glPushMatrix, void);
WRAP_NORETURN(void, glPopMatrix, void);
WRAP_NORETURN(void, glLoadIdentity, void);
WRAP_NORETURN(void, glLoadMatrixf, const GLfloat *m);
WRAP_NORETURN(void, glMultMatrixf, const GLfloat *m);

WRAP_NORETURN(void, glTranslatef, GLfloat x, GLfloat y, GLfloat z);
WRAP_NORETURN(void, glTranslated, GLdouble x, GLdouble y, GLdouble z);
WRAP_NORETURN(void, glRotatef, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
WRAP_NORETURN(void, glRotated, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
WRAP_NORETURN(void, glScalef, GLfloat x, GLfloat y, GLfloat z);
WRAP_NORETURN(void, glScaled, GLdouble x, GLdouble y, GLdouble z);

WRAP_NORETURN(void, glCallList, GLuint list);
WRAP_NORETURN(void, glCallLists, GLsizei n, GLenum type, const GLvoid *lists);
WRAP_NORETURN(GLuint, glGenLists, GLsizei range);
WRAP_NORETURN(void, glNewList, GLuint list, GLenum mode);
WRAP_NORETURN(void, glEndList, void);
WRAP_NORETURN(void, glDeleteLists, GLuint list, GLsizei range);

WRAP_NORETURN(void, glClear, GLbitfield mask);
WRAP_NORETURN(void, glClearColor, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
WRAP_NORETURN(void, glClearDepth, GLclampd depth);
WRAP_NORETURN(void, glClearStencil, GLint s);

WRAP_NORETURN(void, glDrawBuffer, GLenum mode);
WRAP_NORETURN(void, glReadBuffer, GLenum mode);

WRAP_NORETURN(void, glShadeModel, GLenum mode);
WRAP_NORETURN(void, glEnable, GLenum cap);
WRAP_NORETURN(void, glDisable, GLenum cap);
WRAP_NORETURN(void, glColorMaterial, GLenum face, GLenum mode);

WRAP_NORETURN(void, glNormal3f, GLfloat nx, GLfloat ny, GLfloat nz);
WRAP_NORETURN(void, glNormal3d, GLdouble nx, GLdouble ny, GLdouble nz);
WRAP_NORETURN(void, glNormal3fv, const GLfloat *v);
WRAP_NORETURN(void, glVertex3f, GLfloat x, GLfloat y, GLfloat z);
WRAP_NORETURN(void, glVertex3d, GLdouble x, GLdouble y, GLdouble z);
WRAP_NORETURN(void, glVertex3fv, const GLfloat *v);
WRAP_NORETURN(void, glBegin, GLenum mode);
WRAP_NORETURN(void, glEnd, void);

WRAP_NORETURN(void, glColor3f, GLfloat red, GLfloat green, GLfloat blue);
WRAP_NORETURN(void, glColor3d, GLdouble red, GLdouble green, GLdouble blue);
WRAP_NORETURN(void, glColor3fv, const GLfloat *v);
WRAP_NORETURN(void, glColor4f, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

WRAP_NORETURN(void, glMaterialf, GLenum face, GLenum pname, GLfloat param);
WRAP_NORETURN(void, glMaterialfv, GLenum face, GLenum pname, const GLfloat *params);
WRAP_NORETURN(void, glLightf, GLenum light, GLenum pname, GLfloat param);
WRAP_NORETURN(void, glLightfv, GLenum light, GLenum pname, const GLfloat *params);
WRAP_NORETURN(void, glLightModeli, GLenum pname, GLint param);

WRAP_NORETURN(void, glTexImage2D, GLenum target, GLint level, GLint internalFormat,
              GLsizei width, GLsizei height, GLint border, GLenum format,
              GLenum type, const GLvoid *pixels);
WRAP_NORETURN(void, glTexParameteri, GLenum target, GLenum pname, GLint param);
WRAP_NORETURN(void, glTexParameterf, GLenum target, GLenum pname, GLfloat param);
WRAP_NORETURN(void, glBindTexture, GLenum target, GLuint texture);
WRAP_NORETURN(void, glDeleteTextures, GLsizei n, const GLuint *textures);
WRAP_NORETURN(void, glGenTextures, GLsizei n, GLuint *textures);

WRAP_NORETURN(void, glViewport, GLint x, GLint y, GLsizei width, GLsizei height);
WRAP_NORETURN(void, glOrtho, GLdouble left, GLdouble right, GLdouble bottom,
              GLdouble top, GLdouble nearVal, GLdouble farVal);
WRAP_NORETURN(void, glFrustum, GLdouble left, GLdouble right, GLdouble bottom,
              GLdouble top, GLdouble nearVal, GLdouble farVal);
WRAP_NORETURN(void, glPerspective, GLdouble fovy, GLdouble aspect,
              GLdouble zNear, GLdouble zFar);
//WRAP_NORETURN(void, gluLookAt, GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
//              GLdouble centerX, GLdouble centerY, GLdouble centerZ,
//              GLdouble upX, GLdouble upY, GLdouble upZ);

WRAP_NORETURN(void, glRenderMode, GLenum mode);
WRAP_NORETURN(void, glSelectBuffer, GLsizei size, GLuint *buffer);
WRAP_NORETURN(void, glInitNames, void);
WRAP_NORETURN(void, glPushName, GLuint name);
WRAP_NORETURN(void, glPopName, void);
WRAP_NORETURN(void, glLoadName, GLuint name);

WRAP_NORETURN(void, glGenBuffers, GLsizei n, GLuint *buffers);
WRAP_NORETURN(void, glDeleteBuffers, GLsizei n, const GLuint *buffers);
WRAP_NORETURN(void, glBindBuffer, GLenum target, GLuint buffer);
WRAP_NORETURN(void, glBufferData, GLenum target, GLsizeiptr size,
              const GLvoid *data, GLenum usage);
WRAP_NORETURN(void, glBufferSubData, GLenum target, GLintptr offset,
              GLsizeiptr size, const GLvoid *data);
WRAP_NORETURN(void, glVertexPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP_NORETURN(void, glNormalPointer, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP_NORETURN(void, glColorPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP_NORETURN(void, glTexCoordPointer, GLint size, GLenum type, GLsizei stride,
              const GLvoid *pointer);
WRAP_NORETURN(void, glEnableClientState, GLenum array);
WRAP_NORETURN(void, glDisableClientState, GLenum array);
WRAP_NORETURN(void, glDrawArrays, GLenum mode, GLint first, GLsizei count);
WRAP_NORETURN(void, glDrawElements, GLenum mode, GLsizei count, GLenum type,
              const GLvoid *indices);

WRAP_NORETURN(GLenum, glGetError, void);
WRAP_NORETURN(void, glGetIntegerv, GLenum pname, GLint *params);
WRAP_NORETURN(void, glGetFloatv, GLenum pname, GLfloat *params);
WRAP_NORETURN(void, glGetDoublev, GLenum pname, GLdouble *params);
WRAP_NORETURN(void, glGetBooleanv, GLenum pname, GLboolean *params);

WRAP_NORETURN(const GLubyte*, glGetString, GLenum name);
WRAP_NORETURN(void, glGetPointerv, GLenum pname, GLvoid **params);

WRAP_NORETURN(void, glStencilFunc, GLenum func, GLint ref, GLuint mask);
WRAP_NORETURN(void, glStencilOp, GLenum fail, GLenum zfail, GLenum zpass);
WRAP_NORETURN(void, glDepthFunc, GLenum func);
WRAP_NORETURN(void, glDepthMask, GLboolean flag);

WRAP_NORETURN(void, glBlendFunc, GLenum sfactor, GLenum dfactor);
WRAP_NORETURN(void, glBlendEquation, GLenum mode);

WRAP_NORETURN(void, glPolygonMode, GLenum face, GLenum mode);
WRAP_NORETURN(void, glCullFace, GLenum mode);
WRAP_NORETURN(void, glFrontFace, GLenum mode);

WRAP_NORETURN(void, glFlush, void);
WRAP_NORETURN(void, glFinish, void);
WRAP_NORETURN(void, glutSwapBuffers, void);
