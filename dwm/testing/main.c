#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#define W_WIDTH 640
#define W_HEIGHT 480

#define X_POS 100
#define Y_POS 120
#define BORDER_WIDTH 2
#define CORNERRAD 4



void testRoundCorners(Window w, Display *dpy){
  XWindowAttributes wa;
  XGetWindowAttributes(dpy, w, &wa);

  // If this returns null, the window is invalid.
  if (!XGetWindowAttributes(dpy, w, &wa))
    return;

  int width = BORDER_WIDTH * 2 + wa.width;
  int height = BORDER_WIDTH * 2 + wa.height;
  int rad = CORNERRAD;
  int dia = 2 * rad;

  // do not try to round if the window would be smaller than the corners
  if (width < dia || height < dia)
    return;

  Pixmap mask = XCreatePixmap(dpy, w, width, height, 1);
  // if this returns null, the mask is not drawable
  if (!mask)
    return;

  XGCValues xgcv;
  GC shape_gc = XCreateGC(dpy, mask, 0, &xgcv);
  if (!shape_gc) {
    XFreePixmap(dpy, mask);
    return;
  }

  XSetForeground(dpy, shape_gc, 0);
  XFillRectangle(dpy, mask, shape_gc, 0, 0, width, height);
  XSetForeground(dpy, shape_gc, 1);
  XFillArc(dpy, mask, shape_gc, 0, 0, dia, dia, 0, 23040); //360*64 -> circle
  XFillArc(dpy, mask, shape_gc, width - dia - 1, 0, dia, dia, 0, 23040);
  XFillArc(dpy, mask, shape_gc, 0, height - dia - 1, dia, dia, 0, 23040);
  XFillArc(dpy, mask, shape_gc, width - dia - 1, height - dia - 1, dia, dia, 0,
           23040);
  XFillRectangle(dpy, mask, shape_gc, rad, 0, width - dia, height);
  XFillRectangle(dpy, mask, shape_gc, 0, rad, width, height - dia);
  XShapeCombineMask(dpy, w, ShapeBounding, 0 - wa.border_width,
                    0 - wa.border_width, mask, ShapeSet);
  XFreePixmap(dpy, mask);
  XFreeGC(dpy, shape_gc);
}

int main(int argc, char *argv[]) {
    XRectangle rectangles[4] = {
        { X_POS, Y_POS, W_WIDTH, BORDER_WIDTH },
        { X_POS, Y_POS, BORDER_WIDTH, W_HEIGHT },
        { X_POS, W_HEIGHT - BORDER_WIDTH, W_WIDTH, BORDER_WIDTH },
        { W_WIDTH - BORDER_WIDTH, Y_POS, BORDER_WIDTH, W_HEIGHT }
    };
    Display *dpy = XOpenDisplay(NULL);
    XSetWindowAttributes attr = {0};
    XGCValues gcv = {0};
    XVisualInfo vinfo;
    GC gc;

    Window w;

    int run = 1;

    XMatchVisualInfo(dpy, DefaultScreen(dpy), 32, TrueColor, &vinfo);
    attr.colormap = XCreateColormap(dpy, DefaultRootWindow(dpy), vinfo.visual, AllocNone);

    XColor color;
    char orangeDark[] = "#FF8000";
    XParseColor(dpy, attr.colormap, orangeDark, &color);
    XAllocColor(dpy, attr.colormap, &color);

    w = XCreateWindow(dpy, DefaultRootWindow(dpy), X_POS, Y_POS,
                      W_WIDTH, W_HEIGHT, BORDER_WIDTH, vinfo.depth,
                      InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);

    gcv.line_width = BORDER_WIDTH;
    gc = XCreateGC(dpy, w, GCLineWidth, &gcv);

    XSelectInput(dpy, w, ExposureMask);
    long value = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(dpy, w, XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False),
                    XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);
    XMapWindow(dpy, w);
    XSync(dpy, False);

    while(run) {
        XEvent xe;
        XNextEvent(dpy, &xe);
        switch (xe.type) {
            case Expose:
                XSetForeground(dpy, gc, color.pixel);
                XDrawRectangles(dpy, w, gc, rectangles, 4);
                XFillRectangles(dpy, w, gc, rectangles, 4);
                testRoundCorners(w, dpy);
                XSync(dpy, False);
                break;

            default:
                break;
        }
    }

    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}