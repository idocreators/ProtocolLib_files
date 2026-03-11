#include <gtk/gtk.h>
#include "mkWatchFace.h"

static void activate(GtkApplication *app, gpointer user_data) {
    MkWatchFace *win = mk_watch_face_new(app);
    gtk_widget_show_all(GTK_WIDGET(win));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.idoosmart.watchdial", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}