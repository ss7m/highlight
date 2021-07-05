#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>

#define SIZE 60

Display *dpy;
Window root;
int screen;

static void
hl_die(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        exit(EXIT_FAILURE);
}

static void
hl_get_cursor_position(int *x, int *y)
{
        Window w;
        int i;
        XQueryPointer(dpy, root, &w, &w, x, y, &i, &i, (unsigned int *)&i);
        *x -= SIZE / 2;
        *y -= SIZE / 2;
}

int
main(void)
{
        Window window;
        GC gc;
        int mouse_x, mouse_y;
        struct timespec time_sleep = { .tv_sec = 0, .tv_nsec = 10000000L };

        dpy = XOpenDisplay(NULL);
        if (!dpy) {
                hl_die("Unable to open display");
        }
        root = DefaultRootWindow(dpy);
        screen = DefaultScreen(dpy);
        hl_get_cursor_position(&mouse_x, &mouse_y);

        {
                // create window
                XVisualInfo vinfo;
                XSetWindowAttributes attr;

                XMatchVisualInfo(dpy, screen, 32, TrueColor, &vinfo);
                attr.colormap = XCreateColormap(dpy, root, vinfo.visual, AllocNone);
                attr.override_redirect = true;
                // argb, run xcompmgr to get transparency
                attr.background_pixel = 0x00000000;
                window = XCreateWindow(
                        dpy, root,
                        mouse_x, mouse_y,
                        SIZE, SIZE,
                        0, // border width,
                        vinfo.depth,
                        CopyFromParent,
                        vinfo.visual,
                        CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect,
                        &attr
                );
        }

        gc = XCreateGC(dpy, window, 0, NULL);
        XSetForeground(dpy, gc, 0x44aa0000);

        {
                // clicks pass through the window
                // I stole this code from stack overflow
                XRectangle rect;
                XserverRegion region = XFixesCreateRegion(dpy, &rect, 1);
                XFixesSetWindowShapeRegion(dpy, window, ShapeInput, 0, 0, region);
                XFixesDestroyRegion(dpy, region);
        }

        XMapWindow(dpy, window);
        XRaiseWindow(dpy, window);
        XFillArc(dpy, window, gc, 0, 0, SIZE, SIZE, 0, 64 * 360);
        XFlush(dpy);

        for(int i = 0; i < 100; i++) {
                hl_get_cursor_position(&mouse_x, &mouse_y);
                XMoveWindow(dpy, window, mouse_x, mouse_y);
                XRaiseWindow(dpy,window);
                XFlush(dpy);
                nanosleep(&time_sleep, NULL);
        }

        XFreeGC(dpy, gc);
        XDestroyWindow(dpy, window);
        XCloseDisplay(dpy);
}
