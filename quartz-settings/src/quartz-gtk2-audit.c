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
    DIALOG_LEGACY_FILE
};

static GtkWidget *audit_window;
static GtkWidget *audit_statusbar;
static GtkWidget *audit_default_button;
static GtkWidget *audit_spinner;

static GtkWidget *
new_vbox(gint spacing)
{
    return gtk_vbox_new(FALSE, spacing);
}

static GtkWidget *
new_hbox(gint spacing)
{
    return gtk_hbox_new(FALSE, spacing);
}

static GtkWidget *
padded_label(const gchar *text, guint padding)
{
    GtkWidget *alignment = gtk_alignment_new(0.5f, 0.5f, 1.0f, 1.0f);

    gtk_container_set_border_width(GTK_CONTAINER(alignment), padding);
    gtk_container_add(GTK_CONTAINER(alignment), gtk_label_new(text));
    return alignment;
}

static GtkWidget *
add_section(GtkWidget *page, const gchar *title)
{
    GtkWidget *frame = gtk_frame_new(title);
    GtkWidget *content = new_vbox(8);

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
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), page);
    return scrolled;
}

static void
add_cell(GtkWidget *table,
         GtkWidget *widget,
         const gchar *caption,
         guint column,
         guint row)
{
    GtkWidget *box = new_vbox(3);
    GtkWidget *label = gtk_label_new(caption);

    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_table_attach(GTK_TABLE(table),
                     box,
                     column,
                     column + 1,
                     row,
                     row + 1,
                     GTK_FILL | GTK_EXPAND,
                     GTK_FILL,
                     6,
                     5);
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
menu_item(GtkWidget *menu, const gchar *label, const gchar *status)
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
    GtkWidget *submenu = gtk_menu_new();
    GtkWidget *item;
    GtkWidget *separator;
    GSList *radio_group = NULL;
    GtkAccelGroup *accelerators = gtk_accel_group_new();

    gtk_window_add_accel_group(GTK_WINDOW(audit_window), accelerators);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(push_status), "New selected");
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(push_status), "Open selected");

    item = menu_item(file_menu, "Open _Recent", NULL);
    menu_item(submenu, "Audit document 1", "Recent item selected");
    menu_item(submenu, "Audit document 2", "Recent item selected");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), separator);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), item);
    g_signal_connect_swapped(item, "activate", G_CALLBACK(gtk_widget_destroy), audit_window);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, accelerators);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), item);
    gtk_widget_set_sensitive(item, FALSE);

    item = gtk_check_menu_item_new_with_mnemonic("Show _Toolbar");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_check_menu_item_new_with_mnemonic("_Mixed State");
    gtk_check_menu_item_set_inconsistent(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);

    item = gtk_radio_menu_item_new_with_mnemonic(radio_group, "_Small");
    radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_radio_menu_item_new_with_mnemonic(radio_group, "_Medium");
    radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);
    item = gtk_radio_menu_item_new_with_mnemonic(radio_group, "_Large");
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), item);

    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accelerators);
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
    item = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_tool_item_set_tooltip_text(item, "New audit item");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
    separator = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);
    item = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_BOLD);
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_radio_tool_button_new_from_stock(NULL, GTK_STOCK_JUSTIFY_LEFT);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    item = gtk_radio_tool_button_new_with_stock_from_widget(GTK_RADIO_TOOL_BUTTON(item),
                                                            GTK_STOCK_JUSTIFY_CENTER);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    separator = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
    gtk_tool_item_set_expand(separator, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);
    item = gtk_menu_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
    gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(item), gtk_menu_new());
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    return toolbar;
}

