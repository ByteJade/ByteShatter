#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

static int x_error_handler(Display *dpy, XErrorEvent *e) {
    char buffer[256];
    XGetErrorText(dpy, e->error_code, buffer, 256);
    printf("X Error: %s (code: %d)\n", buffer, e->error_code);
    return 0;
}

int main() {
    Display *dpy;
    Window win;
    XVisualInfo *visinfo;
    GLXContext ctx;
    int screen;
    
    // Установка обработчика ошибок
    XSetErrorHandler(x_error_handler);
    
    // Открыть дисплей
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        printf("Cannot open display\n");
        return 1;
    }
    printf("Display: %s\n", DisplayString(dpy));
    screen = DefaultScreen(dpy);
    
    // Атрибуты для выбора визуала
    int attribs[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 1,
        None
    };
    
    // Выбрать визуал
    visinfo = glXChooseVisual(dpy, screen, attribs);
    if (!visinfo) {
        printf("glXChooseVisual failed\n");
        XCloseDisplay(dpy);
        return 1;
    }
    printf("Visual: 0x%lx\n", visinfo->visualid);
    
    // Создать простое окно
    Window root = RootWindow(dpy, screen);
    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
    attr.event_mask = ExposureMask;
    win = XCreateWindow(dpy, root, 0, 0, 100, 100, 0,
                        visinfo->depth, InputOutput,
                        visinfo->visual, CWColormap | CWEventMask, &attr);
    
    // Создать контекст (сначала пробуем без прямого рендеринга)
    printf("Creating context...\n");
    ctx = glXCreateContext(dpy, visinfo, NULL, True);
    
    if (!ctx) {
        printf("glXCreateContext failed completely\n");
        XDestroyWindow(dpy, win);
        XFree(visinfo);
        XCloseDisplay(dpy);
        return 1;
    }
    
    printf("Success! Context: %p\n", ctx);
    
    // Сделать контекст текущим
    if (glXMakeCurrent(dpy, win, ctx)) {
        printf("glXMakeCurrent OK\n");
        printf("GL Version: %s\n", glGetString(GL_VERSION));
        printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
        glXMakeCurrent(dpy, None, NULL);
    }
    
    // Очистка
    glXDestroyContext(dpy, ctx);
    XDestroyWindow(dpy, win);
    XFree(visinfo);
    XCloseDisplay(dpy);
    
    return 0;
}