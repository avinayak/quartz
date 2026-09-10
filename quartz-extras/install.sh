#!/usr/bin/env bash

# Download and install optional Quartz desktop resources for the current MATE user.

set -Eeuo pipefail
IFS=$'\n\t'

readonly collection_id="ycaoGBS5pZ8"
readonly collection_name="System7"
readonly collection_url="https://unsplash.com/collections/$collection_id/system7"
readonly collection_api="https://unsplash.com/napi/collections/$collection_id/photos"
readonly data_home="${XDG_DATA_HOME:-$HOME/.local/share}"
readonly background_directory="$data_home/backgrounds/quartz-extras"
readonly wallpaper_destination="$background_directory/system7"
readonly catalog_directory="$data_home/mate-background-properties"
readonly catalog_destination="$catalog_directory/quartz-extras.xml"

download_tmp_dir=""
catalog_tmp=""

cleanup() {
  if [[ -n "$download_tmp_dir" && -d "$download_tmp_dir" ]]; then
    rm -rf -- "$download_tmp_dir"
  fi
  if [[ -n "$catalog_tmp" && -f "$catalog_tmp" ]]; then
    unlink -- "$catalog_tmp"
  fi
}
trap cleanup EXIT

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

usage() {
  cat <<EOF
Usage: install.sh

Download the current $collection_name Unsplash collection and register it as
optional wallpapers for the current MATE user. The active background is not
changed. Rerun this script after changing the collection to refresh the pack.

Collection: $collection_url
EOF
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || die "required command is missing: $1"
}

xml_escape() {
  sed \
    -e 's/&/\&amp;/g' \
    -e 's/</\&lt;/g' \
    -e 's/>/\&gt;/g' \
    -e 's/"/\&quot;/g' \
    -e "s/'/\&apos;/g"
}

if (( $# > 1 )); then
  usage >&2
  die "this installer does not accept positional arguments"
fi

case "${1:-}" in
  "") ;;
  -h|--help)
    usage
    exit 0
    ;;
  *)
    usage >&2
    die "unknown argument: $1"
    ;;
esac

if (( EUID == 0 )); then
  die "run this installer as the MATE desktop user, without sudo"
fi

require_command uname
[[ "$(uname -s)" == "Linux" ]] ||
  die "wallpaper registration is supported only in the Linux MATE desktop"
require_command mate-appearance-properties

