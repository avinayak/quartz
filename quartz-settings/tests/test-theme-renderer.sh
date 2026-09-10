#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly project_dir="$(cd -- "$settings_dir/.." && pwd -P)"
readonly theme_dir="$project_dir/theme/Quartz-System6"
readonly renderer="$theme_dir/render-theme.sh"
readonly helper="${1:-$settings_dir/build/quartz-theme-apply}"

test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-settings-test.XXXXXX")"
readonly test_tmp

cleanup() {
  if [[ -d "$test_tmp" && ! -L "$test_tmp" ]]; then
    find "$test_tmp" -depth -delete
  fi
}
trap cleanup EXIT

fail() {
  printf 'test failure: %s\n' "$*" >&2
  exit 1
}

assert_line() {
  local expected="$1"
  local tested_file="$2"

  grep -Fqx -- "$expected" "$tested_file" ||
    fail "missing '$expected' in $tested_file"
}

bash "$renderer" "$test_tmp/default"
assert_line 'QUARTZ_BORDER_RADIUS_PX=3' "$test_tmp/default/theme.conf"
assert_line 'QUARTZ_WINDOW_LAYOUT=cupertino' "$test_tmp/default/theme.conf"
assert_line 'QUARTZ_MENUBAR_BG=#ffffff' "$test_tmp/default/theme.conf"
assert_line 'QUARTZ_TITLEBAR_BG=#ffffff' "$test_tmp/default/theme.conf"
assert_line 'QUARTZ_APPLICATION_BG=#ffffff' "$test_tmp/default/theme.conf"

# Legacy configurations without a layout also use the Cupertino default.
sed '/^QUARTZ_WINDOW_LAYOUT=/d' "$theme_dir/theme.conf" >"$test_tmp/legacy.conf"
bash "$renderer" --config "$test_tmp/legacy.conf" "$test_tmp/legacy"
assert_line 'QUARTZ_WINDOW_LAYOUT=cupertino' "$test_tmp/legacy/theme.conf"
cmp "$test_tmp/default/metacity-1/metacity-theme-1.xml" \
  "$test_tmp/legacy/metacity-1/metacity-theme-1.xml"
assert_line '           x="frame_x_center-title_width/2"' \
  "$test_tmp/default/metacity-1/metacity-theme-1.xml"
assert_line '            left="7" right="7"' \
  "$test_tmp/default/metacity-1/metacity-theme-1.xml"

# A hand-edited configuration need not end in a newline. Palette values that
# equal a source bitmap's other color roles must not be recolored twice.
for application_color in '#ebebeb' '#d9d9d9' '#777777' '#7f7f7f'; do
  collision_dir="$test_tmp/collision-${application_color#\#}"
  sed '/^QUARTZ_APPLICATION_BG=/d' "$theme_dir/theme.conf" >"$test_tmp/collision.conf"
  printf 'QUARTZ_APPLICATION_BG=%s' "$application_color" >>"$test_tmp/collision.conf"
  bash "$renderer" --config "$test_tmp/collision.conf" "$collision_dir"
  assert_line "QUARTZ_APPLICATION_BG=$application_color" "$collision_dir/theme.conf"
  assert_line "\". c $application_color\"," "$collision_dir/gtk-2.0/assets/check-off.xpm"
  assert_line "\"+ c $application_color\"," "$collision_dir/gtk-3.0/assets/scale-dither.xpm"
done
cp "$theme_dir/theme.conf" "$test_tmp/no-newline-invalid.conf"
printf 'UNKNOWN_SETTING=invalid' >>"$test_tmp/no-newline-invalid.conf"
if bash "$renderer" --config "$test_tmp/no-newline-invalid.conf" "$test_tmp/invalid" \
  >/dev/null 2>&1; then
  fail 'renderer ignored an invalid unterminated configuration line'
fi

printf '%s\n' \
  'QUARTZ_BORDER_RADIUS_PX=7' \
  'QUARTZ_WINDOW_LAYOUT=redmond-reversed' \
  'QUARTZ_MENUBAR_BG=#dcecff' \
  'QUARTZ_TITLEBAR_BG=#bdd8fb' \
  'QUARTZ_APPLICATION_BG=#f0f7ff' \
  >"$test_tmp/custom.conf"
