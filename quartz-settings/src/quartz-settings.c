#include <gio/gio.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#include <string.h>

#ifndef QUARTZ_AUDIT_DIR
#define QUARTZ_AUDIT_DIR "/usr/local/libexec/quartz-settings"
#endif

#ifndef QUARTZ_APPLY_PROGRAM
#define QUARTZ_APPLY_PROGRAM "/usr/local/libexec/quartz-settings/quartz-theme-apply"
#endif

#ifndef QUARTZ_EXTRAS_PROGRAM
#define QUARTZ_EXTRAS_PROGRAM "/usr/local/share/quartz-settings/extras/install.sh"
#endif

typedef struct {
    const gchar *name;
    const gchar *menubar;
    const gchar *titlebar;
    const gchar *application;
} ThemePreset;

#define FLAT_PRESET(label, color) { label, color, color, color }
#define MIXED_PRESET(label, menubar, titlebar, application) \
    { "Mixed - " label, menubar, titlebar, application }

/* The first bank applies one medium pastel to all three neutral surface
 * roles. The second bank uses that same pastel for the application, a richer
 * midpoint for menus, and the strongest tint for titles. All retain Quartz's
 * black one-bit ink and therefore use light surface colors. */
static const ThemePreset theme_presets[] = {
    FLAT_PRESET("Classic",    "#ffffff"),
    FLAT_PRESET("Platinum",   "#e8e8e8"),
    FLAT_PRESET("Graphite",   "#dce0e5"),
    FLAT_PRESET("Blueberry",  "#dcecff"),
    FLAT_PRESET("Sky",        "#dff3ff"),
    FLAT_PRESET("Aqua",       "#d9f3f4"),
    FLAT_PRESET("Teal",       "#d7eee9"),
    FLAT_PRESET("Mint",       "#dcf3df"),
    FLAT_PRESET("Sage",       "#e5eedc"),
    FLAT_PRESET("Lime",       "#edf4cf"),
    FLAT_PRESET("Lemon",      "#fff3bd"),
    FLAT_PRESET("Sand",       "#f4e8cf"),
    FLAT_PRESET("Tangerine",  "#ffe3c5"),
    FLAT_PRESET("Coral",      "#fbded4"),
    FLAT_PRESET("Strawberry", "#f8dce0"),
    FLAT_PRESET("Rose",       "#f5dfea"),
    FLAT_PRESET("Lavender",   "#eee3fa"),
    FLAT_PRESET("Grape",      "#e8dff5"),
    FLAT_PRESET("Iris",       "#e1e3fa"),
    FLAT_PRESET("Mocha",      "#ede3db"),

    MIXED_PRESET("Classic",    "#e3e3e3", "#d2d2d2", "#ffffff"),
    MIXED_PRESET("Platinum",   "#dddddd", "#d2d2d2", "#e8e8e8"),
    MIXED_PRESET("Graphite",   "#d2d7dd", "#c7cdd4", "#dce0e5"),
    MIXED_PRESET("Blueberry",  "#cde2fd", "#bdd8fb", "#dcecff"),
    MIXED_PRESET("Sky",        "#cfebfa", "#bfe2f5", "#dff3ff"),
    MIXED_PRESET("Aqua",       "#cbeded", "#bce3e5", "#d9f3f4"),
    MIXED_PRESET("Teal",       "#c8e6df", "#b9ddd5", "#d7eee9"),
    MIXED_PRESET("Mint",       "#cdecd2", "#bee5c5", "#dcf3df"),
    MIXED_PRESET("Sage",       "#d9e6cc", "#cdddbc", "#e5eedc"),
    MIXED_PRESET("Lime",       "#e4eebd", "#dae7aa", "#edf4cf"),
    MIXED_PRESET("Lemon",      "#ffebab", "#ffe398", "#fff3bd"),
    MIXED_PRESET("Sand",       "#eadabb", "#dfcca7", "#f4e8cf"),
    MIXED_PRESET("Tangerine",  "#fad6ae", "#f5c897", "#ffe3c5"),
    MIXED_PRESET("Coral",      "#f5cfc3", "#efc0b1", "#fbded4"),
    MIXED_PRESET("Strawberry", "#f1cbd1", "#eabac2", "#f8dce0"),
    MIXED_PRESET("Rose",       "#edcfde", "#e5bfd2", "#f5dfea"),
    MIXED_PRESET("Lavender",   "#e2d3f4", "#d6c2ed", "#eee3fa"),
    MIXED_PRESET("Grape",      "#dbcdee", "#cdbbe5", "#e8dff5"),
    MIXED_PRESET("Iris",       "#d2d5f3", "#c2c7ec", "#e1e3fa"),
    MIXED_PRESET("Mocha",      "#e2d3c7", "#d7c2b3", "#ede3db")
};

#undef MIXED_PRESET
#undef FLAT_PRESET

G_STATIC_ASSERT(G_N_ELEMENTS(theme_presets) == 40);

typedef enum {
    WINDOW_LAYOUT_CUPERTINO,
    WINDOW_LAYOUT_REDMOND,
    WINDOW_LAYOUT_REDMOND_REVERSED,
    WINDOW_LAYOUT_COUNT
} WindowLayout;

typedef enum {
    WINDOW_CONTROL_CLOSE,
    WINDOW_CONTROL_MINIMIZE,
    WINDOW_CONTROL_MAXIMIZE
} WindowControl;

typedef struct {
    const gchar *name;
    const gchar *config_value;
} WindowLayoutOption;

static const WindowLayoutOption window_layout_options[] = {
    { "Cupertino", "cupertino" },
    { "Redmond", "redmond" },
    { "Redmond reversed", "redmond-reversed" }
};

G_STATIC_ASSERT(G_N_ELEMENTS(window_layout_options) == WINDOW_LAYOUT_COUNT);

enum {
    PREVIEW_WINDOW_WIDTH = 390,
    PREVIEW_WINDOW_HEIGHT = 128,
    PREVIEW_TITLEBAR_HEIGHT = 19,
    PREVIEW_MENUBAR_HEIGHT = 25,
    PREVIEW_BUTTON_WIDTH = 82,
    PREVIEW_BUTTON_HEIGHT = 22,
    PREVIEW_DEFAULT_RADIUS = 5,
    PREVIEW_TITLE_INSET = 7,
    PREVIEW_CONTROL_SIZE = 13,
    PREVIEW_CLASSIC_CONTROL_BORDER = 7,
    PREVIEW_COMPACT_CONTROL_BORDER = 2,
    PREVIEW_COMPACT_CONTROL_STRIDE =
        PREVIEW_CONTROL_SIZE + 2 * PREVIEW_COMPACT_CONTROL_BORDER
};

G_STATIC_ASSERT(PREVIEW_TITLEBAR_HEIGHT == 19);
G_STATIC_ASSERT(PREVIEW_MENUBAR_HEIGHT == 25);

typedef struct {
    GtkWidget *window;
    GtkWidget *window_layout_combo;
    GtkWidget *preset_combo;
    GtkWidget *menubar_color;
    GtkWidget *titlebar_color;
    GtkWidget *application_color;
    GtkWidget *preview;
    GtkWidget *apply_button;
    GtkWidget *status_label;
    GtkWidget *extras_button;
    GtkWidget *extras_status;
    GtkWidget *extras_spinner;
    GtkWidget *firefox_button;
    GtkWidget *firefox_status;
    GtkWidget *firefox_spinner;
    GtkWidget *logo_button;
    GtkWidget *logo_status;
    GtkWidget *logo_spinner;
    GtkWidget *dither_colors;
    GtkWidget *dither_pixel_size;
    GtkWidget *dither_button;
    GtkWidget *restore_wallpaper_button;
    GtkWidget *dither_status;
    GtkWidget *dither_spinner;
    gboolean downloading_extras;
    guint border_radius;
    WindowLayout window_layout;
    gboolean syncing_controls;
    gboolean applying;
    gboolean typography_applying;
} SettingsView;

typedef struct {
    SettingsView *view;
    GtkWidget *window;
    GSubprocess *process;
    gboolean firefox;
    gboolean logo;
    gboolean dither;
    gboolean restore;
} HelperRequest;

