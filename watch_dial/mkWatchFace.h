#ifndef MKWATCHFACE_H
#define MKWATCHFACE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MK_TYPE_WATCH_FACE (mk_watch_face_get_type())
#define MK_WATCH_FACE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MK_TYPE_WATCH_FACE, MkWatchFace))
#define MK_WATCH_FACE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MK_TYPE_WATCH_FACE, MkWatchFaceClass))
#define MK_IS_WATCH_FACE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MK_TYPE_WATCH_FACE))
#define MK_IS_WATCH_FACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MK_TYPE_WATCH_FACE))
#define MK_WATCH_FACE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MK_TYPE_WATCH_FACE, MkWatchFaceClass))

typedef struct _MkWatchFace MkWatchFace;
typedef struct _MkWatchFaceClass MkWatchFaceClass;
typedef struct _MkWatchFacePrivate MkWatchFacePrivate;

struct _MkWatchFace {
    GtkApplicationWindow parent;
    MkWatchFacePrivate *priv;
};

struct _MkWatchFaceClass {
    GtkApplicationWindowClass parent_class;
};

GType mk_watch_face_get_type(void) G_GNUC_CONST;
MkWatchFace* mk_watch_face_new(GtkApplication *app);

/* Public methods */
void mk_watch_face_load_iwf_json(MkWatchFace *win, const gchar *filename);
void mk_watch_face_save_iwf_json(MkWatchFace *win, const gchar *filename);
void mk_watch_face_load_font_json(MkWatchFace *win, const gchar *filename);
void mk_watch_face_save_font_json(MkWatchFace *win, const gchar *filename);
void mk_watch_face_export_iwf(MkWatchFace *win, const gchar *filename);
void mk_watch_face_export_iwf_lz(MkWatchFace *win, const gchar *filename);
void mk_watch_face_new_dial(MkWatchFace *win);
void mk_watch_face_update_preview(MkWatchFace *win);
void on_unknown_clicked(GtkButton *button, gpointer user_data);
void on_save_font_clicked(GtkButton *button, gpointer user_data);

/* Signal handlers */
void on_load_json_clicked(GtkButton *button, gpointer user_data);
void on_save_json_clicked(GtkButton *button, gpointer user_data);
void on_apply_preview_json_clicked(GtkButton *button, gpointer user_data);
void on_add_hands_clicked(GtkButton *button, gpointer user_data);
void on_add_bg_clicked(GtkButton *button, gpointer user_data);
void on_save_preview_clicked(GtkButton *button, gpointer user_data);
void on_create_launch_image_clicked(GtkButton *button, gpointer user_data);
void on_add_widget_clicked(GtkButton *button, gpointer user_data);
void on_remove_widget_clicked(GtkButton *button, gpointer user_data);
void on_edit_colors_clicked(GtkButton *button, gpointer user_data);
void on_apply_iwf_clicked(GtkButton *button, gpointer user_data);
void on_apply_font_clicked(GtkButton *button, gpointer user_data);
void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data);
void on_action_exit(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_action_new_dial(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_action_about(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_action_save_iwf(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_action_save_iwf_lz(GSimpleAction *action, GVariant *parameter, gpointer user_data);

/* Drawing function */
gboolean on_preview_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

G_END_DECLS

#endif /* MKWATCHFACE_H */