bash "$renderer" --config "$test_tmp/custom.conf" "$test_tmp/custom"

assert_line 'QUARTZ_WINDOW_LAYOUT=redmond-reversed' "$test_tmp/custom/theme.conf"
assert_line '@define-color quartz_application #f0f7ff;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_paper @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_disabled #707377;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_disabled_bg #ccd2d9;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_pressed #dde4eb;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_toggle_on #dde4eb;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_scroll_thumb @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_menubar #dcecff;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_titlebar #bdd8fb;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color theme_base_color @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color theme_unfocused_base_color @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color content_view_bg @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color text_view_bg @quartz_application;' "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '@define-color quartz_application #f0f7ff;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_paper @quartz_application;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_disabled #707377;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_disabled_bg #ccd2d9;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_pressed #dde4eb;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_toggle_on #dde4eb;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color quartz_scroll_thumb @quartz_application;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '@define-color view_bg_color @quartz_application;' "$test_tmp/custom/gtk-4.0/gtk.css"
assert_line '"+ c #dcecff",' "$test_tmp/custom/gtk-2.0/assets/menubar-app.xpm"
assert_line '"+ c #dcecff",' "$test_tmp/custom/gtk-3.0/assets/panel-bar.xpm"
custom_panel_digest="$(sha256sum -- \
  "$test_tmp/custom/gtk-3.0/assets/panel-bar.xpm" | awk '{ print $1 }')"
cmp -s -- \
  "$test_tmp/custom/gtk-3.0/assets/panel-bar.xpm" \
  "$test_tmp/custom/gtk-3.0/assets/panel-bar-$custom_panel_digest.xpm" ||
  fail "content-addressed panel bitmap differs from the rendered panel bitmap"
assert_line '"+ c #f0f7ff",' "$test_tmp/custom/gtk-2.0/assets/menu-shadow.xpm"
assert_line '"+ c #f0f7ff",' "$test_tmp/custom/gtk-3.0/assets/menu-frame.xpm"
custom_menu_digest="$(sha256sum -- \
  "$test_tmp/custom/gtk-3.0/assets/menu-frame.xpm" | awk '{ print $1 }')"
cmp -s -- \
  "$test_tmp/custom/gtk-3.0/assets/menu-frame.xpm" \
  "$test_tmp/custom/gtk-3.0/assets/menu-frame-$custom_menu_digest.xpm" ||
  fail "content-addressed menu frame differs from the rendered menu frame"
assert_line \
  "  border-image-source: url(\"assets/menu-frame-$custom_menu_digest.xpm\");" \
  "$test_tmp/custom/gtk-3.0/gtk.css"
assert_line '"+ c #f0f7ff",' "$test_tmp/custom/gtk-2.0/assets/scrollbar-thumb.xpm"
grep -Fq 'bg_color:#f0f7ff' "$test_tmp/custom/gtk-2.0/gtkrc" ||
  fail "GTK 2 does not use the application background"
grep -Fq 'base_color:#f0f7ff' "$test_tmp/custom/gtk-2.0/gtkrc" ||
  fail "GTK 2 content views do not use the application background"
assert_line '  base[NORMAL] = "#f0f7ff"' "$test_tmp/custom/gtk-2.0/gtkrc"
assert_line '  fg[INSENSITIVE] = "#707377"' "$test_tmp/custom/gtk-2.0/gtkrc"
assert_line '  bg[ACTIVE] = "#dde4eb"' "$test_tmp/custom/gtk-2.0/gtkrc"
assert_line '"+ c #dde4eb",' "$test_tmp/custom/gtk-2.0/assets/button-shade-raised.xpm"
assert_line '". c #707377",' "$test_tmp/custom/gtk-2.0/assets/button-disabled-raised.xpm"
assert_line '"+ c #ccd2d9",' "$test_tmp/custom/gtk-2.0/assets/button-disabled-raised.xpm"
assert_line '". c #707377",' "$test_tmp/custom/gtk-3.0/assets/arrow-down-disabled.xpm"
assert_line 'widget_class "*.<GtkTextView>" style "quartz-default"' "$test_tmp/custom/gtk-2.0/gtkrc"
grep -Fq 'color="#bdd8fb"' "$test_tmp/custom/metacity-1/metacity-theme-1.xml" ||
  fail "Marco does not use the titlebar background"