static void
show_error(GtkWindow *parent, const gchar *primary, const gchar *secondary)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(parent,
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "%s",
                                    primary);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
                                             "%s",
                                             secondary);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void
show_launch_error(GtkWindow *parent, const gchar *program, const GError *error)
{
    gchar *primary = g_strdup_printf("Could not launch %s", program);

    show_error(parent, primary, error->message);
    g_free(primary);
}

static void
launch_audit(GtkButton *button, gpointer user_data)
{
    GtkWindow *parent = GTK_WINDOW(user_data);
    const gchar *program = g_object_get_data(G_OBJECT(button), "quartz-program");
    gchar *installed_path = g_build_filename(QUARTZ_AUDIT_DIR, program, NULL);
    gchar *argv[] = { installed_path, NULL };
    GError *error = NULL;

    if (!g_file_test(installed_path, G_FILE_TEST_IS_EXECUTABLE)) {
        argv[0] = (gchar *)program;
    }

    if (!g_spawn_async(NULL,
                       argv,
                       NULL,
                       G_SPAWN_SEARCH_PATH,
                       NULL,
                       NULL,
                       NULL,
                       &error)) {
        show_launch_error(parent, program, error);
        g_clear_error(&error);
    }

    g_free(installed_path);
}

static GtkWidget *
audit_button(GtkWindow *window, const gchar *label, const gchar *program)
{
    GtkWidget *button = gtk_button_new_with_label(label);

    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_size_request(button, 120, 36);
    g_object_set_data_full(G_OBJECT(button),
                           "quartz-program",
                           g_strdup(program),
                           g_free);
    g_signal_connect(button, "clicked", G_CALLBACK(launch_audit), window);
    return button;
}

static gchar *
color_to_hex(GtkColorChooser *chooser)
{
    GdkRGBA color;
    guint red;
    guint green;
    guint blue;

    gtk_color_chooser_get_rgba(chooser, &color);
    red = (guint)(color.red * 255.0 + 0.5);
    green = (guint)(color.green * 255.0 + 0.5);
    blue = (guint)(color.blue * 255.0 + 0.5);
    return g_strdup_printf("#%02x%02x%02x", red, green, blue);
}

static void
set_color(GtkWidget *chooser, const gchar *hex_color)
{
    GdkRGBA color;

    if (gdk_rgba_parse(&color, hex_color)) {
        color.alpha = 1.0;
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(chooser), &color);
    }
}

static gboolean
window_layout_from_value(const gchar *value, WindowLayout *layout)
{
    guint layout_index;

    for (layout_index = 0;
         layout_index < G_N_ELEMENTS(window_layout_options);
         layout_index++) {
        if (g_strcmp0(value,
                      window_layout_options[layout_index].config_value) == 0) {
            *layout = (WindowLayout)layout_index;
            return TRUE;
        }
    }

    return FALSE;
}

static gint
matching_preset(SettingsView *view)
{
    gchar *menubar = color_to_hex(GTK_COLOR_CHOOSER(view->menubar_color));
    gchar *titlebar = color_to_hex(GTK_COLOR_CHOOSER(view->titlebar_color));
    gchar *application = color_to_hex(GTK_COLOR_CHOOSER(view->application_color));
    guint preset_index;
    gint match = -1;

    for (preset_index = 0; preset_index < G_N_ELEMENTS(theme_presets); preset_index++) {
        if (g_ascii_strcasecmp(menubar, theme_presets[preset_index].menubar) == 0 &&
            g_ascii_strcasecmp(titlebar, theme_presets[preset_index].titlebar) == 0 &&
            g_ascii_strcasecmp(application, theme_presets[preset_index].application) == 0) {
            match = (gint)preset_index;
            break;
        }
    }

    g_free(menubar);
    g_free(titlebar);
    g_free(application);
    return match;
}

static void
update_preset_selection(SettingsView *view)
{
    gint preset_index;

    if (view->syncing_controls) {
        return;
    }

    preset_index = matching_preset(view);
    view->syncing_controls = TRUE;
    gtk_combo_box_set_active(GTK_COMBO_BOX(view->preset_combo),
                             preset_index >= 0 ? preset_index :
                             (gint)G_N_ELEMENTS(theme_presets));
    view->syncing_controls = FALSE;
}

static void
color_changed(GtkColorButton *button, gpointer user_data)
{
    SettingsView *view = user_data;

    (void)button;
    update_preset_selection(view);
    gtk_widget_queue_draw(view->preview);
    gtk_label_set_text(GTK_LABEL(view->status_label),
                       "Colors changed. Select Apply to update the shared desktop theme.");
}

static gboolean
window_delete_requested(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    SettingsView *view = user_data;

    (void)widget;
    (void)event;
    return view->applying || view->typography_applying || view->downloading_extras;
}

static void
preset_changed(GtkComboBox *combo, gpointer user_data)
{
    SettingsView *view = user_data;
    gint preset_index = gtk_combo_box_get_active(combo);

    if (view->syncing_controls || preset_index < 0 ||
        preset_index >= (gint)G_N_ELEMENTS(theme_presets)) {
        return;
    }

    view->syncing_controls = TRUE;
    set_color(view->menubar_color, theme_presets[preset_index].menubar);
    set_color(view->titlebar_color, theme_presets[preset_index].titlebar);
    set_color(view->application_color, theme_presets[preset_index].application);
    view->syncing_controls = FALSE;
    gtk_widget_queue_draw(view->preview);
    gtk_label_set_text(GTK_LABEL(view->status_label),
                       "Preset selected. Select Apply to update the shared desktop theme.");
}

static void
window_layout_changed(GtkComboBox *combo, gpointer user_data)
{
    SettingsView *view = user_data;
    gint layout_index = gtk_combo_box_get_active(combo);

    if (view->syncing_controls || layout_index < 0 ||
        layout_index >= WINDOW_LAYOUT_COUNT) {
        return;
    }

    view->window_layout = (WindowLayout)layout_index;
    gtk_widget_queue_draw(view->preview);
    gtk_label_set_text(GTK_LABEL(view->status_label),
                       "Window layout changed. Select Apply to update the shared desktop theme.");
}

static gboolean
read_theme_file(const gchar *path,
                GdkRGBA *menubar,
                GdkRGBA *titlebar,
                GdkRGBA *application,
                guint *border_radius,
                WindowLayout *window_layout)
{
    gchar *contents = NULL;
    gchar **lines;
    guint line_index;
    gboolean found_menubar = FALSE;
    gboolean found_titlebar = FALSE;
    gboolean found_application = FALSE;

    if (!g_file_get_contents(path, &contents, NULL, NULL)) {
        return FALSE;
    }

    lines = g_strsplit(contents, "\n", -1);
    for (line_index = 0; lines[line_index] != NULL; line_index++) {
        const gchar *line = lines[line_index];
        const gchar *value = strchr(line, '=');
        GdkRGBA parsed;

        if (value == NULL) {
            continue;
        }
        if (g_str_has_prefix(line, "QUARTZ_BORDER_RADIUS_PX=")) {
            gchar *end = NULL;
            guint64 parsed_radius = g_ascii_strtoull(value + 1, &end, 10);

            if (end != value + 1 && *end == '\0' && parsed_radius <= 64) {
                *border_radius = (guint)parsed_radius;
            }
            continue;
        }
        if (g_str_has_prefix(line, "QUARTZ_WINDOW_LAYOUT=")) {
            window_layout_from_value(value + 1, window_layout);
            continue;
        }
        if (!gdk_rgba_parse(&parsed, value + 1)) {
            continue;
        }
        parsed.alpha = 1.0;
        if (g_str_has_prefix(line, "QUARTZ_MENUBAR_BG=")) {
            *menubar = parsed;
            found_menubar = TRUE;
        } else if (g_str_has_prefix(line, "QUARTZ_TITLEBAR_BG=")) {
            *titlebar = parsed;
            found_titlebar = TRUE;
        } else if (g_str_has_prefix(line, "QUARTZ_APPLICATION_BG=")) {
            *application = parsed;
            found_application = TRUE;
        }
    }

    g_strfreev(lines);
    g_free(contents);
    return found_menubar && found_titlebar && found_application;
}