[[ "$data_home" == /* ]] || die "XDG_DATA_HOME must be an absolute path"
[[ "$data_home" != *$'\n'* ]] || die "XDG_DATA_HOME must not contain a newline"

for required_command in cat chmod cmp curl file grep install jq mktemp mv rm sed unlink; do
  require_command "$required_command"
done

[[ ! -L "$background_directory" ]] ||
  die "background directory must not be a symbolic link: $background_directory"
[[ ! -L "$wallpaper_destination" ]] ||
  die "wallpaper destination must not be a symbolic link: $wallpaper_destination"
[[ ! -L "$catalog_directory" ]] ||
  die "catalog directory must not be a symbolic link: $catalog_directory"
[[ ! -L "$catalog_destination" ]] ||
  die "catalog destination must not be a symbolic link: $catalog_destination"

install -d -m 0755 -- \
  "$background_directory" "$wallpaper_destination" "$catalog_directory"
download_tmp_dir="$(mktemp -d "$background_directory/.system7-download.XXXXXX")"
readonly metadata_file="$download_tmp_dir/photos.jsonl"
readonly catalog_body="$download_tmp_dir/catalog-body.xml"
readonly current_wallpaper_list="$download_tmp_dir/current-wallpapers.txt"
: > "$metadata_file"
: > "$catalog_body"
: > "$current_wallpaper_list"

page=1
per_page=30
while :; do
  page_file="$download_tmp_dir/page-$page.json"
  printf 'Reading %s collection page %d...\n' "$collection_name" "$page"
  curl --fail --silent --show-error --location \
    --retry 3 --retry-all-errors --connect-timeout 15 --max-time 60 \
    --output "$page_file" \
    "$collection_api?page=$page&per_page=$per_page&order_by=latest" ||
    die "could not read the Unsplash collection"

  jq -e 'type == "array"' "$page_file" >/dev/null ||
    die "Unsplash returned an unexpected collection response"
  page_count="$(jq 'length' "$page_file")"
  jq -c '.[]' "$page_file" >> "$metadata_file"
  (( page_count == per_page )) || break
  (( page += 1 ))
done

wallpaper_count="$(jq -s 'length' "$metadata_file")"
(( wallpaper_count > 0 )) || die "the Unsplash collection is empty"

while IFS= read -r photo; do
  photo_id="$(jq -r '.id // empty' <<< "$photo")"
  [[ "$photo_id" =~ ^[A-Za-z0-9_-]+$ ]] ||
    die "Unsplash returned an unsafe photo ID"

  image_url="$(jq -r '.urls.raw // empty' <<< "$photo")"
  [[ "$image_url" == https://images.unsplash.com/* ]] ||
    die "Unsplash returned an unexpected image URL for $photo_id"

  artist="$(jq -r '(.user.name // "Unknown photographer") | gsub("[\u0000-\u001f]"; " ")' <<< "$photo")"
  title="$(jq -r '(.alt_description // .description // ("Photo " + .id)) | gsub("[\u0000-\u001f]"; " ")' <<< "$photo")"
  color="$(jq -r '.color // "#000000"' <<< "$photo")"
  [[ "$color" =~ ^#[[:xdigit:]]{6}$ ]] || color="#000000"

  wallpaper_name="system7-$photo_id.jpg"
  wallpaper_tmp="$download_tmp_dir/$wallpaper_name"
  printf '%s\n' "$wallpaper_name" >> "$current_wallpaper_list"

  printf 'Downloading %s by %s...\n' "$photo_id" "$artist"
  curl --fail --silent --show-error --location \
    --retry 3 --retry-all-errors --connect-timeout 15 --max-time 300 \
    --output "$wallpaper_tmp" \
    "${image_url}&fm=jpg&fit=max&w=3840&q=85" ||
    die "could not download Unsplash photo $photo_id"
  [[ "$(file --brief --mime-type -- "$wallpaper_tmp")" == "image/jpeg" ]] ||
    die "Unsplash photo $photo_id was not downloaded as a JPEG"
  chmod 0644 "$wallpaper_tmp"

  escaped_title="$(printf '%s' "$title" | xml_escape)"
  escaped_artist="$(printf '%s' "$artist" | xml_escape)"
  escaped_filename="$(printf '%s/%s' "$wallpaper_destination" "$wallpaper_name" | xml_escape)"
  cat >> "$catalog_body" <<EOF
  <wallpaper deleted="false">
    <name>$collection_name — $escaped_title</name>
    <filename>$escaped_filename</filename>
    <options>zoom</options>
    <shade_type>solid</shade_type>
    <pcolor>$color</pcolor>
    <scolor>$color</scolor>
    <artist>$escaped_artist</artist>
  </wallpaper>
EOF
done < "$metadata_file"

# All downloads succeeded, so replace the installed copies and then discard
# images that are no longer part of the collection.
for wallpaper_tmp in "$download_tmp_dir"/system7-*.jpg; do
  wallpaper_name="${wallpaper_tmp##*/}"
  [[ ! -L "$wallpaper_destination/$wallpaper_name" ]] ||
    die "wallpaper destination must not be a symbolic link: $wallpaper_name"
  mv -- "$wallpaper_tmp" "$wallpaper_destination/$wallpaper_name"
done

shopt -s nullglob
for installed_wallpaper in "$wallpaper_destination"/system7-*.jpg; do
  wallpaper_name="${installed_wallpaper##*/}"
  if ! grep -Fxq -- "$wallpaper_name" "$current_wallpaper_list"; then
    unlink -- "$installed_wallpaper"
  fi
done
shopt -u nullglob

for obsolete_metadata in SHA256SUMS SOURCE.md; do
  if [[ -e "$wallpaper_destination/$obsolete_metadata" ]]; then
    [[ ! -L "$wallpaper_destination/$obsolete_metadata" ]] ||
      die "wallpaper metadata must not be a symbolic link: $obsolete_metadata"
    unlink -- "$wallpaper_destination/$obsolete_metadata"
  fi
done

catalog_tmp="$(mktemp "$catalog_directory/.quartz-extras.xml.XXXXXX")"
{
  printf '%s\n' '<?xml version="1.0" encoding="UTF-8"?>'
  printf '%s\n' '<!DOCTYPE wallpapers SYSTEM "mate-wp-list.dtd">'
  printf '%s\n' '<wallpapers>'
  cat "$catalog_body"
  printf '%s\n' '</wallpapers>'
} > "$catalog_tmp"
chmod 0644 "$catalog_tmp"

if [[ -f "$catalog_destination" ]] && cmp -s -- "$catalog_tmp" "$catalog_destination"; then
  unlink -- "$catalog_tmp"
  catalog_tmp=""
else
  mv -- "$catalog_tmp" "$catalog_destination"
  catalog_tmp=""
fi

printf 'Installed %d Quartz Extras wallpapers in %s\n' \
  "$wallpaper_count" "$wallpaper_destination"
printf 'Choose one in Control Center > Appearance > Background.\n'
printf 'If Appearance was already open, close and reopen it once.\n'

# Preserve an explicit desktop-logo choice when refreshing the shared theme.
python3 "$(dirname -- "${BASH_SOURCE[0]}")/logo/install.py" --refresh-if-enabled
