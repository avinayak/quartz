#include <gtk/gtk.h>

enum {
    DATA_ICON,
    DATA_NAME,
    DATA_ENABLED,
    DATA_PROGRESS,
    DATA_COLUMNS
};

enum {
    DIALOG_INFO,
    DIALOG_WARNING,
    DIALOG_QUESTION,
    DIALOG_ERROR,
    DIALOG_CUSTOM,
    DIALOG_FILE,
    DIALOG_RECENT,
    DIALOG_COLOR,
    DIALOG_FONT,
    DIALOG_ABOUT,
    DIALOG_ASSISTANT,
    DIALOG_APP_CHOOSER,
    DIALOG_SHORTCUTS,
    DIALOG_CSD
};

static GtkWidget *audit_window;
static GtkWidget *audit_statusbar;

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

    gtk_container_set_border_width(GTK_CONTAINER(content), 10);
    gtk_container_add(GTK_CONTAINER(frame), content);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    return content;
}

static GtkWidget *
scroll_page(GtkWidget *page)
{
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);

    gtk_container_set_border_width(GTK_CONTAINER(page), 10);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), page);
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
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(grid), box, column, row, 1, 1);
}

static void
push_status(GtkWidget *widget, gpointer user_data)
{
    guint context;

    (void)widget;
    context = gtk_statusbar_get_context_id(GTK_STATUSBAR(audit_statusbar),
                                           "audit-action");
    gtk_statusbar_push(GTK_STATUSBAR(audit_statusbar),
                       context,
                       (const gchar *)user_data);
}

static GtkWidget *
append_menu_item(GtkWidget *menu, const gchar *label, const gchar *status)
{
    GtkWidget *item = gtk_menu_item_new_with_mnemonic(label);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    if (status != NULL) {
        g_signal_connect(item, "activate", G_CALLBACK(push_status), (gpointer)status);
    }
    return item;
}

static GtkWidget *
build_menu_bar(void)
{
    GtkWidget *bar = gtk_menu_bar_new();
    GtkWidget *file_root = gtk_menu_item_new_with_mnemonic("_File");
    GtkWidget *edit_root = gtk_menu_item_new_with_mnemonic("_Edit");
    GtkWidget *view_root = gtk_menu_item_new_with_mnemonic("_View");
    GtkWidget *help_root = gtk_menu_item_new_with_mnemonic("_Help");
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *view_menu = gtk_menu_new();
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *recent_menu = gtk_menu_new();
    GtkWidget *item;
    GSList *group = NULL;

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    item = append_menu_item(file_menu, "Open _Recent", NULL);
    append_menu_item(recent_menu, "Audit document 1", "Recent item selected");
    append_menu_item(recent_menu, "Audit document 2", "Recent item selected");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), recent_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    g_signal_connect_swapped(item, "activate", G_CALLBACK(gtk_widget_destroy), audit_window);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
    gtk_widget_set_sensitive(item, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);

    item = gtk_check_menu_item_new_with_mnemonic("Show _Toolbar");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_check_menu_item_new_with_mnemonic("_Mixed State");
    gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    item = gtk_radio_menu_item_new_with_mnemonic(group, "_Small");
    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_radio_menu_item_new_with_mnemonic(group, "_Medium");
    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_radio_menu_item_new_with_mnemonic(group, "_Large");
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(push_status), "About selected");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_root), file_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_root), edit_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_root), view_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_root), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), file_root);
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), edit_root);
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), view_root);
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), help_root);
    return bar;
}

static GtkWidget *
build_toolbar(void)
{
    GtkWidget *toolbar = gtk_toolbar_new();
    GtkToolItem *item;
    GtkToolItem *separator;

    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    item = gtk_tool_button_new(NULL, "New");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item), "document-new");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_tool_button_new(NULL, "Open");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item), "document-open");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_tool_button_new(NULL, "Save");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item), "document-save");
    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    separator = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);
    item = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item), "format-text-bold");
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), "Bold");
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    separator = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
    gtk_tool_item_set_expand(separator, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);
    item = gtk_menu_tool_button_new(NULL, "Options");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item), "preferences-system");
    gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(item), gtk_menu_new());
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    return toolbar;
}

static void
add_style_class(GtkWidget *widget, const gchar *class_name)
{
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), class_name);
}