static void
load_theme(SettingsView *view)
{
    GdkRGBA menubar;
    GdkRGBA titlebar;
    GdkRGBA application;
    gchar *user_path;
    gchar *installed_path;
    gboolean loaded;

    gdk_rgba_parse(&menubar, theme_presets[0].menubar);
    gdk_rgba_parse(&titlebar, theme_presets[0].titlebar);
    gdk_rgba_parse(&application, theme_presets[0].application);
    view->border_radius = PREVIEW_DEFAULT_RADIUS;
    view->window_layout = WINDOW_LAYOUT_CUPERTINO;

    user_path = g_build_filename(g_get_user_config_dir(),
                                 "quartz-settings",
                                 "theme.conf",
                                 NULL);
    installed_path = g_build_filename(g_get_home_dir(),
                                      ".themes",
                                      "Quartz-System6",
                                      "theme.conf",
                                      NULL);
    loaded = read_theme_file(user_path,
                             &menubar,
                             &titlebar,
                             &application,
                             &view->border_radius,
                             &view->window_layout);
    if (!loaded) {
        read_theme_file(installed_path,
                        &menubar,
                        &titlebar,
                        &application,
                        &view->border_radius,
                        &view->window_layout);
    }

    view->syncing_controls = TRUE;
    gtk_combo_box_set_active(GTK_COMBO_BOX(view->window_layout_combo),
                             (gint)view->window_layout);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(view->menubar_color), &menubar);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(view->titlebar_color), &titlebar);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(view->application_color), &application);
    view->syncing_controls = FALSE;
    update_preset_selection(view);

    g_free(user_path);
    g_free(installed_path);
}

static void
set_source_color(cairo_t *context, const GdkRGBA *color)
{
    cairo_set_source_rgb(context, color->red, color->green, color->blue);
}

static void
fill_preview_rectangle(cairo_t *context,
                       const GdkRGBA *color,
                       gint x,
                       gint y,
                       gint width,
                       gint height)
{
    set_source_color(context, color);
    cairo_rectangle(context, x, y, width, height);
    cairo_fill(context);
}

static void
fill_black_rectangle(cairo_t *context,
                     gint x,
                     gint y,
                     gint width,
                     gint height)
{
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    cairo_rectangle(context, x, y, width, height);
    cairo_fill(context);
}

static void
draw_preview_outline(cairo_t *context,
                     gint x,
                     gint y,
                     gint width,
                     gint height)
{
    fill_black_rectangle(context, x, y, width, 1);
    fill_black_rectangle(context, x, y + height - 1, width, 1);
    fill_black_rectangle(context, x, y + 1, 1, height - 2);
    fill_black_rectangle(context, x + width - 1, y + 1, 1, height - 2);
}

static PangoLayout *
preview_text_layout(GtkWidget *widget,
                    const gchar *text,
                    const gchar *family,
                    gint pixel_size)
{
    PangoFontDescription *description = pango_font_description_new();
    PangoLayout *layout = gtk_widget_create_pango_layout(widget, text);

    pango_font_description_set_family(description, family);
    pango_font_description_set_style(description, PANGO_STYLE_NORMAL);
    pango_font_description_set_weight(description, PANGO_WEIGHT_NORMAL);
    pango_font_description_set_absolute_size(description,
                                             pixel_size * PANGO_SCALE);
    pango_layout_set_font_description(layout, description);
    pango_layout_set_single_paragraph_mode(layout, TRUE);
    pango_font_description_free(description);
    return layout;
}

static void
draw_preview_layout_centered(cairo_t *context,
                             PangoLayout *layout,
                             gint x,
                             gint y,
                             gint width,
                             gint height)
{
    PangoRectangle ink;

    pango_layout_get_pixel_extents(layout, &ink, NULL);
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    cairo_move_to(context,
                  x + (width - ink.width) / 2 - ink.x,
                  y + (height - ink.height) / 2 - ink.y);
    pango_cairo_show_layout(context, layout);
}

static gboolean
inside_preview_rounding(gdouble pixel_x,
                        gdouble pixel_y,
                        gint left,
                        gint top,
                        gint right,
                        gint bottom,
                        guint radius)
{
    gdouble delta_x;
    gdouble delta_y;

    if (pixel_x < left || pixel_x >= right ||
        pixel_y < top || pixel_y >= bottom) {
        return FALSE;
    }
    if (radius == 0) {
        return TRUE;
    }

    if (pixel_x < left + (gint)radius && pixel_y < top + (gint)radius) {
        delta_x = left + radius - pixel_x;
        delta_y = top + radius - pixel_y;
    } else if (pixel_x >= right - (gint)radius &&
               pixel_y < top + (gint)radius) {
        delta_x = pixel_x - (right - radius);
        delta_y = top + radius - pixel_y;
    } else if (pixel_x < left + (gint)radius &&
               pixel_y >= bottom - (gint)radius) {
        delta_x = left + radius - pixel_x;
        delta_y = pixel_y - (bottom - radius);
    } else if (pixel_x >= right - (gint)radius &&
               pixel_y >= bottom - (gint)radius) {
        delta_x = pixel_x - (right - radius);
        delta_y = pixel_y - (bottom - radius);
    } else {
        return TRUE;
    }

    return delta_x * delta_x + delta_y * delta_y <= radius * radius;
}

static void
draw_preview_button(GtkWidget *widget,
                    cairo_t *context,
                    const GdkRGBA *application,
                    guint configured_radius,
                    gint x,
                    gint y)
{
    const gint body_height = PREVIEW_BUTTON_HEIGHT - 1;
    guint radius = MIN(configured_radius,
                       (guint)(MIN(PREVIEW_BUTTON_WIDTH, body_height) / 2));
    guint inner_radius = radius > 0 ? radius - 1 : 0;
    PangoLayout *label;
    gint pixel_x;
    gint pixel_y;

    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    for (pixel_y = 0; pixel_y < PREVIEW_BUTTON_HEIGHT; pixel_y++) {
        for (pixel_x = 0; pixel_x < PREVIEW_BUTTON_WIDTH; pixel_x++) {
            gdouble center_x = pixel_x + 0.5;
            gdouble center_y = pixel_y + 0.5;
            gboolean body = inside_preview_rounding(center_x,
                                                    center_y,
                                                    0,
                                                    0,
                                                    PREVIEW_BUTTON_WIDTH,
                                                    body_height,
                                                    radius);
            gboolean shadow = pixel_y > 0 &&
                inside_preview_rounding(center_x,
                                        pixel_y - 0.5,
                                        0,
                                        0,
                                        PREVIEW_BUTTON_WIDTH,
                                        body_height,
                                        radius);

            if (body || shadow) {
                cairo_rectangle(context, x + pixel_x, y + pixel_y, 1, 1);
            }
        }
    }
    cairo_fill(context);

    set_source_color(context, application);
    for (pixel_y = 0; pixel_y < body_height; pixel_y++) {
        for (pixel_x = 0; pixel_x < PREVIEW_BUTTON_WIDTH; pixel_x++) {
            if (inside_preview_rounding(pixel_x + 0.5,
                                        pixel_y + 0.5,
                                        1,
                                        1,
                                        PREVIEW_BUTTON_WIDTH - 1,
                                        body_height - 1,
                                        inner_radius)) {
                cairo_rectangle(context, x + pixel_x, y + pixel_y, 1, 1);
            }
        }
    }
    cairo_fill(context);

    label = preview_text_layout(widget, "Button", "Geneva", 15);
    draw_preview_layout_centered(context,
                                 label,
                                 x,
                                 y,
                                 PREVIEW_BUTTON_WIDTH,
                                 body_height);
    g_object_unref(label);
}

static void
draw_preview_titlebar_control(cairo_t *context,
                              const GdkRGBA *titlebar,
                              gint allocation_x,
                              gint allocation_y,
                              WindowControl control)
{
    fill_preview_rectangle(context,
                           titlebar,
                           allocation_x,
                           allocation_y,
                           PREVIEW_CONTROL_SIZE,
                           PREVIEW_CONTROL_SIZE);
    draw_preview_outline(context,
                         allocation_x + 1,
                         allocation_y + 1,
                         11,
                         11);

    if (control == WINDOW_CONTROL_MINIMIZE) {
        fill_black_rectangle(context,
                             allocation_x + 3,
                             allocation_y + 9,
                             7,
                             1);
    } else if (control == WINDOW_CONTROL_MAXIMIZE) {
        fill_black_rectangle(context,
                             allocation_x + 7,
                             allocation_y + 2,
                             1,
                             7);
        fill_black_rectangle(context,
                             allocation_x + 2,
                             allocation_y + 7,
                             7,
                             1);
    }
}