static GtkWidget *
build_buttons_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *table;
    GtkWidget *widget;
    GtkWidget *image;
    GtkWidget *box;
    GSList *group = NULL;

    section = add_section(page, "Push buttons and relief");
    table = gtk_table_new(2, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    add_cell(table, gtk_button_new_with_label("Normal"), "Normal", 0, 0);
    widget = gtk_button_new_with_mnemonic("_Mnemonic");
    add_cell(table, widget, "Mnemonic", 1, 0);
    add_cell(table, gtk_button_new_from_stock(GTK_STOCK_OPEN), "Stock + image", 2, 0);
    widget = gtk_button_new_with_label("Insensitive");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled", 3, 0);
    widget = gtk_button_new_with_label("Active/default");
    GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
    audit_default_button = widget;
    add_cell(table, widget, "Default", 0, 1);
    widget = gtk_button_new_with_label("Flat relief");
    gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
    add_cell(table, widget, "Relief none", 1, 1);
    widget = gtk_button_new();
    box = new_hbox(4);
    image = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Image + text"), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(widget), box);
    add_cell(table, widget, "Custom content", 2, 1);
    widget = gtk_link_button_new_with_label("https://example.invalid", "Link button");
    add_cell(table, widget, "Link", 3, 1);

    section = add_section(page, "Toggle, check, and radio states");
    table = gtk_table_new(2, 5, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    widget = gtk_toggle_button_new_with_label("Off");
    add_cell(table, widget, "Toggle off", 0, 0);
    widget = gtk_toggle_button_new_with_label("On");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(table, widget, "Toggle on", 1, 0);
    widget = gtk_check_button_new_with_label("Unchecked");
    add_cell(table, widget, "Check off", 2, 0);
    widget = gtk_check_button_new_with_label("Checked");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(table, widget, "Check on", 3, 0);
    widget = gtk_check_button_new_with_label("Inconsistent");
    gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(widget), TRUE);
    add_cell(table, widget, "Mixed", 4, 0);
    widget = gtk_radio_button_new_with_label(group, "Choice A");
    group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
    add_cell(table, widget, "Radio selected", 0, 1);
    widget = gtk_radio_button_new_with_label(group, "Choice B");
    group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
    add_cell(table, widget, "Radio clear", 1, 1);
    widget = gtk_check_button_new_with_label("Disabled off");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled off", 2, 1);
    widget = gtk_check_button_new_with_label("Disabled on");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled on", 3, 1);
    widget = gtk_radio_button_new_with_label(group, "Disabled radio");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled radio", 4, 1);

    section = add_section(page, "Button box layouts");
    box = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_START);
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_from_stock(GTK_STOCK_APPLY));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_from_stock(GTK_STOCK_CANCEL));
    gtk_container_add(GTK_CONTAINER(box), gtk_button_new_from_stock(GTK_STOCK_OK));
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    box = gtk_hbutton_box_new();
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
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *table;
    GtkWidget *widget;
    GtkWidget *entry;
    GtkWidget *box;
    GtkObject *adjustment;

    section = add_section(page, "Labels and text states");
    table = gtk_table_new(2, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    add_cell(table, gtk_label_new("Plain label"), "Plain", 0, 0);
    widget = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(widget), "<b>Bold</b>, <i>italic</i>, <u>underline</u>");
    add_cell(table, widget, "Pango markup", 1, 0);
    widget = gtk_label_new("Selectable text");
    gtk_label_set_selectable(GTK_LABEL(widget), TRUE);
    add_cell(table, widget, "Selectable", 2, 0);
    widget = gtk_label_new("A deliberately long label that demonstrates end ellipsizing");
    gtk_label_set_ellipsize(GTK_LABEL(widget), PANGO_ELLIPSIZE_END);
    gtk_widget_set_size_request(widget, 150, -1);
    add_cell(table, widget, "Ellipsized", 3, 0);
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Mnemonic target");
    widget = gtk_label_new_with_mnemonic("_Name:");
    gtk_label_set_mnemonic_widget(GTK_LABEL(widget), entry);
    box = new_hbox(5);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 0);
    add_cell(table, box, "Label + mnemonic", 0, 1);
    widget = gtk_label_new("This label wraps onto more than one line when its allocation is narrow.");
    gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
    gtk_widget_set_size_request(widget, 170, -1);
    add_cell(table, widget, "Wrapped", 1, 1);
    widget = gtk_accel_label_new("Keyboard accelerator");
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(widget), NULL);
    add_cell(table, widget, "Accel label", 2, 1);
    widget = gtk_label_new("Insensitive label");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled", 3, 1);

    section = add_section(page, "Entries and editable states");
    table = gtk_table_new(3, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Editable text");
    add_cell(table, widget, "Normal entry", 0, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
    add_cell(table, widget, "Password", 1, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Read only");
    gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
    add_cell(table, widget, "Read-only", 2, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Disabled");
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Insensitive", 3, 0);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Primary and secondary icons");
    gtk_entry_set_icon_from_stock(GTK_ENTRY(widget), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
    gtk_entry_set_icon_from_stock(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
    add_cell(table, widget, "Entry icons", 0, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Entry progress");
    gtk_entry_set_progress_fraction(GTK_ENTRY(widget), 0.64);
    add_cell(table, widget, "Progress", 1, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Has selection");
    gtk_editable_select_region(GTK_EDITABLE(widget), 4, 13);
    add_cell(table, widget, "Selected text", 2, 1);
    adjustment = gtk_adjustment_new(7, -10, 50, 1, 5, 0);
    widget = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), TRUE);
    add_cell(table, widget, "Spin button", 3, 1);
    widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widget), "Error tooltip target");
    gtk_widget_set_tooltip_text(widget, "GTK 2 themed tooltip surface");
    add_cell(table, widget, "Hover for tooltip", 0, 2);
    widget = gtk_spin_button_new_with_range(0.0, 1.0, 0.05);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 0.35);
    add_cell(table, widget, "Decimal spin", 1, 2);

    section = add_section(page, "Choice and chooser controls");
    table = gtk_table_new(2, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    widget = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "First item");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "Second item");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "Third item");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 1);
    add_cell(table, widget, "Combo box", 0, 0);
    widget = gtk_combo_box_entry_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "Editable choice");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "Another choice");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    add_cell(table, widget, "Editable combo", 1, 0);
    widget = gtk_file_chooser_button_new("Choose a file", GTK_FILE_CHOOSER_ACTION_OPEN);
    add_cell(table, widget, "File chooser", 2, 0);
    widget = gtk_font_button_new_with_font("Sans 12");
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(widget), TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(widget), TRUE);
    add_cell(table, widget, "Font button", 3, 0);
    widget = gtk_color_button_new();
    gtk_color_button_set_title(GTK_COLOR_BUTTON(widget), "Choose audit color");
    add_cell(table, widget, "Color button", 0, 1);
    widget = gtk_volume_button_new();
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.65);
    add_cell(table, widget, "Volume button", 1, 1);
    widget = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "Disabled choice");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled combo", 2, 1);

    return scroll_page(page);
}

