#include <gtk/gtk.h>

enum {
    DIALOG_INFO,
    DIALOG_WARNING,
    DIALOG_QUESTION,
    DIALOG_ERROR,
    DIALOG_CUSTOM,
    DIALOG_FILE,
    DIALOG_COLOR,
    DIALOG_FONT,
    DIALOG_ABOUT,
    DIALOG_ASSISTANT,
    DIALOG_APP_CHOOSER,
    DIALOG_SHORTCUTS
};

enum {
    COLUMN_TEXT,
    COLUMN_STATE
};

static GtkWidget *audit_window;
static GtkWidget *audit_status_label;

static GtkWidget *
new_box(GtkOrientation orientation, gint spacing)
{
    return gtk_box_new(orientation, spacing);
}

static GtkWidget *
add_section(GtkWidget *page, const gchar *title)
{
    GtkWidget *frame = gtk_frame_new(title);
    GtkWidget *content = new_box(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_set_margin_start(content, 10);
    gtk_widget_set_margin_end(content, 10);
    gtk_widget_set_margin_top(content, 10);
    gtk_widget_set_margin_bottom(content, 10);
    gtk_frame_set_child(GTK_FRAME(frame), content);
    gtk_box_append(GTK_BOX(page), frame);
    return content;
}

static GtkWidget *
scroll_page(GtkWidget *page)
{
    GtkWidget *scrolled = gtk_scrolled_window_new();

    gtk_widget_set_margin_start(page, 10);
    gtk_widget_set_margin_end(page, 10);
    gtk_widget_set_margin_top(page, 10);
    gtk_widget_set_margin_bottom(page, 10);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), page);
    return scrolled;
}

static void
add_cell(GtkWidget *grid,
         GtkWidget *widget,
         const gchar *caption,
         gint column,
         gint row)
{
    GtkWidget *box = new_box(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *label = gtk_label_new(caption);

    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_grid_attach(GTK_GRID(grid), box, column, row, 1, 1);
}

static void
set_status(const gchar *message)
{
    if (audit_status_label != NULL) {
        gtk_label_set_text(GTK_LABEL(audit_status_label), message);
    }
}

static void
push_status(GtkWidget *widget, gpointer user_data)
{
    (void)widget;
    set_status((const gchar *)user_data);
}

static void
menu_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    const gchar *name = g_action_get_name(G_ACTION(action));

    (void)parameter;
    (void)user_data;
    if (g_str_equal(name, "quit")) {
        gtk_window_destroy(GTK_WINDOW(audit_window));
    } else {
        gchar *message = g_strdup_printf("Menu action: %s", name);
        set_status(message);
        g_free(message);
    }
}

static void
menu_change_state(GSimpleAction *action, GVariant *value, gpointer user_data)
{
    gchar *printed;
    gchar *message;

    (void)user_data;
    g_simple_action_set_state(action, value);
    printed = g_variant_print(value, TRUE);
    message = g_strdup_printf("Menu state: %s = %s",
                              g_action_get_name(G_ACTION(action)),
                              printed);
    set_status(message);
    g_free(message);
    g_free(printed);
}

static void
install_actions(GtkApplication *application)
{
    static const GActionEntry entries[] = {
        { .name = "new", .activate = menu_action },
        { .name = "open", .activate = menu_action },
        { .name = "save", .activate = menu_action },
        { .name = "quit", .activate = menu_action },
        { .name = "about", .activate = menu_action },
        {
            .name = "show-guides",
            .change_state = menu_change_state,
            .state = "true"
        },
        {
            .name = "size",
            .parameter_type = "s",
            .change_state = menu_change_state,
            .state = "'medium'"
        }
    };

    g_action_map_add_action_entries(G_ACTION_MAP(application),
                                    entries,
                                    G_N_ELEMENTS(entries),
                                    application);
}

static GMenu *
build_menu_model(void)
{
    GMenu *bar = g_menu_new();
    GMenu *file = g_menu_new();
    GMenu *view = g_menu_new();
    GMenu *size = g_menu_new();
    GMenu *help = g_menu_new();
    GMenuItem *item;

    g_menu_append(file, "New", "app.new");
    g_menu_append(file, "Open…", "app.open");
    g_menu_append(file, "Save", "app.save");
    g_menu_append(file, "Quit", "app.quit");
    g_menu_append(view, "Show Guides", "app.show-guides");
    item = g_menu_item_new("Small", "app.size");
    g_menu_item_set_attribute(item, "target", "s", "small");
    g_menu_append_item(size, item);
    g_object_unref(item);
    item = g_menu_item_new("Medium", "app.size");
    g_menu_item_set_attribute(item, "target", "s", "medium");
    g_menu_append_item(size, item);
    g_object_unref(item);
    item = g_menu_item_new("Large", "app.size");
    g_menu_item_set_attribute(item, "target", "s", "large");
    g_menu_append_item(size, item);
    g_object_unref(item);
    g_menu_append_submenu(view, "Size", G_MENU_MODEL(size));
    g_menu_append(help, "About", "app.about");
    g_menu_append_submenu(bar, "File", G_MENU_MODEL(file));
    g_menu_append_submenu(bar, "View", G_MENU_MODEL(view));
    g_menu_append_submenu(bar, "Help", G_MENU_MODEL(help));
    g_object_unref(file);
    g_object_unref(view);
    g_object_unref(size);
    g_object_unref(help);
    return bar;
}

static GtkWidget *
build_menu_bar(void)
{
    GMenu *model = build_menu_model();
    GtkWidget *bar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(model));

    g_object_unref(model);
    return bar;
}

static GtkWidget *
new_window_title(const gchar *title_text, const gchar *subtitle_text)
{
    GtkWidget *box = new_box(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *title = gtk_label_new(title_text);
    GtkWidget *subtitle = gtk_label_new(subtitle_text);

    gtk_widget_add_css_class(title, "title");
    gtk_widget_add_css_class(subtitle, "subtitle");
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), subtitle);
    return box;
}

static GtkWidget *
build_header_bar(void)
{
    GtkWidget *header = gtk_header_bar_new();
    GtkWidget *title = new_window_title("Quartz GTK 4 Audit",
                                        "Native toolkit compatibility gallery");
    GtkWidget *button;

    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title);
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    button = gtk_button_new_from_icon_name("document-new-symbolic");
    gtk_widget_set_tooltip_text(button, "New");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);
    button = gtk_button_new_from_icon_name("open-menu-symbolic");
    gtk_widget_set_tooltip_text(button, "Menu");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), button);
    return header;
}