static void
draw_preview_titlebar(GtkWidget *widget,
                      cairo_t *context,
                      const GdkRGBA *titlebar,
                      gint left,
                      gint top,
                      WindowLayout layout)
{
    static const WindowControl windows_controls[] = {
        WINDOW_CONTROL_MINIMIZE,
        WINDOW_CONTROL_MAXIMIZE,
        WINDOW_CONTROL_CLOSE
    };
    static const WindowControl inverted_controls[] = {
        WINDOW_CONTROL_CLOSE,
        WINDOW_CONTROL_MINIMIZE,
        WINDOW_CONTROL_MAXIMIZE
    };
    const gint control_border =
        layout == WINDOW_LAYOUT_CUPERTINO
            ? PREVIEW_CLASSIC_CONTROL_BORDER
            : PREVIEW_COMPACT_CONTROL_BORDER;
    const gint left_control_x = left + control_border;
    const gint right_control_x = left + PREVIEW_WINDOW_WIDTH -
                                 control_border - PREVIEW_CONTROL_SIZE;
    const gint control_allocation_y = top + 3;
    PangoLayout *title;
    PangoRectangle ink;
    PangoRectangle logical;
    gint title_left;
    gint stripe_y;

    fill_preview_rectangle(context,
                           titlebar,
                           left + 1,
                           top + 1,
                           PREVIEW_WINDOW_WIDTH - 2,
                           PREVIEW_TITLEBAR_HEIGHT - 2);
    for (stripe_y = 4; stripe_y < PREVIEW_TITLEBAR_HEIGHT - 4; stripe_y += 2) {
        fill_black_rectangle(context,
                             left + 2,
                             top + stripe_y,
                             PREVIEW_WINDOW_WIDTH - 4,
                             1);
    }

    /* Each 13px allocation clears a one-pixel moat around its 11px System 6
     * box. Cupertino retains a seven-pixel edge border; the two Redmond modes
     * use two pixels per side for a compact six-pixel gap between boxes. */
    if (layout == WINDOW_LAYOUT_CUPERTINO) {
        draw_preview_titlebar_control(context,
                                      titlebar,
                                      left_control_x,
                                      control_allocation_y,
                                      WINDOW_CONTROL_CLOSE);
        draw_preview_titlebar_control(context,
                                      titlebar,
                                      right_control_x,
                                      control_allocation_y,
                                      WINDOW_CONTROL_MAXIMIZE);
    } else if (layout == WINDOW_LAYOUT_REDMOND) {
        for (guint control_index = 0; control_index < 3; control_index++) {
            draw_preview_titlebar_control(context,
                                          titlebar,
                                          right_control_x -
                                            (2 - (gint)control_index) * PREVIEW_COMPACT_CONTROL_STRIDE,
                                          control_allocation_y,
                                          windows_controls[control_index]);
        }
    } else {
        for (guint control_index = 0; control_index < 3; control_index++) {
            draw_preview_titlebar_control(context,
                                          titlebar,
                                          left_control_x +
                                            (gint)control_index * PREVIEW_COMPACT_CONTROL_STRIDE,
                                          control_allocation_y,
                                          inverted_controls[control_index]);
        }
    }

    title = preview_text_layout(widget, "Preview", "ChiKareGo2", 16);
    pango_layout_get_pixel_extents(title, &ink, &logical);
    if (layout == WINDOW_LAYOUT_CUPERTINO) {
        title_left = left + (PREVIEW_WINDOW_WIDTH - logical.width) / 2;
    } else if (layout == WINDOW_LAYOUT_REDMOND) {
        title_left = left + PREVIEW_TITLE_INSET;
    } else {
        title_left = left + PREVIEW_WINDOW_WIDTH -
                     PREVIEW_TITLE_INSET - logical.width;
    }
    fill_preview_rectangle(context,
                           titlebar,
                           title_left - 6,
                           top + 4,
                           logical.width + 12,
                           PREVIEW_TITLEBAR_HEIGHT - 8);
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    cairo_move_to(context,
                  title_left + (logical.width - ink.width) / 2 - ink.x,
                  top + (PREVIEW_TITLEBAR_HEIGHT - 1 - ink.height) / 2 - ink.y);
    pango_cairo_show_layout(context, title);
    g_object_unref(title);

    fill_black_rectangle(context,
                         left,
                         top + PREVIEW_TITLEBAR_HEIGHT - 1,
                         PREVIEW_WINDOW_WIDTH,
                         1);
}

static void
draw_preview_menubar(GtkWidget *widget,
                     cairo_t *context,
                     const GdkRGBA *menubar,
                     gint left,
                     gint top)
{
    static const gchar *const labels[] = { "File", "Edit", "View", "Window" };
    gint label_x = left + 12;
    guint label_index;

    fill_preview_rectangle(context,
                           menubar,
                           left + 1,
                           top,
                           PREVIEW_WINDOW_WIDTH - 2,
                           PREVIEW_MENUBAR_HEIGHT - 1);

    for (label_index = 0; label_index < G_N_ELEMENTS(labels); label_index++) {
        PangoLayout *layout = preview_text_layout(widget,
                                                  labels[label_index],
                                                  "ChiKareGo2",
                                                  16);
        PangoRectangle logical;

        pango_layout_get_pixel_extents(layout, NULL, &logical);
        draw_preview_layout_centered(context,
                                     layout,
                                     label_x,
                                     top,
                                     logical.width,
                                     PREVIEW_MENUBAR_HEIGHT - 1);
        label_x += logical.width + 12;
        g_object_unref(layout);
    }

    fill_black_rectangle(context,
                         left,
                         top + PREVIEW_MENUBAR_HEIGHT - 1,
                         PREVIEW_WINDOW_WIDTH,
                         1);
}

static gboolean
draw_preview(GtkWidget *widget, cairo_t *context, gpointer user_data)
{
    SettingsView *view = user_data;
    GtkAllocation allocation;
    cairo_font_options_t *font_options;
    GdkRGBA menubar;
    GdkRGBA titlebar;
    GdkRGBA application;
    gint left;
    gint top;
    gint content_top;
    gint button_x;
    gint button_y;

    gtk_widget_get_allocation(widget, &allocation);
    left = (allocation.width - PREVIEW_WINDOW_WIDTH) / 2;
    top = (allocation.height - PREVIEW_WINDOW_HEIGHT) / 2;
    content_top = top + PREVIEW_TITLEBAR_HEIGHT + PREVIEW_MENUBAR_HEIGHT;
    button_x = left + PREVIEW_WINDOW_WIDTH - PREVIEW_BUTTON_WIDTH - 22;
    button_y = top + PREVIEW_WINDOW_HEIGHT - PREVIEW_BUTTON_HEIGHT - 16;

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(view->menubar_color), &menubar);
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(view->titlebar_color), &titlebar);
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(view->application_color),
                               &application);

    cairo_set_operator(context, CAIRO_OPERATOR_SOURCE);
    cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);
    font_options = cairo_font_options_create();
    cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_NONE);
    cairo_set_font_options(context, font_options);
    cairo_font_options_destroy(font_options);

    /* A black backing supplies the one-pixel top/side outline, the penultimate
     * bottom outline, and Marco's final opaque one-pixel lower shadow. */
    fill_black_rectangle(context,
                         left,
                         top,
                         PREVIEW_WINDOW_WIDTH,
                         PREVIEW_WINDOW_HEIGHT);
    fill_preview_rectangle(context,
                           &application,
                           left + 1,
                           top + 1,
                           PREVIEW_WINDOW_WIDTH - 2,
                           PREVIEW_WINDOW_HEIGHT - 3);

    draw_preview_titlebar(widget,
                          context,
                          &titlebar,
                          left,
                          top,
                          view->window_layout);
    draw_preview_menubar(widget,
                         context,
                         &menubar,
                         left,
                         top + PREVIEW_TITLEBAR_HEIGHT);
    fill_preview_rectangle(context,
                           &application,
                           left + 1,
                           content_top,
                           PREVIEW_WINDOW_WIDTH - 2,
                           top + PREVIEW_WINDOW_HEIGHT - 2 - content_top);
    draw_preview_button(widget,
                        context,
                        &application,
                        view->border_radius,
                        button_x,
                        button_y);

    return FALSE;
}