static GtkWidget *
build_buttons_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *grid;
    GtkWidget *widget;
    GtkWidget *box;
    GtkWidget *image;
    GSList *radio_group = NULL;

    section = add_section(page, "Push buttons and semantic styles");
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    add_cell(grid, gtk_button_new_with_label("Normal"), "Normal", 0, 0);
    add_cell(grid, gtk_button_new_with_mnemonic("_Mnemonic"), "Mnemonic", 1, 0);
    widget = gtk_button_new_with_label("Suggested");
    add_style_class(widget, GTK_STYLE_CLASS_SUGGESTED_ACTION);
    add_cell(grid, widget, "Suggested action", 2, 0);
    widget = gtk_button_new_with_label("Destructive");
    add_style_class(widget, GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
    add_cell(grid, widget, "Destructive action", 3, 0);
    widget = gtk_button_new_with_label("Flat");
    add_style_class(widget, GTK_STYLE_CLASS_FLAT);
    add_cell(grid, widget, "Flat", 0, 1);
    widget = gtk_button_new_from_icon_name("view-refresh", GTK_ICON_SIZE_BUTTON);
    add_style_class(widget, "circular");
    add_cell(grid, widget, "Icon/circular", 1, 1);
    widget = gtk_button_new();
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 4);
    image = gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Image + text"), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widget), box);
    add_cell(grid, widget, "Custom content", 2, 1);
    widget = gtk_button_new_with_label("Insensitive");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 3, 1);

    section = add_section(page, "Linked buttons");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 0);
    add_style_class(box, GTK_STYLE_CLASS_LINKED);
    gtk_box_pack_start(GTK_BOX(box), gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_button_new_with_label("Today"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

    section = add_section(page, "Toggle, check, radio, and switch states");
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    widget = gtk_toggle_button_new_with_label("Off");
    add_cell(grid, widget, "Toggle off", 0, 0);
    widget = gtk_toggle_button_new_with_label("On");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Toggle on", 1, 0);
    widget = gtk_check_button_new_with_label("Unchecked");
    add_cell(grid, widget, "Check off", 2, 0);
    widget = gtk_check_button_new_with_label("Checked");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Check on", 3, 0);
    widget = gtk_check_button_new_with_label("Inconsistent");
    gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Mixed", 4, 0);
    widget = gtk_radio_button_new_with_label(radio_group, "Choice A");
    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
    add_cell(grid, widget, "Radio selected", 0, 1);
    widget = gtk_radio_button_new_with_label(radio_group, "Choice B");
    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
    add_cell(grid, widget, "Radio clear", 1, 1);
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
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled check", 0, 2);
    widget = gtk_radio_button_new_with_label(radio_group, "Disabled radio");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled radio", 1, 2);
    add_cell(grid,
             gtk_link_button_new_with_label("https://example.invalid", "Link button"),
             "Link",
             2,
             2);

    section = add_section(page, "Button box layouts");
    box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_START);
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Apply"));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Reset"));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Cancel"));
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_END);
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Back"));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Forward"));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_with_label("Finish"));
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

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

    section = add_section(page, "Labels and text states");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
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
    gtk_entry_set_text(GTK_ENTRY(entry), "Mnemonic target");
    widget = gtk_label_new_with_mnemonic("_Name:");
    gtk_label_set_mnemonic_widget(GTK_LABEL(widget), entry);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 0);
    add_cell(grid, box, "Label + mnemonic", 0, 1);
    widget = gtk_label_new("This label wraps onto more than one line when its allocation is narrow.");
    gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
    gtk_widget_set_size_request(widget, 180, -1);
    add_cell(grid, widget, "Wrapped", 1, 1);
    widget = gtk_label_new("Insensitive label");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled", 2, 1);

    section = add_section(page, "Entries and editable states");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Editable text");
    add_cell(grid, widget, "Normal", 0, 0);
    widget = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "Placeholder text");
    add_cell(grid, widget, "Placeholder", 1, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, "view-reveal-symbolic");
    add_cell(grid, widget, "Password", 2, 0);
    widget = gtk_search_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Search query");
    add_cell(grid, widget, "Search entry", 3, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Read only");
    gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
    add_cell(grid, widget, "Read-only", 0, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Disabled");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Insensitive", 1, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Primary and secondary icons");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_PRIMARY, "edit-find-symbolic");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, "edit-clear-symbolic");
    add_cell(grid, widget, "Entry icons", 2, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Entry progress");
    gtk_entry_set_progress_fraction(GTK_ENTRY(widget), 0.64);
    add_cell(grid, widget, "Progress", 3, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Has selection");
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
    gtk_entry_set_text(GTK_ENTRY(widget), "Hover for tooltip");
    gtk_widget_set_tooltip_text(widget, "GTK 3 themed tooltip surface");
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
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    widget = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "First item");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "Second item");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "Third item");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 1);
    add_cell(grid, widget, "Combo box", 0, 0);
    widget = gtk_combo_box_text_new_with_entry();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "Editable choice");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "Another choice");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    add_cell(grid, widget, "Editable combo", 1, 0);
    widget = gtk_file_chooser_button_new("Choose a file", GTK_FILE_CHOOSER_ACTION_OPEN);
    add_cell(grid, widget, "File chooser", 2, 0);
    widget = gtk_font_button_new_with_font("Sans 12");
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(widget), TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(widget), TRUE);
    add_cell(grid, widget, "Font button", 3, 0);
    widget = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), TRUE);
    add_cell(grid, widget, "Color button", 0, 1);
    widget = gtk_app_chooser_button_new("text/plain");
    gtk_app_chooser_button_set_show_default_item(GTK_APP_CHOOSER_BUTTON(widget), TRUE);
    add_cell(grid, widget, "App chooser", 1, 1);
    widget = gtk_volume_button_new();
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.65);
    add_cell(grid, widget, "Volume button", 2, 1);
    widget = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), "Disabled choice");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(grid, widget, "Disabled combo", 3, 1);

    return scroll_page(page);
}

