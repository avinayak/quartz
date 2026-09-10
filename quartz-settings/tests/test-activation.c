/* Exercise the real activation callback without invoking Apply or an audit. */
#define main quartz_settings_main
#include "../src/quartz-settings.c"
#undef main

int
main(int argc, char **argv)
{
    GtkApplication *application_instance;
    GtkWindow *first_window;
    GError *error = NULL;

    gtk_init(&argc, &argv);
    application_instance = gtk_application_new("org.quartz.ActivationTest",
                                               G_APPLICATION_NON_UNIQUE);
    g_assert_true(g_application_register(G_APPLICATION(application_instance),
                                        NULL, &error));
    g_assert_no_error(error);
    activate(application_instance, NULL);
    first_window = gtk_application_get_active_window(application_instance);
    g_assert_nonnull(first_window);
    activate(application_instance, NULL);
    g_assert_cmpuint(g_list_length(gtk_application_get_windows(application_instance)),
                     ==, 1);
    g_assert_true(first_window == gtk_application_get_active_window(application_instance));
    gtk_widget_destroy(GTK_WIDGET(first_window));
    g_object_unref(application_instance);
    g_print("Quartz repeated activation test passed.\n");
    return 0;
}
