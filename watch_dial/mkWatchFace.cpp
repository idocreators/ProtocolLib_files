#include "mkWatchFace.h"
#include <string.h>
#include <time.h>
#include <math.h>

struct _MkWatchFacePrivate {
    /* Main widgets */
    GtkWidget *main_box;
    GtkWidget *tab_widget;
    GtkWidget *preview_tab;
    GtkWidget *editor_tab;
    GtkWidget *about_tab;
    
    /* Preview tab widgets */
    GtkWidget *preview_canvas;
    GtkWidget *bottom_tabs;
    GtkWidget *tree_view;
    GtkWidget *tree_scroll;
    GtkTreeStore *tree_store;
    GtkWidget *preview_json_edit;
    GtkWidget *load_json_btn;
    GtkWidget *save_json_btn;
    GtkWidget *add_hands_btn;
    GtkWidget *add_bg_btn;
    GtkWidget *save_preview_btn;
    GtkWidget *create_launch_btn;
    GtkWidget *unknown_btn;
    
    /* Add this line */
    GtkWidget *apply_preview_json_btn;
    
    /* Group boxes for ring/progressbar */
    GtkWidget *ring_group;
    GtkWidget *ring_combo;
    GtkWidget *ring_edit_colors_btn;
    GtkWidget *progress_group;
    GtkWidget *progress_combo;
    GtkWidget *progress_edit_colors_btn;
    
    /* Editor tab widgets */
    GtkWidget *right_editor_tabs;
    GtkWidget *iwf_text_edit;
    GtkWidget *font_text_edit;
    GtkWidget *apply_iwf_btn;
    GtkWidget *apply_font_btn;
    GtkWidget *save_font_btn;
    
    /* Widget group boxes */
    GtkWidget *custom_widget_group;
    GtkWidget *ring_widget_group;
    GtkWidget *progress_widget_group;
    GtkWidget *custom_widget_combo;
    GtkWidget *ring_widget_combo;
    GtkWidget *progress_widget_combo;
    GtkWidget *add_custom_btn;
    GtkWidget *remove_custom_btn;
    GtkWidget *add_ring_btn;
    GtkWidget *remove_ring_btn;
    GtkWidget *add_progress_btn;
    GtkWidget *remove_progress_btn;
    
    /* About tab */
    GtkWidget *about_text;
    
    /* Data */
    gchar *iwf_json_content;
    gchar *font_json_content;
    
    /* Actions */
    GSimpleActionGroup *action_group;
};

G_DEFINE_TYPE_WITH_PRIVATE(MkWatchFace, mk_watch_face, GTK_TYPE_APPLICATION_WINDOW)

static void setup_actions(MkWatchFace *win) {
    MkWatchFacePrivate *priv = win->priv;
    const GActionEntry win_entries[] = {
        { "exit", on_action_exit, NULL, NULL, NULL },
        { "new_dial", on_action_new_dial, NULL, NULL, NULL },
        { "about", on_action_about, NULL, NULL, NULL },
        { "save_iwf", on_action_save_iwf, NULL, NULL, NULL },
        { "save_iwf_lz", on_action_save_iwf_lz, NULL, NULL, NULL }
    };
    
    priv->action_group = g_simple_action_group_new();
    g_action_map_add_action_entries(G_ACTION_MAP(priv->action_group),
                                     win_entries, G_N_ELEMENTS(win_entries),
                                     win);
    gtk_widget_insert_action_group(GTK_WIDGET(win), "win",
                                   G_ACTION_GROUP(priv->action_group));
}