static gboolean
pulse_progress(gpointer user_data)
{
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(user_data));
    return G_SOURCE_CONTINUE;
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
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
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
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
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
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    adjustment = gtk_adjustment_new(32, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, adjustment);
    gtk_widget_set_size_request(widget, 380, -1);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    adjustment = gtk_adjustment_new(48, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment);
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    adjustment = gtk_adjustment_new(20, 0, 100, 1, 10, 20);
    widget = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment);
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    widget = gtk_volume_button_new();
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.7);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static GtkListStore *
build_list_store(void)
{
    GtkListStore *store = gtk_list_store_new(DATA_COLUMNS,
                                             G_TYPE_ICON,
                                             G_TYPE_STRING,
                                             G_TYPE_BOOLEAN,
                                             G_TYPE_INT);
    GtkTreeIter iter;
    GIcon *icon = g_themed_icon_new("text-x-generic");

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       DATA_ICON, icon,
                       DATA_NAME, "Normal row",
                       DATA_ENABLED, TRUE,
                       DATA_PROGRESS, 20,
                       -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       DATA_ICON, icon,
                       DATA_NAME, "Selected row",
                       DATA_ENABLED, FALSE,
                       DATA_PROGRESS, 55,
                       -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       DATA_ICON, icon,
                       DATA_NAME, "Completed row",
                       DATA_ENABLED, TRUE,
                       DATA_PROGRESS, 100,
                       -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       DATA_ICON, icon,
                       DATA_NAME, "Long text for ellipsizing and column sizing",
                       DATA_ENABLED, FALSE,
                       DATA_PROGRESS, 78,
                       -1);
    g_object_unref(icon);
    return store;
}