static void
set_theme_controls_sensitive(SettingsView *view, gboolean sensitive)
{
    gtk_widget_set_sensitive(view->window_layout_combo, sensitive);
    gtk_widget_set_sensitive(view->preset_combo, sensitive);
    gtk_widget_set_sensitive(view->menubar_color, sensitive);
    gtk_widget_set_sensitive(view->titlebar_color, sensitive);
    gtk_widget_set_sensitive(view->application_color, sensitive);
    gtk_widget_set_sensitive(view->apply_button, sensitive);
}

static void
apply_finished(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
    HelperRequest *request = user_data;
    gchar *standard_output = NULL;
    gchar *standard_error = NULL;
    GError *error = NULL;
    gboolean communicated;

    communicated = g_subprocess_communicate_utf8_finish(G_SUBPROCESS(source_object),
                                                         result,
                                                         &standard_output,
                                                         &standard_error,
                                                         &error);
    request->view->applying = FALSE;
    set_theme_controls_sensitive(request->view, TRUE);

    if (!communicated) {
        gtk_label_set_text(GTK_LABEL(request->view->status_label),
                           "The theme could not be applied.");
        show_error(GTK_WINDOW(request->window),
                   "Could not apply the Quartz theme",
                   error != NULL ? error->message : "The theme helper could not be read.");
    } else if (!g_subprocess_get_successful(request->process)) {
        const gchar *details = standard_error != NULL && *standard_error != '\0'
                             ? standard_error
                             : "The theme helper exited without applying the theme.";
        gtk_label_set_text(GTK_LABEL(request->view->status_label),
                           "The theme could not be applied.");
        show_error(GTK_WINDOW(request->window),
                   "Could not apply the Quartz theme",
                   details);
    } else {
        const gchar *message = "Applied globally. Restart open applications that retain an older GTK stylesheet.";

        if (standard_output != NULL && *g_strstrip(standard_output) != '\0') {
            message = standard_output;
        }
        gtk_label_set_text(GTK_LABEL(request->view->status_label), message);
    }

    g_clear_error(&error);
    g_free(standard_output);
    g_free(standard_error);
    g_object_unref(request->process);
    g_object_unref(request->window);
    g_free(request);
}

static void
apply_theme(GtkButton *button, gpointer user_data)
{
    SettingsView *view = user_data;
    gchar *menubar = color_to_hex(GTK_COLOR_CHOOSER(view->menubar_color));
    gchar *titlebar = color_to_hex(GTK_COLOR_CHOOSER(view->titlebar_color));
    gchar *application = color_to_hex(GTK_COLOR_CHOOSER(view->application_color));
    const gchar *window_layout =
        window_layout_options[view->window_layout].config_value;
    gchar *fallback_program = NULL;
    const gchar *program = QUARTZ_APPLY_PROGRAM;
    GSubprocess *process;
    GError *error = NULL;
    HelperRequest *request;

    (void)button;

    if (view->typography_applying) goto out;
    if (!g_file_test(program, G_FILE_TEST_IS_EXECUTABLE)) {
        fallback_program = g_find_program_in_path("quartz-theme-apply");
        if (fallback_program == NULL) {
            show_error(GTK_WINDOW(view->window),
                       "Quartz theme helper is not installed",
                       "Run the top-level apply-ubuntu-mate-theme.sh installer, then reopen Quartz Settings.");
            goto out;
        }
        program = fallback_program;
    }

    process = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                               G_SUBPROCESS_FLAGS_STDERR_PIPE,
                               &error,
                               program,
                               "--menubar", menubar,
                               "--titlebar", titlebar,
                               "--application", application,
                               "--window-layout", window_layout,
                               NULL);
    if (process == NULL) {
        show_launch_error(GTK_WINDOW(view->window), program, error);
        g_clear_error(&error);
        goto out;
    }

    set_theme_controls_sensitive(view, FALSE);
    view->applying = TRUE;
    gtk_label_set_text(GTK_LABEL(view->status_label),
                       "Rendering and applying the shared theme…");

    request = g_new0(HelperRequest, 1);
    request->view = view;
    request->window = g_object_ref(view->window);
    request->process = process;
    g_subprocess_communicate_utf8_async(process,
                                        NULL,
                                        NULL,
                                        apply_finished,
                                        request);

out:
    g_free(fallback_program);
    g_free(menubar);
    g_free(titlebar);
    g_free(application);
}

static void
extras_finished(GObject *source, GAsyncResult *result, gpointer user_data)
{
    HelperRequest *request = user_data;
    SettingsView *view = request->view;
    gchar *output = NULL;
    gchar *errors = NULL;
    GError *error = NULL;
    gboolean communicated;

    (void)source;
    communicated = g_subprocess_communicate_utf8_finish(request->process, result,
                                                       &output, &errors, &error);
    view->downloading_extras = FALSE;
    gtk_spinner_stop(GTK_SPINNER(request->dither ? view->dither_spinner : request->logo ? view->logo_spinner :
                                       request->firefox ? view->firefox_spinner : view->extras_spinner));
    gtk_widget_set_sensitive(view->extras_button, TRUE);
    gtk_widget_set_sensitive(view->firefox_button, TRUE);
    gtk_widget_set_sensitive(view->logo_button, TRUE);
    gtk_widget_set_sensitive(view->dither_button, TRUE);
    gtk_widget_set_sensitive(view->dither_colors, TRUE);
    gtk_widget_set_sensitive(view->dither_pixel_size, TRUE);
    gtk_widget_set_sensitive(view->restore_wallpaper_button, TRUE);
    GtkWidget *status = request->dither ? view->dither_status : request->logo ? view->logo_status : request->firefox ? view->firefox_status : view->extras_status;
    if (communicated && g_subprocess_get_successful(request->process)) {
        gtk_label_set_text(GTK_LABEL(status),
                           request->dither ? (request->restore ? "Original wallpaper restored." : "Dithered copy selected. Your original image is unchanged.") :
                           request->logo ? "Quartz diamond selected for desktop menus." :
                           request->firefox ? "Quartz theme added. Open Firefox to see the change." :
                           "Wallpapers added. Open Appearance → Background to choose one.");
    } else {
        gtk_label_set_text(GTK_LABEL(status),
                           request->dither ? "Wallpaper action failed. See the error and try again." :
                           request->logo ? "Logo installation failed. See the error and try again." :
                           request->firefox ? "Theme installation failed. See the error and try again." :
                           "Download failed. Check your connection and try again.");
        show_error(GTK_WINDOW(request->window), request->dither ?
                   "Could not change the wallpaper" : request->logo ?
                   "Could not use the Quartz logo" : request->firefox ?
                   "Could not add the Firefox theme" : "Could not download wallpapers",
                   error != NULL ? error->message :
                   (errors != NULL && *errors != '\0' ? errors :
                    "The Extras installer did not complete. Please try again."));
    }
    g_clear_error(&error);
    g_free(output);
    g_free(errors);
    g_object_unref(request->process);
    g_object_unref(request->window);
    g_free(request);
}

