#!/usr/bin/env bash

# Exercise installer validation and optional desktop refresh without installing
# anything or connecting to the user's settings service.
set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly project_dir="$(cd -- "$settings_dir/.." && pwd -P)"
readonly test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-guards.XXXXXX")"
trap 'find "$test_tmp" -depth -delete' EXIT

die() { printf 'error: %s\n' "$*" >&2; exit 1; }

# Load only the actual function under test, never the installer's entry point.
load_function() {
  local definition
  definition="$(awk -v name="$2" '
    $0 == name "() {" { copying = 1 }
    copying { print }
    copying && /^}$/ { exit }
  ' "$1")"
  [[ -n "$definition" ]] || die "missing function: $2"
  eval "$definition"
}

load_function "$project_dir/apply-ubuntu-mate-theme.sh" install_extras_if_present
install_extras_if_present "$test_tmp/missing-extras"
mkdir "$test_tmp/extras"
if (install_extras_if_present "$test_tmp/extras") 2>/dev/null; then
  die 'silently skipped an existing extras folder without an installer'
fi
printf 'exit 7\n' >"$test_tmp/extras/install.sh"
extras_status=0
(install_extras_if_present "$test_tmp/extras") || extras_status=$?
[[ "$extras_status" == 7 ]] || die 'extras installer failure was swallowed'
export QUARTZ_EXTRAS_TEST_LOG="$test_tmp/extras.log"
printf '%s\n' \
  'printf "wallpapers\n" >>"$QUARTZ_EXTRAS_TEST_LOG"' \
  'printf "other extras\n" >>"$QUARTZ_EXTRAS_TEST_LOG"' \
  >"$test_tmp/extras/install.sh"
install_extras_if_present "$test_tmp/extras"
install_extras_if_present "$test_tmp/extras"
printf 'wallpapers\nother extras\nwallpapers\nother extras\n' >"$test_tmp/expected-extras.log"
cmp "$test_tmp/expected-extras.log" "$QUARTZ_EXTRAS_TEST_LOG" ||
  die 'the complete extras installer did not run on every invocation'

load_function "$project_dir/apply-ubuntu-mate-theme.sh" validate_managed_block
validate_managed_block "$test_tmp/missing" BEGIN END
for content in '' 'user text' $'before\nBEGIN\nmanaged\nEND\nafter'; do
  printf '%s\n' "$content" >"$test_tmp/settings"
  validate_managed_block "$test_tmp/settings" BEGIN END
done
for content in \
  $'END\nuser text\nBEGIN' \
  $'BEGIN\nBEGIN\nEND\nEND' \
  $'BEGIN\nEND\nBEGIN\nEND' \
  'BEGIN' 'END'; do
  printf '%s\n' "$content" >"$test_tmp/settings"
  cp "$test_tmp/settings" "$test_tmp/original"
  if (validate_managed_block "$test_tmp/settings" BEGIN END) 2>/dev/null; then
    die "accepted malformed managed block: $content"
  fi
  cmp "$test_tmp/settings" "$test_tmp/original"
done