static GtkWidget *
build_tree_view(GtkListStore *store)
{
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreePath *path;

    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes("Icon", renderer, "gicon", DATA_ICON, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", DATA_NAME, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, DATA_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    renderer = gtk_cell_renderer_toggle_new();
    g_object_set(renderer, "activatable", TRUE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Enabled", renderer, "active", DATA_ENABLED, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    renderer = gtk_cell_renderer_progress_new();
    column = gtk_tree_view_column_new_with_attributes("Progress", renderer, "value", DATA_PROGRESS, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(view), GTK_TREE_VIEW_GRID_LINES_BOTH);
    path = gtk_tree_path_new_from_indices(1, -1);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(view), path, NULL, FALSE);
    gtk_tree_path_free(path);
    return view;
}

static GtkWidget *
build_data_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *scrolled;
    GtkWidget *view;
    GtkWidget *box;
    GtkWidget *list_box;
    GtkWidget *row;
    GtkWidget *flow;
    GtkWidget *label;
    GtkListStore *store = build_list_store();
    GtkTreeStore *tree_store;
    GtkTreeIter parent;
    GtkTreeIter child;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gint index;

    section = add_section(page, "Tree view and cell renderers");
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, -1, 160);
    view = build_tree_view(store);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(section), scrolled, FALSE, FALSE, 0);

    section = add_section(page, "Tree hierarchy and icon view");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    tree_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    gtk_tree_store_append(tree_store, &parent, NULL);
    gtk_tree_store_set(tree_store, &parent, 0, "Expanded parent", 1, TRUE, -1);
    gtk_tree_store_append(tree_store, &child, &parent);
    gtk_tree_store_set(tree_store, &child, 0, "Child row", 1, FALSE, -1);
    gtk_tree_store_append(tree_store, &child, &parent);
    gtk_tree_store_set(tree_store, &child, 0, "Another child", 1, TRUE, -1);
    gtk_tree_store_append(tree_store, &parent, NULL);
    gtk_tree_store_set(tree_store, &parent, 0, "Collapsed parent", 1, FALSE, -1);
    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Hierarchy", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("State", renderer, "active", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    {
        GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
        gtk_tree_view_expand_row(GTK_TREE_VIEW(view), path, FALSE);
        gtk_tree_path_free(path);
    }
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 330, 175);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(view), DATA_NAME);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(view), GTK_SELECTION_MULTIPLE);
    {
        GtkCellRenderer *pixbuf = gtk_cell_renderer_pixbuf_new();
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(view), pixbuf, FALSE);
        gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(view), pixbuf, "gicon", DATA_ICON);
    }
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 390, 175);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    section = add_section(page, "List box and flow box");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    for (index = 0; index < 4; index++) {
        row = gtk_list_box_row_new();
        label = gtk_label_new(index == 0 ? "Selected list row" :
                              index == 1 ? "Activatable row" :
                              index == 2 ? "Insensitive row" : "Final list row");
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 7);
        gtk_widget_set_margin_bottom(label, 7);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_container_add(GTK_CONTAINER(row), label);
        if (index == 2) {
            gtk_widget_set_sensitive(row, FALSE);
        }
        gtk_container_add(GTK_CONTAINER(list_box), row);
        if (index == 0) {
            gtk_list_box_select_row(GTK_LIST_BOX(list_box), GTK_LIST_BOX_ROW(row));
        }
    }
    gtk_widget_set_size_request(list_box, 320, 150);
    gtk_box_pack_start(GTK_BOX(box), list_box, TRUE, TRUE, 0);
    flow = gtk_flow_box_new();
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow), GTK_SELECTION_MULTIPLE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 3);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 5);
    for (index = 1; index <= 8; index++) {
        gchar *text = g_strdup_printf("Flow %d", index);
        gtk_flow_box_insert(GTK_FLOW_BOX(flow), gtk_button_new_with_label(text), -1);
        g_free(text);
    }
    gtk_widget_set_size_request(flow, 420, 150);
    gtk_box_pack_start(GTK_BOX(box), flow, TRUE, TRUE, 0);

    section = add_section(page, "Text view, tags, selection, and wrapping");
    view = gtk_text_view_new();
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
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
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 5);
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, -1, 130);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(section), scrolled, FALSE, FALSE, 0);

    g_object_unref(store);
    g_object_unref(tree_store);
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
    gint index;
    const gchar *shadow_names[] = { "None", "In", "Out", "Etched in", "Etched out" };
    GtkShadowType shadows[] = {
        GTK_SHADOW_NONE,
        GTK_SHADOW_IN,
        GTK_SHADOW_OUT,
        GTK_SHADOW_ETCHED_IN,
        GTK_SHADOW_ETCHED_OUT
    };

    section = add_section(page, "Frames, separators, and expanders");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    for (index = 0; index < 5; index++) {
        widget = gtk_frame_new(shadow_names[index]);
        gtk_frame_set_shadow_type(GTK_FRAME(widget), shadows[index]);
        child = gtk_label_new("Frame content");
        gtk_widget_set_margin_start(child, 12);
        gtk_widget_set_margin_end(child, 12);
        gtk_widget_set_margin_top(child, 12);
        gtk_widget_set_margin_bottom(child, 12);
        gtk_container_add(GTK_CONTAINER(widget), child);
        gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(section), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_expander_new_with_mnemonic("_Expanded section");
    gtk_expander_set_expanded(GTK_EXPANDER(widget), TRUE);
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("Visible expander child"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    widget = gtk_expander_new("Collapsed section");
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("Hidden expander child"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

    section = add_section(page, "Notebook and paned containers");
    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("First page"), gtk_label_new("First"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Second page"), gtk_label_new("Second"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Third page"), gtk_label_new("Third"));
    gtk_widget_set_size_request(notebook, -1, 100);
    gtk_box_pack_start(GTK_BOX(section), notebook, FALSE, FALSE, 0);
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    child = gtk_frame_new("Left pane");
    gtk_container_add(GTK_CONTAINER(child), gtk_label_new("Resizable left content"));
    gtk_paned_pack1(GTK_PANED(paned), child, TRUE, FALSE);
    child = gtk_frame_new("Right pane");
    gtk_container_add(GTK_CONTAINER(child), gtk_label_new("Resizable right content"));
    gtk_paned_pack2(GTK_PANED(paned), child, TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(paned), 300);
    gtk_widget_set_size_request(paned, -1, 95);
    gtk_box_pack_start(GTK_BOX(section), paned, FALSE, FALSE, 0);

    section = add_section(page, "Stack switcher, stack sidebar, and transitions");
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("General stack page"), "general", "General");
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("Advanced stack page"), "advanced", "Advanced");
    gtk_stack_add_titled(GTK_STACK(stack), gtk_label_new("Details stack page"), "details", "Details");
    switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher), GTK_STACK(stack));
    gtk_box_pack_start(GTK_BOX(section), switcher, FALSE, FALSE, 0);
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 8);
    sidebar = gtk_stack_sidebar_new();
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));
    gtk_widget_set_size_request(sidebar, 180, 120);
    gtk_box_pack_start(GTK_BOX(box), sidebar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), stack, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

    section = add_section(page, "Revealer, overlay, search bar, center box, and action bar");
    toggle = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(toggle), TRUE);
    gtk_box_pack_start(GTK_BOX(section), toggle, FALSE, FALSE, 0);
    revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_container_add(GTK_CONTAINER(revealer), gtk_label_new("Revealed child surface"));
    g_object_bind_property(toggle,
                           "active",
                           revealer,
                           "reveal-child",
                           G_BINDING_SYNC_CREATE);
    gtk_box_pack_start(GTK_BOX(section), revealer, FALSE, FALSE, 0);
    overlay = gtk_overlay_new();
    child = gtk_frame_new("Overlay base");
    gtk_widget_set_size_request(child, -1, 90);
    gtk_container_add(GTK_CONTAINER(child), gtk_label_new("Base child"));
    gtk_container_add(GTK_CONTAINER(overlay), child);
    widget = gtk_button_new_with_label("Overlay child");
    gtk_widget_set_halign(widget, GTK_ALIGN_END);
    gtk_widget_set_valign(widget, GTK_ALIGN_START);
    gtk_widget_set_margin_end(widget, 8);
    gtk_widget_set_margin_top(widget, 8);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), widget);
    gtk_box_pack_start(GTK_BOX(section), overlay, FALSE, FALSE, 0);
    search_bar = gtk_search_bar_new();
    search_entry = gtk_search_entry_new();
    gtk_search_bar_connect_entry(GTK_SEARCH_BAR(search_bar), GTK_ENTRY(search_entry));
    gtk_container_add(GTK_CONTAINER(search_bar), search_entry);
    gtk_search_bar_set_search_mode(GTK_SEARCH_BAR(search_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(section), search_bar, FALSE, FALSE, 0);
    center = new_box(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(center), gtk_button_new_with_label("Start"), FALSE, FALSE, 0);
    gtk_box_set_center_widget(GTK_BOX(center), gtk_label_new("Centered"));
    gtk_box_pack_end(GTK_BOX(center), gtk_button_new_with_label("End"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), center, FALSE, FALSE, 0);
    widget = gtk_action_bar_new();
    gtk_action_bar_pack_start(GTK_ACTION_BAR(widget), gtk_button_new_with_label("Previous"));
    gtk_action_bar_set_center_widget(GTK_ACTION_BAR(widget), gtk_label_new("Action bar"));
    gtk_action_bar_pack_end(GTK_ACTION_BAR(widget), gtk_button_new_with_label("Next"));
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static GtkWidget *
new_info_bar(GtkMessageType type, const gchar *text)
{
    GtkWidget *bar = gtk_info_bar_new();
    GtkWidget *content = gtk_info_bar_get_content_area(GTK_INFO_BAR(bar));

    gtk_info_bar_set_message_type(GTK_INFO_BAR(bar), type);
    gtk_info_bar_add_button(GTK_INFO_BAR(bar), "Close", GTK_RESPONSE_CLOSE);
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new(text), FALSE, FALSE, 0);
    return bar;
}