static GtkWidget *
build_buttons_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *grid;
    GtkWidget *widget;
    GtkWidget *box;
    GtkWidget *radio_a;

    section = add_section(page, "Push buttons and semantic styles");
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_append(GTK_BOX(section), grid);
    add_cell(grid, gtk_button_new_with_label("Normal"), "Normal", 0, 0);
    add_cell(grid, gtk_button_new_with_mnemonic("_Mnemonic"), "Mnemonic", 1, 0);
    widget = gtk_button_new_with_label("Suggested");
    gtk_widget_add_css_class(widget, "suggested-action");
    add_cell(grid, widget, "Suggested action", 2, 0);
    widget = gtk_button_new_with_label("Destructive");
    gtk_widget_add_css_class(widget, "destructive-action");
    add_cell(grid, widget, "Destructive action", 3, 0);
    widget = gtk_button_new_with_label("Flat");
    gtk_button_set_has_frame(GTK_BUTTON(widget), FALSE);
    add_cell(grid, widget, "Flat", 0, 1);
    widget = gtk_button_new_from_icon_name("view-refresh-symbolic");
    gtk_widget_add_css_class(widget, "circular");
    add_cell(grid, widget, "Icon/circular", 1, 1);
    widget = gtk_button_new_with_label("Insensitive");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 2, 1);
    add_cell(grid,
             gtk_link_button_new_with_label("https://example.invalid", "Link button"),
             "Link",
             3,
             1);

    section = add_section(page, "Linked buttons");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(box, "linked");
    gtk_box_append(GTK_BOX(box), gtk_button_new_from_icon_name("go-previous-symbolic"));
    gtk_box_append(GTK_BOX(box), gtk_button_new_with_label("Today"));
    gtk_box_append(GTK_BOX(box), gtk_button_new_from_icon_name("go-next-symbolic"));
    gtk_box_append(GTK_BOX(section), box);

    section = add_section(page, "Toggle, check, radio, and switch states");
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_append(GTK_BOX(section), grid);
    widget = gtk_toggle_button_new_with_label("Off");
    add_cell(grid, widget, "Toggle off", 0, 0);
    widget = gtk_toggle_button_new_with_label("On");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Toggle on", 1, 0);
    widget = gtk_check_button_new_with_label("Unchecked");
    add_cell(grid, widget, "Check off", 2, 0);
    widget = gtk_check_button_new_with_label("Checked");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Check on", 3, 0);
    widget = gtk_check_button_new_with_label("Inconsistent");
    gtk_check_button_set_inconsistent(GTK_CHECK_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Mixed", 4, 0);
    radio_a = gtk_check_button_new_with_label("Choice A");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(radio_a), TRUE);
    add_cell(grid, radio_a, "Grouped selected", 0, 1);
    widget = gtk_check_button_new_with_label("Choice B");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(widget), GTK_CHECK_BUTTON(radio_a));
    add_cell(grid, widget, "Grouped clear", 1, 1);
    widget = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(widget), TRUE);
    add_cell(grid, widget, "Switch on", 2, 1);
    widget = gtk_switch_new();
    add_cell(grid, widget, "Switch off", 3, 1);
    widget = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(widget), TRUE);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled switch", 4, 1);
    widget = gtk_check_button_new_with_label("Disabled checked");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), TRUE);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled check", 0, 2);
    widget = gtk_check_button_new_with_label("Disabled radio");
    gtk_check_button_set_group(GTK_CHECK_BUTTON(widget), GTK_CHECK_BUTTON(radio_a));
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled grouped", 1, 2);

    section = add_section(page, "Button alignment and size behavior");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    widget = gtk_button_new_with_label("Start aligned");
    gtk_widget_set_halign(widget, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_button_new_with_label("Expanding button");
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_button_new_with_label("End aligned");
    gtk_widget_set_halign(widget, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    return scroll_page(page);
}

static GtkWidget *
build_inputs_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *grid;
    GtkWidget *widget;
    GtkWidget *entry;
    GtkWidget *box;
    const gchar *choices[] = { "First item", "Second item", "Third item", NULL };

    section = add_section(page, "Labels and text states");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(section), grid);
    add_cell(grid, gtk_label_new("Plain label"), "Plain", 0, 0);
    widget = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(widget), "<b>Bold</b>, <i>italic</i>, <u>underline</u>");
    add_cell(grid, widget, "Pango markup", 1, 0);
    widget = gtk_label_new("Selectable text");
    gtk_label_set_selectable(GTK_LABEL(widget), TRUE);
    add_cell(grid, widget, "Selectable", 2, 0);
    widget = gtk_label_new("A deliberately long label that demonstrates end ellipsizing");
    gtk_label_set_ellipsize(GTK_LABEL(widget), PANGO_ELLIPSIZE_END);
    gtk_widget_set_size_request(widget, 160, -1);
    add_cell(grid, widget, "Ellipsized", 3, 0);
    entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), "Mnemonic target");
    widget = gtk_label_new_with_mnemonic("_Name:");
    gtk_label_set_mnemonic_widget(GTK_LABEL(widget), entry);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(box), entry);
    add_cell(grid, box, "Label + mnemonic", 0, 1);
    widget = gtk_label_new("This label wraps onto more than one line when its allocation is narrow.");
    gtk_label_set_wrap(GTK_LABEL(widget), TRUE);
    gtk_widget_set_size_request(widget, 180, -1);
    add_cell(grid, widget, "Wrapped", 1, 1);
    widget = gtk_label_new("Insensitive label");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 2, 1);
    widget = gtk_editable_label_new("Click to edit this label");
    add_cell(grid, widget, "Editable label", 3, 1);

    section = add_section(page, "Entries and editable states");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(section), grid);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Editable text");
    add_cell(grid, widget, "Normal", 0, 0);
    widget = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "Placeholder text");
    add_cell(grid, widget, "Placeholder", 1, 0);
    widget = gtk_password_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Password");
    gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(widget), TRUE);
    add_cell(grid, widget, "Password + reveal", 2, 0);
    widget = gtk_search_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Search query");
    add_cell(grid, widget, "Search entry", 3, 0);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Read only");
    gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
    add_cell(grid, widget, "Read-only", 0, 1);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Disabled");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Insensitive", 1, 1);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Primary and secondary icons");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_PRIMARY, "edit-find-symbolic");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, "edit-clear-symbolic");
    add_cell(grid, widget, "Entry icons", 2, 1);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Entry progress");
    gtk_entry_set_progress_fraction(GTK_ENTRY(widget), 0.64);
    add_cell(grid, widget, "Progress", 3, 1);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Has selection");
    gtk_editable_select_region(GTK_EDITABLE(widget), 4, 13);
    add_cell(grid, widget, "Selected text", 0, 2);
    widget = gtk_spin_button_new_with_range(-10, 50, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 7);
    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Spin button", 1, 2);
    widget = gtk_spin_button_new_with_range(0, 1, 0.05);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 0.35);
    add_cell(grid, widget, "Decimal spin", 2, 2);
    widget = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widget), "Hover for tooltip");
    gtk_widget_set_tooltip_text(widget, "GTK 4 themed tooltip surface");
    add_cell(grid, widget, "Tooltip", 3, 2);

    widget = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 12);
    add_cell(grid, widget, "Vertical spin", 0, 3);
    widget = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled vertical spin", 1, 3);

    section = add_section(page, "Choice and chooser controls");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(section), grid);
    widget = gtk_drop_down_new_from_strings(choices);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(widget), 1);
    add_cell(grid, widget, "Drop-down", 0, 0);
    widget = gtk_drop_down_new_from_strings(choices);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(widget), TRUE);
    add_cell(grid, widget, "Searchable drop-down", 1, 0);
    widget = gtk_font_button_new();
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(widget), "Sans 12");
    add_cell(grid, widget, "Font button", 2, 0);
    widget = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), TRUE);
    add_cell(grid, widget, "Color button", 3, 0);
    widget = gtk_app_chooser_button_new("text/plain");
    gtk_app_chooser_button_set_show_default_item(GTK_APP_CHOOSER_BUTTON(widget), TRUE);
    add_cell(grid, widget, "App chooser", 0, 1);
    widget = gtk_volume_button_new();
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.65);
    add_cell(grid, widget, "Volume button", 1, 1);
    widget = gtk_drop_down_new_from_strings(choices);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled drop-down", 2, 1);

    return scroll_page(page);
}