static void
download_extras(GtkButton *button, gpointer user_data)
{
    SettingsView *view = user_data;
    GSubprocess *process;
    GError *error = NULL;
    HelperRequest *request;

    gboolean firefox = GTK_WIDGET(button) == view->firefox_button;
    gboolean logo = GTK_WIDGET(button) == view->logo_button;
    gboolean restore = GTK_WIDGET(button) == view->restore_wallpaper_button;
    gboolean dither = restore || GTK_WIDGET(button) == view->dither_button;
    gchar *directory;
    gchar *program;

    if (view->downloading_extras)
        return;
    directory = g_path_get_dirname(QUARTZ_EXTRAS_PROGRAM);
    program = dither ? g_build_filename(directory, "dither", "install.py", NULL) :
              logo ? g_build_filename(directory, "logo", "install.py", NULL) :
              firefox ? g_build_filename(directory, "firefox", "install.py", NULL) :
                        g_strdup(QUARTZ_EXTRAS_PROGRAM);
    g_free(directory);
    const gchar *colors = gtk_combo_box_get_active_id(GTK_COMBO_BOX(view->dither_colors));
    gchar pixel_size[12];
    gtk_spin_button_update(GTK_SPIN_BUTTON(view->dither_pixel_size));
    g_snprintf(pixel_size, sizeof pixel_size, "%d",
               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(view->dither_pixel_size)));
    const gchar *arguments[] = { program, NULL, NULL, NULL, NULL, NULL };
    if (restore) {
        arguments[1] = "--restore";
    } else if (dither) {
        arguments[1] = "--colors";
        arguments[2] = colors != NULL ? colors : "64";
        arguments[3] = "--pixel-size";
        arguments[4] = pixel_size;
    }
    process = g_subprocess_newv(arguments,
                                G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE,
                                &error);
    if (process == NULL) {
        show_launch_error(GTK_WINDOW(view->window), program, error);
        g_free(program);
        g_clear_error(&error);
        return;
    }
    g_free(program);
    view->downloading_extras = TRUE;
    gtk_widget_set_sensitive(view->extras_button, FALSE);
    gtk_widget_set_sensitive(view->firefox_button, FALSE);
    gtk_widget_set_sensitive(view->logo_button, FALSE);
    gtk_widget_set_sensitive(view->dither_button, FALSE);
    gtk_widget_set_sensitive(view->dither_colors, FALSE);
    gtk_widget_set_sensitive(view->dither_pixel_size, FALSE);
    gtk_widget_set_sensitive(view->restore_wallpaper_button, FALSE);
    gtk_spinner_start(GTK_SPINNER(dither ? view->dither_spinner : logo ? view->logo_spinner : firefox ? view->firefox_spinner : view->extras_spinner));
    gtk_label_set_text(GTK_LABEL(dither ? view->dither_status : logo ? view->logo_status : firefox ? view->firefox_status : view->extras_status),
                       dither ? (restore ? "Restoring original wallpaper…" : "Creating a dithered wallpaper copy…") :
                       logo ? "Adding the Quartz desktop logo…" :
                       firefox ? "Adding the Quartz Firefox theme…" :
                       "Downloading and adding recommended wallpapers…");
    request = g_new0(HelperRequest, 1);
    request->view = view;
    request->firefox = firefox;
    request->logo = logo;
    request->dither = dither;
    request->restore = restore;
    request->window = g_object_ref(view->window);
    request->process = process;
    g_subprocess_communicate_utf8_async(process, NULL, NULL, extras_finished, request);
}

static GtkWidget *
build_extras_page(SettingsView *view)
{
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *frame = gtk_frame_new("Recommended wallpapers");
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *description = gtk_label_new(
        "Download the System7 wallpaper collection and add it to your wallpapers.");
    GtkWidget *progress = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_container_set_border_width(GTK_CONTAINER(page), 12);
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(frame), content);
    gtk_label_set_xalign(GTK_LABEL(description), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(description), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(description), 54);
    gtk_box_pack_start(GTK_BOX(content), description, FALSE, FALSE, 0);
    view->extras_button = gtk_button_new_with_mnemonic("_Download Recommended Wallpapers");
    gtk_widget_set_halign(view->extras_button, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(content), view->extras_button, FALSE, FALSE, 0);
    view->extras_spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(progress), view->extras_spinner, FALSE, FALSE, 0);
    view->extras_status = gtk_label_new("Your current background stays selected.");
    gtk_label_set_xalign(GTK_LABEL(view->extras_status), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(view->extras_status), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(view->extras_status), 50);
    gtk_box_pack_start(GTK_BOX(progress), view->extras_status, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), progress, FALSE, FALSE, 0);
    g_signal_connect(view->extras_button, "clicked", G_CALLBACK(download_extras), view);

    frame = gtk_frame_new("Wallpaper dithering");
    content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    gtk_container_add(GTK_CONTAINER(frame), content);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    GtkWidget *color_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *color_label = gtk_label_new_with_mnemonic("_Color count:");
    view->dither_colors = gtk_combo_box_text_new();
    const gchar *color_counts[] = { "2", "4", "8", "16", "32", "64", "128", "256" };
    for (guint i = 0; i < G_N_ELEMENTS(color_counts); i++) {
        const gchar *label = color_counts[i];
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(view->dither_colors), color_counts[i], label);
    }
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(view->dither_colors), "64");
    gtk_label_set_mnemonic_widget(GTK_LABEL(color_label), view->dither_colors);
    gtk_box_pack_start(GTK_BOX(color_row), color_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(color_row), view->dither_colors, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), color_row, FALSE, FALSE, 0);
    GtkWidget *pixel_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *pixel_label = gtk_label_new_with_mnemonic("_Pixel size:");
    view->dither_pixel_size = gtk_spin_button_new_with_range(1, 32, 1);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(view->dither_pixel_size), TRUE);
    gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(view->dither_pixel_size), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(view->dither_pixel_size), 1);
    gtk_label_set_mnemonic_widget(GTK_LABEL(pixel_label), view->dither_pixel_size);
    gtk_box_pack_start(GTK_BOX(pixel_row), pixel_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pixel_row), view->dither_pixel_size, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pixel_row), gtk_label_new("px · larger values make chunkier pixels"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), pixel_row, FALSE, FALSE, 0);
    view->dither_button = gtk_button_new_with_label("Dither current wallpaper");
    view->restore_wallpaper_button = gtk_button_new_with_label("Restore original wallpaper");
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(actions), view->dither_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), view->restore_wallpaper_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), actions, FALSE, FALSE, 0);
    progress = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    view->dither_spinner = gtk_spinner_new();
    view->dither_status = gtk_label_new("Dither using colors from your original wallpaper. Saves a separate copy.");
    gtk_label_set_xalign(GTK_LABEL(view->dither_status), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(view->dither_status), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(view->dither_status), 54);
    gtk_box_pack_start(GTK_BOX(progress), view->dither_spinner, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(progress), view->dither_status, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), progress, FALSE, FALSE, 0);
    g_signal_connect(view->dither_button, "clicked", G_CALLBACK(download_extras), view);
    g_signal_connect(view->restore_wallpaper_button, "clicked", G_CALLBACK(download_extras), view);

    frame = gtk_frame_new("Firefox theme");
    content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    gtk_container_add(GTK_CONTAINER(frame), content);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    description = gtk_label_new(
        "Give Firefox Quartz tabs, toolbars, fonts, and your saved Quartz colors. "
        "Close Firefox before adding the theme, then reopen it.");
    gtk_label_set_xalign(GTK_LABEL(description), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(description), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(description), 54);
    gtk_box_pack_start(GTK_BOX(content), description, FALSE, FALSE, 0);
    view->firefox_button = gtk_button_new_with_label("Add quartz theme to firefox");
    gtk_widget_set_halign(view->firefox_button, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(content), view->firefox_button, FALSE, FALSE, 0);
    view->firefox_status = gtk_label_new(
        "Applies to your existing Firefox profiles. Click again after changing Quartz colors.");
    gtk_label_set_xalign(GTK_LABEL(view->firefox_status), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(view->firefox_status), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(view->firefox_status), 54);
    progress = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    view->firefox_spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(progress), view->firefox_spinner, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(progress), view->firefox_status, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), progress, FALSE, FALSE, 0);
    g_signal_connect(view->firefox_button, "clicked", G_CALLBACK(download_extras), view);

    frame = gtk_frame_new("Desktop menu logo");
    content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    gtk_container_add(GTK_CONTAINER(frame), content);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    description = gtk_label_new("Replace the MATE logo in desktop menus with a simple Quartz diamond.");
    gtk_label_set_xalign(GTK_LABEL(description), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(description), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(description), 54);
    gtk_box_pack_start(GTK_BOX(content), description, FALSE, FALSE, 0);
    view->logo_button = gtk_button_new_with_label("Use Quartz logo instead of MATE");
    gtk_widget_set_halign(view->logo_button, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(content), view->logo_button, FALSE, FALSE, 0);
    progress = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    view->logo_spinner = gtk_spinner_new();
    view->logo_status = gtk_label_new("Keeps the rest of your current desktop icons.");
    gtk_label_set_xalign(GTK_LABEL(view->logo_status), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(view->logo_status), TRUE);
    gtk_box_pack_start(GTK_BOX(progress), view->logo_spinner, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(progress), view->logo_status, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), progress, FALSE, FALSE, 0);
    g_signal_connect(view->logo_button, "clicked", G_CALLBACK(download_extras), view);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), page);
    return scroll;
}