static void setup_menu(MkWatchFace *win) {
    MkWatchFacePrivate *priv = win->priv;
    
    /* Create menu structure */
    GMenu *menubar = g_menu_new();
    
    /* File menu */
    GMenu *file_menu = g_menu_new();
    g_menu_append(file_menu, "Save .iwf", "win.save_iwf");
    g_menu_append(file_menu, "Save .iwf.lz", "win.save_iwf_lz");
    
    /* Add separator */
    GMenuItem *sep = g_menu_item_new(NULL, NULL);
    g_menu_item_set_attribute(sep, "display-hint", "s", "separator");
    g_menu_append_item(file_menu, sep);
    g_object_unref(sep);
    
    g_menu_append(file_menu, "Exit", "win.exit");
    g_menu_append_submenu(menubar, "File", G_MENU_MODEL(file_menu));
    
    /* Edit menu */
    GMenu *edit_menu = g_menu_new();
    g_menu_append(edit_menu, "New Dial", "win.new_dial");
    g_menu_append_submenu(menubar, "Edit", G_MENU_MODEL(edit_menu));
    
    /* Help menu */
    GMenu *help_menu = g_menu_new();
    g_menu_append(help_menu, "About", "win.about");
    g_menu_append_submenu(menubar, "Help", G_MENU_MODEL(help_menu));
    
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(win), TRUE);
    g_object_unref(menubar);
}