static gboolean
pulse_progress(gpointer user_data)
{
    if (GTK_IS_PROGRESS_BAR(user_data)) {
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(user_data));
        return G_SOURCE_CONTINUE;
    }
    return G_SOURCE_REMOVE;
}

static GtkWidget *
build_ranges_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *grid;
    GtkWidget *widget;
    GtkWidget *box;
    GtkAdjustment *adjustment;

    section = add_section(page, "Scales and marks");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_append(GTK_BOX(section), grid);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 42);
    gtk_scale_set_digits(GTK_SCALE(widget), 0);
    gtk_scale_add_mark(GTK_SCALE(widget), 0, GTK_POS_BOTTOM, "0");
    gtk_scale_add_mark(GTK_SCALE(widget), 50, GTK_POS_BOTTOM, "50");
    gtk_scale_add_mark(GTK_SCALE(widget), 100, GTK_POS_BOTTOM, "100");
    add_cell(grid, widget, "Horizontal + marks", 0, 0);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1, 1, 0.1);
    gtk_range_set_value(GTK_RANGE(widget), 0.3);
    gtk_scale_set_value_pos(GTK_SCALE(widget), GTK_POS_TOP);
    add_cell(grid, widget, "Value above", 1, 0);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 67);
    gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
    add_cell(grid, widget, "Inverted", 0, 1);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 35);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 1, 1);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 58);
    gtk_scale_set_value_pos(GTK_SCALE(widget), GTK_POS_RIGHT);
    gtk_widget_set_size_request(widget, -1, 140);
    add_cell(grid, widget, "Vertical", 0, 2);
    widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 25);
    gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(widget), FALSE);
    gtk_widget_set_size_request(widget, -1, 140);
    add_cell(grid, widget, "Vertical inverted", 1, 2);

    section = add_section(page, "Progress and level bars");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_append(GTK_BOX(section), grid);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.38);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), TRUE);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), "38%");
    add_cell(grid, widget, "Determinate", 0, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.76);
    gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(widget), TRUE);
    add_cell(grid, widget, "Inverted", 1, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(widget), 0.12);
    g_timeout_add_full(G_PRIORITY_DEFAULT,
                       120,
                       pulse_progress,
                       g_object_ref(widget),
                       g_object_unref);
    add_cell(grid, widget, "Activity pulse", 2, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.55);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 3, 0);
    widget = gtk_level_bar_new_for_interval(0, 100);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(widget), 24);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "low", 25);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "high", 75);
    add_cell(grid, widget, "Level low", 0, 1);
    widget = gtk_level_bar_new_for_interval(0, 100);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(widget), 62);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "low", 25);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "high", 75);
    add_cell(grid, widget, "Level middle", 1, 1);
    widget = gtk_level_bar_new_for_interval(0, 100);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(widget), 92);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "high", 75);
    gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(widget), "full", 90);
    add_cell(grid, widget, "Level full", 2, 1);
    widget = gtk_level_bar_new_for_interval(0, 5);
    gtk_level_bar_set_mode(GTK_LEVEL_BAR(widget), GTK_LEVEL_BAR_MODE_DISCRETE);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(widget), 3);
    add_cell(grid, widget, "Discrete level", 3, 1);

    section = add_section(page, "Native scrollbars and scale buttons");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 18);
    gtk_box_append(GTK_BOX(section), box);
    adjustment = gtk_adjustment_new(32, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, adjustment);
    gtk_widget_set_size_request(widget, 380, -1);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    adjustment = gtk_adjustment_new(48, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment);
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_append(GTK_BOX(box), widget);
    adjustment = gtk_adjustment_new(20, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment);
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_volume_button_new();
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.7);
    gtk_box_append(GTK_BOX(box), widget);

    return scroll_page(page);
}

static void
string_factory_setup(GtkSignalListItemFactory *factory,
                     GtkListItem *list_item,
                     gpointer user_data)
{
    GtkWidget *box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *image = gtk_image_new_from_icon_name("text-x-generic-symbolic");
    GtkWidget *label = gtk_label_new(NULL);

    (void)factory;
    (void)user_data;
    gtk_widget_set_margin_start(box, 8);
    gtk_widget_set_margin_end(box, 8);
    gtk_widget_set_margin_top(box, 6);
    gtk_widget_set_margin_bottom(box, 6);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_box_append(GTK_BOX(box), image);
    gtk_box_append(GTK_BOX(box), label);
    gtk_list_item_set_child(list_item, box);
}

static void
string_factory_bind(GtkSignalListItemFactory *factory,
                    GtkListItem *list_item,
                    gpointer user_data)
{
    GtkStringObject *string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    GtkWidget *box = gtk_list_item_get_child(list_item);
    GtkWidget *label = gtk_widget_get_last_child(box);

    (void)factory;
    (void)user_data;
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(string_object));
}

static GtkListItemFactory *
new_string_factory(void)
{
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

    g_signal_connect(factory, "setup", G_CALLBACK(string_factory_setup), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(string_factory_bind), NULL);
    return factory;
}

static void
grid_factory_setup(GtkSignalListItemFactory *factory,
                   GtkListItem *list_item,
                   gpointer user_data)
{
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = new_box(GTK_ORIENTATION_VERTICAL, 4);

    (void)factory;
    (void)user_data;
    gtk_widget_set_margin_start(box, 8);
    gtk_widget_set_margin_end(box, 8);
    gtk_widget_set_margin_top(box, 8);
    gtk_widget_set_margin_bottom(box, 8);
    gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name("folder-symbolic"));
    gtk_box_append(GTK_BOX(box), gtk_label_new(NULL));
    gtk_button_set_child(GTK_BUTTON(button), box);
    gtk_list_item_set_child(list_item, button);
}

static void
grid_factory_bind(GtkSignalListItemFactory *factory,
                  GtkListItem *list_item,
                  gpointer user_data)
{
    GtkStringObject *string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    GtkWidget *button = gtk_list_item_get_child(list_item);
    GtkWidget *box = gtk_button_get_child(GTK_BUTTON(button));
    GtkWidget *label = gtk_widget_get_last_child(box);

    (void)factory;
    (void)user_data;
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(string_object));
}