static gboolean
draw_audit_area(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    GtkAllocation allocation;
    GtkStyleContext *style_context;
    GdkRGBA background = {0.85, 0.85, 0.85, 1.0};

    (void)user_data;
    gtk_widget_get_allocation(widget, &allocation);
    style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_lookup_color(style_context, "theme_bg_color", &background);
    gdk_cairo_set_source_rgba(cr, &background);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0.5, 0.5, allocation.width - 1.0, allocation.height - 1.0);
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, allocation.width, allocation.height);
    cairo_move_to(cr, allocation.width, 0.0);
    cairo_line_to(cr, 0.0, allocation.height);
    cairo_stroke(cr);
    return FALSE;
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
    guint context;

    section = add_section(page, "Info bars by message type");
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_INFO, "Information message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_WARNING, "Warning message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_QUESTION, "Question message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_ERROR, "Error message"), FALSE, FALSE, 0);

    section = add_section(page, "Calendar, spinners, images, and drawing area");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    calendar = gtk_calendar_new();
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 8);
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 21);
    gtk_box_pack_start(GTK_BOX(box), calendar, FALSE, FALSE, 0);
    widget = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(widget));
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    widget = gtk_spinner_new();
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_image_new_from_icon_name("dialog-information", GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 0);
    widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(widget, 160, 95);
    g_signal_connect(widget, "draw", G_CALLBACK(draw_audit_area), NULL);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

    section = add_section(page, "Header bar and status bar surfaces");
    header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Header Bar");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "Subtitle and window controls");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), gtk_button_new_from_icon_name("document-new-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), gtk_button_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(section), header, FALSE, FALSE, 0);
    widget = gtk_statusbar_new();
    context = gtk_statusbar_get_context_id(GTK_STATUSBAR(widget), "embedded");
    gtk_statusbar_push(GTK_STATUSBAR(widget), context, "Ready — message area and resize edge");
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static void
destroy_assistant(GtkWidget *widget, gpointer user_data)
{
    (void)user_data;
    gtk_widget_destroy(widget);
}