static gboolean
pulse_progress(gpointer user_data)
{
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(user_data));
    return TRUE;
}

static GtkWidget *
build_ranges_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *table;
    GtkWidget *widget;
    GtkWidget *box;
    GtkObject *adjustment;

    section = add_section(page, "Scales and marks");
    table = gtk_table_new(3, 2, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    widget = gtk_hscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 42);
    gtk_scale_set_digits(GTK_SCALE(widget), 0);
    gtk_scale_add_mark(GTK_SCALE(widget), 0, GTK_POS_BOTTOM, "0");
    gtk_scale_add_mark(GTK_SCALE(widget), 50, GTK_POS_BOTTOM, "50");
    gtk_scale_add_mark(GTK_SCALE(widget), 100, GTK_POS_BOTTOM, "100");
    add_cell(table, widget, "Horizontal + marks", 0, 0);
    widget = gtk_hscale_new_with_range(-1, 1, 0.1);
    gtk_range_set_value(GTK_RANGE(widget), 0.3);
    gtk_scale_set_value_pos(GTK_SCALE(widget), GTK_POS_TOP);
    add_cell(table, widget, "Value above", 1, 0);
    widget = gtk_hscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 67);
    gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
    add_cell(table, widget, "Inverted", 0, 1);
    widget = gtk_hscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 35);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled", 1, 1);
    widget = gtk_vscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 58);
    gtk_scale_set_value_pos(GTK_SCALE(widget), GTK_POS_RIGHT);
    gtk_widget_set_size_request(widget, -1, 130);
    add_cell(table, widget, "Vertical", 0, 2);
    widget = gtk_vscale_new_with_range(0, 100, 1);
    gtk_range_set_value(GTK_RANGE(widget), 25);
    gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(widget), FALSE);
    gtk_widget_set_size_request(widget, -1, 130);
    add_cell(table, widget, "Vertical inverted", 1, 2);

    section = add_section(page, "Progress indicators");
    table = gtk_table_new(2, 3, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.38);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), "38%");
    add_cell(table, widget, "Determinate", 0, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.76);
    gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(widget), GTK_PROGRESS_RIGHT_TO_LEFT);
    add_cell(table, widget, "Right to left", 1, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(widget), 0.12);
    g_timeout_add(120, pulse_progress, widget);
    add_cell(table, widget, "Activity pulse", 2, 0);
    widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.55);
    gtk_widget_set_sensitive(widget, FALSE);
    add_cell(table, widget, "Disabled", 0, 1);

    section = add_section(page, "Native scrollbars");
    box = new_hbox(18);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    adjustment = gtk_adjustment_new(32, 0, 100, 1, 10, 20);
    widget = gtk_hscrollbar_new(GTK_ADJUSTMENT(adjustment));
    gtk_widget_set_size_request(widget, 360, -1);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    adjustment = gtk_adjustment_new(48, 0, 100, 1, 10, 20);
    widget = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustment));
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    adjustment = gtk_adjustment_new(20, 0, 100, 1, 10, 20);
    widget = gtk_vscrollbar_new(GTK_ADJUSTMENT(adjustment));
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, -1, 150);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static GtkListStore *
build_list_store(void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GdkPixbuf *icon;

    store = gtk_list_store_new(DATA_COLUMNS,
                               GDK_TYPE_PIXBUF,
                               G_TYPE_STRING,
                               G_TYPE_BOOLEAN,
                               G_TYPE_INT);
    icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
                                    "text-x-generic",
                                    48,
                                    0,
                                    NULL);

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

    if (icon != NULL) {
        g_object_unref(icon);
    }
    return store;
}