static GtkWidget *
left_label(const gchar *text, GtkWidget *mnemonic_widget)
{
    GtkWidget *label = gtk_label_new_with_mnemonic(text);

    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), mnemonic_widget);
    return label;
}

static GtkWidget *
new_color_button(const gchar *title)
{
    GtkWidget *button = gtk_color_button_new();

    gtk_color_button_set_title(GTK_COLOR_BUTTON(button), title);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(button), FALSE);
    gtk_widget_set_hexpand(button, TRUE);
    return button;
}

#ifndef QUARTZ_TYPOGRAPHY_PROGRAM
#define QUARTZ_TYPOGRAPHY_PROGRAM "/usr/local/share/quartz-settings/theme/Quartz-System6/typography.py"
#endif

typedef struct {
    GtkWidget *family[3], *size[3], *sample[3], *status, *apply, *page;
    gchar **catalog;
    SettingsView *owner;
} TypographyView;

static void typography_free(gpointer data)
{
    TypographyView *view = data;
    g_strfreev(view->catalog);
    g_free(view);
}

static void typography_sample(GtkComboBox *combo, gpointer data)
{
    TypographyView *view = data;
    guint i;
    (void)combo;
    for (i = 0; i < 3; i++) {
        gchar *family = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->family[i]));
        gchar *size = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->size[i]));
        if (family != NULL && size != NULL) {
            gchar *name = g_strdup_printf("%s, %spx", family, size);
            PangoFontDescription *font = pango_font_description_from_string(name);
            PangoAttrList *attrs = pango_attr_list_new();
            pango_attr_list_insert(attrs, pango_attr_font_desc_new(font));
            gtk_label_set_attributes(GTK_LABEL(view->sample[i]), attrs);
            pango_attr_list_unref(attrs);
            pango_font_description_free(font);
            g_free(name);
        }
        g_free(family);
        g_free(size);
    }
}

static void typography_family(GtkComboBox *combo, gpointer data)
{
    TypographyView *view = data;
    guint i, j;
    for (i = 0; i < 3; i++) {
        if (GTK_WIDGET(combo) == view->family[i]) {
            gint index = gtk_combo_box_get_active(combo);
            gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(view->size[i]));
            if (index >= 0) {
                gchar **fields = g_strsplit(view->catalog[index], "\t", 2);
                gchar **sizes = g_strsplit(fields[1], ",", -1);
                for (j = 0; sizes[j] != NULL; j++)
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(view->size[i]), sizes[j]);
                gtk_combo_box_set_active(GTK_COMBO_BOX(view->size[i]), 0);
                g_strfreev(sizes);
                g_strfreev(fields);
            }
        }
    }
    typography_sample(NULL, view);
}

static void typography_finished(GObject *source, GAsyncResult *result, gpointer data)
{
    TypographyView *view = data;
    gchar *output = NULL, *errors = NULL;
    GError *error = NULL;
    gboolean ok = g_subprocess_communicate_utf8_finish(G_SUBPROCESS(source), result, &output, &errors, &error);
    gtk_label_set_text(GTK_LABEL(view->status),
        ok && g_subprocess_get_successful(G_SUBPROCESS(source))
        ? "Typography applied to the shared desktop theme."
        : error != NULL ? error->message : errors != NULL ? errors : "Typography could not be applied.");
    gtk_widget_set_sensitive(view->apply, TRUE);
    view->owner->typography_applying = FALSE;
    g_clear_error(&error);
    g_free(output);
    g_free(errors);
    g_object_unref(source);
    g_object_unref(view->page);
}

static void typography_apply(GtkButton *button, gpointer data)
{
    TypographyView *view = data;
    gchar *names[3];
    guint i;
    GError *error = NULL;
    GSubprocess *process;
    (void)button;
    if (view->owner->applying || view->owner->typography_applying) return;
    for (i = 0; i < 3; i++) {
        gchar *family = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->family[i]));
        gchar *size = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->size[i]));
        names[i] = g_strdup_printf("%s %spx", family, size);
        g_free(family);
        g_free(size);
    }
    process = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE,
        &error, "python3", QUARTZ_TYPOGRAPHY_PROGRAM, "--apply", names[0], names[1], names[2], NULL);
    for (i = 0; i < 3; i++) g_free(names[i]);
    if (process == NULL) {
        gtk_label_set_text(GTK_LABEL(view->status), error->message);
        g_clear_error(&error);
        return;
    }
    view->owner->typography_applying = TRUE;
    gtk_widget_set_sensitive(view->apply, FALSE);
    gtk_label_set_text(GTK_LABEL(view->status), "Applying shared typography…");
    g_object_ref(view->page);
    g_subprocess_communicate_utf8_async(process, NULL, NULL, typography_finished, view);
}

static GtkWidget *build_typography_page(SettingsView *owner)
{
    const gchar *roles[] = { "Menu bar", "Window titles", "Application" };
    TypographyView *view = g_new0(TypographyView, 1);
    gchar *output = NULL, *current = NULL;
    gchar **saved = NULL;
    GError *error = NULL;
    gint status;
    guint i, j;
    gchar *argv[] = { "python3", QUARTZ_TYPOGRAPHY_PROGRAM, "--catalog", NULL };
    view->owner = owner;
    view->page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(view->page), 16);
    g_object_set_data_full(G_OBJECT(view->page), "typography-view", view, typography_free);
    view->status = gtk_label_new("Choose a pixel font and its native size. Application size requests use the nearest available size; ties use the smaller size.");
    gtk_label_set_line_wrap(GTK_LABEL(view->status), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(view->status), 55);
    gtk_box_pack_start(GTK_BOX(view->page), view->status, FALSE, FALSE, 0);
    if (!g_spawn_sync(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &output, NULL, &status, &error)
        || !g_spawn_check_wait_status(status, error == NULL ? &error : NULL) || output == NULL || *output == '\0') {
        gtk_label_set_text(GTK_LABEL(view->status), error != NULL ? error->message : "No installed pixel fonts were found.");
        g_clear_error(&error);
        g_free(output);
        return view->page;
    }
    view->catalog = g_strsplit(g_strchomp(output), "\n", -1);
    g_free(output);
    argv[2] = "--current";
    if (g_spawn_sync(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &current, NULL, &status, NULL)
        && status == 0 && current != NULL)
        saved = g_strsplit(g_strchomp(current), "\n", -1);
    g_free(current);
    for (i = 0; i < 3; i++) {
        GtkWidget *frame = gtk_frame_new(roles[i]);
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_container_set_border_width(GTK_CONTAINER(box), 10);
        gtk_container_add(GTK_CONTAINER(frame), box);
        gtk_box_pack_start(GTK_BOX(view->page), frame, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), row, FALSE, FALSE, 0);
        view->family[i] = gtk_combo_box_text_new();
        view->size[i] = gtk_combo_box_text_new();
        gtk_widget_set_tooltip_text(view->family[i], roles[i]);
        gtk_widget_set_tooltip_text(view->size[i], "Native size in pixels");
        gtk_box_pack_start(GTK_BOX(row), view->family[i], TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(row), view->size[i], FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(row), gtk_label_new("px"), FALSE, FALSE, 0);
        view->sample[i] = gtk_label_new("File  Edit  View — Quartz 0123456789");
        gtk_label_set_ellipsize(GTK_LABEL(view->sample[i]), PANGO_ELLIPSIZE_END);
        gtk_box_pack_start(GTK_BOX(box), view->sample[i], FALSE, FALSE, 0);
        for (j = 0; view->catalog[j] != NULL; j++) {
            gchar **fields = g_strsplit(view->catalog[j], "\t", 2);
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(view->family[i]), fields[0]);
            g_strfreev(fields);
        }
    }
    for (i = 0; i < 3; i++) {
        g_signal_connect(view->family[i], "changed", G_CALLBACK(typography_family), view);
        g_signal_connect(view->size[i], "changed", G_CALLBACK(typography_sample), view);
        gtk_combo_box_set_active(GTK_COMBO_BOX(view->family[i]), 0);
        if (saved != NULL && g_strv_length(saved) == 3) {
            for (j = 0; view->catalog[j] != NULL; j++) {
                gchar **fields = g_strsplit(view->catalog[j], "\t", 2);
                gchar **sizes = g_strsplit(fields[1], ",", -1);
                guint k;
                for (k = 0; sizes[k] != NULL; k++) {
                    gchar *name = g_strdup_printf("%s %spx", fields[0], sizes[k]);
                    if (g_str_equal(name, saved[i])) {
                        gtk_combo_box_set_active(GTK_COMBO_BOX(view->family[i]), (gint)j);
                        gtk_combo_box_set_active(GTK_COMBO_BOX(view->size[i]), (gint)k);
                    }
                    g_free(name);
                }
                g_strfreev(sizes);
                g_strfreev(fields);
            }
        }
    }
    g_strfreev(saved);
    view->apply = gtk_button_new_with_label("Apply Typography System-Wide");
    gtk_box_pack_start(GTK_BOX(view->page), view->apply, FALSE, FALSE, 0);
    g_signal_connect(view->apply, "clicked", G_CALLBACK(typography_apply), view);
    return view->page;
}