static GtkListItemFactory *
new_grid_factory(void)
{
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

    g_signal_connect(factory, "setup", G_CALLBACK(grid_factory_setup), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(grid_factory_bind), NULL);
    return factory;
}

static void
column_factory_setup(GtkSignalListItemFactory *factory,
                     GtkListItem *list_item,
                     gpointer user_data)
{
    gint kind = GPOINTER_TO_INT(user_data);
    GtkWidget *child;

    (void)factory;
    if (kind == COLUMN_STATE) {
        child = gtk_check_button_new();
    } else {
        child = gtk_label_new(NULL);
        gtk_label_set_xalign(GTK_LABEL(child), 0.0f);
    }
    gtk_widget_set_margin_start(child, 6);
    gtk_widget_set_margin_end(child, 6);
    gtk_widget_set_margin_top(child, 5);
    gtk_widget_set_margin_bottom(child, 5);
    gtk_list_item_set_child(list_item, child);
}

static void
column_factory_bind(GtkSignalListItemFactory *factory,
                    GtkListItem *list_item,
                    gpointer user_data)
{
    gint kind = GPOINTER_TO_INT(user_data);
    GtkStringObject *string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    GtkWidget *child = gtk_list_item_get_child(list_item);

    (void)factory;
    if (kind == COLUMN_STATE) {
        const gchar *text = gtk_string_object_get_string(string_object);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(child),
                                    g_str_has_prefix(text, "Selected") ||
                                    g_str_has_prefix(text, "Completed"));
    } else {
        gtk_label_set_text(GTK_LABEL(child), gtk_string_object_get_string(string_object));
    }
}

static GtkListItemFactory *
new_column_factory(gint kind)
{
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

    g_signal_connect(factory,
                     "setup",
                     G_CALLBACK(column_factory_setup),
                     GINT_TO_POINTER(kind));
    g_signal_connect(factory,
                     "bind",
                     G_CALLBACK(column_factory_bind),
                     GINT_TO_POINTER(kind));
    return factory;
}

static GtkWidget *
build_data_page(void)
{
    static const gchar *items[] = {
        "Normal row",
        "Selected row",
        "Completed row",
        "Long text for ellipsizing and column sizing",
        "Additional list item",
        "Final list item",
        NULL
    };
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *scrolled;
    GtkWidget *view;
    GtkWidget *list_box;
    GtkWidget *row;
    GtkWidget *flow;
    GtkWidget *text_view;
    GtkStringList *strings = gtk_string_list_new(items);
    GtkSingleSelection *selection;
    GtkColumnViewColumn *column;
    gint index;

    section = add_section(page, "List view and grid view factories");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(section), box);
    selection = gtk_single_selection_new(G_LIST_MODEL(g_object_ref(strings)));
    gtk_single_selection_set_selected(selection, 1);
    view = gtk_list_view_new(GTK_SELECTION_MODEL(selection), new_string_factory());
    scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, 430, 190);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled);
    selection = gtk_single_selection_new(G_LIST_MODEL(g_object_ref(strings)));
    gtk_single_selection_set_selected(selection, 2);
    view = gtk_grid_view_new(GTK_SELECTION_MODEL(selection), new_grid_factory());
    gtk_grid_view_set_min_columns(GTK_GRID_VIEW(view), 2);
    gtk_grid_view_set_max_columns(GTK_GRID_VIEW(view), 4);
    scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, 430, 190);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled);

    section = add_section(page, "Column view and factories");
    selection = gtk_single_selection_new(G_LIST_MODEL(g_object_ref(strings)));
    gtk_single_selection_set_selected(selection, 1);
    view = gtk_column_view_new(GTK_SELECTION_MODEL(selection));
    gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(view), TRUE);
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(view), TRUE);
    column = gtk_column_view_column_new("Name", new_column_factory(COLUMN_TEXT));
    gtk_column_view_column_set_expand(column, TRUE);
    gtk_column_view_column_set_resizable(column, TRUE);
    gtk_column_view_append_column(GTK_COLUMN_VIEW(view), column);
    g_object_unref(column);
    column = gtk_column_view_column_new("Enabled", new_column_factory(COLUMN_STATE));
    gtk_column_view_column_set_fixed_width(column, 110);
    gtk_column_view_append_column(GTK_COLUMN_VIEW(view), column);
    g_object_unref(column);
    scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, -1, 180);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_box_append(GTK_BOX(section), scrolled);

    section = add_section(page, "List box and flow box");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(section), box);
    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    for (index = 0; index < 4; index++) {
        GtkWidget *label;
        row = gtk_list_box_row_new();
        label = gtk_label_new(index == 0 ? "Selected list row" :
                              index == 1 ? "Activatable row" :
                              index == 2 ? "Insensitive row" : "Final list row");
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 7);
        gtk_widget_set_margin_bottom(label, 7);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        if (index == 2) {
            gtk_widget_set_sensitive(row, FALSE);
        }
        gtk_list_box_append(GTK_LIST_BOX(list_box), row);
        if (index == 0) {
            gtk_list_box_select_row(GTK_LIST_BOX(list_box), GTK_LIST_BOX_ROW(row));
        }
    }
    gtk_widget_set_size_request(list_box, 330, 150);
    gtk_widget_set_hexpand(list_box, TRUE);
    gtk_box_append(GTK_BOX(box), list_box);
    flow = gtk_flow_box_new();
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow), GTK_SELECTION_MULTIPLE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 3);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 5);
    for (index = 1; index <= 8; index++) {
        gchar *text = g_strdup_printf("Flow %d", index);
        gtk_flow_box_append(GTK_FLOW_BOX(flow), gtk_button_new_with_label(text));
        g_free(text);
    }
    gtk_widget_set_size_request(flow, 430, 150);
    gtk_widget_set_hexpand(flow, TRUE);
    gtk_box_append(GTK_BOX(box), flow);

    section = add_section(page, "Text view, tags, selection, and wrapping");
    text_view = gtk_text_view_new();
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter start;
        GtkTextIter end;
        GtkTextTag *tag;

        gtk_text_buffer_set_text(buffer,
                                 "Editable multi-line text view\nBold, italic, underline, and selected ranges\nA long wrapped line exercises the text-view background, caret, margins, and scrollbar integration.",
                                 -1);
        gtk_text_buffer_get_iter_at_offset(buffer, &start, 30);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, 34);
        tag = gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        gtk_text_buffer_get_iter_at_offset(buffer, &start, 36);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, 42);
        tag = gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        gtk_text_buffer_get_iter_at_offset(buffer, &start, 44);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, 53);
        tag = gtk_text_buffer_create_tag(buffer, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        gtk_text_buffer_get_iter_at_offset(buffer, &start, 59);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, 67);
        gtk_text_buffer_select_range(buffer, &start, &end);
    }
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 5);
    scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled), TRUE);
    gtk_widget_set_size_request(scrolled, -1, 135);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text_view);
    gtk_box_append(GTK_BOX(section), scrolled);

    g_object_unref(strings);
    return scroll_page(page);
}

