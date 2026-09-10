#!/usr/bin/env bash

# Rebuild the checked-in Xcursor files from the native 16x16 XPM sources.

set -Eeuo pipefail
IFS=$'\n\t'
umask 022

readonly script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
readonly source_dir="$script_dir/src"
readonly output_dir="${1:-$script_dir/cursors}"

build_tmp=""

cleanup() {
  if [[ -n "$build_tmp" && -d "$build_tmp" && ! -L "$build_tmp" ]]; then
    find "$build_tmp" -depth -delete || true
  fi
}
trap cleanup EXIT

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

resolve_command() {
  local command_name="$1"
  local candidate=""

  if command -v "$command_name" >/dev/null 2>&1; then
    command -v "$command_name"
    return
  fi

  for candidate in "/opt/X11/bin/$command_name" "/usr/X11/bin/$command_name"; do
    if [[ -x "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return
    fi
  done

  return 1
}

readonly image_command="$(resolve_command magick || resolve_command convert || true)"
readonly xcursorgen_command="$(resolve_command xcursorgen || true)"
(( $# <= 1 )) || die "usage: $0 [OUTPUT_DIRECTORY]"
[[ -n "$image_command" ]] || die "ImageMagick's magick or convert command is required"
[[ -n "$xcursorgen_command" ]] || die "xcursorgen is required"
[[ "$output_dir" != "/" ]] || die "refusing to use the filesystem root as output"
[[ ! -L "$output_dir" ]] || die "output directory must not be a symbolic link: $output_dir"
[[ ! -e "$output_dir" || -d "$output_dir" ]] ||
  die "output path is not a directory: $output_dir"

build_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-system7-cursors.XXXXXX")"
readonly staged_cursor_dir="$build_tmp/cursors"
install -d -m 0755 -- "$staged_cursor_dir"

build_cursor() {
  local name="$1"
  local hotspot_x="$2"
  local hotspot_y="$3"
  local png="$build_tmp/$name.png"
  local config="$build_tmp/$name.cursor"

  [[ -r "$source_dir/$name.xpm" ]] || die "cursor source is missing: $name.xpm"
  "$image_command" "$source_dir/$name.xpm" -strip "PNG32:$png"
  printf '16 %s %s %s.png\n' "$hotspot_x" "$hotspot_y" "$name" >"$config"
  "$xcursorgen_command" -p "$build_tmp" "$config" "$staged_cursor_dir/$name"
}

copy_aliases() {
  local target="$1"
  shift
  local alias_name=""

  for alias_name in "$@"; do
    cp -- "$staged_cursor_dir/$target" "$staged_cursor_dir/$alias_name"
  done
}

build_cursor arrow 1 1
build_cursor ibeam 7 4
build_cursor crosshair 5 5
build_cursor plus 8 8
build_cursor wristwatch 8 8

copy_aliases arrow default left_ptr top_left_arrow
copy_aliases ibeam text xterm
copy_aliases crosshair cross cross_reverse diamond_cross tcross
copy_aliases plus cell
copy_aliases wristwatch wait watch progress left_ptr_watch

install -d -m 0755 -- "$output_dir"
# Replace only the cursor payloads built above. An explicit output directory
# may contain other files; never empty it as part of a rebuild.
for cursor_file in "$staged_cursor_dir"/*; do
  destination="$output_dir/${cursor_file##*/}"
  [[ ! -L "$destination" && ( ! -e "$destination" || -f "$destination" ) ]] ||
    die "cursor destination must be a regular file: $destination"
done
for cursor_file in "$staged_cursor_dir"/*; do
  install -m 0644 -- "$cursor_file" "$output_dir/${cursor_file##*/}"
done

printf 'Built Quartz System 7 cursors in %s\n' "$output_dir"