static void
activate(GtkApplication *application_instance, gpointer user_data)
{
    GtkWindow *existing_window;
    SettingsView *view;
    GtkWidget *root;
    GtkWidget *notebook;
    GtkWidget *heading;
    GtkWidget *description;
    GtkWidget *colors_frame;
    GtkWidget *colors_box;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *hint;
    GtkWidget *preview_frame;
    GtkWidget *actions;
    GtkWidget *audits_frame;
    GtkWidget *audits;
    guint preset_index;
    guint layout_index;

    (void)user_data;

    existing_window = gtk_application_get_active_window(application_instance);
    if (existing_window != NULL) {
        gtk_window_present(existing_window);
        return;
    }

    view = g_new0(SettingsView, 1);
    view->window = gtk_application_window_new(application_instance);
    gtk_window_set_title(GTK_WINDOW(view->window), "Quartz Settings");
    gtk_window_set_default_icon_name("preferences-desktop-theme");
    gtk_window_set_default_size(GTK_WINDOW(view->window), 520, 600);
    gtk_window_set_resizable(GTK_WINDOW(view->window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(view->window), 12);
    g_object_set_data_full(G_OBJECT(view->window), "quartz-settings-view", view, g_free);

    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(view->window), notebook);
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(root), 16);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), root, gtk_label_new("Appearance"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), build_typography_page(view),
                             gtk_label_new("Typography"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), build_extras_page(view),
                             gtk_label_new("Extras"));

    heading = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(heading), "<span size=\"large\">Shared Quartz appearance</span>");
    gtk_label_set_xalign(GTK_LABEL(heading), 0.0f);
    gtk_box_pack_start(GTK_BOX(root), heading, FALSE, FALSE, 0);

    description = gtk_label_new("Choose a window layout and a flat or mixed palette for all GTK applications, window titles, and desktop menu panels.");
    gtk_label_set_xalign(GTK_LABEL(description), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(description), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(description), 68);
    gtk_box_pack_start(GTK_BOX(root), description, FALSE, FALSE, 0);

    colors_frame = gtk_frame_new("Theme");
    gtk_box_pack_start(GTK_BOX(root), colors_frame, FALSE, FALSE, 0);
    colors_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(colors_box), 10);
    gtk_container_add(GTK_CONTAINER(colors_frame), colors_box);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 7);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_pack_start(GTK_BOX(colors_box), grid, FALSE, FALSE, 0);

    view->window_layout_combo = gtk_combo_box_text_new();
    for (layout_index = 0;
         layout_index < G_N_ELEMENTS(window_layout_options);
         layout_index++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(view->window_layout_combo),
                                       window_layout_options[layout_index].name);
    }
    gtk_widget_set_hexpand(view->window_layout_combo, TRUE);
    label = left_label("_Window layout", view->window_layout_combo);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view->window_layout_combo, 1, 0, 1, 1);

    view->preset_combo = gtk_combo_box_text_new();
    for (preset_index = 0; preset_index < G_N_ELEMENTS(theme_presets); preset_index++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(view->preset_combo),
                                       theme_presets[preset_index].name);
    }
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(view->preset_combo), "Custom");
    gtk_widget_set_hexpand(view->preset_combo, TRUE);
    label = left_label("Color _theme", view->preset_combo);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view->preset_combo, 1, 1, 1, 1);

    view->menubar_color = new_color_button("Choose menu bar background");
    label = left_label("_Menu bar background", view->menubar_color);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view->menubar_color, 1, 2, 1, 1);

    view->titlebar_color = new_color_button("Choose title bar background");
    label = left_label("_Title bar background", view->titlebar_color);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view->titlebar_color, 1, 3, 1, 1);

    view->application_color = new_color_button("Choose application background");
    label = left_label("_Application background", view->application_color);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), view->application_color, 1, 4, 1, 1);

    hint = gtk_label_new("Quartz retains black one-bit ink; light custom colors give the clearest contrast.");
    gtk_label_set_xalign(GTK_LABEL(hint), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(hint), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(hint), 68);
    gtk_box_pack_start(GTK_BOX(colors_box), hint, FALSE, FALSE, 0);

    preview_frame = gtk_frame_new("Preview");
    gtk_box_pack_start(GTK_BOX(colors_box), preview_frame, TRUE, TRUE, 0);
    view->preview = gtk_drawing_area_new();
    gtk_widget_set_size_request(view->preview, 420, 145);
    gtk_container_add(GTK_CONTAINER(preview_frame), view->preview);

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(colors_box), actions, FALSE, FALSE, 0);
    view->status_label = gtk_label_new("Choose a window layout and colors, then apply them system-wide.");
    gtk_label_set_xalign(GTK_LABEL(view->status_label), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(view->status_label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(view->status_label), 42);
    gtk_widget_set_hexpand(view->status_label, TRUE);
    gtk_box_pack_start(GTK_BOX(actions), view->status_label, TRUE, TRUE, 0);
    view->apply_button = gtk_button_new_with_mnemonic("_Apply System-Wide");
    gtk_widget_set_valign(view->apply_button, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(actions), view->apply_button, FALSE, FALSE, 0);

    audits_frame = gtk_frame_new("Toolkit compatibility");
    gtk_box_pack_start(GTK_BOX(root), audits_frame, FALSE, FALSE, 0);
    audits = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(audits), 10);
    gtk_container_add(GTK_CONTAINER(audits_frame), audits);
    gtk_box_pack_start(GTK_BOX(audits),
                       audit_button(GTK_WINDOW(view->window), "GTK 2 Audit", "quartz-gtk2-audit"),
                       TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(audits),
                       audit_button(GTK_WINDOW(view->window), "GTK 3 Audit", "quartz-gtk3-audit"),
                       TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(audits),
                       audit_button(GTK_WINDOW(view->window), "GTK 4 Audit", "quartz-gtk4-audit"),
                       TRUE, TRUE, 0);

    g_signal_connect(view->window_layout_combo,
                     "changed",
                     G_CALLBACK(window_layout_changed),
                     view);
    g_signal_connect(view->preset_combo, "changed", G_CALLBACK(preset_changed), view);
    g_signal_connect(view->menubar_color, "color-set", G_CALLBACK(color_changed), view);
    g_signal_connect(view->titlebar_color, "color-set", G_CALLBACK(color_changed), view);
    g_signal_connect(view->application_color, "color-set", G_CALLBACK(color_changed), view);
    g_signal_connect(view->preview, "draw", G_CALLBACK(draw_preview), view);
    g_signal_connect(view->apply_button, "clicked", G_CALLBACK(apply_theme), view);
    g_signal_connect(view->window, "delete-event", G_CALLBACK(window_delete_requested), view);

    load_theme(view);
    gtk_widget_show_all(view->window);
}

int
main(int argc, char **argv)
{
    GtkApplication *application_instance;
    int status;

    application_instance = gtk_application_new("org.quartz.Settings",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(application_instance, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(application_instance), argc, argv);
    g_object_unref(application_instance);
    return status;
}
