#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly helper="${1:-$settings_dir/build/quartz-panel-theme}"

test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-panel-theme-test.XXXXXX")"
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

assert_value() {
  local expected="$1"
  local tested_file="$2"

  [[ -r "$tested_file" ]] || fail "missing fake setting: $tested_file"
  [[ "$(<"$tested_file")" == "$expected" ]] ||
    fail "expected '$expected' in $tested_file"
}

run_helper() {
  PATH="$test_tmp/bin:$PATH" \
  QUARTZ_FAKE_GSETTINGS_STATE="$test_tmp/state" \
  QUARTZ_PANEL_BACKGROUND="$test_tmp/panel-bar.xpm" \
    "$@"
}

mkdir -p \
  "$test_tmp/bin" \
  "$test_tmp/state/top" \
  "$test_tmp/state/bottom" \
  "$test_tmp/state/left"
printf 'palette-one\n' >"$test_tmp/panel-bar.xpm"
first_panel_digest="$(sha256sum -- "$test_tmp/panel-bar.xpm" | awk '{ print $1 }')"
first_panel_generation="$test_tmp/panel-bar-$first_panel_digest.xpm"
cp -- "$test_tmp/panel-bar.xpm" "$first_panel_generation"
touch "$test_tmp/set.log"
printf '%s\n' "'Quartz-System6'" >"$test_tmp/state/theme"
printf '%s\n' "'top'" >"$test_tmp/state/top/orientation"
printf '%s\n' '28' >"$test_tmp/state/top/size"
printf '%s\n' "'none'" >"$test_tmp/state/top/type"
printf '%s\n' "''" >"$test_tmp/state/top/image"
printf '%s\n' 'true' >"$test_tmp/state/top/fit"
printf '%s\n' 'true' >"$test_tmp/state/top/stretch"
printf '%s\n' 'true' >"$test_tmp/state/top/rotate"
printf '%s\n' '12000' >"$test_tmp/state/top/opacity"
printf '%s\n' "'bottom'" >"$test_tmp/state/bottom/orientation"
printf '%s\n' '24' >"$test_tmp/state/bottom/size"
printf '%s\n' "'color'" >"$test_tmp/state/bottom/type"
printf '%s\n' "'/old/panel.xpm'" >"$test_tmp/state/bottom/image"
printf '%s\n' 'false' >"$test_tmp/state/bottom/fit"
printf '%s\n' 'false' >"$test_tmp/state/bottom/stretch"
printf '%s\n' 'false' >"$test_tmp/state/bottom/rotate"
printf '%s\n' '6000' >"$test_tmp/state/bottom/opacity"
printf '%s\n' "'left'" >"$test_tmp/state/left/orientation"
printf '%s\n' '41' >"$test_tmp/state/left/size"
printf '%s\n' "'none'" >"$test_tmp/state/left/type"
printf '%s\n' "''" >"$test_tmp/state/left/image"
printf '%s\n' 'false' >"$test_tmp/state/left/fit"
printf '%s\n' 'false' >"$test_tmp/state/left/stretch"
printf '%s\n' 'false' >"$test_tmp/state/left/rotate"
printf '%s\n' '4096' >"$test_tmp/state/left/opacity"

cp "$settings_dir/tests/fixtures/fake-gsettings" "$test_tmp/bin/gsettings"
cp "$settings_dir/tests/fixtures/fake-dconf" "$test_tmp/bin/dconf"
chmod 0755 "$test_tmp/bin/gsettings" "$test_tmp/bin/dconf"

[[ -x "$helper" ]] || fail "panel helper is not executable: $helper"
run_helper "$helper" --apply