load_function "$project_dir/apply-ubuntu-mate-theme.sh" validate_pixel_frame_rule
for radius in 0 1 2 5 7 39 40 64; do
  sed "s/^QUARTZ_BORDER_RADIUS_PX=.*/QUARTZ_BORDER_RADIUS_PX=$radius/" \
    "$project_dir/theme/Quartz-System6/theme.conf" >"$test_tmp/theme.conf"
  bash "$project_dir/theme/Quartz-System6/render-theme.sh" \
    --config "$test_tmp/theme.conf" "$test_tmp/rendered-$radius"
  inset="$((2 * (radius > 0 ? radius + 1 : 2)))px"
  validate_pixel_frame_rule "$test_tmp/rendered-$radius/gtk-3.0/gtk.css" \
    'frame > border {' "$inset" "$radius"
  validate_pixel_frame_rule "$test_tmp/rendered-$radius/gtk-4.0/gtk.css" \
    'frame {' "$inset" "$radius"
  if (( radius >= 40 )); then
    awk -v radius="$radius" '
      FNR == 1 { corner++ }
      FNR == 3 && $0 != "\"" radius + 1 " " radius + 1 " 2 1\"," { exit 1 }
      FNR == 4 && $0 != "\"  c None\"," { exit 1 }
      FNR == 5 && $0 != "\". c #000000\"," { exit 1 }
      FNR >= 6 {
        row = substr($0, 2, radius + 1)
        if (length(row) != radius + 1 || row ~ /[^ .]/) exit 1
        for (x = 0; x <= radius; x++) {
          pixel[corner, x, FNR - 6] = substr(row, x + 1, 1)
          if (substr(row, x + 1, 1) == ".") dots[corner]++
        }
        rows[corner]++
      }
      END {
        for (corner = 1; corner <= 4; corner++) {
          if (rows[corner] != radius + 1 ||
              dots[corner] != (radius == 40 ? 32 : 50)) exit 1
        }
        for (y = 0; y <= radius; y++) {
          for (x = 0; x <= radius; x++) {
            if (pixel[1,x,y] != pixel[2,radius-x,y] ||
                pixel[1,x,y] != pixel[3,x,radius-y] ||
                pixel[1,x,y] != pixel[4,radius-x,radius-y]) exit 1
          }
        }
      }
    ' "$test_tmp/rendered-$radius/gtk-3.0/assets/"frame-corner-*.xpm
  fi
done

load_function "$settings_dir/data/quartz-theme-apply.in" read_window_layout
eval "$(sed -n '/^readonly default_window_layout=/p' "$settings_dir/data/quartz-theme-apply.in")"
palette_file="$test_tmp/layout.conf"
theme_destination="$test_tmp/missing-theme"
theme_source_dir="$test_tmp/missing-source"
[[ "$(read_window_layout)" == cupertino ]] || die 'layout fallback is not Cupertino'
for saved_layout in cupertino redmond redmond-reversed; do
  printf 'QUARTZ_WINDOW_LAYOUT=%s\n' "$saved_layout" >"$palette_file"
  [[ "$(read_window_layout)" == "$saved_layout" ]] || die 'saved layout was overridden'
done

load_function "$settings_dir/data/quartz-theme-apply.in" reload_theme_setting
load_function "$settings_dir/data/quartz-theme-apply.in" apply_window_button_layout
readonly theme_name=Quartz-System6
readonly window_button_layout=close:minimize,maximize
schema_exists() { [[ "$schema_available" == yes ]]; }
setting_is_writable() { [[ "$setting_writable" == yes ]]; }
find_fallback_theme() { :; }
gsettings() { printf '%s %s %s %s\n' "$@" >>"$test_tmp/gsettings.log"; }
for schema_available in no yes; do
  for setting_writable in no yes; do
    : >"$test_tmp/gsettings.log"
    reload_theme_setting test.schema theme gtk-3.0
    apply_window_button_layout test.schema button-layout
    if [[ "$schema_available" == yes && "$setting_writable" == yes ]]; then
      [[ "$(wc -l <"$test_tmp/gsettings.log" | tr -d ' ')" == 2 ]] ||
        die 'writable desktop settings were not refreshed'
    else
      [[ ! -s "$test_tmp/gsettings.log" ]] || die 'unavailable setting was written'
    fi
  done
done

# This must fail before any root/live-session checks, commands, or cleanup.
if TARGET_DISK=/dev/unsupported bash "$project_dir/rebuild-ubuntu-mate-guest.sh" \
  >"$test_tmp/rebuild.log" 2>&1; then
  die 'rebuild accepted an unsupported disk'
fi
grep -Fx 'error: the only supported target disk is /dev/vda' "$test_tmp/rebuild.log" >/dev/null

printf 'Quartz shared installer and refresh guard tests passed.\n'
