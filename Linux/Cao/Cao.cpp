#include "Cao.h"

#include <iostream>
#include <cstdlib>

#include <gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Cross.h"

static GApplication *app;
static GtkApplication *app_gtk;

static void
activate (
    GtkApplication* __app,
    gpointer        user_data)
{
    GtkWidget *window_widget;
    window_widget = gtk_application_window_new(app_gtk);
    GtkWindow *window = (GtkWindow*)window_widget;
    gtk_window_set_title(window, "Cao");
    gtk_window_set_default_size(window, 500, 50);
    gtk_widget_show_all(window_widget);



    GNotification *notification;
    notification = g_notification_new ("Lunch is ready");
    g_notification_set_body (notification, "Today we have pancakes and salad, and fruit and cake for dessert");
    g_application_send_notification(app, "test", notification);
}

int
main (
    int    argc,
    char **argv)
{
    app_gtk = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    app = (GApplication*)app_gtk;

    g_signal_connect(app_gtk, "activate", G_CALLBACK(activate), NULL);




    int status;
    status = g_application_run(app, argc, argv);




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




    g_object_unref (app);
    return status;
}