static GtkWidget *
build_shortcuts_window(void)
{
    GtkWidget *window = g_object_new(GTK_TYPE_SHORTCUTS_WINDOW, NULL);
    GtkWidget *section = g_object_new(GTK_TYPE_SHORTCUTS_SECTION,
                                      "section-name", "general",
                                      "title", "General",
                                      NULL);
    GtkWidget *group = g_object_new(GTK_TYPE_SHORTCUTS_GROUP,
                                    "title", "Window",
                                    NULL);
    GtkWidget *shortcut;

    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Open",
                            "accelerator", "<Primary>O",
                            NULL);
    gtk_container_add(GTK_CONTAINER(group), shortcut);
    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Search",
                            "accelerator", "<Primary>F",
                            NULL);
    gtk_container_add(GTK_CONTAINER(group), shortcut);
    shortcut = g_object_new(GTK_TYPE_SHORTCUTS_SHORTCUT,
                            "title", "Close",
                            "accelerator", "<Primary>W",
                            NULL);
    gtk_container_add(GTK_CONTAINER(group), shortcut);
    gtk_container_add(GTK_CONTAINER(section), group);
    gtk_container_add(GTK_CONTAINER(window), section);
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
    GtkWidget *header;
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
    } else if (kind == DIALOG_CUSTOM || kind == DIALOG_CSD) {
        dialog = gtk_dialog_new_with_buttons(kind == DIALOG_CSD ? "CSD Dialog" : "Custom GTK 3 Dialog",
                                             GTK_WINDOW(audit_window),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "OK",
                                             GTK_RESPONSE_OK,
                                             NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
        if (kind == DIALOG_CSD) {
            header = gtk_header_bar_new();
            gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Client-side Dialog");
            gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
            gtk_window_set_titlebar(GTK_WINDOW(dialog), header);
        }
        content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_set_border_width(GTK_CONTAINER(content), 12);
        gtk_box_pack_start(GTK_BOX(content), gtk_label_new("A content-area label and entry:"), FALSE, FALSE, 4);
        entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), "Editable dialog value");
        gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 4);
    } else if (kind == DIALOG_FILE) {
        dialog = gtk_file_chooser_dialog_new("GTK 3 File Chooser",
                                             GTK_WINDOW(audit_window),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "Open",
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_RECENT) {
        dialog = gtk_recent_chooser_dialog_new("GTK 3 Recent Chooser",
                                               GTK_WINDOW(audit_window),
                                               "Cancel",
                                               GTK_RESPONSE_CANCEL,
                                               "Open",
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);
    } else if (kind == DIALOG_COLOR) {
        dialog = gtk_color_chooser_dialog_new("GTK 3 Color Chooser", GTK_WINDOW(audit_window));
        gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_FONT) {
        dialog = gtk_font_chooser_dialog_new("GTK 3 Font Chooser", GTK_WINDOW(audit_window));
        gtk_font_chooser_set_preview_text(GTK_FONT_CHOOSER(dialog), "Quartz GTK 3 font preview");
    } else if (kind == DIALOG_APP_CHOOSER) {
        dialog = gtk_app_chooser_dialog_new_for_content_type(GTK_WINDOW(audit_window),
                                                             GTK_DIALOG_MODAL,
                                                             "text/plain");
    } else if (kind == DIALOG_ABOUT) {
        dialog = gtk_about_dialog_new();
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Quartz GTK 3 Audit");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                      "A native GTK 3 compatibility surface for the shared Quartz theme.");
        gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://example.invalid/quartz");
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog),
                                     (const gchar *[]){ "Quartz Theme Project", NULL });
    } else if (kind == DIALOG_ASSISTANT) {
        dialog = gtk_assistant_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "GTK 3 Assistant");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_window_set_default_size(GTK_WINDOW(dialog), 520, 340);
        page = new_box(GTK_ORIENTATION_VERTICAL, 8);
        gtk_container_set_border_width(GTK_CONTAINER(page), 18);
        gtk_box_pack_start(GTK_BOX(page), gtk_label_new("Introduction page with a complete state."), FALSE, FALSE, 0);
        gtk_assistant_append_page(GTK_ASSISTANT(dialog), page);
        gtk_assistant_set_page_title(GTK_ASSISTANT(dialog), page, "Introduction");
        gtk_assistant_set_page_type(GTK_ASSISTANT(dialog), page, GTK_ASSISTANT_PAGE_INTRO);
        gtk_assistant_set_page_complete(GTK_ASSISTANT(dialog), page, TRUE);
        page = new_box(GTK_ORIENTATION_VERTICAL, 8);
        gtk_container_set_border_width(GTK_CONTAINER(page), 18);
        gtk_box_pack_start(GTK_BOX(page), gtk_check_button_new_with_label("Assistant option"), FALSE, FALSE, 0);
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
        gtk_widget_show_all(dialog);
        return;
    } else if (kind == DIALOG_SHORTCUTS) {
        dialog = build_shortcuts_window();
        gtk_window_set_title(GTK_WINDOW(dialog), "Keyboard Shortcuts");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_widget_show_all(dialog);
        return;
    }

    if (dialog != NULL) {
        gtk_widget_show_all(dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
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

    section = add_section(page, "Message dialogs");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    add_cell(grid, dialog_button("Open information", DIALOG_INFO), "Information", 0, 0);
    add_cell(grid, dialog_button("Open warning", DIALOG_WARNING), "Warning", 1, 0);
    add_cell(grid, dialog_button("Open question", DIALOG_QUESTION), "Question", 2, 0);
    add_cell(grid, dialog_button("Open error", DIALOG_ERROR), "Error", 3, 0);

    section = add_section(page, "Chooser and utility dialogs");
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(section), grid, FALSE, FALSE, 0);
    add_cell(grid, dialog_button("Custom dialog", DIALOG_CUSTOM), "GtkDialog", 0, 0);
    add_cell(grid, dialog_button("CSD dialog", DIALOG_CSD), "GtkHeaderBar dialog", 1, 0);
    add_cell(grid, dialog_button("File chooser", DIALOG_FILE), "GtkFileChooserDialog", 2, 0);
    add_cell(grid, dialog_button("Recent chooser", DIALOG_RECENT), "GtkRecentChooserDialog", 3, 0);
    add_cell(grid, dialog_button("Color chooser", DIALOG_COLOR), "GtkColorChooserDialog", 0, 1);
    add_cell(grid, dialog_button("Font chooser", DIALOG_FONT), "GtkFontChooserDialog", 1, 1);
    add_cell(grid, dialog_button("App chooser", DIALOG_APP_CHOOSER), "GtkAppChooserDialog", 2, 1);
    add_cell(grid, dialog_button("About dialog", DIALOG_ABOUT), "GtkAboutDialog", 3, 1);
    add_cell(grid, dialog_button("Assistant", DIALOG_ASSISTANT), "GtkAssistant", 0, 2);
    add_cell(grid, dialog_button("Shortcuts", DIALOG_SHORTCUTS), "GtkShortcutsWindow", 1, 2);

    section = add_section(page, "Embedded specialized chooser widgets");
    {
        GtkWidget *box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
        widget = gtk_recent_chooser_widget_new();
        gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(widget), 5);
        gtk_widget_set_size_request(widget, 410, 190);
        gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
        widget = gtk_app_chooser_widget_new("text/plain");
        gtk_widget_set_size_request(widget, 410, 190);
        gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    }
    return scroll_page(page);
}