static void setup_preview_tab(MkWatchFace *win) {
    MkWatchFacePrivate *priv = win->priv;
    
    priv->preview_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    /* Top section with preview canvas and bottom tabs */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
    
    /* Preview canvas container - to center the 466x466 canvas */
    GtkWidget *canvas_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(canvas_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(canvas_container, GTK_ALIGN_CENTER);
    
    /* Preview canvas - 466x466 black square */
    priv->preview_canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(priv->preview_canvas, 466, 466);
    gtk_widget_set_name(priv->preview_canvas, "preview-canvas");
    g_signal_connect(priv->preview_canvas, "draw", G_CALLBACK(on_preview_draw), win);
    
    gtk_box_pack_start(GTK_BOX(canvas_container), priv->preview_canvas, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), canvas_container, FALSE, FALSE, 0);
    
    /* Bottom tabs (unchanged) */
    priv->bottom_tabs = gtk_notebook_new();
    gtk_widget_set_size_request(priv->bottom_tabs, 641, 661);
    
    /* Tree tab */
    GtkWidget *tree_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *tree_label = gtk_label_new("Tree iwf.json");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->bottom_tabs), tree_tab, tree_label);
    
    GtkWidget *tree_header = gtk_label_new("iwf.json tree (Cannot be edited, use raw editor instead)");
    gtk_box_pack_start(GTK_BOX(tree_tab), tree_header, FALSE, FALSE, 0);
    
    /* Tree view */
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    priv->tree_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    priv->tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(priv->tree_store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Property",
                                                                         renderer,
                                                                         "text", 0,
                                                                         NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(priv->tree_view), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Value",
                                                      renderer,
                                                      "text", 1,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(priv->tree_view), column);
    
    gtk_container_add(GTK_CONTAINER(scrolled), priv->tree_view);
    gtk_box_pack_start(GTK_BOX(tree_tab), scrolled, TRUE, TRUE, 0);
    
    /* JSON tab */
    GtkWidget *json_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *json_label = gtk_label_new("Raw iwf.json");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->bottom_tabs), json_tab, json_label);
    
    GtkWidget *json_header = gtk_label_new("Raw iwf.json editor");
    gtk_box_pack_start(GTK_BOX(json_tab), json_header, FALSE, FALSE, 0);
    
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    priv->preview_json_edit = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(priv->preview_json_edit), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), priv->preview_json_edit);
    gtk_box_pack_start(GTK_BOX(json_tab), scrolled, TRUE, TRUE, 0);
    
    priv->apply_preview_json_btn = gtk_button_new_with_label("Apply IWF.JSON Changes");
    gtk_box_pack_start(GTK_BOX(json_tab), priv->apply_preview_json_btn, FALSE, FALSE, 0);
    g_signal_connect(priv->apply_preview_json_btn, "clicked",
                    G_CALLBACK(on_apply_preview_json_clicked), win);
    
    gtk_box_pack_start(GTK_BOX(hbox), priv->bottom_tabs, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), hbox, FALSE, FALSE, 0);
    
    /* Button row */
    GtkWidget *button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    priv->load_json_btn = gtk_button_new_with_label("Load iwf.json");
    g_signal_connect(priv->load_json_btn, "clicked", G_CALLBACK(on_load_json_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->load_json_btn, TRUE, TRUE, 0);
    
    priv->save_json_btn = gtk_button_new_with_label("Save iwf.json");
    g_signal_connect(priv->save_json_btn, "clicked", G_CALLBACK(on_save_json_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->save_json_btn, TRUE, TRUE, 0);
    
    priv->save_preview_btn = gtk_button_new_with_label("Create Preview");
    g_signal_connect(priv->save_preview_btn, "clicked", G_CALLBACK(on_save_preview_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->save_preview_btn, TRUE, TRUE, 0);
    
    priv->add_bg_btn = gtk_button_new_with_label("Add Background");
    g_signal_connect(priv->add_bg_btn, "clicked", G_CALLBACK(on_add_bg_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->add_bg_btn, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), button_row, FALSE, FALSE, 0);
    
    /* Second button row */
    button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    priv->add_hands_btn = gtk_button_new_with_label("Add Clock Hands");
    g_signal_connect(priv->add_hands_btn, "clicked", G_CALLBACK(on_add_hands_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->add_hands_btn, TRUE, TRUE, 0);
    
    priv->unknown_btn = gtk_button_new_with_label("???");
    g_signal_connect(priv->unknown_btn, "clicked", G_CALLBACK(on_unknown_clicked), win);
    gtk_box_pack_start(GTK_BOX(button_row), priv->unknown_btn, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), button_row, FALSE, FALSE, 0);
    
    /* Separator */
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                      FALSE, FALSE, 5);
    
    /* Ring and Progressbar groups */
    GtkWidget *groups_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    /* Ring group */
    priv->ring_group = gtk_frame_new("Ring");
    GtkWidget *ring_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(priv->ring_group), ring_vbox);
    
    priv->ring_combo = gtk_combo_box_text_new();
    const gchar *ring_types[] = {"heartrate", "calorie", "distance", "step",
                                  "battery", "exercise", "walk", NULL};
    for (int i = 0; ring_types[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv->ring_combo), ring_types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->ring_combo), 0);
    gtk_box_pack_start(GTK_BOX(ring_vbox), priv->ring_combo, FALSE, FALSE, 0);
    
    priv->ring_edit_colors_btn = gtk_button_new_with_label("Edit Colors");
    g_signal_connect(priv->ring_edit_colors_btn, "clicked",
                    G_CALLBACK(on_edit_colors_clicked), win);
    gtk_box_pack_start(GTK_BOX(ring_vbox), priv->ring_edit_colors_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(groups_row), priv->ring_group, TRUE, TRUE, 0);
    
    /* Progressbar group */
    priv->progress_group = gtk_frame_new("Progressbar");
    GtkWidget *progress_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(priv->progress_group), progress_vbox);
    
    priv->progress_combo = gtk_combo_box_text_new();
    for (int i = 0; ring_types[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv->progress_combo), ring_types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->progress_combo), 0);
    gtk_box_pack_start(GTK_BOX(progress_vbox), priv->progress_combo, FALSE, FALSE, 0);
    
    priv->progress_edit_colors_btn = gtk_button_new_with_label("Edit Colors");
    g_signal_connect(priv->progress_edit_colors_btn, "clicked",
                    G_CALLBACK(on_edit_colors_clicked), win);
    gtk_box_pack_start(GTK_BOX(progress_vbox), priv->progress_edit_colors_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(groups_row), priv->progress_group, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), groups_row, FALSE, FALSE, 0);
    
    /* Create launch image button */
    priv->create_launch_btn = gtk_button_new_with_label("Create Launch Image");
    g_signal_connect(priv->create_launch_btn, "clicked",
                    G_CALLBACK(on_create_launch_image_clicked), win);
    gtk_box_pack_start(GTK_BOX(priv->preview_tab), priv->create_launch_btn, FALSE, FALSE, 0);
}

/* Move the draw function HERE, outside of setup_preview_tab */
gboolean on_preview_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    MkWatchFace *win = MK_WATCH_FACE(user_data);
    MkWatchFacePrivate *priv = win->priv;
    
    /* Get the widget dimensions */
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    /* Fill with black background */
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); /* Black */
    cairo_paint(cr);
    
    /* Draw a subtle border to show the edge */
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2); /* Dark gray */
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0.5, 0.5, width - 1, height - 1);
    cairo_stroke(cr);
    
    return FALSE;
}