static GtkWidget *
build_containers_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *child;
    GtkWidget *paned;
    GtkWidget *notebook;
    GtkWidget *stack;
    GtkWidget *switcher;
    GtkWidget *sidebar;
    GtkWidget *revealer;
    GtkWidget *toggle;
    GtkWidget *overlay;
    GtkWidget *search_bar;
    GtkWidget *search_entry;
    GtkWidget *center;

    section = add_section(page, "Frames, separators, and expanders");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(section), box);
    widget = gtk_frame_new("Labelled frame");
    child = gtk_label_new("Standard GTK 4 frame content");
    gtk_widget_set_margin_start(child, 14);
    gtk_widget_set_margin_end(child, 14);
    gtk_widget_set_margin_top(child, 14);
    gtk_widget_set_margin_bottom(child, 14);
    gtk_frame_set_child(GTK_FRAME(widget), child);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_frame_new(NULL);
    child = gtk_label_new("Unlabelled frame content");
    gtk_widget_set_margin_start(child, 14);
    gtk_widget_set_margin_end(child, 14);
    gtk_widget_set_margin_top(child, 14);
    gtk_widget_set_margin_bottom(child, 14);
    gtk_frame_set_child(GTK_FRAME(widget), child);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_expander_new_with_mnemonic("_Expanded section");
    gtk_expander_set_expanded(GTK_EXPANDER(widget), TRUE);
    gtk_expander_set_child(GTK_EXPANDER(widget), gtk_label_new("Visible expander child"));
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_expander_new("Collapsed section");
    gtk_expander_set_child(GTK_EXPANDER(widget), gtk_label_new("Hidden expander child"));
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    section = add_section(page, "Notebook and paned containers");
    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("First page"), gtk_label_new("First"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Second page"), gtk_label_new("Second"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Third page"), gtk_label_new("Third"));
    gtk_widget_set_size_request(notebook, -1, 100);
    gtk_box_append(GTK_BOX(section), notebook);
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    child = gtk_frame_new("Left pane");
    gtk_frame_set_child(GTK_FRAME(child), gtk_label_new("Resizable left content"));
    gtk_paned_set_start_child(GTK_PANED(paned), child);
    child = gtk_frame_new("Right pane");
    gtk_frame_set_child(GTK_FRAME(child), gtk_label_new("Resizable right content"));
    gtk_paned_set_end_child(GTK_PANED(paned), child);
    gtk_paned_set_position(GTK_PANED(paned), 320);
    gtk_widget_set_size_request(paned, -1, 95);
    gtk_box_append(GTK_BOX(section), paned);

    section = add_section(page, "Stack switcher, stack sidebar, and transitions");
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("General stack page"), "general", "General");
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("Advanced stack page"), "advanced", "Advanced");
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("Details stack page"), "details", "Details");
    switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher), GTK_STACK(stack));
    gtk_box_append(GTK_BOX(section), switcher);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    sidebar = gtk_stack_sidebar_new();
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));
    gtk_widget_set_size_request(sidebar, 180, 120);
    gtk_box_append(GTK_BOX(box), sidebar);
    gtk_widget_set_hexpand(stack, TRUE);
    gtk_box_append(GTK_BOX(box), stack);
    gtk_box_append(GTK_BOX(section), box);

    section = add_section(page, "Revealer, overlay, search bar, center box, and action bar");
    toggle = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(toggle), TRUE);
    gtk_box_append(GTK_BOX(section), toggle);
    revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_child(GTK_REVEALER(revealer), gtk_label_new("Revealed child surface"));
    g_object_bind_property(toggle,
                           "active",
                           revealer,
                           "reveal-child",
                           G_BINDING_SYNC_CREATE);
    gtk_box_append(GTK_BOX(section), revealer);
    overlay = gtk_overlay_new();
    child = gtk_frame_new("Overlay base");
    gtk_widget_set_size_request(child, -1, 90);
    gtk_frame_set_child(GTK_FRAME(child), gtk_label_new("Base child"));
    gtk_overlay_set_child(GTK_OVERLAY(overlay), child);
    widget = gtk_button_new_with_label("Overlay child");
    gtk_widget_set_halign(widget, GTK_ALIGN_END);
    gtk_widget_set_valign(widget, GTK_ALIGN_START);
    gtk_widget_set_margin_end(widget, 8);
    gtk_widget_set_margin_top(widget, 8);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), widget);
    gtk_box_append(GTK_BOX(section), overlay);
    search_bar = gtk_search_bar_new();
    search_entry = gtk_search_entry_new();
    gtk_search_bar_connect_entry(GTK_SEARCH_BAR(search_bar), GTK_EDITABLE(search_entry));
    gtk_search_bar_set_child(GTK_SEARCH_BAR(search_bar), search_entry);
    gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(search_bar), TRUE);
    gtk_box_append(GTK_BOX(section), search_bar);
    center = gtk_center_box_new();
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center), gtk_button_new_with_label("Start"));
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(center), gtk_label_new("Centered"));
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(center), gtk_button_new_with_label("End"));
    gtk_box_append(GTK_BOX(section), center);
    widget = gtk_action_bar_new();
    gtk_action_bar_pack_start(GTK_ACTION_BAR(widget), gtk_button_new_with_label("Previous"));
    gtk_action_bar_set_center_widget(GTK_ACTION_BAR(widget), gtk_label_new("Action bar"));
    gtk_action_bar_pack_end(GTK_ACTION_BAR(widget), gtk_button_new_with_label("Next"));
    gtk_box_append(GTK_BOX(section), widget);

    section = add_section(page, "Fixed and aspect-frame layout");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_fixed_new();
    gtk_widget_set_size_request(widget, 360, 130);
    gtk_fixed_put(GTK_FIXED(widget), gtk_label_new("Fixed"), 12.0, 12.0);
    gtk_fixed_put(GTK_FIXED(widget), gtk_button_new_with_label("Positioned"), 110.0, 55.0);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_aspect_frame_new(0.5f, 0.5f, 1.8f, FALSE);
    gtk_aspect_frame_set_child(GTK_ASPECT_FRAME(widget), gtk_label_new("1.8:1 aspect child"));
    gtk_widget_set_size_request(widget, 300, 130);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    return scroll_page(page);
}

static GtkWidget *
new_info_bar(GtkMessageType type, const gchar *text)
{
    GtkWidget *bar = gtk_info_bar_new();

    gtk_info_bar_set_message_type(GTK_INFO_BAR(bar), type);
    gtk_info_bar_set_show_close_button(GTK_INFO_BAR(bar), TRUE);
    gtk_info_bar_add_child(GTK_INFO_BAR(bar), gtk_label_new(text));
    return bar;
}

