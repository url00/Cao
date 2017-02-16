#include "Cao.h"

#include <cstdlib>
#include <gtk/gtk.h>
#include "Cross.h"

static void
activate (
    GtkApplication* app,
    gpointer        user_data)
{
    srand(time(NULL));
    for (int i = 0; i < 40; i++)
    {
        GtkWidget *window_widget;

        window_widget = gtk_application_window_new(app);
        GtkWindow *window = (GtkWindow*)window_widget;

        gtk_window_set_title(window, "Cao");
        gtk_window_set_default_size(window, 10 + rand() % 200, 10 + rand() % 200);
        gtk_window_move(window, (i * 2) + rand() % 600, (i * 2) + rand() % 600);
        gtk_widget_show_all(window_widget);
    }
}

int
main (
    int    argc,
    char **argv)
{
    GtkApplication *app;
    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

    int status;
    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref (app);

    return status;
}