static void setup_editor_tab(MkWatchFace *win) {
    MkWatchFacePrivate *priv = win->priv;
    
    priv->editor_tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    /* Left panel */
    GtkWidget *left_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    /* Add Custom Widget group */
    priv->custom_widget_group = gtk_frame_new("Add Custom Widget");
    GtkWidget *custom_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(priv->custom_widget_group), custom_vbox);
    
    priv->custom_widget_combo = gtk_combo_box_text_new();
    const gchar *custom_types[] = {"time", "date", "day", "second", "hour", "hourhi",
                                   "hourlo", "min", "minhi", "minlo", "year", "heartrate",
                                   "calorie", "distance", "step", "battery", "exercise",
                                   "walk", "weather", "apm", "anima", "gradient", 
                                   "units", "multimeter", "bluetooth", "sleep", 
                                   "redpoint", "shortcut", "anima", "icon", NULL};
    for (int i = 0; custom_types[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv->custom_widget_combo), custom_types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->custom_widget_combo), 0);
    gtk_box_pack_start(GTK_BOX(custom_vbox), priv->custom_widget_combo, FALSE, FALSE, 0);
    
    priv->add_custom_btn = gtk_button_new_with_label("Add Custom Widget");
    g_signal_connect(priv->add_custom_btn, "clicked", G_CALLBACK(on_add_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(custom_vbox), priv->add_custom_btn, FALSE, FALSE, 0);
    
    priv->remove_custom_btn = gtk_button_new_with_label("Remove Custom Widget");
    g_signal_connect(priv->remove_custom_btn, "clicked", G_CALLBACK(on_remove_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(custom_vbox), priv->remove_custom_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(left_panel), priv->custom_widget_group, FALSE, FALSE, 0);
    
    /* Add Ring Widget group */
    priv->ring_widget_group = gtk_frame_new("Add Ring Widget");
    GtkWidget *ring_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(priv->ring_widget_group), ring_vbox);
    
    priv->ring_widget_combo = gtk_combo_box_text_new();
    const gchar *ring_types[] = {"heartrate", "calorie", "distance", "step",
                                  "battery", "exercise", "walk", NULL};
    for (int i = 0; ring_types[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv->ring_widget_combo), ring_types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->ring_widget_combo), 0);
    gtk_box_pack_start(GTK_BOX(ring_vbox), priv->ring_widget_combo, FALSE, FALSE, 0);
    
    priv->add_ring_btn = gtk_button_new_with_label("Add Ring Widget");
    g_signal_connect(priv->add_ring_btn, "clicked", G_CALLBACK(on_add_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(ring_vbox), priv->add_ring_btn, FALSE, FALSE, 0);
    
    priv->remove_ring_btn = gtk_button_new_with_label("Remove Ring Widget");
    g_signal_connect(priv->remove_ring_btn, "clicked", G_CALLBACK(on_remove_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(ring_vbox), priv->remove_ring_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(left_panel), priv->ring_widget_group, FALSE, FALSE, 0);
    
    /* Add Progressbar Widget group */
    priv->progress_widget_group = gtk_frame_new("Add Progressbar Widget");
    GtkWidget *progress_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(priv->progress_widget_group), progress_vbox);
    
    priv->progress_widget_combo = gtk_combo_box_text_new();
    for (int i = 0; ring_types[i]; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priv->progress_widget_combo), ring_types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(priv->progress_widget_combo), 0);
    gtk_box_pack_start(GTK_BOX(progress_vbox), priv->progress_widget_combo, FALSE, FALSE, 0);
    
    priv->add_progress_btn = gtk_button_new_with_label("Add Progressbar Widget");
    g_signal_connect(priv->add_progress_btn, "clicked", G_CALLBACK(on_add_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(progress_vbox), priv->add_progress_btn, FALSE, FALSE, 0);
    
    priv->remove_progress_btn = gtk_button_new_with_label("Remove Progressbar Widget");
    g_signal_connect(priv->remove_progress_btn, "clicked", G_CALLBACK(on_remove_widget_clicked), win);
    gtk_box_pack_start(GTK_BOX(progress_vbox), priv->remove_progress_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(left_panel), priv->progress_widget_group, FALSE, FALSE, 0);
    
    /* Save font button */
    priv->save_font_btn = gtk_button_new_with_label("Save font.json");
    g_signal_connect(priv->save_font_btn, "clicked", G_CALLBACK(on_save_font_clicked), win);
    gtk_box_pack_start(GTK_BOX(left_panel), priv->save_font_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(priv->editor_tab), left_panel, FALSE, FALSE, 0);
    
    /* Right panel with tabs */
    priv->right_editor_tabs = gtk_notebook_new();
    gtk_widget_set_size_request(priv->right_editor_tabs, 741, 661);
    
    /* iwf.json tab */
    GtkWidget *iwf_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *iwf_label = gtk_label_new("iwf.json");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->right_editor_tabs), iwf_tab, iwf_label);
    
    GtkWidget *iwf_header = gtk_label_new("Raw iwf.json editor");
    gtk_box_pack_start(GTK_BOX(iwf_tab), iwf_header, FALSE, FALSE, 0);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    priv->iwf_text_edit = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(priv->iwf_text_edit), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), priv->iwf_text_edit);
    gtk_box_pack_start(GTK_BOX(iwf_tab), scrolled, TRUE, TRUE, 0);
    
    priv->apply_iwf_btn = gtk_button_new_with_label("Apply IWF.JSON Changes");
    g_signal_connect(priv->apply_iwf_btn, "clicked", G_CALLBACK(on_apply_iwf_clicked), win);
    gtk_box_pack_start(GTK_BOX(iwf_tab), priv->apply_iwf_btn, FALSE, FALSE, 0);
    
    /* font.json tab */
    GtkWidget *font_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *font_label = gtk_label_new("font.json");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->right_editor_tabs), font_tab, font_label);
    
    GtkWidget *font_header = gtk_label_new("Raw font.json editor");
    gtk_box_pack_start(GTK_BOX(font_tab), font_header, FALSE, FALSE, 0);
    
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    priv->font_text_edit = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(priv->font_text_edit), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), priv->font_text_edit);
    gtk_box_pack_start(GTK_BOX(font_tab), scrolled, TRUE, TRUE, 0);
    
    priv->apply_font_btn = gtk_button_new_with_label("Apply FONT.JSON Changes");
    g_signal_connect(priv->apply_font_btn, "clicked", G_CALLBACK(on_apply_font_clicked), win);
    gtk_box_pack_start(GTK_BOX(font_tab), priv->apply_font_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(priv->editor_tab), priv->right_editor_tabs, TRUE, TRUE, 0);
}

