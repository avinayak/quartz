#!/usr/bin/env bash

# Requires a display (or xvfb-run); only test windows are opened. Desktop
# settings and the installed theme are never changed.
set -Eeuo pipefail
IFS=$'\n\t'
readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-ui-test.XXXXXX")"
trap 'find "$test_tmp" -depth -delete' EXIT

GSETTINGS_BACKEND=memory "$settings_dir/build/test-activation"
GSETTINGS_BACKEND=memory "$settings_dir/build/test-extras"
for ((radius = 0; radius <= 64; radius++)); do
  sed "s/^QUARTZ_BORDER_RADIUS_PX=.*/QUARTZ_BORDER_RADIUS_PX=$radius/" \
    "$settings_dir/../theme/Quartz-System6/theme.conf" >"$test_tmp/theme.conf"
  bash "$settings_dir/../theme/Quartz-System6/render-theme.sh" \
    --config "$test_tmp/theme.conf" "$test_tmp/rendered-$radius"
  for toolkit in 3 4; do
    GSETTINGS_BACKEND=memory "$settings_dir/build/test-css$toolkit" \
      "$test_tmp/rendered-$radius/gtk-$toolkit.0/gtk.css"
  done
done
printf 'Quartz native GTK 3/4 CSS parser tests passed (all 65 radii).\n'