static GtkWidget *
build_tree_view(GtkListStore *store)
{
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreePath *selected_path;

    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes("Icon", renderer, "pixbuf", DATA_ICON, NULL);
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
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(view), TRUE);
    selected_path = gtk_tree_path_new_from_indices(1, -1);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(view), selected_path, NULL, FALSE);
    gtk_tree_path_free(selected_path);
    return view;
}

static GtkWidget *
build_data_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *scrolled;
    GtkWidget *view;
    GtkWidget *box;
    GtkListStore *store;
    GtkTreeStore *tree_store;
    GtkTreeIter parent;
    GtkTreeIter child;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    store = build_list_store();
    section = add_section(page, "Tree view and cell renderers");
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled, -1, 155);
    view = build_tree_view(store);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(section), scrolled, FALSE, FALSE, 0);

    section = add_section(page, "Hierarchical tree and icon view");
    box = new_hbox(10);
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
    gtk_tree_view_expand_all(GTK_TREE_VIEW(view));
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 330, 175);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(view), DATA_ICON);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(view), DATA_NAME);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(view), GTK_SELECTION_MULTIPLE);
    {
        GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
        gtk_icon_view_select_path(GTK_ICON_VIEW(view), path);
        gtk_tree_path_free(path);
    }
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 390, 175);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

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
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *child;
    GtkWidget *paned;
    GtkWidget *notebook;
    GtkWidget *layout;
    GtkWidget *fixed;
    GtkWidget *viewport;
    GtkWidget *scrolled;
    gint shadow;
    const gchar *shadow_names[] = { "None", "In", "Out", "Etched in", "Etched out" };
    GtkShadowType shadows[] = {
        GTK_SHADOW_NONE,
        GTK_SHADOW_IN,
        GTK_SHADOW_OUT,
        GTK_SHADOW_ETCHED_IN,
        GTK_SHADOW_ETCHED_OUT
    };

    section = add_section(page, "Frames and separator styles");
    box = new_hbox(8);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    for (shadow = 0; shadow < 5; shadow++) {
        widget = gtk_frame_new(shadow_names[shadow]);
        gtk_frame_set_shadow_type(GTK_FRAME(widget), shadows[shadow]);
        child = padded_label("Frame content", 12);
        gtk_container_add(GTK_CONTAINER(widget), child);
        gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(section), gtk_hseparator_new(), FALSE, FALSE, 0);

    section = add_section(page, "Expanders and notebooks");
    box = new_hbox(10);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    widget = gtk_expander_new_with_mnemonic("_Expanded section");
    gtk_expander_set_expanded(GTK_EXPANDER(widget), TRUE);
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("Visible expander child"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    widget = gtk_expander_new("Collapsed section");
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("Hidden expander child"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("First page"), gtk_label_new("First"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Second page"), gtk_label_new("Second"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_label_new("Third page"), gtk_label_new("Third"));
    gtk_widget_set_size_request(notebook, 320, 100);
    gtk_box_pack_start(GTK_BOX(section), notebook, FALSE, FALSE, 0);

    section = add_section(page, "Horizontal and vertical paned containers");
    paned = gtk_hpaned_new();
    child = gtk_frame_new("Left pane");
    gtk_container_add(GTK_CONTAINER(child), gtk_label_new("Resizable left content"));
    gtk_paned_pack1(GTK_PANED(paned), child, TRUE, FALSE);
    child = gtk_frame_new("Right pane");
    gtk_container_add(GTK_CONTAINER(child), gtk_label_new("Resizable right content"));
    gtk_paned_pack2(GTK_PANED(paned), child, TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(paned), 260);
    gtk_widget_set_size_request(paned, -1, 90);
    gtk_box_pack_start(GTK_BOX(section), paned, FALSE, FALSE, 0);
    paned = gtk_vpaned_new();
    gtk_paned_pack1(GTK_PANED(paned), gtk_label_new("Upper pane"), TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned), gtk_label_new("Lower pane"), TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(paned), 42);
    gtk_widget_set_size_request(paned, -1, 90);
    gtk_box_pack_start(GTK_BOX(section), paned, FALSE, FALSE, 0);

    section = add_section(page, "Viewport, layout, fixed, alignment, and aspect frame");
    box = new_hbox(10);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, 230, 120);
    viewport = gtk_viewport_new(NULL, NULL);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_IN);
    child = gtk_label_new("A viewport contains a larger child.\n\nScroll horizontally and vertically.\n\nEnd of viewport content.");
    gtk_widget_set_size_request(child, 380, 210);
    gtk_container_add(GTK_CONTAINER(viewport), child);
    gtk_container_add(GTK_CONTAINER(scrolled), viewport);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    layout = gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT(layout), 360, 190);
    gtk_layout_put(GTK_LAYOUT(layout), gtk_button_new_with_label("Layout at 12, 12"), 12, 12);
    gtk_layout_put(GTK_LAYOUT(layout), gtk_button_new_with_label("Layout at 155, 75"), 155, 75);
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, 230, 120);
    gtk_container_add(GTK_CONTAINER(scrolled), layout);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    fixed = gtk_fixed_new();
    gtk_widget_set_size_request(fixed, 230, 120);
    gtk_fixed_put(GTK_FIXED(fixed), gtk_label_new("Fixed"), 12, 12);
    gtk_fixed_put(GTK_FIXED(fixed), gtk_button_new_with_label("Positioned"), 70, 50);
    widget = gtk_frame_new("Fixed container");
    gtk_container_add(GTK_CONTAINER(widget), fixed);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    widget = gtk_aspect_frame_new("Aspect frame", 0.5f, 0.5f, 1.8f, FALSE);
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("1.8:1 child"));
    gtk_widget_set_size_request(widget, 200, 110);
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static GtkWidget *
new_info_bar(GtkMessageType type, const gchar *text)
{
    GtkWidget *bar = gtk_info_bar_new();
    GtkWidget *content = gtk_info_bar_get_content_area(GTK_INFO_BAR(bar));

    gtk_info_bar_set_message_type(GTK_INFO_BAR(bar), type);
    gtk_info_bar_add_button(GTK_INFO_BAR(bar), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new(text), FALSE, FALSE, 0);
    return bar;
}

