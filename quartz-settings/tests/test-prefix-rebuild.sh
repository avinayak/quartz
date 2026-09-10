#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-prefix-test.XXXXXX")"
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

assert_generated_prefix() {
  local prefix="$1"

  grep -Fqx "Exec=$prefix/bin/quartz-settings" \
    "$test_tmp/build/org.quartz.Settings.desktop" ||
    fail "desktop entry retained a stale installation prefix"
  grep -Fq \
    "readonly theme_source_dir=\"\${QUARTZ_THEME_SOURCE_DIR:-$prefix/share/quartz-settings/theme/Quartz-System6}\"" \
    "$test_tmp/build/quartz-theme-apply" ||
    fail "theme helper retained a stale installation prefix"
  grep -Fqx \
    "ExecStart=$prefix/libexec/quartz-settings/quartz-panel-theme --watch" \
    "$test_tmp/build/quartz-panel-theme.service" ||
    fail "panel service retained a stale installation prefix"
}

generate_for_prefix() {
  local prefix="$1"

  make --no-print-directory -C "$settings_dir" \
    builddir="$test_tmp/build" \
    PREFIX="$prefix" \
    "$test_tmp/build/org.quartz.Settings.desktop" \
    "$test_tmp/build/quartz-theme-apply" \
    "$test_tmp/build/quartz-panel-theme.service" \
    >/dev/null
}

generate_for_prefix "$test_tmp/prefix-one"
assert_generated_prefix "$test_tmp/prefix-one"
generate_for_prefix "$test_tmp/prefix-two"
assert_generated_prefix "$test_tmp/prefix-two"

printf 'Quartz prefix-sensitive rebuild tests passed.\n'