[[ "$(grep -Fc 'x="width-title_width-7"' "$test_tmp/custom/metacity-1/metacity-theme-1.xml")" == "2" ]] ||
  fail "Marco does not right-align focused and unfocused titles"
grep -Fq 'x="width-title_width-13"' "$test_tmp/custom/metacity-1/metacity-theme-1.xml" ||
  fail "Marco does not right-align the focused-title stripe mask"
assert_line '            left="2" right="2"' \
  "$test_tmp/custom/metacity-1/metacity-theme-1.xml"
if grep -Fq 'frame_x_center-title_width' "$test_tmp/custom/metacity-1/metacity-theme-1.xml"; then
  fail "Marco still contains centered-title geometry"
fi

sed 's/^QUARTZ_WINDOW_LAYOUT=.*/QUARTZ_WINDOW_LAYOUT=cupertino/' \
  "$test_tmp/custom.conf" >"$test_tmp/cupertino.conf"
bash "$renderer" --config "$test_tmp/cupertino.conf" "$test_tmp/cupertino"
assert_line 'QUARTZ_WINDOW_LAYOUT=cupertino' "$test_tmp/cupertino/theme.conf"
[[ "$(grep -Fxc '           x="frame_x_center-title_width/2"' "$test_tmp/cupertino/metacity-1/metacity-theme-1.xml")" == "2" ]] ||
  fail "Cupertino does not center focused and unfocused titles"
assert_line '               x="frame_x_center-title_width/2-6" y="4+height*(1-(title_width `min` 1))"' \
  "$test_tmp/cupertino/metacity-1/metacity-theme-1.xml"
assert_line '            left="7" right="7"' \
  "$test_tmp/cupertino/metacity-1/metacity-theme-1.xml"

sed 's/^QUARTZ_WINDOW_LAYOUT=.*/QUARTZ_WINDOW_LAYOUT=redmond/' \
  "$test_tmp/custom.conf" >"$test_tmp/redmond.conf"
bash "$renderer" --config "$test_tmp/redmond.conf" "$test_tmp/redmond"
assert_line 'QUARTZ_WINDOW_LAYOUT=redmond' "$test_tmp/redmond/theme.conf"
[[ "$(grep -Fxc '           x="7"' "$test_tmp/redmond/metacity-1/metacity-theme-1.xml")" == "2" ]] ||
  fail "Redmond layout does not left-align focused and unfocused titles"
assert_line '               x="1" y="4+height*(1-(title_width `min` 1))"' \
  "$test_tmp/redmond/metacity-1/metacity-theme-1.xml"
assert_line '            left="2" right="2"' \
  "$test_tmp/redmond/metacity-1/metacity-theme-1.xml"

if grep -ERq '@QUARTZ_[A-Z0-9_]+@' "$test_tmp/custom"; then
  fail "rendered theme contains an unresolved placeholder"
fi

while IFS= read -r rendered_color; do
  rendered_color="${rendered_color#\#}"
  if [[ "${rendered_color:0:2}" == "${rendered_color:2:2}" &&
        "${rendered_color:2:2}" == "${rendered_color:4:2}" &&
        "$rendered_color" != "000000" ]]; then
    fail "colored palette left neutral #$rendered_color in the rendered theme"
  fi
done < <(
  grep -ERho '#[[:xdigit:]]{6}' "$test_tmp/custom" |
    tr '[:upper:]' '[:lower:]' |
    sort -u
)

printf '%s\n' \
  'QUARTZ_BORDER_RADIUS_PX=7' \
  'QUARTZ_MENUBAR_BG=blue' \
  'QUARTZ_TITLEBAR_BG=#bdd8fb' \
  'QUARTZ_APPLICATION_BG=#f0f7ff' \
  >"$test_tmp/invalid.conf"