static gboolean
draw_audit_area(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    gint width = widget->allocation.width;
    gint height = widget->allocation.height;

    (void)event;
    (void)user_data;
    gdk_draw_rectangle(widget->window,
                       widget->style->bg_gc[GTK_STATE_NORMAL],
                       TRUE,
                       0,
                       0,
                       width,
                       height);
    gdk_draw_rectangle(widget->window,
                       widget->style->black_gc,
                       FALSE,
                       0,
                       0,
                       width - 1,
                       height - 1);
    gdk_draw_line(widget->window,
                  widget->style->black_gc,
                  0,
                  0,
                  width - 1,
                  height - 1);
    gdk_draw_line(widget->window,
                  widget->style->black_gc,
                  width - 1,
                  0,
                  0,
                  height - 1);
    return TRUE;
}

static GtkWidget *
build_feedback_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *calendar;
    guint context;

    section = add_section(page, "Info bars by message type");
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_INFO, "Information message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_WARNING, "Warning message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_QUESTION, "Question message"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(section), new_info_bar(GTK_MESSAGE_ERROR, "Error message"), FALSE, FALSE, 0);

    section = add_section(page, "Calendar, spinner, images, arrows, and drawing area");
    box = new_hbox(15);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    calendar = gtk_calendar_new();
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 8);
    gtk_calendar_mark_day(GTK_CALENDAR(calendar), 21);
    gtk_box_pack_start(GTK_BOX(box), calendar, FALSE, FALSE, 0);
    widget = gtk_spinner_new();
    audit_spinner = widget;
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    widget = gtk_spinner_new();
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_size_request(widget, 40, 40);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_IN), FALSE, FALSE, 0);
    widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(widget, 150, 90);
    g_signal_connect(widget, "expose-event", G_CALLBACK(draw_audit_area), NULL);
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

    section = add_section(page, "Embedded status bar");
    widget = gtk_statusbar_new();
    context = gtk_statusbar_get_context_id(GTK_STATUSBAR(widget), "embedded");
    gtk_statusbar_push(GTK_STATUSBAR(widget), context, "Ready — resize grip and message area");
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    section = add_section(page, "Color selection widget");
    widget = gtk_color_selection_new();
    gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(widget), TRUE);
    gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(widget), TRUE);
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    return scroll_page(page);
}

