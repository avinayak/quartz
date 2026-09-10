#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly settings_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
readonly source_file="${1:-$settings_dir/src/quartz-settings.c}"

[[ -r "$source_file" ]] || {
  printf 'test failure: preset source is not readable: %s\n' "$source_file" >&2
  exit 1
}

awk '
  function fail(message) {
    print "test failure: " message > "/dev/stderr"
    failed = 1
  }

  function valid_color(value) {
    return length(value) == 7 && substr(value, 1, 1) == "#" &&
      substr(value, 2) ~ /^[0-9a-f]+$/
  }

  function remember(display_name, palette_key) {
    if (seen_name[display_name]++) {
      fail("duplicate preset name: " display_name)
    }
    if (seen_palette[palette_key]++) {
      fail("duplicate preset palette: " display_name)
    }
  }

  /^[[:space:]]*FLAT_PRESET\(/ {
    count = split($0, field, "\"")
    if (count < 5 || !valid_color(field[4])) {
      fail("malformed flat preset: " $0)
      next
    }
    remember(field[2], field[4] SUBSEP field[4] SUBSEP field[4])
    flat_color[field[2]] = field[4]
    flat++
  }

  /^[[:space:]]*MIXED_PRESET\(/ {
    count = split($0, field, "\"")
    if (count < 9 || !valid_color(field[4]) ||
        !valid_color(field[6]) || !valid_color(field[8])) {
      fail("malformed mixed preset: " $0)
      next
    }
    if (field[4] == field[6] && field[6] == field[8]) {
      fail("mixed preset is flat: " field[2])
    }
    remember("Mixed - " field[2], field[4] SUBSEP field[6] SUBSEP field[8])
    mixed_application[field[2]] = field[8]
    mixed++
  }

  END {
    if (flat != 20) {
      fail("expected 20 flat presets, found " flat)
    }
    if (mixed != 20) {
      fail("expected 20 mixed presets, found " mixed)
    }
    for (name in flat_color) {
      if (!(name in mixed_application)) {
        fail("flat preset has no mixed counterpart: " name)
      } else if (flat_color[name] != mixed_application[name]) {
        fail("flat and mixed application tiers differ: " name)
      }
    }
    exit failed ? 1 : 0
  }
' "$source_file"

grep -Fq 'G_STATIC_ASSERT(G_N_ELEMENTS(theme_presets) == 40);' "$source_file" || {
  printf 'test failure: compile-time preset count is not 40\n' >&2
  exit 1
}

printf 'Quartz preset catalog tests passed (20 flat + 20 mixed).\n'