static void
draw_audit_area(GtkDrawingArea *area,
                cairo_t *cr,
                gint width,
                gint height,
                gpointer user_data)
{
    GtkStyleContext *style_context;
    GdkRGBA background = {0.85, 0.85, 0.85, 1.0};

    (void)area;
    (void)user_data;
    style_context = gtk_widget_get_style_context(GTK_WIDGET(area));
    gtk_style_context_lookup_color(style_context, "window_bg_color", &background);
    gdk_cairo_set_source_rgba(cr, &background);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0.5, 0.5, width - 1.0, height - 1.0);
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, width, height);
    cairo_move_to(cr, width, 0.0);
    cairo_line_to(cr, 0.0, height);
    cairo_stroke(cr);
}

static void
show_gl_area(GtkWidget *button, gpointer user_data)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *area = gtk_gl_area_new();

    (void)button;
    (void)user_data;
    gtk_window_set_title(GTK_WINDOW(window), "GTK 4 GL Area");
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(audit_window));
    gtk_window_set_default_size(GTK_WINDOW(window), 480, 300);
    gtk_widget_set_tooltip_text(area, "Native GtkGLArea rendering surface");
    gtk_window_set_child(GTK_WINDOW(window), area);
    gtk_window_present(GTK_WINDOW(window));
}

static GtkWidget *
build_feedback_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *calendar;
    GtkWidget *header;
    GtkWidget *title;

    section = add_section(page, "Info bars by message type");
    gtk_box_append(GTK_BOX(section), new_info_bar(GTK_MESSAGE_INFO, "Information message"));
    gtk_box_append(GTK_BOX(section), new_info_bar(GTK_MESSAGE_WARNING, "Warning message"));
    gtk_box_append(GTK_BOX(section), new_info_bar(GTK_MESSAGE_QUESTION, "Question message"));
    gtk_box_append(GTK_BOX(section), new_info_bar(GTK_MESSAGE_ERROR, "Error message"));

    section = add_section(page, "Calendar, spinners, images, drawing, and GL areas");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_append(GTK_BOX(section), box);
    calendar = gtk_calendar_new();
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 8);
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 21);
    gtk_box_append(GTK_BOX(box), calendar);
    widget = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(widget));
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_spinner_new();
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name("dialog-information-symbolic"));
    widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(widget, 160, 95);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(widget), draw_audit_area, NULL, NULL);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_button_new_with_label("Open GL area");
    gtk_widget_set_tooltip_text(widget, "Realize GtkGLArea in a separate test window");
    g_signal_connect(widget, "clicked", G_CALLBACK(show_gl_area), NULL);
    gtk_box_append(GTK_BOX(box), widget);

    section = add_section(page, "Header bar and window-control surfaces");
    header = gtk_header_bar_new();
    title = new_window_title("Embedded Header Bar", "Subtitle and title buttons");
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title);
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), gtk_button_new_from_icon_name("document-new-symbolic"));
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), gtk_button_new_from_icon_name("open-menu-symbolic"));
    gtk_box_append(GTK_BOX(section), header);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_append(GTK_BOX(box), gtk_window_controls_new(GTK_PACK_START));
    widget = new_window_title("Window Title", "Standalone title widget");
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(box), gtk_window_controls_new(GTK_PACK_END));
    gtk_box_append(GTK_BOX(section), box);

    section = add_section(page, "Picture, video, and media controls");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 12);
    widget = gtk_picture_new();
    gtk_picture_set_content_fit(GTK_PICTURE(widget), GTK_CONTENT_FIT_CONTAIN);
    gtk_widget_set_size_request(widget, 180, 100);
    gtk_widget_set_tooltip_text(widget, "Empty GtkPicture surface");
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_video_new();
    gtk_widget_set_size_request(widget, 240, 120);
    gtk_widget_set_tooltip_text(widget, "GtkVideo without a media source");
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_media_controls_new(NULL);
    gtk_widget_set_size_request(widget, 280, 100);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    return scroll_page(page);
}

static void
dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    (void)response_id;
    (void)user_data;
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void
destroy_assistant(GtkWidget *widget, gpointer user_data)
{
    (void)user_data;
    gtk_window_destroy(GTK_WINDOW(widget));
}

static GtkWidget *
build_shortcuts_window(void)
{
    GtkWidget *window = g_object_new(GTK_TYPE_SHORTCUTS_WINDOW, NULL);
    GtkShortcutsSection *section = g_object_new(GTK_TYPE_SHORTCUTS_SECTION,
                                                "section-name", "general",
                                                "title", "General",
                                                NULL);
    GtkShortcutsGroup *group = g_object_new(GTK_TYPE_SHORTCUTS_GROUP,
                                            "title", "Window",
                                            NULL);
    GtkShortcutsShortcut *shortcut;

    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Open",
                            "accelerator", "<Primary>O",
                            NULL);
    gtk_shortcuts_group_add_shortcut(group, shortcut);
    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Search",
                            "accelerator", "<Primary>F",
                            NULL);
    gtk_shortcuts_group_add_shortcut(group, shortcut);
    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Close",
                            "accelerator", "<Primary>W",
                            NULL);
    gtk_shortcuts_group_add_shortcut(group, shortcut);
    gtk_shortcuts_section_add_group(section, group);
    gtk_shortcuts_window_add_section(GTK_SHORTCUTS_WINDOW(window), section);
    return window;
}

