/* Exercise async Extras actions with a disposable installer, without downloads. */
#include <glib/gstdio.h>
static gchar *test_extras_program;
#define QUARTZ_EXTRAS_PROGRAM test_extras_program
#define main quartz_settings_main
#include "../src/quartz-settings.c"
#undef main

static gboolean
close_error_dialog(gpointer data)
{
    GList *windows = gtk_window_list_toplevels();
    (void)data;
    for (GList *item = windows; item != NULL; item = item->next) {
        if (GTK_IS_MESSAGE_DIALOG(item->data))
            gtk_dialog_response(GTK_DIALOG(item->data), GTK_RESPONSE_CLOSE);
    }
    g_list_free(windows);
    return G_SOURCE_CONTINUE;
}

static void
wait_for_download(SettingsView *view)
{
    gint64 deadline = g_get_monotonic_time() + 5 * G_TIME_SPAN_SECOND;
    while (view->downloading_extras) {
        g_assert_cmpint(g_get_monotonic_time(), <, deadline);
        while (g_main_context_iteration(NULL, FALSE)) {}
        g_usleep(1000);
    }
    g_assert_true(gtk_widget_get_sensitive(view->extras_button));
    g_assert_false(window_delete_requested(view->window, NULL, view));
}

int
main(int argc, char **argv)
{
    GError *error = NULL;
    gchar *directory = g_dir_make_tmp("quartz-extras-ui-XXXXXX", &error);
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *notebook;
    SettingsView *view;
    guint dialog_timer;

    g_assert_no_error(error);
    gtk_init(&argc, &argv);
    test_extras_program = g_build_filename(directory, "install.sh", NULL);
    app = gtk_application_new("org.quartz.ExtrasTest", G_APPLICATION_NON_UNIQUE);
    g_assert_true(g_application_register(G_APPLICATION(app), NULL, &error));
    g_assert_no_error(error);
    activate(app, NULL);
    window = GTK_WIDGET(gtk_application_get_active_window(app));
    view = g_object_get_data(G_OBJECT(window), "quartz-settings-view");
    notebook = gtk_bin_get_child(GTK_BIN(window));
    g_assert_true(GTK_IS_NOTEBOOK(notebook));
    g_assert_cmpint(gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)), ==, 3);
    g_assert_cmpstr(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),
        gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2)), ==, "Extras");
    g_assert_cmpstr(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),
        gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1)), ==, "Typography");
    {
        TypographyView *typography = g_object_get_data(G_OBJECT(
            gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 1)), "typography-view");
        g_assert_nonnull(typography);
        g_assert_nonnull(typography->catalog);
        for (guint role = 0; role < 3; role++) {
            g_assert_cmpint(gtk_combo_box_get_active(GTK_COMBO_BOX(typography->family[role])), >=, 0);
            g_assert_cmpint(gtk_combo_box_get_active(GTK_COMBO_BOX(typography->size[role])), >=, 0);
            g_assert_nonnull(gtk_label_get_attributes(GTK_LABEL(typography->sample[role])));
        }
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 2);

    g_assert_true(g_file_set_contents(test_extras_program,
        "#!/bin/sh\nsleep 0.1\nexit 0\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(test_extras_program, 0700), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->extras_button));
    g_assert_true(view->downloading_extras);
    g_assert_false(gtk_widget_get_sensitive(view->extras_button));
    g_assert_true(window_delete_requested(window, NULL, view));
    wait_for_download(view);
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->extras_status)),
                                  "Wallpapers added."));

    g_assert_true(g_file_set_contents(test_extras_program,
        "#!/bin/sh\necho 'Test download failure' >&2\nexit 1\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(test_extras_program, 0700), ==, 0);
    dialog_timer = g_timeout_add(20, close_error_dialog, NULL);
    gtk_button_clicked(GTK_BUTTON(view->extras_button));
    wait_for_download(view);
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->extras_status)),
                                  "Download failed."));
    gchar *dither_dir = g_build_filename(directory, "dither", NULL);
    gchar *dither_program = g_build_filename(dither_dir, "install.py", NULL);
    g_assert_cmpint(g_mkdir(dither_dir, 0700), ==, 0);
    g_assert_true(g_file_set_contents(dither_program,
        "#!/bin/sh\ntest \"$1\" = --colors && test \"$2\" = 16 && test \"$3\" = --pixel-size && test \"$4\" = 4\n", -1, &error));
    g_assert_cmpint(g_chmod(dither_program, 0700), ==, 0);
    g_assert_cmpstr(gtk_combo_box_get_active_id(GTK_COMBO_BOX(view->dither_colors)), ==, "64");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(view->dither_colors), "16");
    g_assert_cmpint(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(view->dither_pixel_size)), ==, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(view->dither_pixel_size), 4);
    gtk_button_clicked(GTK_BUTTON(view->dither_button));
    g_assert_false(gtk_widget_get_sensitive(view->dither_pixel_size));
    g_assert_false(gtk_widget_get_sensitive(view->dither_button));
    wait_for_download(view);
    g_assert_true(gtk_widget_get_sensitive(view->dither_colors));
    g_assert_true(gtk_widget_get_sensitive(view->dither_pixel_size));
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->dither_status)),
                                  "Dithered copy selected."));
    g_assert_true(g_file_set_contents(dither_program,
        "#!/bin/sh\ntest \"$1\" = --restore\n", -1, &error));
    g_assert_cmpint(g_chmod(dither_program, 0700), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->restore_wallpaper_button));
    wait_for_download(view);
    g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(view->dither_status)), ==,
                    "Original wallpaper restored.");
    g_assert_cmpint(g_unlink(dither_program), ==, 0);
    g_assert_cmpint(g_rmdir(dither_dir), ==, 0);
    g_free(dither_program);
    g_free(dither_dir);
    gchar *firefox_dir = g_build_filename(directory, "firefox", NULL);
    gchar *firefox_program = g_build_filename(firefox_dir, "install.py", NULL);
    g_assert_cmpint(g_mkdir(firefox_dir, 0700), ==, 0);
    g_assert_true(g_file_set_contents(firefox_program,
        "#!/bin/sh\nsleep 0.1\nexit 0\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(firefox_program, 0700), ==, 0);
    g_assert_cmpstr(gtk_button_get_label(GTK_BUTTON(view->firefox_button)), ==,
                    "Add quartz theme to firefox");
    gtk_button_clicked(GTK_BUTTON(view->firefox_button));
    g_assert_false(gtk_widget_get_sensitive(view->firefox_button));
    wait_for_download(view);
    g_assert_true(gtk_widget_get_sensitive(view->firefox_button));
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->firefox_status)),
                                  "Quartz theme added."));
    g_assert_true(g_file_set_contents(firefox_program,
        "#!/bin/sh\necho 'Close Firefox completely' >&2\nexit 1\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(firefox_program, 0700), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->firefox_button));
    wait_for_download(view);
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->firefox_status)),
                                  "Theme installation failed."));
    g_assert_cmpint(g_unlink(firefox_program), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->firefox_button));
    g_assert_false(view->downloading_extras);
    g_assert_true(gtk_widget_get_sensitive(view->firefox_button));
    g_assert_cmpint(g_rmdir(firefox_dir), ==, 0);
    g_free(firefox_program);
    g_free(firefox_dir);
    gchar *logo_dir = g_build_filename(directory, "logo", NULL);
    gchar *logo_program = g_build_filename(logo_dir, "install.py", NULL);
    g_assert_cmpint(g_mkdir(logo_dir, 0700), ==, 0);
    g_assert_true(g_file_set_contents(logo_program,
        "#!/bin/sh\nsleep 0.1\nexit 0\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(logo_program, 0700), ==, 0);
    g_assert_cmpstr(gtk_button_get_label(GTK_BUTTON(view->logo_button)), ==,
                    "Use Quartz logo instead of MATE");
    gtk_button_clicked(GTK_BUTTON(view->logo_button));
    g_assert_false(gtk_widget_get_sensitive(view->logo_button));
    wait_for_download(view);
    g_assert_true(gtk_widget_get_sensitive(view->logo_button));
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->logo_status)),
                                  "Quartz diamond selected"));
    g_assert_true(g_file_set_contents(logo_program,
        "#!/bin/sh\necho 'Test logo failure' >&2\nexit 1\n", -1, &error));
    g_assert_no_error(error);
    g_assert_cmpint(g_chmod(logo_program, 0700), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->logo_button));
    wait_for_download(view);
    g_assert_true(g_str_has_prefix(gtk_label_get_text(GTK_LABEL(view->logo_status)),
                                  "Logo installation failed."));
    g_assert_cmpint(g_unlink(logo_program), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->logo_button));
    g_assert_false(view->downloading_extras);
    g_assert_true(gtk_widget_get_sensitive(view->logo_button));
    g_assert_cmpint(g_rmdir(logo_dir), ==, 0);
    g_free(logo_program);
    g_free(logo_dir);
    g_assert_cmpint(g_unlink(test_extras_program), ==, 0);
    gtk_button_clicked(GTK_BUTTON(view->extras_button));
    g_assert_false(view->downloading_extras);
    g_assert_true(gtk_widget_get_sensitive(view->extras_button));
    g_source_remove(dialog_timer);
    gtk_widget_destroy(window);
    g_object_unref(app);
    g_assert_cmpint(g_rmdir(directory), ==, 0);
    g_free(test_extras_program);
    g_free(directory);
    g_print("Quartz Extras tabs, async success, failure, and missing installer tests passed.\n");
    return 0;
}