static void setup_about_tab(MkWatchFace *win) {
    MkWatchFacePrivate *priv = win->priv;
    
    priv->about_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    priv->about_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->about_text), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->about_text), FALSE);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->about_text));
    
    const gchar *about_content = 
        "About \"mkWatchFace\"\n\n"
        "Version 1.0.0\n\n"
        "Developed by:\n"
        "• TBA - Publisher\n"
        "• TBA - Source Code\n"
        "• TBA - UI Designer\n"
        "• TBA - App Tester";
    
    gtk_text_buffer_set_text(buffer, about_content, -1);
    
    gtk_container_add(GTK_CONTAINER(scrolled), priv->about_text);
    gtk_box_pack_start(GTK_BOX(priv->about_tab), scrolled, TRUE, TRUE, 0);
}

static void mk_watch_face_init(MkWatchFace *win) {
    MkWatchFacePrivate *priv;
    
    /* Fix the cast here */
    win->priv = (MkWatchFacePrivate*)mk_watch_face_get_instance_private(win);
    priv = win->priv;
    
    /* Initialize private members */
    priv->iwf_json_content = NULL;
    priv->font_json_content = NULL;
    
    /* Create main layout */
    priv->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), priv->main_box);
    
    /* Create notebook (tab widget) */
    priv->tab_widget = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(priv->main_box), priv->tab_widget, TRUE, TRUE, 0);
    
    /* Setup tabs */
    setup_preview_tab(win);
    setup_editor_tab(win);
    setup_about_tab(win);
    
    /* Add tabs to notebook */
    GtkWidget *preview_label = gtk_label_new("Preview");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->tab_widget), priv->preview_tab, preview_label);
    
    GtkWidget *editor_label = gtk_label_new("Editor");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->tab_widget), priv->editor_tab, editor_label);
    
    GtkWidget *about_label = gtk_label_new("About");
    gtk_notebook_append_page(GTK_NOTEBOOK(priv->tab_widget), priv->about_tab, about_label);
    
    /* Setup actions and menu */
    setup_actions(win);
    setup_menu(win);
    
    /* Apply CSS */
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "#preview-canvas { background-color: #111; border: 1px solid #555; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void mk_watch_face_dispose(GObject *object) {
    MkWatchFace *win = MK_WATCH_FACE(object);
    MkWatchFacePrivate *priv = win->priv;
    
    if (priv->iwf_json_content) {
        g_free(priv->iwf_json_content);
        priv->iwf_json_content = NULL;
    }
    
    if (priv->font_json_content) {
        g_free(priv->font_json_content);
        priv->font_json_content = NULL;
    }
    
    G_OBJECT_CLASS(mk_watch_face_parent_class)->dispose(object);
}