static void
show_dialog(GtkWidget *button, gpointer user_data)
{
    gint kind = GPOINTER_TO_INT(user_data);
    GtkWidget *dialog = NULL;
    GtkWidget *content;
    GtkWidget *entry;
    GtkWidget *page;
    GtkMessageType message_type = GTK_MESSAGE_INFO;
    const gchar *primary = "Information dialog";

    (void)button;

    if (kind >= DIALOG_INFO && kind <= DIALOG_ERROR) {
        if (kind == DIALOG_WARNING) {
            message_type = GTK_MESSAGE_WARNING;
            primary = "Warning dialog";
        } else if (kind == DIALOG_QUESTION) {
            message_type = GTK_MESSAGE_QUESTION;
            primary = "Question dialog";
        } else if (kind == DIALOG_ERROR) {
            message_type = GTK_MESSAGE_ERROR;
            primary = "Error dialog";
        }
        dialog = gtk_message_dialog_new(GTK_WINDOW(audit_window),
                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        message_type,
                                        kind == DIALOG_QUESTION ? GTK_BUTTONS_YES_NO : GTK_BUTTONS_OK,
                                        "%s",
                                        primary);
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
                                                 "Secondary text exercises wrapping and dialog hierarchy.");
    } else if (kind == DIALOG_CUSTOM) {
        dialog = gtk_dialog_new_with_buttons("Custom GTK 4 Dialog",
                                             GTK_WINDOW(audit_window),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_USE_HEADER_BAR,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "OK",
                                             GTK_RESPONSE_OK,
                                             NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
        content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_widget_set_margin_start(content, 12);
        gtk_widget_set_margin_end(content, 12);
        gtk_widget_set_margin_top(content, 12);
        gtk_widget_set_margin_bottom(content, 12);
        gtk_box_append(GTK_BOX(content), gtk_label_new("A content-area label and entry:"));
        entry = gtk_entry_new();
        gtk_editable_set_text(GTK_EDITABLE(entry), "Editable dialog value");
        gtk_box_append(GTK_BOX(content), entry);
    } else if (kind == DIALOG_FILE) {
        dialog = gtk_file_chooser_dialog_new("GTK 4 File Chooser",
                                             GTK_WINDOW(audit_window),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "Open",
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_COLOR) {
        dialog = gtk_color_chooser_dialog_new("GTK 4 Color Chooser", GTK_WINDOW(audit_window));
        gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_FONT) {
        dialog = gtk_font_chooser_dialog_new("GTK 4 Font Chooser", GTK_WINDOW(audit_window));
        gtk_font_chooser_set_preview_text(GTK_FONT_CHOOSER(dialog), "Quartz GTK 4 font preview");
    } else if (kind == DIALOG_APP_CHOOSER) {
        dialog = gtk_app_chooser_dialog_new_for_content_type(GTK_WINDOW(audit_window),
                                                             GTK_DIALOG_MODAL,
                                                             "text/plain");
    } else if (kind == DIALOG_ABOUT) {
        dialog = gtk_about_dialog_new();
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Quartz GTK 4 Audit");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                      "A native GTK 4 compatibility surface for the shared Quartz theme.");
        gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://example.invalid/quartz");
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog),
                                     (const gchar *[]){ "Quartz Theme Project", NULL });
        gtk_window_present(GTK_WINDOW(dialog));
        return;
    } else if (kind == DIALOG_ASSISTANT) {
        dialog = gtk_assistant_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "GTK 4 Assistant");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_window_set_default_size(GTK_WINDOW(dialog), 540, 350);
        page = new_box(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(page, 18);
        gtk_widget_set_margin_end(page, 18);
        gtk_widget_set_margin_top(page, 18);
        gtk_widget_set_margin_bottom(page, 18);
        gtk_box_append(GTK_BOX(page), gtk_label_new("Introduction page with a complete state."));
        gtk_assistant_append_page(GTK_ASSISTANT(dialog), page);
        gtk_assistant_set_page_title(GTK_ASSISTANT(dialog), page, "Introduction");
        gtk_assistant_set_page_type(GTK_ASSISTANT(dialog), page, GTK_ASSISTANT_PAGE_INTRO);
        gtk_assistant_set_page_complete(GTK_ASSISTANT(dialog), page, TRUE);
        page = new_box(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_margin_start(page, 18);
        gtk_widget_set_margin_end(page, 18);
        gtk_widget_set_margin_top(page, 18);
        gtk_widget_set_margin_bottom(page, 18);
        gtk_box_append(GTK_BOX(page), gtk_check_button_new_with_label("Assistant option"));
        gtk_assistant_append_page(GTK_ASSISTANT(dialog), page);
        gtk_assistant_set_page_title(GTK_ASSISTANT(dialog), page, "Configuration");
        gtk_assistant_set_page_type(GTK_ASSISTANT(dialog), page, GTK_ASSISTANT_PAGE_CONTENT);
        gtk_assistant_set_page_complete(GTK_ASSISTANT(dialog), page, TRUE);
        page = gtk_label_new("Assistant summary page");
        gtk_assistant_append_page(GTK_ASSISTANT(dialog), page);
        gtk_assistant_set_page_title(GTK_ASSISTANT(dialog), page, "Summary");
        gtk_assistant_set_page_type(GTK_ASSISTANT(dialog), page, GTK_ASSISTANT_PAGE_SUMMARY);
        gtk_assistant_set_page_complete(GTK_ASSISTANT(dialog), page, TRUE);
        g_signal_connect(dialog, "cancel", G_CALLBACK(destroy_assistant), NULL);
        g_signal_connect(dialog, "close", G_CALLBACK(destroy_assistant), NULL);
        gtk_window_present(GTK_WINDOW(dialog));
        return;
    } else if (kind == DIALOG_SHORTCUTS) {
        dialog = build_shortcuts_window();
        gtk_window_set_title(GTK_WINDOW(dialog), "Keyboard Shortcuts");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_window_present(GTK_WINDOW(dialog));
        return;
    }

    if (dialog != NULL) {
        g_signal_connect(dialog, "response", G_CALLBACK(dialog_response), NULL);
        gtk_window_present(GTK_WINDOW(dialog));
    }
}

static GtkWidget *
dialog_button(const gchar *label, gint kind)
{
    GtkWidget *button = gtk_button_new_with_label(label);

    g_signal_connect(button, "clicked", G_CALLBACK(show_dialog), GINT_TO_POINTER(kind));
    return button;
}

static GtkWidget *
build_dialogs_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *grid;
    GtkWidget *widget;
    GtkWidget *box;

    section = add_section(page, "Message dialogs");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(section), grid);
    add_cell(grid, dialog_button("Open information", DIALOG_INFO), "Information", 0, 0);
    add_cell(grid, dialog_button("Open warning", DIALOG_WARNING), "Warning", 1, 0);
    add_cell(grid, dialog_button("Open question", DIALOG_QUESTION), "Question", 2, 0);
    add_cell(grid, dialog_button("Open error", DIALOG_ERROR), "Error", 3, 0);

    section = add_section(page, "Chooser and utility dialogs");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(section), grid);
    add_cell(grid, dialog_button("Custom dialog", DIALOG_CUSTOM), "GtkDialog", 0, 0);
    add_cell(grid, dialog_button("File chooser", DIALOG_FILE), "GtkFileChooserDialog", 1, 0);
    add_cell(grid, dialog_button("Color chooser", DIALOG_COLOR), "GtkColorChooserDialog", 2, 0);
    add_cell(grid, dialog_button("Font chooser", DIALOG_FONT), "GtkFontChooserDialog", 3, 0);
    add_cell(grid, dialog_button("App chooser", DIALOG_APP_CHOOSER), "GtkAppChooserDialog", 0, 1);
    add_cell(grid, dialog_button("About dialog", DIALOG_ABOUT), "GtkAboutDialog", 1, 1);
    add_cell(grid, dialog_button("Assistant", DIALOG_ASSISTANT), "GtkAssistant", 2, 1);
    add_cell(grid, dialog_button("Shortcuts", DIALOG_SHORTCUTS), "GtkShortcutsWindow", 3, 1);

    section = add_section(page, "Embedded specialized chooser widgets");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_font_chooser_widget_new();
    gtk_font_chooser_set_preview_text(GTK_FONT_CHOOSER(widget), "Quartz GTK 4 preview");
    gtk_widget_set_size_request(widget, 470, 300);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_color_chooser_widget_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), TRUE);
    gtk_widget_set_size_request(widget, 360, 300);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);
    return scroll_page(page);
}

static void
context_menu_pressed(GtkGestureClick *gesture,
                     gint n_press,
                     gdouble x,
                     gdouble y,
                     gpointer user_data)
{
    GdkRectangle rectangle = { (gint)x, (gint)y, 1, 1 };

    (void)gesture;
    (void)n_press;
    gtk_popover_set_pointing_to(GTK_POPOVER(user_data), &rectangle);
    gtk_popover_popup(GTK_POPOVER(user_data));
}

