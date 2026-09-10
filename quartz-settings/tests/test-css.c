/* Parse a rendered shared stylesheet without installing it on the display. */
#include <gtk/gtk.h>

static void
parsing_error(GtkCssProvider *provider, GtkCssSection *section,
              GError *error, gpointer user_data)
{
    gboolean *failed = user_data;

    (void)provider;
    (void)section;
    *failed = TRUE;
    g_printerr("CSS parser: %s\n", error->message);
}

int
main(int argc, char **argv)
{
    GtkCssProvider *provider;
    gboolean failed = FALSE;

    if (argc != 2) {
        g_printerr("usage: %s RENDERED_CSS\n", argv[0]);
        return 1;
    }
#if GTK_MAJOR_VERSION == 3
    gtk_init(&argc, &argv);
#else
    gtk_init();
#endif
    provider = gtk_css_provider_new();
    g_signal_connect(provider, "parsing-error", G_CALLBACK(parsing_error), &failed);
#if GTK_MAJOR_VERSION == 3
    gtk_css_provider_load_from_path(provider, argv[1], NULL);
#else
    gtk_css_provider_load_from_path(provider, argv[1]);
#endif
    g_object_unref(provider);
    return failed ? 1 : 0;
}
