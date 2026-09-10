#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly source_file="${1:-$settings_dir/src/quartz-settings.c}"

fail() {
  printf 'test failure: %s\n' "$*" >&2
  exit 1
}

assert_source() {
  grep -Fq -- "$1" "$source_file" || fail "preview is missing: $1"
}

[[ -r "$source_file" ]] || fail "preview source is not readable: $source_file"

assert_source 'PREVIEW_TITLEBAR_HEIGHT = 19'
assert_source 'PREVIEW_MENUBAR_HEIGHT = 25'
assert_source 'PREVIEW_BUTTON_HEIGHT = 22'
assert_source 'cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);'
assert_source 'cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_NONE);'
assert_source 'title = preview_text_layout(widget, "Preview", "ChiKareGo2", 16);'
assert_source 'label = preview_text_layout(widget, "Button", "Geneva", 15);'
assert_source 'draw_preview_outline(context,'
assert_source 'inside_preview_rounding('
assert_source 'view->border_radius = PREVIEW_DEFAULT_RADIUS;'
assert_source 'view->window_layout = WINDOW_LAYOUT_CUPERTINO;'
assert_source 'g_str_has_prefix(line, "QUARTZ_BORDER_RADIUS_PX=")'

if grep -Fq 'cairo_select_font_face' "$source_file" ||
   grep -Fq 'cairo_show_text' "$source_file" ||
   grep -Fq 'cairo_set_source_rgb(context, 1.0, 1.0, 1.0)' "$source_file"; then
  fail "preview contains a generic-font or hard-coded-white drawing fallback"
fi

printf 'Quartz pixel-preview source checks passed (19px title + 25px menu).\n'