static void
destroy_assistant(GtkWidget *widget, gpointer user_data)
{
    (void)user_data;
    gtk_widget_destroy(widget);
}

static void
show_dialog(GtkWidget *button, gpointer user_data)
{
    gint kind = GPOINTER_TO_INT(user_data);
    GtkWidget *dialog = NULL;
    GtkWidget *content;
    GtkWidget *entry;
    GtkWidget *page;
    GtkWidget *label;
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
        dialog = gtk_dialog_new_with_buttons("Custom GTK 2 Dialog",
                                             GTK_WINDOW(audit_window),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK,
                                             GTK_RESPONSE_OK,
                                             NULL);
        gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
        content = GTK_DIALOG(dialog)->vbox;
        gtk_container_set_border_width(GTK_CONTAINER(content), 10);
        gtk_box_pack_start(GTK_BOX(content), gtk_label_new("A content-area label and entry:"), FALSE, FALSE, 4);
        entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), "Editable dialog value");
        gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 4);
    } else if (kind == DIALOG_FILE) {
        dialog = gtk_file_chooser_dialog_new("GTK 2 File Chooser",
                                             GTK_WINDOW(audit_window),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OPEN,
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_RECENT) {
        dialog = gtk_recent_chooser_dialog_new("GTK 2 Recent Chooser",
                                               GTK_WINDOW(audit_window),
                                               GTK_STOCK_CANCEL,
                                               GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OPEN,
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);
        gtk_recent_chooser_set_show_icons(GTK_RECENT_CHOOSER(dialog), TRUE);
    } else if (kind == DIALOG_COLOR) {
        dialog = gtk_color_selection_dialog_new("GTK 2 Color Selection");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
    } else if (kind == DIALOG_FONT) {
        dialog = gtk_font_selection_dialog_new("GTK 2 Font Selection");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
    } else if (kind == DIALOG_ABOUT) {
        dialog = gtk_about_dialog_new();
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Quartz GTK 2 Audit");
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                      "A native GTK 2 compatibility surface for the shared Quartz theme.");
        gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://example.invalid/quartz");
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog),
                                     (const gchar *[]){ "Quartz Theme Project", NULL });
    } else if (kind == DIALOG_ASSISTANT) {
        dialog = gtk_assistant_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "GTK 2 Assistant");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
        gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 320);
        page = new_vbox(8);
        gtk_container_set_border_width(GTK_CONTAINER(page), 18);
        label = gtk_label_new("Introduction page with a complete state.");
        gtk_box_pack_start(GTK_BOX(page), label, FALSE, FALSE, 0);
        gtk_assistant_append_page(GTK_ASSISTANT(dialog), page);
        gtk_assistant_set_page_title(GTK_ASSISTANT(dialog), page, "Introduction");
        gtk_assistant_set_page_type(GTK_ASSISTANT(dialog), page, GTK_ASSISTANT_PAGE_INTRO);
        gtk_assistant_set_page_complete(GTK_ASSISTANT(dialog), page, TRUE);
        page = new_vbox(8);
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
    } else if (kind == DIALOG_LEGACY_FILE) {
        dialog = gtk_file_selection_new("Legacy GtkFileSelection");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(audit_window));
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
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *table;

    section = add_section(page, "Message dialogs");
    table = gtk_table_new(1, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    add_cell(table, dialog_button("Open information", DIALOG_INFO), "Information", 0, 0);
    add_cell(table, dialog_button("Open warning", DIALOG_WARNING), "Warning", 1, 0);
    add_cell(table, dialog_button("Open question", DIALOG_QUESTION), "Question", 2, 0);
    add_cell(table, dialog_button("Open error", DIALOG_ERROR), "Error", 3, 0);

    section = add_section(page, "Chooser and utility dialogs");
    table = gtk_table_new(2, 4, TRUE);
    gtk_box_pack_start(GTK_BOX(section), table, FALSE, FALSE, 0);
    add_cell(table, dialog_button("Custom dialog", DIALOG_CUSTOM), "GtkDialog", 0, 0);
    add_cell(table, dialog_button("File chooser", DIALOG_FILE), "GtkFileChooserDialog", 1, 0);
    add_cell(table, dialog_button("Recent chooser", DIALOG_RECENT), "GtkRecentChooserDialog", 2, 0);
    add_cell(table, dialog_button("Color chooser", DIALOG_COLOR), "GtkColorSelectionDialog", 3, 0);
    add_cell(table, dialog_button("Font chooser", DIALOG_FONT), "GtkFontSelectionDialog", 0, 1);
    add_cell(table, dialog_button("About dialog", DIALOG_ABOUT), "GtkAboutDialog", 1, 1);
    add_cell(table, dialog_button("Assistant", DIALOG_ASSISTANT), "GtkAssistant", 2, 1);
    add_cell(table, dialog_button("Legacy file dialog", DIALOG_LEGACY_FILE), "GtkFileSelection", 3, 1);

    return scroll_page(page);
}

