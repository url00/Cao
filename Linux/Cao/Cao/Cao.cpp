#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include "../../../Cross/Cross.h"
#include "Cao.h"



void Draw(cairo_t *cr, int width, int height) {
    int quarter_w = width / 4;
    int quarter_h = height / 4;
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_rectangle(cr, quarter_w, quarter_h, quarter_w * 2, quarter_h * 2);
    cairo_fill(cr);
}

int main(int argc, char **argv)
{
    struct timespec ts = {0, 5000000};

    Display *display = XOpenDisplay(NULL);
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    XCompositeRedirectSubwindows(display, root, CompositeRedirectAutomatic);
    XSelectInput(display, root, SubstructureNotifyMask);

    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);



    Window overlay = XCompositeGetOverlayWindow(display, root);
    // Allow input pass-through for this overlay window.
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);

    XFixesSetWindowShapeRegion(display, overlay, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(display, overlay, ShapeInput, 0, 0, region);

    XFixesDestroyRegion(display, region);



    cairo_surface_t *surf = cairo_xlib_surface_create(display, overlay, DefaultVisual(display, screen), width, height);
    cairo_t *cr = cairo_create(surf);

    XSelectInput(display, overlay, ExposureMask);

    Draw(cr, width, height);

    XEvent ev;
    while(1) {
      overlay = XCompositeGetOverlayWindow(display, root);
      Draw(cr, width, height);
      XCompositeReleaseOverlayWindow(display, root);
      nanosleep(&ts, NULL);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    XCloseDisplay(display);
    return 0; 


    // Global hotkeys.
    /*
    {
        Display* dpy = XOpenDisplay(0);
        Window root = DefaultRootWindow(dpy);
        XEvent ev;

        unsigned int modifiers = ControlMask | ShiftMask;
        int keycode = XKeysymToKeycode(dpy,XK_Y);
        Window grab_window = root;
        Bool owner_events = False;
        int pointer_mode = GrabModeAsync;
        int keyboard_mode = GrabModeAsync;

        XGrabKey(
            dpy,
            keycode,
            modifiers,
            grab_window,
            owner_events,
            pointer_mode,
            keyboard_mode);

        XSelectInput(dpy, root, KeyPressMask);
        while(true)
        {
            bool shouldQuit = false;
            XNextEvent(dpy, &ev);
            switch(ev.type)
            {
                case KeyPress:
                    std::cout << "Hot key pressed!" << std::endl;
                    XUngrabKey(dpy,keycode,modifiers,grab_window);
                    shouldQuit = true;

                default:
                    break;
            }

            if(shouldQuit)
                break;
        }

        XCloseDisplay(dpy);
    }
    */



    return 0;
}



void MainWindow_DrawTestRect(int x, int y, int height, int width)
{
}
