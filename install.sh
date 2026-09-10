#!/usr/bin/env bash
# The shared installer also runs the entire Quartz Extras installer.
set -Eeuo pipefail
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
if [[ "$(uname -s)" != Linux ]]; then
  printf 'Quartz requires Ubuntu MATE 24.04. Run this installer in your MATE desktop session.\n' >&2
  exit 1
fi
exec bash "$source_dir/apply-ubuntu-mate-theme.sh" "$@"