if bash "$renderer" --config "$test_tmp/invalid.conf" "$test_tmp/invalid" \
  >/dev/null 2>&1; then
  fail "renderer accepted a non-#RRGGBB palette"
fi

printf '%s\n' \
  'QUARTZ_BORDER_RADIUS_PX=7' \
  'QUARTZ_WINDOW_LAYOUT=sideways' \
  'QUARTZ_MENUBAR_BG=#dcecff' \
  'QUARTZ_TITLEBAR_BG=#bdd8fb' \
  'QUARTZ_APPLICATION_BG=#f0f7ff' \
  >"$test_tmp/invalid-layout.conf"
if bash "$renderer" --config "$test_tmp/invalid-layout.conf" "$test_tmp/invalid-layout" \
  >/dev/null 2>&1; then
  fail "renderer accepted an unknown window layout"
fi

[[ -x "$helper" ]] || fail "theme helper is not executable: $helper"
HOME="$test_tmp/home" \
XDG_CONFIG_HOME="$test_tmp/home/config" \
XDG_DATA_HOME="$test_tmp/home/data" \
QUARTZ_THEME_SOURCE_DIR="$theme_dir" \
QUARTZ_SKIP_DESKTOP_REFRESH=1 \
  "$helper" \
    --menubar '#f8dce0' \
    --titlebar '#eabac2' \
    --application '#fff1f3' \
    --window-layout redmond \
    >/dev/null

readonly installed_theme="$test_tmp/home/.themes/Quartz-System6"
readonly stored_config="$test_tmp/home/config/quartz-settings/theme.conf"
assert_line 'QUARTZ_WINDOW_LAYOUT=redmond' "$stored_config"
assert_line 'QUARTZ_MENUBAR_BG=#f8dce0' "$stored_config"
assert_line 'QUARTZ_TITLEBAR_BG=#eabac2' "$stored_config"
assert_line 'QUARTZ_APPLICATION_BG=#fff1f3' "$stored_config"
assert_line '@define-color quartz_application #fff1f3;' "$installed_theme/gtk-4.0/gtk.css"
assert_line '"+ c #f8dce0",' "$installed_theme/gtk-3.0/assets/panel-bar.xpm"
installed_panel_digest="$(sha256sum -- \
  "$installed_theme/gtk-3.0/assets/panel-bar.xpm" | awk '{ print $1 }')"
cmp -s -- \
  "$installed_theme/gtk-3.0/assets/panel-bar.xpm" \
  "$installed_theme/gtk-3.0/assets/panel-bar-$installed_panel_digest.xpm" ||
  fail "installed content-addressed panel bitmap differs from the panel bitmap"
installed_menu_digest="$(sha256sum -- \
  "$installed_theme/gtk-3.0/assets/menu-frame.xpm" | awk '{ print $1 }')"
cmp -s -- \
  "$installed_theme/gtk-3.0/assets/menu-frame.xpm" \
  "$installed_theme/gtk-3.0/assets/menu-frame-$installed_menu_digest.xpm" ||
  fail "installed content-addressed menu frame differs from the menu frame"
assert_line \
  "  border-image-source: url(\"assets/menu-frame-$installed_menu_digest.xpm\");" \
  "$installed_theme/gtk-3.0/gtk.css"
[[ "$(grep -Fxc '           x="7"' "$installed_theme/metacity-1/metacity-theme-1.xml")" == "2" ]] ||
  fail "theme helper did not install the selected Redmond title geometry"
assert_line '            left="2" right="2"' \
  "$installed_theme/metacity-1/metacity-theme-1.xml"

if command -v xmllint >/dev/null 2>&1; then
  xmllint --noout "$test_tmp/custom/metacity-1/metacity-theme-1.xml"
  xmllint --noout "$test_tmp/cupertino/metacity-1/metacity-theme-1.xml"
  xmllint --noout "$test_tmp/redmond/metacity-1/metacity-theme-1.xml"
fi

printf 'Quartz shared-theme renderer tests passed (3 window layouts).\n'