static gboolean
popup_menu_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    (void)widget;
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        gtk_menu_popup(GTK_MENU(user_data),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       event->button,
                       event->time);
        return TRUE;
    }
    return FALSE;
}

static void
popup_menu_clicked(GtkWidget *widget, gpointer user_data)
{
    (void)widget;
    gtk_menu_popup(GTK_MENU(user_data),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   0,
                   gtk_get_current_event_time());
}

static GtkWidget *
build_menus_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *menu;
    GtkWidget *item;
    GtkWidget *target;
    GtkWidget *button;
    GtkWidget *option;
    GSList *group = NULL;

    section = add_section(page, "Popup menu states");
    menu = gtk_menu_new();
    item = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
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
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    item = gtk_menu_item_new_with_label("Insensitive item");
    gtk_widget_set_sensitive(item, FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show_all(menu);
    target = gtk_event_box_new();
    gtk_widget_set_size_request(target, -1, 90);
    gtk_container_add(GTK_CONTAINER(target), gtk_label_new("Right-click this themed event-box target"));
    gtk_widget_add_events(target, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(target, "button-press-event", G_CALLBACK(popup_menu_event), menu);
    gtk_box_pack_start(GTK_BOX(section), target, FALSE, FALSE, 0);
    button = gtk_button_new_with_label("Open popup menu");
    g_signal_connect(button, "clicked", G_CALLBACK(popup_menu_clicked), menu);
    gtk_box_pack_start(GTK_BOX(section), button, FALSE, FALSE, 0);

    section = add_section(page, "Option menu");
    option = gtk_option_menu_new();
    menu = gtk_menu_new();
    menu_item(menu, "First option", "First option selected");
    menu_item(menu, "Second option", "Second option selected");
    menu_item(menu, "Third option", "Third option selected");
    gtk_option_menu_set_menu(GTK_OPTION_MENU(option), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option), 1);
    gtk_box_pack_start(GTK_BOX(section), option, FALSE, FALSE, 0);

    section = add_section(page, "Tearoff and nested submenu");
    menu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_tearoff_menu_item_new());
    item = menu_item(menu, "Parent submenu", NULL);
    {
        GtkWidget *nested = gtk_menu_new();
        menu_item(nested, "Nested item A", "Nested item selected");
        menu_item(nested, "Nested item B", "Nested item selected");
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), nested);
    }
    gtk_widget_show_all(menu);
    button = gtk_button_new_with_label("Open tearoff/nested menu");
    g_signal_connect(button, "clicked", G_CALLBACK(popup_menu_clicked), menu);
    gtk_box_pack_start(GTK_BOX(section), button, FALSE, FALSE, 0);
    return scroll_page(page);
}