static void mk_watch_face_class_init(MkWatchFaceClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    
    object_class->dispose = mk_watch_face_dispose;
}

MkWatchFace* mk_watch_face_new(GtkApplication *app) {
    return MK_WATCH_FACE(g_object_new(MK_TYPE_WATCH_FACE,
                                      "application", app,
                                      "title", "mkWatchFace",
                                      "default-width", 980,
                                      "default-height", 768,
                                      "resizable", TRUE,
                                      NULL));
}

/* Signal handler stubs - to be implemented later */
void on_load_json_clicked(GtkButton *button, gpointer user_data) {
    g_print("Load JSON clicked - to be implemented\n");
}

void on_save_json_clicked(GtkButton *button, gpointer user_data) {
    g_print("Save JSON clicked - to be implemented\n");
}

void on_apply_preview_json_clicked(GtkButton *button, gpointer user_data) {
    g_print("Apply Preview JSON clicked - to be implemented\n");
}

void on_add_hands_clicked(GtkButton *button, gpointer user_data) {
    g_print("Add Hands clicked - to be implemented\n");
}

void on_add_bg_clicked(GtkButton *button, gpointer user_data) {
    g_print("Add Background clicked - to be implemented\n");
}

void on_save_preview_clicked(GtkButton *button, gpointer user_data) {
    g_print("Create Preview clicked - to be implemented\n");
}

void on_create_launch_image_clicked(GtkButton *button, gpointer user_data) {
    g_print("Create Launch Image clicked - to be implemented\n");
}

void on_add_widget_clicked(GtkButton *button, gpointer user_data) {
    g_print("Add Widget clicked - to be implemented\n");
}

void on_remove_widget_clicked(GtkButton *button, gpointer user_data) {
    g_print("Remove Widget clicked - to be implemented\n");
}

void on_edit_colors_clicked(GtkButton *button, gpointer user_data) {
    g_print("Edit Colors clicked - to be implemented\n");
}

void on_apply_iwf_clicked(GtkButton *button, gpointer user_data) {
    g_print("Apply IWF clicked - to be implemented\n");
}

void on_apply_font_clicked(GtkButton *button, gpointer user_data) {
    g_print("Apply Font clicked - to be implemented\n");
}

void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    g_print("Tree selection changed - to be implemented\n");
}

void on_action_exit(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    MkWatchFace *win = MK_WATCH_FACE(user_data);
    gtk_window_close(GTK_WINDOW(win));
}

void on_action_new_dial(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    g_print("New Dial - to be implemented\n");
}

void on_action_about(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    g_print("warn: please check the about page\n");
}

void on_action_save_iwf(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    g_print("Save IWF - to be implemented\n");
}

void on_action_save_iwf_lz(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    g_print("Save IWF LZ - to be implemented\n");
}

/* Add missing function implementations */
void on_unknown_clicked(GtkButton *button, gpointer user_data) {
    g_print("Unknown button clicked - to be implemented\n");
}

void on_save_font_clicked(GtkButton *button, gpointer user_data) {
    g_print("Save font clicked - to be implemented\n");
}