static GtkWidget *
build_content_popover(void)
{
    GtkWidget *popover = gtk_popover_new();
    GtkWidget *box = new_box(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *widget;

    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_box_append(GTK_BOX(box), gtk_label_new("Popover content"));
    widget = gtk_check_button_new_with_label("Checked option");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(box), gtk_entry_new());
    gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_box_append(GTK_BOX(box), gtk_button_new_with_label("Popover action"));
    gtk_popover_set_child(GTK_POPOVER(popover), box);
    return popover;
}

static GtkWidget *
build_menus_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *menu_button;
    GtkWidget *popover;
    GtkWidget *target;
    GtkGesture *gesture;
    GMenu *model;
    GMenu *section_model;
    GMenu *submenu;

    section = add_section(page, "Menu buttons, model menus, popovers, and emoji chooser");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(section), box);
    menu_button = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), "Model menu");
    model = build_menu_model();
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), G_MENU_MODEL(model));
    g_object_unref(model);
    gtk_box_append(GTK_BOX(box), menu_button);
    menu_button = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), "Content popover");
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_button), build_content_popover());
    gtk_box_append(GTK_BOX(box), menu_button);
    menu_button = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), "Emoji chooser");
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_button), gtk_emoji_chooser_new());
    gtk_box_append(GTK_BOX(box), menu_button);

    section = add_section(page, "Context popover-menu state matrix");
    model = g_menu_new();
    section_model = g_menu_new();
    g_menu_append(section_model, "Normal item", "app.open");
    g_menu_append(section_model, "Checked item", "app.show-guides");
    g_menu_append_section(model, NULL, G_MENU_MODEL(section_model));
    g_object_unref(section_model);
    submenu = g_menu_new();
    {
        GMenuItem *item = g_menu_item_new("Small", "app.size");
        g_menu_item_set_attribute(item, "target", "s", "small");
        g_menu_append_item(submenu, item);
        g_object_unref(item);
        item = g_menu_item_new("Medium", "app.size");
        g_menu_item_set_attribute(item, "target", "s", "medium");
        g_menu_append_item(submenu, item);
        g_object_unref(item);
        item = g_menu_item_new("Large", "app.size");
        g_menu_item_set_attribute(item, "target", "s", "large");
        g_menu_append_item(submenu, item);
        g_object_unref(item);
    }
    g_menu_append_submenu(model, "Radio choices", G_MENU_MODEL(submenu));
    g_object_unref(submenu);
    popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(model));
    g_object_unref(model);
    target = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(target),
                              "Right-click here for a GtkPopoverMenu");
    gtk_widget_set_size_request(target, -1, 110);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(target), popover);
    gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "pressed", G_CALLBACK(context_menu_pressed), popover);
    gtk_widget_add_controller(target, GTK_EVENT_CONTROLLER(gesture));
    gtk_box_append(GTK_BOX(section), target);

    section = add_section(page, "Nested popover menu");
    model = g_menu_new();
    section_model = g_menu_new();
    g_menu_append(section_model, "New", "app.new");
    g_menu_append(section_model, "Open", "app.open");
    g_menu_append_section(model, "Actions", G_MENU_MODEL(section_model));
    g_object_unref(section_model);
    submenu = g_menu_new();
    g_menu_append(submenu, "About", "app.about");
    g_menu_append(submenu, "Quit", "app.quit");
    g_menu_append_submenu(model, "More", G_MENU_MODEL(submenu));
    g_object_unref(submenu);
    menu_button = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), "Open nested menu");
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), G_MENU_MODEL(model));
    g_object_unref(model);
    gtk_box_append(GTK_BOX(section), menu_button);
    return scroll_page(page);
}

static GtkWidget *
build_specialized_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;

    section = add_section(page, "File and application chooser widgets");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_widget_set_size_request(widget, 550, 300);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_app_chooser_widget_new("text/plain");
    gtk_widget_set_size_request(widget, 360, 300);
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    section = add_section(page, "Orientable controls and separators");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 12);
    widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(widget, 1, 100);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_progress_bar_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.65);
    gtk_widget_set_size_request(widget, 24, 130);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_level_bar_new_for_interval(0, 100);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(widget), 72);
    gtk_widget_set_size_request(widget, 30, 130);
    gtk_box_append(GTK_BOX(box), widget);
    widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(widget, 1, 100);
    gtk_box_append(GTK_BOX(box), widget);
    gtk_box_append(GTK_BOX(section), box);

    return scroll_page(page);
}

static GtkWidget *
build_bottom_bar(void)
{
    GtkWidget *bar = gtk_action_bar_new();
    GtkWidget *button;

    audit_status_label = gtk_label_new("Native GTK 4.14 audit — interact with controls to inspect every state");
    gtk_label_set_xalign(GTK_LABEL(audit_status_label), 0.0f);
    gtk_widget_set_hexpand(audit_status_label, TRUE);
    gtk_action_bar_pack_start(GTK_ACTION_BAR(bar), audit_status_label);
    button = gtk_button_new_from_icon_name("help-about-symbolic");
    gtk_widget_set_tooltip_text(button, "Audit information");
    g_signal_connect(button, "clicked", G_CALLBACK(push_status), "GTK 4 action-bar button selected");
    gtk_action_bar_pack_end(GTK_ACTION_BAR(bar), button);
    return bar;
}

static void
append_page(GtkWidget *notebook, GtkWidget *page, const gchar *label)
{
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, gtk_label_new(label));
}

static void
activate(GtkApplication *application, gpointer user_data)
{
    GtkWidget *root;
    GtkWidget *notebook;

    (void)user_data;
    install_actions(application);

    audit_window = gtk_application_window_new(application);
    gtk_window_set_title(GTK_WINDOW(audit_window), "Quartz GTK 4 Audit");
    gtk_window_set_default_size(GTK_WINDOW(audit_window), 1120, 820);
    gtk_window_set_titlebar(GTK_WINDOW(audit_window), build_header_bar());

    root = new_box(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(audit_window), root);
    gtk_box_append(GTK_BOX(root), build_menu_bar());

    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    append_page(notebook, build_buttons_page(), "Buttons & States");
    append_page(notebook, build_inputs_page(), "Text & Inputs");
    append_page(notebook, build_ranges_page(), "Ranges & Progress");
    append_page(notebook, build_data_page(), "Lists, Grids & Text");
    append_page(notebook, build_containers_page(), "Containers");
    append_page(notebook, build_feedback_page(), "Feedback & Media");
    append_page(notebook, build_menus_page(), "Menus & Popovers");
    append_page(notebook, build_dialogs_page(), "Dialogs & Choosers");
    append_page(notebook, build_specialized_page(), "Specialized Widgets");
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_box_append(GTK_BOX(root), notebook);
    gtk_box_append(GTK_BOX(root), build_bottom_bar());

    gtk_window_present(GTK_WINDOW(audit_window));
}

int
main(int argc, char **argv)
{
    GtkApplication *application;
    int status;

    application = gtk_application_new("org.quartz.Gtk4Audit",
                                      G_APPLICATION_NON_UNIQUE);
    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