static GtkWidget *
build_legacy_page(void)
{
    GtkWidget *page = new_vbox(10);
    GtkWidget *section;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *list;
    GtkWidget *scrolled;
    GtkWidget *handle;
    GList *strings = NULL;
    gchar *titles[] = { (gchar *)"Legacy column", (gchar *)"Value" };
    gchar *row1[] = { (gchar *)"GtkCList row one", (gchar *)"Ready" };
    gchar *row2[] = { (gchar *)"GtkCList row two", (gchar *)"Disabled" };

    section = add_section(page, "Deprecated GTK 2 selection widgets");
    box = new_hbox(12);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);
    widget = gtk_combo_new();
    strings = g_list_append(strings, (gpointer)"Legacy editable combo");
    strings = g_list_append(strings, (gpointer)"Second legacy value");
    strings = g_list_append(strings, (gpointer)"Third legacy value");
    gtk_combo_set_popdown_strings(GTK_COMBO(widget), strings);
    g_list_free(strings);
    gtk_widget_set_size_request(widget, 230, -1);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    list = gtk_list_new();
    gtk_list_set_selection_mode(GTK_LIST(list), GTK_SELECTION_BROWSE);
    gtk_container_add(GTK_CONTAINER(list), gtk_list_item_new_with_label("GtkList item one"));
    gtk_container_add(GTK_CONTAINER(list), gtk_list_item_new_with_label("GtkList item two"));
    gtk_container_add(GTK_CONTAINER(list), gtk_list_item_new_with_label("GtkList item three"));
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 240, 100);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), list);
    gtk_box_pack_start(GTK_BOX(box), scrolled, FALSE, FALSE, 0);
    widget = gtk_clist_new_with_titles(2, titles);
    gtk_clist_append(GTK_CLIST(widget), row1);
    gtk_clist_append(GTK_CLIST(widget), row2);
    gtk_clist_select_row(GTK_CLIST(widget), 0, 0);
    gtk_clist_set_column_width(GTK_CLIST(widget), 0, 180);
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled, 320, 115);
    gtk_container_add(GTK_CONTAINER(scrolled), widget);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    section = add_section(page, "Handle box and legacy ruler");
    handle = gtk_handle_box_new();
    gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(handle), GTK_POS_LEFT);
    gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handle), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(handle), build_toolbar());
    gtk_box_pack_start(GTK_BOX(section), handle, FALSE, FALSE, 0);
    widget = gtk_hruler_new();
    gtk_ruler_set_range(GTK_RULER(widget), 0, 100, 37, 100);
    gtk_widget_set_size_request(widget, -1, 30);
    gtk_box_pack_start(GTK_BOX(section), widget, FALSE, FALSE, 0);

    section = add_section(page, "Alignment and event box");
    box = new_hbox(10);
    widget = gtk_alignment_new(0.0f, 0.5f, 0.0f, 0.0f);
    gtk_alignment_set_padding(GTK_ALIGNMENT(widget), 8, 8, 18, 18);
    gtk_container_add(GTK_CONTAINER(widget), gtk_button_new_with_label("Left aligned child"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    widget = gtk_event_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(widget), 10);
    gtk_container_add(GTK_CONTAINER(widget), gtk_label_new("GtkEventBox input surface"));
    gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(section), box, FALSE, FALSE, 0);

    return scroll_page(page);
}

static void
append_page(GtkWidget *notebook, GtkWidget *page, const gchar *label)
{
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, gtk_label_new(label));
}

int
main(int argc, char **argv)
{
    GtkWidget *root;
    GtkWidget *notebook;
    guint context;

    gtk_init(&argc, &argv);

    audit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(audit_window), "Quartz GTK 2 Audit");
    gtk_window_set_default_size(GTK_WINDOW(audit_window), 1040, 780);
    gtk_window_set_position(GTK_WINDOW(audit_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_icon_name("preferences-desktop-theme");
    g_signal_connect(audit_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    root = new_vbox(0);
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
    append_page(notebook, build_feedback_page(), "Feedback & Display");
    append_page(notebook, build_menus_page(), "Menus & Popups");
    append_page(notebook, build_dialogs_page(), "Dialogs & Choosers");
    append_page(notebook, build_legacy_page(), "GTK 2 Legacy");
    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 0);

    audit_statusbar = gtk_statusbar_new();
    context = gtk_statusbar_get_context_id(GTK_STATUSBAR(audit_statusbar), "audit");
    gtk_statusbar_push(GTK_STATUSBAR(audit_statusbar),
                       context,
                       "Native GTK 2.24 audit — interact with controls to inspect every state");
    gtk_box_pack_end(GTK_BOX(root), audit_statusbar, FALSE, FALSE, 0);

    gtk_widget_show_all(audit_window);
    gtk_widget_grab_default(audit_default_button);
    gtk_spinner_start(GTK_SPINNER(audit_spinner));
    gtk_main();
    return 0;
}
