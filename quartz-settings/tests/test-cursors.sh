#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'
readonly project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
readonly cursor_dir="$project_dir/cursors/Quartz-System7-cursors"
readonly test_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-cursor-test.XXXXXX")"
trap 'find "$test_tmp" -depth -delete' EXIT
fail() { printf 'test failure: %s\n' "$*" >&2; exit 1; }

mkdir "$test_tmp/output" "$test_tmp/temp with spaces"
printf 'unrelated data\n' >"$test_tmp/output/keep.txt"
cp "$test_tmp/output/keep.txt" "$test_tmp/original"
for pass in 1 2; do
  TMPDIR="$test_tmp/temp with spaces" bash "$cursor_dir/build-cursors.sh" "$test_tmp/output"
  cmp "$test_tmp/original" "$test_tmp/output/keep.txt"
  for cursor in "$cursor_dir/cursors"/*; do
    cmp "$cursor" "$test_tmp/output/${cursor##*/}"
  done
done

ln -s "$test_tmp/output" "$test_tmp/link"
if bash "$cursor_dir/build-cursors.sh" "$test_tmp/link" >/dev/null 2>&1; then
  fail 'accepted a symlink output directory'
fi
mkdir "$test_tmp/unsafe"
ln -s "$test_tmp/original" "$test_tmp/unsafe/arrow"
if bash "$cursor_dir/build-cursors.sh" "$test_tmp/unsafe" >/dev/null 2>&1; then
  fail 'accepted a symlink cursor destination'
fi
cmp "$test_tmp/original" "$test_tmp/output/keep.txt"
[[ ! -e "$test_tmp/unsafe/cell" ]] || fail 'partially installed unsafe output'
if bash "$cursor_dir/build-cursors.sh" "$test_tmp/output" extra >/dev/null 2>&1; then
  fail 'accepted extra arguments'
fi
printf 'Quartz cursor rebuild tests passed (19 payloads, repeatability, output safety).\n'