assert_value '25' "$test_tmp/state/top/size"
assert_value "'image'" "$test_tmp/state/top/type"
assert_value "'$first_panel_generation'" "$test_tmp/state/top/image"
assert_value 'false' "$test_tmp/state/top/fit"
assert_value 'false' "$test_tmp/state/top/stretch"
assert_value 'false' "$test_tmp/state/top/rotate"
assert_value '65535' "$test_tmp/state/top/opacity"
assert_value '25' "$test_tmp/state/bottom/size"
assert_value "'image'" "$test_tmp/state/bottom/type"
assert_value "'$first_panel_generation'" "$test_tmp/state/bottom/image"
assert_value '65535' "$test_tmp/state/bottom/opacity"
assert_value '41' "$test_tmp/state/left/size"
assert_value '4096' "$test_tmp/state/left/opacity"
if grep -Fq ':type=none' "$test_tmp/set.log"; then
  fail 'panel helper exposed an invalid background during synchronization'
fi

for panel_id in top bottom; do
  image_line="$(grep -n -m1 -F "$panel_id:image=" "$test_tmp/set.log" | cut -d: -f1)"
  type_line="$(grep -n -m1 -F "$panel_id:type=image" "$test_tmp/set.log" | cut -d: -f1)"
  [[ -n "$image_line" && -n "$type_line" && "$image_line" -lt "$type_line" ]] ||
    fail "$panel_id panel selected its image type before configuring the image"
done

run_helper "$helper" --check || fail 'panel helper rejected synchronized settings'

printf '%s\n' '6000' >"$test_tmp/state/top/opacity"
if run_helper "$helper" --check; then
  fail 'panel helper accepted a translucent horizontal panel'
fi
run_helper "$helper" --apply
assert_value '65535' "$test_tmp/state/top/opacity"

: >"$test_tmp/set.log"
run_helper "$helper" --refresh
assert_value "'$first_panel_generation'" "$test_tmp/state/top/image"
assert_value "'$first_panel_generation'" "$test_tmp/state/bottom/image"
if grep -Fq ':type=none' "$test_tmp/set.log"; then
  fail 'panel helper exposed an invalid background during a palette refresh'
fi
[[ ! -s "$test_tmp/set.log" ]] ||
  fail 'an unchanged content-addressed bitmap caused redundant setting writes'

printf 'palette-two\n' >"$test_tmp/panel-bar.xpm"
second_panel_digest="$(sha256sum -- "$test_tmp/panel-bar.xpm" | awk '{ print $1 }')"
second_panel_generation="$test_tmp/panel-bar-$second_panel_digest.xpm"
cp -- "$test_tmp/panel-bar.xpm" "$second_panel_generation"
if run_helper "$helper" --check; then
  fail 'panel helper accepted a stale content-addressed bitmap'
fi
run_helper "$helper" --refresh
assert_value "'$second_panel_generation'" "$test_tmp/state/top/image"
assert_value "'$second_panel_generation'" "$test_tmp/state/bottom/image"

printf '%s\n' "'Other-Theme'" >"$test_tmp/state/theme"
printf '%s\n' '32' >"$test_tmp/state/top/size"
run_helper "$helper" --apply
assert_value '32' "$test_tmp/state/top/size"

printf '%s\n' "'Quartz-System6'" >"$test_tmp/state/theme"
if run_helper "$helper" --check; then
  fail 'panel helper accepted a mismatched horizontal panel'
fi

run_helper "$helper" --apply
assert_value '25' "$test_tmp/state/top/size"

: >"$test_tmp/set.log"
set +e
run_helper timeout 5 "$helper" --watch
watch_status=$?
set -e
[[ "$watch_status" == "124" || "$watch_status" == "143" ]] ||
  fail "panel watcher exited unexpectedly with status $watch_status"
assert_value '25' "$test_tmp/state/top/size"
assert_value '65535' "$test_tmp/state/top/opacity"
[[ "$(grep -Fc 'top:size=25' "$test_tmp/set.log")" == "1" ]] ||
  fail 'panel watcher did not debounce the layout size burst'
[[ "$(grep -Fc 'top:opacity=65535' "$test_tmp/set.log")" == "1" ]] ||
  fail 'panel watcher did not debounce the layout background burst'

printf 'Quartz panel synchronization tests passed.\n'