static void
show_popover(GtkWidget *button, gpointer user_data)
{
    (void)button;
    gtk_widget_show_all(GTK_WIDGET(user_data));
    gtk_popover_popup(GTK_POPOVER(user_data));
}

static GtkWidget *
build_popover(GtkWidget *relative_to)
{
    GtkWidget *popover = gtk_popover_new(relative_to);
    GtkWidget *box = new_box(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *widget;

    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Popover content"), FALSE, FALSE, 0);
    widget = gtk_check_button_new_with_label("Checked option");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_entry_new(), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_button_new_with_label("Popover action"), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(popover), box);
    return popover;
}

static gboolean
popup_menu_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    (void)widget;
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
        gtk_menu_popup_at_pointer(GTK_MENU(user_data), (GdkEvent *)event);
        return TRUE;
    }
    return FALSE;
}

static GtkWidget *
build_menus_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *menu_button;
    GtkWidget *popover;
    GtkWidget *menu;
    GtkWidget *item;
    GtkWidget *target;
    GSList *group = NULL;

    section = add_section(page, "Menu button and popover");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    menu_button = gtk_menu_button_new();
    gtk_button_set_label(GTK_BUTTON(menu_button), "Menu button");
    menu = gtk_menu_new();
    append_menu_item(menu, "First item", "Menu-button item selected");
    item = gtk_check_menu_item_new_with_label("Checked item");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    item = gtk_menu_item_new_with_label("Disabled item");
    gtk_widget_set_sensitive(item, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show_all(menu);
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(menu_button), menu);
    gtk_box_pack_start(GTK_BOX(box), menu_button, FALSE, FALSE, 0);
    button = gtk_button_new_with_label("Open content popover");
    popover = build_popover(button);
    g_signal_connect(button, "clicked", G_CALLBACK(show_popover), popover);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    section = add_section(page, "Context menu state matrix");
    menu = gtk_menu_new();
    item = gtk_menu_item_new_with_label("Normal item");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    item = gtk_check_menu_item_new_with_label("Checked item");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    item = gtk_check_menu_item_new_with_label("Mixed item");
    gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    item = gtk_radio_menu_item_new_with_label(group, "Radio one");
    group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    item = gtk_radio_menu_item_new_with_label(group, "Radio two");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    item = gtk_menu_item_new_with_label("Disabled item");
    gtk_widget_set_sensitive(item, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show_all(menu);
    target = gtk_event_box_new();
    gtk_widget_set_size_request(target, -1, 100);
    gtk_container_add(GTK_CONTAINER(target), gtk_label_new("Right-click this target for a traditional GtkMenu"));
    gtk_widget_add_events(target, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(target, "button-press-event", G_CALLBACK(popup_menu_event), menu);
    gtk_box_pack_start(GTK_BOX(section), target, FALSE, FALSE, 0);

    section = add_section(page, "Model buttons inside a popover-style box");
    box = new_box(GTK_ORIENTATION_VERTICAL, 0);
    add_style_class(box, GTK_STYLE_CLASS_MENU);
    item = gtk_model_button_new();
    g_object_set(item, "text", "Model button", NULL);
    gtk_box_pack_start(GTK_BOX(box), item, FALSE, FALSE, 0);
    item = gtk_model_button_new();
    g_object_set(item, "text", "Checked model button", "role", GTK_BUTTON_ROLE_CHECK, "active", TRUE, NULL);
    gtk_box_pack_start(GTK_BOX(box), item, FALSE, FALSE, 0);
    item = gtk_model_button_new();
    g_object_set(item, "text", "Submenu model button", "menu-name", "submenu", NULL);
    gtk_box_pack_start(GTK_BOX(box), item, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    return scroll_page(page);
}

static GtkWidget *
build_specialized_page(void)
{
    GtkWidget *page = new_box(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;

    section = add_section(page, "Places sidebar");
    widget = gtk_places_sidebar_new();
    gtk_places_sidebar_set_show_desktop(GTK_PLACES_SIDEBAR(widget), TRUE);
    gtk_places_sidebar_set_show_recent(GTK_PLACES_SIDEBAR(widget), TRUE);
    gtk_places_sidebar_set_show_trash(GTK_PLACES_SIDEBAR(widget), TRUE);
    gtk_widget_set_size_request(widget, -1, 230);
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    section = add_section(page, "Font and color chooser widgets");
    box = new_box(GTK_ORIENTATION_HORIZONTAL, 10);
    widget = gtk_font_chooser_widget_new();
    gtk_font_chooser_set_preview_text(GTK_FONT_CHOOSER(widget), "Quartz GTK 3 preview");
    gtk_widget_set_size_request(widget, 470, 300);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    widget = gtk_color_chooser_widget_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), TRUE);
    gtk_widget_set_size_request(widget, 360, 300);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

    section = add_section(page, "File chooser widget");
    widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_widget_set_size_request(widget, -1, 300);
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    return scroll_page(page);
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
    guint context;

    (void)user_data;
    audit_window = gtk_application_window_new(application);
    gtk_window_set_title(GTK_WINDOW(audit_window), "Quartz GTK 3 Audit");
    gtk_window_set_default_size(GTK_WINDOW(audit_window), 1080, 800);
    gtk_window_set_position(GTK_WINDOW(audit_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_icon_name("preferences-desktop-theme");

    root = new_box(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(audit_window), root);
    gtk_box_pack_start(GTK_BOX(root), build_menu_bar(), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), build_toolbar(), FALSE, FALSE, 0);

    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    append_page(notebook, build_buttons_page(), "Buttons & States");
    append_page(notebook, build_inputs_page(), "Text & Inputs");
    append_page(notebook, build_ranges_page(), "Ranges & Progress");
    append_page(notebook, build_data_page(), "Data & Text Views");
    append_page(notebook, build_containers_page(), "Containers");
    append_page(notebook, build_feedback_page(), "Feedback & Chrome");
    append_page(notebook, build_menus_page(), "Menus & Popovers");
    append_page(notebook, build_dialogs_page(), "Dialogs & Choosers");
    append_page(notebook, build_specialized_page(), "Specialized Widgets");
    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 0);

    audit_statusbar = gtk_statusbar_new();
    context = gtk_statusbar_get_context_id(GTK_STATUSBAR(audit_statusbar), "audit");
    gtk_statusbar_push(GTK_STATUSBAR(audit_statusbar),
                       context,
                       "Native GTK 3.24 audit — interact with controls to inspect every state");
    gtk_box_pack_end(GTK_BOX(root), audit_statusbar, FALSE, FALSE, 0);

    gtk_widget_show_all(audit_window);
}

int
main(int argc, char **argv)
{
    GtkApplication *application;
    int status;

    application = gtk_application_new("org.quartz.Gtk3Audit",
                                      G_APPLICATION_NON_UNIQUE);
    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
