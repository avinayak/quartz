#!/usr/bin/env bash

# Apply the Quartz/System 6 theming layers to an Ubuntu MATE desktop.
# This script is intentionally user-scoped and safe to run more than once.

set -Eeuo pipefail
IFS=$'\n\t'

readonly script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
readonly font_source="$script_dir/fonts/ChiKareGo2.ttf"
readonly expected_font_sha256="7e31d4396bd058fd4c50d05b49d3a63666e84677e59d2b8a9a3b7f3ec3f0ae84"
readonly font_family="ChiKareGo2"
readonly font_native_size_pt="12"
readonly font_native_size_px="16"
readonly font_native_size_pt_min="11.99"
readonly font_native_size_pt_max="12.01"
readonly font_native_size_px_min="15.99"
readonly font_native_size_px_max="16.01"
readonly panel_font_size_px="$font_native_size_px"
readonly menubar_height_px="25"
readonly menubar_interior_height_px="23"
readonly panel_height_px="$menubar_height_px"
readonly titlebar_font_size_pt="$font_native_size_pt"
readonly geneva_family="Geneva"
readonly geneva_default_size_pt="12"
readonly monospace_font_family="Terminus"
readonly monospace_font_size_pt="12"
readonly monospace_font_native_px="16"
readonly -a geneva_strike_sizes=(9 10 12 14 18 24)
readonly -a geneva_native_pixels=(12 12 15 18 22 28)
readonly -a geneva_expected_sha256=(
  "14fd9acc008deb9d388cde878634f92b573907cc7a51c62f4d948ada212934d4"
  "dc0548e85ef06c8c80684d7f7d5f68fcd798cb58d41299de0371f597030cc8f8"
  "81d86217313a0ab8eb02c7069ed27be6ec9b708b624e6f43934e98e84fdb0b72"
  "0fd1295bef667bca1689620234000f66d89859f72cd697a7f413818b33a9e5eb"
  "caeeda462e745fbbfb77abd5371d4faf1dc3fe477b256c6bbf461aa500f88fa4"
  "c2edf28dafae0f81c622bcf0cb7f4ec7cb5801c821c7d5f88fddbb8a97061c77"
)
readonly icon_theme_name="NineIcons48x"
readonly icon_theme_archive="$script_dir/icons/NineIcons48x.tar.gz"
readonly icon_theme_index_source="$script_dir/icons/NineIcons48x.index.theme"
readonly expected_icon_theme_sha256="ff40560cc92d633d25ba4bf824da0ed2261b1f6c00be3bf8f27e63cb3f941897"
readonly cursor_theme_name="Quartz-System7-cursors"
readonly cursor_theme_size_px="16"
readonly cursor_theme_source="$script_dir/cursors/$cursor_theme_name"
readonly -a cursor_theme_base_files=(arrow ibeam crosshair plus wristwatch)
readonly -a cursor_theme_expected_sha256=(
  "9293ec3b5d572d57f6ec8f7c0cf19976659e98208a166529c0199532a3e8dd99"
  "6734e72352aef5cf6963f447f5f423c1c792eef8b188a7e9bb228fee246e4d9e"
  "4538e761f74fc5e028aa70626c4027678ec2dcb9c22c106964376bc0db22828d"
  "713382b95c46450da8daa81060384ba719d146ddc3f2a307e2402106bcc5c67a"
  "cb7ed5c097c34fd7800d6f26379f054fb9f5ecc0c05e9f3b5f5ca49c0e810566"
)
readonly -a cursor_theme_aliases=(
  "default:arrow"
  "left_ptr:arrow"
  "top_left_arrow:arrow"
  "text:ibeam"
  "xterm:ibeam"
  "cross:crosshair"
  "cross_reverse:crosshair"
  "diamond_cross:crosshair"
  "tcross:crosshair"
  "cell:plus"
  "wait:wristwatch"
  "watch:wristwatch"
  "progress:wristwatch"
  "left_ptr_watch:wristwatch"
)
readonly marco_theme_name="Quartz-System6"
readonly theme_template_dir="$script_dir/theme/$marco_theme_name"
readonly theme_renderer="$theme_template_dir/render-theme.sh"
readonly -a obsolete_theme_relative_files=(
  "gtk-2.0/assets/box-black-rounded-2px.xpm"
  "gtk-2.0/assets/box-white-rounded-2px.xpm"
  "gtk-2.0/assets/frame-rounded-2px.xpm"
  "gtk-2.0/assets/button-black-pressed.xpm"
  "gtk-2.0/assets/combo-menu-shadow.xpm"
)
readonly settings_source_dir="$script_dir/quartz-settings"

readonly data_home="${XDG_DATA_HOME:-$HOME/.local/share}"
readonly config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
readonly quartz_config_dir="$config_home/quartz-settings"
readonly quartz_theme_config="$quartz_config_dir/theme.conf"
readonly icon_dir="$data_home/icons"
readonly icon_theme_destination="$icon_dir/$icon_theme_name"
readonly cursor_dir="$HOME/.icons"
readonly cursor_theme_destination="$cursor_dir/$cursor_theme_name"
readonly font_dir="$data_home/fonts/quartz-system7"
readonly font_destination="$font_dir/ChiKareGo2.ttf"
readonly fontconfig_dir="$config_home/fontconfig/conf.d"
readonly fontconfig_file="$fontconfig_dir/99-quartz-chikarego2.conf"
readonly geneva_fontconfig_file="$fontconfig_dir/99-quartz-geneva.conf"
readonly gtk3_dir="$config_home/gtk-3.0"
readonly gtk3_css="$gtk3_dir/gtk.css"
readonly css_begin='/* quartz-system7-fonts:begin */'
readonly css_end='/* quartz-system7-fonts:end */'
readonly gtk4_dir="$config_home/gtk-4.0"
readonly gtk4_css="$gtk4_dir/gtk.css"
readonly gtk4_css_begin='/* quartz-system7-gtk4:begin */'
readonly gtk4_css_end='/* quartz-system7-gtk4:end */'
readonly gtk2_rc="$HOME/.gtkrc-2.0"
readonly gtk2_rc_begin='# quartz-system7-gtk2:begin'
readonly gtk2_rc_end='# quartz-system7-gtk2:end'
readonly xsession_rc="$HOME/.xsessionrc"
readonly xsession_begin='# quartz-compositor:begin'
readonly xsession_end='# quartz-compositor:end'
readonly theme_destination="$HOME/.themes/$marco_theme_name"
readonly terminal_profile='org.mate.terminal.profile:/org/mate/terminal/profiles/default/'
readonly settings_prefix="$HOME/.local"
readonly settings_launcher="$settings_prefix/bin/quartz-settings"
readonly settings_libexec="$settings_prefix/libexec/quartz-settings"
readonly settings_desktop="$settings_prefix/share/applications/org.quartz.Settings.desktop"
readonly settings_apply_helper="$settings_libexec/quartz-theme-apply"
readonly settings_panel_helper="$settings_libexec/quartz-panel-theme"
readonly settings_panel_service="quartz-panel-theme.service"
readonly settings_panel_service_file="$settings_prefix/share/systemd/user/$settings_panel_service"
readonly settings_theme_data="$settings_prefix/share/quartz-settings/theme/$marco_theme_name"

fontconfig_tmp=""
gtk3_tmp=""
gtk4_tmp=""
gtk2_tmp=""
xsession_tmp=""
icon_theme_tmp=""
theme_build_tmp=""
gtk3_user_css_changed=false
gtk4_user_css_changed=false
gtk_theme_setting_refreshed=false
marco_restart_required=false

cleanup() {
  if [[ -n "$xsession_tmp" && -f "$xsession_tmp" ]]; then
    unlink -- "$xsession_tmp"
  fi

  if [[ -n "$icon_theme_tmp" && -d "$icon_theme_tmp" && ! -L "$icon_theme_tmp" ]]; then
    find "$icon_theme_tmp" -depth -delete || true
  fi

  if [[ -n "$fontconfig_tmp" && -f "$fontconfig_tmp" ]]; then
    unlink -- "$fontconfig_tmp"
  fi

  if [[ -n "$gtk3_tmp" && -f "$gtk3_tmp" ]]; then
    unlink -- "$gtk3_tmp"
  fi

  if [[ -n "$gtk4_tmp" && -f "$gtk4_tmp" ]]; then
    unlink -- "$gtk4_tmp"
  fi

  if [[ -n "$gtk2_tmp" && -f "$gtk2_tmp" ]]; then
    unlink -- "$gtk2_tmp"
  fi

  if [[ -n "$theme_build_tmp" && -d "$theme_build_tmp" && ! -L "$theme_build_tmp" ]]; then
    find "$theme_build_tmp" -depth -delete || true
  fi
}
trap cleanup EXIT

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

shade_hex_color() {
  local source_color="$1"
  local scale="$2"
  local color_digits="${source_color#\#}"
  local red="$((16#${color_digits:0:2}))"
  local green="$((16#${color_digits:2:2}))"
  local blue="$((16#${color_digits:4:2}))"

  printf '#%02x%02x%02x' \
    "$(((red * scale + 127) / 255))" \
    "$(((green * scale + 127) / 255))" \
    "$(((blue * scale + 127) / 255))"
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || die "required command is missing: $1"
}

install_extras_if_present() {
  local extras_source="$1"

  if [[ ! -e "$extras_source" && ! -L "$extras_source" ]]; then
    printf 'Quartz Extras installation skipped: folder missing (%s).\n' "$extras_source"
    return 0
  fi
  [[ -d "$extras_source" ]] || die "extras source is not a directory: $extras_source"
  [[ -f "$extras_source/install.sh" && -r "$extras_source/install.sh" ]] ||
    die "extras installer is missing or unreadable: $extras_source/install.sh"
  # Run the complete installer, including any extras added in the future.
  bash "$extras_source/install.sh"
}

validate_managed_block() {
  local config_file="$1"
  local begin_marker="$2"
  local end_marker="$3"

  [[ -e "$config_file" ]] || return 0
  [[ -f "$config_file" && -r "$config_file" ]] ||
    die "configuration is not a readable regular file: $config_file"
  awk -v begin="$begin_marker" -v end="$end_marker" '
    $0 == begin { if (managed || seen++) exit 1; managed = 1; next }
    $0 == end { if (!managed) exit 1; managed = 0 }
    END { if (managed) exit 1 }
  ' "$config_file" ||
    die "cannot safely update malformed managed block in $config_file"
}

schema_exists() {
  # Do not use grep -q here: with pipefail it can close the pipe early and make
  # gsettings' SIGPIPE look like a missing schema.
  gsettings list-schemas | grep -Fx -- "$1" >/dev/null
}

setting_is_writable() {
  [[ "$(gsettings writable "$1" "$2")" == "true" ]]
}

find_fallback_theme() {
  local component="$1"
  local root=""
  local candidate=""
  local candidate_name=""

  for root in "$HOME/.themes" "$data_home/themes" /usr/share/themes; do
    [[ -d "$root" ]] || continue
    for candidate in "$root"/*/"$component"; do
      [[ -d "$candidate" ]] || continue
      candidate_name="${candidate%/"$component"}"
      candidate_name="${candidate_name##*/}"
      if [[ -n "$candidate_name" && "$candidate_name" != "$marco_theme_name" ]]; then
        printf '%s\n' "$candidate_name"
        return
      fi
    done
  done
}

select_shared_theme() {
  local schema="$1"
  local key="$2"
  local component="$3"
  local current_theme=""
  local fallback_theme=""

  current_theme="$(gsettings get "$schema" "$key")"
  if [[ "$current_theme" == "'$marco_theme_name'" ]]; then
    fallback_theme="$(find_fallback_theme "$component" || true)"
    if [[ -n "$fallback_theme" ]]; then
      # Re-selecting an unchanged GSettings value emits no notification. Move
      # through another installed shared theme so live GTK processes discard
      # their cached copy after an in-place Quartz update.
      gsettings set "$schema" "$key" "$fallback_theme"
      # Give the desktop event loop one turn before restoring the original
      # name; otherwise GSettings can coalesce both writes into no visible
      # theme-name transition for long-running consumers.
      sleep 0.1
      gtk_theme_setting_refreshed=true
    fi
  fi
  gsettings set "$schema" "$key" "$marco_theme_name"
}

replace_file_if_changed() {
  local source_file="$1"
  local destination_file="$2"

  if [[ -f "$destination_file" ]] && cmp -s -- "$source_file" "$destination_file"; then
    unlink -- "$source_file"
    return
  fi

  chmod 0644 -- "$source_file"
  mv -- "$source_file" "$destination_file"
}

prepare_live_display() {
  local display_name="${DISPLAY:-}"
  local session_id=""

  if [[ -z "$display_name" ]] && command -v loginctl >/dev/null 2>&1; then
    session_id="$(loginctl show-user "$(id -u)" -p Display --value 2>/dev/null || true)"
    if [[ -n "$session_id" ]]; then
      display_name="$(loginctl show-session "$session_id" -p Display --value 2>/dev/null || true)"
    fi
  fi

  [[ -n "$display_name" ]] || return 1
  export DISPLAY="$display_name"

  if [[ -z "${XAUTHORITY:-}" && -f "$HOME/.Xauthority" ]]; then
    export XAUTHORITY="$HOME/.Xauthority"
  fi

  return 0
}

marco_shadows_disabled() {
  local marco_pid=""
  local found=false

  while IFS= read -r marco_pid; do
    [[ -n "$marco_pid" ]] || continue
    found=true
    [[ -r "/proc/$marco_pid/environ" ]] || return 1
    tr '\0' '\n' < "/proc/$marco_pid/environ" |
      grep -Fx 'META_DEBUG_NO_SHADOW=1' >/dev/null || return 1
  done < <(pgrep -u "$(id -u)" -x marco || true)

  [[ "$found" == true ]]
}

restart_marco_if_running() {
  local candidate_pid=""
  local new_marco_pid=""
  local old_marco_pid=""
  local old_marco_pids=""

  old_marco_pids="$(pgrep -x marco 2>/dev/null || true)"
  [[ -n "$old_marco_pids" ]] || return 1
  prepare_live_display || return 1

  if command -v setsid >/dev/null 2>&1; then
    setsid -f marco --replace >/dev/null 2>&1
  else
    nohup marco --replace >/dev/null 2>&1 &
  fi

  # A replacement that cannot acquire the X11 window-manager selection exits.
  # Only retire the old process after a new Marco has stayed alive long enough
  # to demonstrate that it acquired the selection.
  for _attempt in {1..50}; do
    while IFS= read -r candidate_pid; do
      if [[ -n "$candidate_pid" ]] &&
        ! grep -Fx -- "$candidate_pid" <<<"$old_marco_pids" >/dev/null; then
        new_marco_pid="$candidate_pid"
      fi
    done < <(pgrep -x marco 2>/dev/null || true)

    if [[ -n "$new_marco_pid" ]]; then
      sleep 0.5
      if kill -0 "$new_marco_pid" 2>/dev/null; then
        while IFS= read -r old_marco_pid; do
          if [[ -n "$old_marco_pid" ]] && kill -0 "$old_marco_pid" 2>/dev/null; then
            kill "$old_marco_pid"
          fi
        done <<<"$old_marco_pids"
        return 0
      fi
      new_marco_pid=""
    fi
    sleep 0.1
  done

  return 1
}

reload_marco_theme_if_running() {
  pgrep -x marco >/dev/null 2>&1 || return 1
  prepare_live_display || return 1
  marco-message reload-theme >/dev/null 2>&1
}

if (( EUID == 0 )); then
  die "run this script as the MATE desktop user, without sudo"
fi

for required in \
  awk bash cc cmp cp dconf fc-cache fc-match fc-pattern fc-scan find flock gsettings grep \
  gtk-update-icon-cache install ldd make marco-message mktemp mv pgrep pkg-config sed \
  sha256sum systemctl tar tr; do
  require_command "$required"
done

validate_managed_block "$gtk3_css" "$css_begin" "$css_end"
validate_managed_block "$gtk4_css" "$gtk4_css_begin" "$gtk4_css_end"
validate_managed_block "$gtk2_rc" "$gtk2_rc_begin" "$gtk2_rc_end"
validate_managed_block "$xsession_rc" "$xsession_begin" "$xsession_end"

[[ -r "$font_source" ]] || die "font not found: $font_source"
[[ -r "$icon_theme_archive" ]] || die "icon theme archive not found: $icon_theme_archive"
[[ -r "$icon_theme_index_source" ]] || die "icon theme index not found: $icon_theme_index_source"
for icon_slot in 22 24; do
  [[ -r "$script_dir/icons/overrides/places/$icon_slot/user-desktop.svg" ]] ||
    die "missing $icon_slot pixel desktop icon override"
done
[[ -d "$cursor_theme_source" ]] || die "cursor theme not found: $cursor_theme_source"
[[ -r "$cursor_theme_source/index.theme" ]] ||
  die "cursor theme index not found: $cursor_theme_source/index.theme"
[[ -r "$theme_renderer" ]] || die "theme renderer not found: $theme_renderer"

theme_build_tmp="$(mktemp -d "${TMPDIR:-/tmp}/quartz-theme.XXXXXX")"
readonly theme_source_dir="$theme_build_tmp/$marco_theme_name"
if [[ -r "$quartz_theme_config" ]]; then
  bash "$theme_renderer" --config "$quartz_theme_config" "$theme_source_dir"
else
  bash "$theme_renderer" "$theme_source_dir"
fi
source "$theme_source_dir/theme.conf"
readonly border_radius_px="$QUARTZ_BORDER_RADIUS_PX"
readonly window_layout="$QUARTZ_WINDOW_LAYOUT"
readonly menubar_bg="$QUARTZ_MENUBAR_BG"
readonly titlebar_bg="$QUARTZ_TITLEBAR_BG"
readonly application_bg="$QUARTZ_APPLICATION_BG"
window_button_layout=""
window_layout_label=""
case "$window_layout" in
  cupertino)
    window_button_layout="close:maximize"
    window_layout_label="Cupertino"
    ;;
  redmond)
    window_button_layout=":minimize,maximize,close"
    window_layout_label="Redmond"
    ;;
  redmond-reversed)
    window_button_layout="close,minimize,maximize:"
    window_layout_label="Redmond reversed"
    ;;
  *) die "rendered theme has an invalid window layout: $window_layout" ;;
esac
readonly window_button_layout window_layout_label
unset QUARTZ_BORDER_RADIUS_PX QUARTZ_WINDOW_LAYOUT QUARTZ_MENUBAR_BG
unset QUARTZ_TITLEBAR_BG QUARTZ_APPLICATION_BG
readonly application_shade="$(shade_hex_color "$application_bg" 235)"
readonly application_disabled_bg="$(shade_hex_color "$application_bg" 217)"
readonly application_disabled_fg="$(shade_hex_color "$application_bg" 119)"
readonly border_radius_css="${border_radius_px}px"
readonly border_slice_px="$((border_radius_px > 0 ? border_radius_px : 1))"
readonly border_shadow_slice_px="$((border_slice_px + 1))"
readonly frame_edge_inset_px="$((2 * (border_radius_px > 0 ? border_radius_px + 1 : 2)))"
readonly frame_edge_inset_css="${frame_edge_inset_px}px"
readonly rounded_asset_dimension_px="$((2 * border_slice_px + 1))"
readonly rounded_button_asset_height_px="$((rounded_asset_dimension_px + 1))"
readonly notebook_tab_depth_px="$((border_slice_px + 1))"
readonly marco_theme_source="$theme_source_dir/metacity-1/metacity-theme-1.xml"
readonly gtk2_theme_source="$theme_source_dir/gtk-2.0/gtkrc"
readonly gtk3_theme_source="$theme_source_dir/gtk-3.0/gtk.css"
readonly gtk4_theme_source="$theme_source_dir/gtk-4.0/gtk.css"

[[ -r "$marco_theme_source" ]] || die "Marco theme not found: $marco_theme_source"
[[ -r "$gtk2_theme_source" ]] || die "GTK 2 theme not found: $gtk2_theme_source"
[[ -r "$gtk3_theme_source" ]] || die "GTK 3 theme not found: $gtk3_theme_source"
[[ -r "$gtk4_theme_source" ]] || die "GTK 4 theme not found: $gtk4_theme_source"
[[ -r "$theme_source_dir/index.theme" ]] || die "theme index not found: $theme_source_dir/index.theme"
grep -Fqx "CursorTheme=$cursor_theme_name" "$theme_source_dir/index.theme" ||
  die "theme index does not select the $cursor_theme_name cursor theme"
for settings_source in \
  Makefile \
  data/org.quartz.Settings.desktop.in \
  data/quartz-panel-theme.in \
  data/quartz-panel-theme.service.in \
  data/quartz-theme-apply.in \
  src/quartz-settings.c \
  src/quartz-gtk2-audit.c \
  src/quartz-gtk3-audit.c \
  src/quartz-gtk4-audit.c; do
  [[ -r "$settings_source_dir/$settings_source" ]] ||
    die "Quartz Settings source not found: $settings_source"
done
for gtk_package in gtk+-2.0 gtk+-3.0 gtk4; do
  pkg-config --exists "$gtk_package" ||
    die "Quartz Settings build dependency is missing: $gtk_package"
done
pkg-config --atleast-version=4.14 gtk4 ||
  die "Quartz Settings requires GTK 4.14 or newer"

for indicator_asset in \
  check-off-disabled check-off-inverse check-off check-on-disabled \
  check-on-inverse check-on radio-off-disabled radio-off-inverse radio-off \
  radio-on-disabled radio-on-inverse radio-on; do
  [[ -r "$theme_source_dir/gtk-2.0/assets/$indicator_asset.xpm" ]] ||
    die "native GTK indicator asset is missing: $indicator_asset.xpm"
done
[[ -r "$theme_source_dir/gtk-3.0/assets/menu-frame.xpm" ]] ||
  die "native GTK 3 menu frame asset is missing"

for frame_asset in \
  frame-1px box-white box-black \
  frame-rounded box-white-rounded box-black-rounded \
  button-white-raised button-black-raised button-disabled-pressed \
  button-shade-raised button-shade-pressed \
  frame-gap-cap notebook-gap-horizontal notebook-gap-vertical \
  notebook-tab-top notebook-tab-bottom notebook-tab-left notebook-tab-right \
  menu-shadow menubar-app; do
  [[ -r "$theme_source_dir/gtk-2.0/assets/$frame_asset.xpm" ]] ||
    die "native GTK 2 frame asset is missing: $frame_asset.xpm"
done
grep -Fqx \
  "\"$rounded_asset_dimension_px $rounded_button_asset_height_px 3 1\"," \
  "$theme_source_dir/gtk-2.0/assets/menu-shadow.xpm" ||
  die "native GTK 2 rounded menu asset has the wrong dimensions"
grep -Fqx "\"+ c $application_bg\"," \
  "$theme_source_dir/gtk-2.0/assets/menu-shadow.xpm" ||
  die "native GTK 2 rounded menu asset does not use the application face"
grep -Fq 'file = "assets/menu-shadow.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not use the native menu shadow bitmap"
grep -Fqx '"1 1 1 1",' "$theme_source_dir/gtk-2.0/assets/frame-gap-cap.xpm" ||
  die "native GTK 2 frame gap cap bitmap is not 1x1"
grep -Fqx '"3 1 2 1",' "$theme_source_dir/gtk-2.0/assets/notebook-gap-horizontal.xpm" ||
  die "native GTK 2 horizontal notebook gap bitmap is not 3x1"
grep -Fqx '"1 3 2 1",' "$theme_source_dir/gtk-2.0/assets/notebook-gap-vertical.xpm" ||
  die "native GTK 2 vertical notebook gap bitmap is not 1x3"
for rounded_asset in frame-rounded box-black-rounded; do
  grep -Fqx \
    "\"$rounded_asset_dimension_px $rounded_asset_dimension_px 2 1\"," \
    "$theme_source_dir/gtk-2.0/assets/$rounded_asset.xpm" ||
    die "native GTK 2 rounded asset has the wrong dimensions: $rounded_asset.xpm"
done
grep -Fqx \
  "\"$rounded_asset_dimension_px $rounded_asset_dimension_px 3 1\"," \
  "$theme_source_dir/gtk-2.0/assets/box-white-rounded.xpm" ||
  die "native GTK 2 rounded asset has the wrong dimensions: box-white-rounded.xpm"
grep -Fqx \
  "\"$rounded_asset_dimension_px $rounded_button_asset_height_px 2 1\"," \
  "$theme_source_dir/gtk-2.0/assets/button-black-raised.xpm" ||
  die "native GTK 2 button asset has the wrong dimensions: button-black-raised.xpm"
for rounded_button_asset in \
  button-white-raised button-disabled-pressed \
  button-shade-raised button-shade-pressed; do
  grep -Fqx \
    "\"$rounded_asset_dimension_px $rounded_button_asset_height_px 3 1\"," \
    "$theme_source_dir/gtk-2.0/assets/$rounded_button_asset.xpm" ||
    die "native GTK 2 button asset has the wrong dimensions: $rounded_button_asset.xpm"
done
for shaded_button_asset in button-shade-raised button-shade-pressed; do
  grep -Fqx \
    "\"+ c $application_shade\"," \
    "$theme_source_dir/gtk-2.0/assets/$shaded_button_asset.xpm" ||
    die "native GTK 2 shaded button does not derive from the application color: $shaded_button_asset.xpm"
done
grep -Fqx "\". c $application_disabled_fg\"," \
  "$theme_source_dir/gtk-2.0/assets/button-disabled-pressed.xpm" ||
  die "native GTK 2 disabled button outline does not derive from the application color"
grep -Fqx "\"+ c $application_disabled_bg\"," \
  "$theme_source_dir/gtk-2.0/assets/button-disabled-pressed.xpm" ||
  die "native GTK 2 disabled button face does not derive from the application color"
for notebook_tab_side in top bottom; do
  grep -Fqx \
    "\"$rounded_asset_dimension_px $notebook_tab_depth_px 3 1\"," \
    "$theme_source_dir/gtk-2.0/assets/notebook-tab-$notebook_tab_side.xpm" ||
    die "native GTK 2 notebook tab has the wrong dimensions: notebook-tab-$notebook_tab_side.xpm"
done
for notebook_tab_side in left right; do
  grep -Fqx \
    "\"$notebook_tab_depth_px $rounded_asset_dimension_px 3 1\"," \
    "$theme_source_dir/gtk-2.0/assets/notebook-tab-$notebook_tab_side.xpm" ||
    die "native GTK 2 notebook tab has the wrong dimensions: notebook-tab-$notebook_tab_side.xpm"
done
grep -Fqx "  GtkWidget::tooltip-radius = $border_radius_px" "$gtk2_theme_source" ||
  die "GTK 2 tooltip does not use the shared ${border_radius_css} corner radius"
grep -Fq 'tooltip_bg_color:#ffecb3\ntooltip_fg_color:#000000' \
  "$gtk2_theme_source" ||
  die "GTK 2 tooltip palette is not amber with black text"
awk '
  $0 == "style \"quartz-tooltip\" = \"quartz-default\" {" {
    in_tooltip = 1
  }
  in_tooltip && $0 == "  fg[NORMAL] = \"#000000\"" { black_text = 1 }
  in_tooltip && $0 == "  bg[NORMAL] = \"#ffecb3\"" { amber_face = 1 }
  in_tooltip && $0 == "  bg[SELECTED] = \"#000000\"" {
    black_outline = 1
  }
  in_tooltip && $0 == "}" { in_tooltip = 0 }
  END { exit(black_text && amber_face && black_outline ? 0 : 1) }
' "$gtk2_theme_source" ||
  die "GTK 2 tooltip is not amber with black text and outline"
grep -Fqx '  GtkWidget::focus-line-pattern = "\1\1"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not use the one-pixel dotted focus pattern"
grep -Fqx '  GtkWidget::focus-line-width = 1' "$gtk2_theme_source" ||
  die "GTK 2 theme does not enable the shared focus line"
grep -Fqx '  GtkButton::focus-line-width = 1' "$gtk2_theme_source" ||
  die "GTK 2 theme does not enable the button focus line"
grep -Fqx '  GtkWidget::interior-focus = 1' "$gtk2_theme_source" ||
  die "GTK 2 theme does not keep focus inside controls"
grep -Fq 'file = "assets/frame-rounded.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 entries and default buttons do not use the rounded frame"
grep -Fq 'gap_start_file = "assets/frame-gap-cap.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 labelled frames do not preserve the leading top border"
grep -Fq 'gap_end_file = "assets/frame-gap-cap.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 labelled frames do not preserve the trailing top border"
grep -Fqx "  text[PRELIGHT] = \"$application_bg\"" "$gtk2_theme_source" ||
  die "GTK 2 highlighted menu content does not use the application paper color"
awk '
  $0 == "style \"quartz-menu-item\" = \"quartz-default\" {" {
    in_menu_item = 1
  }
  in_menu_item && $0 == "  font_name = \"ChiKareGo2 12\"" {
    menu_item_font = 1
  }
  in_menu_item && $0 == "}" { in_menu_item = 0 }

  $0 == "style \"quartz-menubar-label\" = \"quartz-menu-item\" {" {
    in_menubar_label = 1
  }
  in_menubar_label && $0 == "  font_name = \"ChiKareGo2 12\"" {
    menubar_label_font = 1
  }
  in_menubar_label && $0 == "}" { in_menubar_label = 0 }

  END { exit(menu_item_font && menubar_label_font ? 0 : 1) }
' "$gtk2_theme_source" ||
  die "GTK 2 menubars and popup commands do not use native ChiKareGo2"
grep -Fqx 'widget_class "*<GtkMenuItem>*" style "quartz-menu-item"' \
  "$gtk2_theme_source" ||
  die "GTK 2 generated combo-menu cells do not inherit highlighted menu text"
grep -Fqx 'widget_class "*.<GtkMenu>" style "quartz-menu"' \
  "$gtk2_theme_source" ||
  die "GTK 2 menus do not use the shared rounded menu style"
if grep -Eq 'combo-menu-shadow|quartz-combo-menu|gtk-combobox-popup-menu' \
  "$gtk2_theme_source"; then
  die "GTK 2 theme still contains a combo-specific menu override"
fi
awk \
  -v expected="border = { $border_slice_px, $border_slice_px, $border_slice_px, $border_shadow_slice_px }" '
    $0 == "style \"quartz-menu\" = \"quartz-default\" {" {
      in_menu = 1
      next
    }
    in_menu && /^style / {
      in_menu = 0
    }
    in_menu {
      line = $0
      sub(/^[[:space:]]*/, "", line)
      if (line == expected) {
        found = 1
      }
    }
    END { exit found ? 0 : 1 }
  ' "$gtk2_theme_source" ||
  die "GTK 2 menus do not preserve rounded corners with a bottom-only shadow"
for notebook_tab_side in top bottom left right; do
  grep -Fq "file = \"assets/notebook-tab-$notebook_tab_side.xpm\"" \
    "$gtk2_theme_source" ||
    die "GTK 2 notebooks do not use the rounded $notebook_tab_side selected-tab asset"
done
for notebook_gap_axis in horizontal vertical; do
  grep -Fq "gap_file = \"assets/notebook-gap-$notebook_gap_axis.xpm\"" \
    "$gtk2_theme_source" ||
    die "GTK 2 notebooks do not preserve the $notebook_gap_axis page-frame gap"
done
grep -Fq 'file = "assets/button-white-raised.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 buttons do not use the raised normal-state box"
grep -Fq 'file = "assets/button-shade-pressed.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 buttons do not use the lightly shaded pressed-state box"
grep -Fq 'style "quartz-toggle-button" = "quartz-button"' "$gtk2_theme_source" ||
  die "GTK 2 toggle buttons do not define a persistent raised state"
grep -Fq 'file = "assets/button-shade-raised.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 latched toggles do not use the raised state cue"
grep -Fqx '  GtkButton::child-displacement-y = 0' "$gtk2_theme_source" ||
  die "GTK 2 latched toggle contents remain physically displaced"
grep -Fqx 'widget_class "*.<GtkToggleButton>" style "quartz-toggle-button"' \
  "$gtk2_theme_source" ||
  die "GTK 2 toggle buttons do not use the persistent raised style"
grep -Fq 'file = "assets/button-disabled-pressed.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 buttons do not use the palette-derived disabled-state box"
grep -Fqx '  GtkButton::child-displacement-y = 1' "$gtk2_theme_source" ||
  die "GTK 2 button contents do not move down one pixel when pressed"
grep -Fqx '  fg[ACTIVE] = "#000000"' "$gtk2_theme_source" ||
  die "GTK 2 pressed button text is not dark"
grep -Fq \
  "border = { $border_slice_px, $border_slice_px, $border_slice_px, $border_slice_px }" \
  "$gtk2_theme_source" ||
  die "GTK 2 rounded assets do not use the configured border slice"
grep -Fq \
  "border = { $border_slice_px, $border_slice_px, $border_slice_px, $border_shadow_slice_px }" \
  "$gtk2_theme_source" ||
  die "GTK 2 raised buttons do not reserve their one-pixel shadow"
grep -Fq \
  "border = { $border_slice_px, $border_slice_px, $border_shadow_slice_px, $border_slice_px }" \
  "$gtk2_theme_source" ||
  die "GTK 2 pressed buttons do not move down one pixel"
grep -Fq 'detail = "bar"' "$gtk2_theme_source" ||
  die "GTK 2 progress bars do not define the rounded fill"
grep -Fqx '"1 25 2 1",' "$theme_source_dir/gtk-2.0/assets/menubar-app.xpm" ||
  die "native GTK 2 application menubar bitmap is not 25px high"
grep -Fqx "\"+ c $menubar_bg\"," "$theme_source_dir/gtk-2.0/assets/menubar-app.xpm" ||
  die "native GTK 2 application menubar does not use the configured background"
grep -Fq 'file = "assets/menubar-app.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not use the application menubar bitmap"
grep -Fqx '  GtkMenuBar::internal-padding = 2' "$gtk2_theme_source" ||
  die "GTK 2 native Chicago menubar does not preserve its 25px allocation"
grep -Fqx '"9 9 3 1",' "$theme_source_dir/gtk-2.0/assets/scrollbar-thumb.xpm" ||
  die "native GTK 2 scrollbar thumb bitmap is not 9x9"
grep -Fqx "\"+ c $application_bg\"," "$theme_source_dir/gtk-2.0/assets/scrollbar-thumb.xpm" ||
  die "native GTK 2 scrollbar thumb does not use the application background"
grep -Fqx '"   ...   ",' "$theme_source_dir/gtk-2.0/assets/scrollbar-thumb.xpm" ||
  die "native GTK 2 scrollbar thumb does not define its inset shadow row"
grep -Fq 'file = "assets/scrollbar-thumb.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not use the pixel-cornered scrollbar thumb"
grep -Fq 'border = { 4, 4, 4, 4 }' "$gtk2_theme_source" ||
  die "GTK 2 scrollbar thumb does not preserve its pixel-curved shadow"
for scrollbar_rail_mask in scrollbar-rail-horizontal-mask scrollbar-rail-vertical-mask; do
  [[ -r "$theme_source_dir/gtk-2.0/assets/$scrollbar_rail_mask.xpm" ]] ||
    die "native GTK 2 scrollbar rail mask is missing: $scrollbar_rail_mask.xpm"
  grep -Fq "overlay_file = \"assets/$scrollbar_rail_mask.xpm\"" "$gtk2_theme_source" ||
    die "GTK 2 theme does not use its $scrollbar_rail_mask gutter mask"
done
grep -Fqx '"1 9 2 1",' "$theme_source_dir/gtk-2.0/assets/scrollbar-rail-horizontal-mask.xpm" ||
  die "native GTK 2 horizontal scrollbar rail does not reserve four-pixel gutters"
grep -Fqx '"9 1 2 1",' "$theme_source_dir/gtk-2.0/assets/scrollbar-rail-vertical-mask.xpm" ||
  die "native GTK 2 vertical scrollbar rail does not reserve four-pixel gutters"
grep -Fq 'file = "../gtk-3.0/assets/scrollbar-dither.xpm" stretch = FALSE' "$gtk2_theme_source" ||
  die "GTK 2 scrollbar does not tile the shared dither rail background"
grep -Fq 'file = "../gtk-3.0/assets/scrollbar-dither-disabled.xpm" stretch = FALSE' "$gtk2_theme_source" ||
  die "GTK 2 disabled scrollbar does not use the shared preblended dither"
for scale_asset in \
  scale-fill-horizontal scale-fill-horizontal-disabled \
  scale-fill-vertical scale-fill-vertical-disabled; do
  [[ -r "$theme_source_dir/gtk-2.0/assets/$scale_asset.xpm" ]] ||
    die "native GTK 2 scale asset is missing: $scale_asset.xpm"
done
[[ -r "$theme_source_dir/gtk-3.0/assets/scale-dither.xpm" ]] ||
  die "shared scale dither bitmap is missing: scale-dither.xpm"
[[ -r "$theme_source_dir/gtk-3.0/assets/scale-dither-disabled.xpm" ]] ||
  die "shared disabled scale dither bitmap is missing: scale-dither-disabled.xpm"
grep -Fqx '"7 14 3 1",' "$theme_source_dir/gtk-2.0/assets/scale-fill-horizontal.xpm" ||
  die "native GTK 2 horizontal scale fill is not a 13px body plus shadow"
grep -Fqx '"14 7 3 1",' "$theme_source_dir/gtk-2.0/assets/scale-fill-vertical.xpm" ||
  die "native GTK 2 vertical scale fill is not a 13px body plus shadow"
grep -Fqx '"4 2 2 1",' "$theme_source_dir/gtk-3.0/assets/scale-dither.xpm" ||
  die "shared scale dither bitmap is not 4x2"
grep -Fqx '"+++.",' "$theme_source_dir/gtk-3.0/assets/scale-dither.xpm" ||
  die "shared scale dither bitmap does not preserve its one-bit pattern"
grep -Fqx '"+.++"};' "$theme_source_dir/gtk-3.0/assets/scale-dither.xpm" ||
  die "shared scale dither bitmap does not preserve its staggered row"
grep -Fqx '" ..... ",' "$theme_source_dir/gtk-2.0/assets/scale-fill-horizontal.xpm" ||
  die "native GTK 2 horizontal scale fill does not cut its upper corners"
grep -Fqx '" ..... "};' "$theme_source_dir/gtk-2.0/assets/scale-fill-horizontal.xpm" ||
  die "native GTK 2 horizontal scale fill does not curve its lower shadow"
grep -Fqx "\". c $application_disabled_fg\"," "$theme_source_dir/gtk-2.0/assets/scale-fill-horizontal-disabled.xpm" ||
  die "native GTK 2 disabled scale fill does not derive from the application color"
grep -Fqx "\". c $application_disabled_fg\"," "$theme_source_dir/gtk-2.0/assets/scale-fill-vertical-disabled.xpm" ||
  die "native GTK 2 disabled vertical scale fill does not derive from the application color"
grep -Fqx "\". c $application_disabled_fg\"," "$theme_source_dir/gtk-3.0/assets/scale-dither-disabled.xpm" ||
  die "shared disabled scale dither does not derive from the application color"
grep -Fq 'bg_pixmap[ACTIVE] = "../gtk-3.0/assets/scale-dither.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 scale does not tile the dithered rail background"
grep -Fq 'bg_pixmap[INSENSITIVE] = "../gtk-3.0/assets/scale-dither-disabled.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 disabled scale does not use the palette-derived dither"
grep -Fq 'GtkScale::slider_length = 1' "$gtk2_theme_source" ||
  die "GTK 2 scale does not collapse its transparent puck"
grep -Fq 'GtkScale::slider_width = 14' "$gtk2_theme_source" ||
  die "GTK 2 scale does not reserve its external one-pixel shadow"
grep -Fq 'file = "assets/scale-fill-horizontal.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not use the pixel-shadowed scale foreground"
grep -Fq 'file = "assets/scale-fill-horizontal-disabled.xpm"' "$gtk2_theme_source" ||
  die "GTK 2 disabled scale does not use the palette-derived foreground"
grep -Fq 'image { function = SLIDER detail = "hscale" }' "$gtk2_theme_source" ||
  die "GTK 2 horizontal scale does not suppress its puck image"
grep -Fq 'image { function = SLIDER detail = "vscale" }' "$gtk2_theme_source" ||
  die "GTK 2 vertical scale does not suppress its puck image"

for gtk3_asset in \
  arrow-down arrow-down-disabled arrow-down-inverse \
  arrow-left arrow-left-disabled arrow-left-inverse \
  arrow-right arrow-right-disabled arrow-right-inverse \
  arrow-up arrow-up-disabled arrow-up-inverse panel-bar; do
  [[ -r "$theme_source_dir/gtk-3.0/assets/$gtk3_asset.xpm" ]] ||
    die "native GTK arrow asset is missing: $gtk3_asset.xpm"
done
for disabled_arrow_asset in \
  arrow-down-disabled arrow-left-disabled \
  arrow-right-disabled arrow-up-disabled; do
  grep -Fqx "\". c $application_disabled_fg\"," \
    "$theme_source_dir/gtk-3.0/assets/$disabled_arrow_asset.xpm" ||
    die "native disabled arrow does not derive from the application color: $disabled_arrow_asset.xpm"
done
for spinner_asset in spinner-symbolic.svg spinner-{1..7}-symbolic.svg; do
  [[ -r "$theme_source_dir/gtk-3.0/assets/$spinner_asset" ]] ||
    die "native GTK spinner asset is missing: $spinner_asset"
done
grep -Fqx -- "\"1 $panel_height_px 2 1\"," \
  "$theme_source_dir/gtk-3.0/assets/panel-bar.xpm" ||
  die "native panel bitmap is not ${panel_height_px}px high"
grep -Fqx "\"+ c $menubar_bg\"," "$theme_source_dir/gtk-3.0/assets/panel-bar.xpm" ||
  die "native panel bitmap does not use the configured menubar background"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  if grep -Fq 'outline-style: solid;' "$gtk_theme_source"; then
    die "GTK theme contains a solid focus outline: $gtk_theme_source"
  fi
  awk -v expected="$border_radius_css" '
    /border-((top|bottom)-(left|right)-)?radius:[[:space:]]*/ {
      value = $0
      sub(/^[^:]*:[[:space:]]*/, "", value)
      sub(/[[:space:]]*;.*/, "", value)
      count = split(value, corners, /[[:space:]]+/)
      for (corner = 1; corner <= count; corner++) {
        if (corners[corner] != "0" &&
            corners[corner] != "0px" &&
            corners[corner] != expected) {
          exit 1
        }
      }
    }
  ' "$gtk_theme_source" ||
    die "GTK theme contains a corner radius other than 0 or $border_radius_css: $gtk_theme_source"
  grep -Fq "border-radius: $border_radius_css;" "$gtk_theme_source" ||
    die "GTK theme does not define the shared $border_radius_css corner radius: $gtk_theme_source"
  grep -Fq \
    "border-radius: $border_radius_css $border_radius_css 0 0;" \
    "$gtk_theme_source" ||
    die "GTK notebook tabs do not use the shared $border_radius_css corner radius: $gtk_theme_source"
  awk '
    $0 == "* {" { in_default = 1 }
    in_default && $0 == "  border-radius: 0;" { found = 1 }
    in_default && $0 == "}" { in_default = 0 }
    END { exit(found ? 0 : 1) }
  ' "$gtk_theme_source" ||
    die "GTK theme does not preserve base-square corners: $gtk_theme_source"
  awk '
    $0 == "window.csd," { in_window = 1 }
    in_window && $0 == "  border-radius: 0;" { found = 1 }
    in_window && $0 == "}" { in_window = 0 }
    END { exit(found ? 0 : 1) }
  ' "$gtk_theme_source" ||
    die "GTK theme does not preserve square window corners: $gtk_theme_source"
done

validate_css_tooltip() {
  local css_file="$1"

  grep -Fqx '@define-color quartz_tooltip #ffecb3;' "$css_file" ||
    return 1
  awk '
    $0 == "tooltip," { in_tooltip = 1 }
    in_tooltip && $0 == "tooltip.background," { background_node = 1 }
    in_tooltip && $0 == ".tooltip {" { compatibility_class = 1 }
    in_tooltip && $0 == "  color: @quartz_ink;" { black_text = 1 }
    in_tooltip && $0 == "  background-color: @quartz_tooltip;" {
      amber_face = 1
    }
    in_tooltip && $0 == "  border: 1px solid @quartz_ink;" {
      black_outline = 1
    }
    in_tooltip && $0 == "}" { in_tooltip = 0 }

    $0 == "tooltip decoration," { in_decoration = 1 }
    in_decoration && $0 == "window.popup.tooltip decoration," {
      popup_decoration = 1
    }
    in_decoration && $0 == ".tooltip decoration {" {
      compatibility_decoration = 1
    }
    in_decoration && $0 == "  border: 0;" { bare_decoration = 1 }
    in_decoration && $0 == "  box-shadow: none;" { no_decoration_shadow = 1 }
    in_decoration && $0 == "}" { in_decoration = 0 }
    END {
      exit(background_node && compatibility_class && black_text &&
        amber_face && black_outline && popup_decoration &&
        compatibility_decoration && bare_decoration &&
        no_decoration_shadow ? 0 : 1)
    }
  ' "$css_file"
}

validate_css_tooltip "$gtk3_theme_source" ||
  die "GTK 3 tooltip is not amber with black text and outline"
validate_css_tooltip "$gtk4_theme_source" ||
  die "GTK 4 tooltip is not amber with black text and outline"
grep -Fqx '@define-color theme_tooltip_bg_color @quartz_tooltip;' \
  "$gtk3_theme_source" ||
  die "GTK 3 public tooltip background does not use the amber face"
grep -Fqx '@define-color theme_tooltip_fg_color @quartz_ink;' \
  "$gtk3_theme_source" ||
  die "GTK 3 public tooltip foreground is not black"

validate_css_menu_font() {
  local css_file="$1"
  local opening_selector="$2"

  awk -v selector="$opening_selector" '
    $0 == selector { in_rule = 1 }
    in_rule && $0 == "  font-family: \"ChiKareGo2\";" { family = 1 }
    in_rule && $0 == "  font-size: 16px;" { size = 1 }
    in_rule && $0 == "  font-style: normal;" { style = 1 }
    in_rule && $0 == "  font-weight: 400;" { weight = 1 }
    in_rule && $0 == "}" { in_rule = 0 }
    END { exit(family && size && style && weight ? 0 : 1) }
  ' "$css_file"
}

validate_css_menu_font "$gtk3_theme_source" 'menubar {' ||
  die "GTK 3 menubars do not use native ChiKareGo2"
validate_css_menu_font "$gtk3_theme_source" 'menu,' ||
  die "GTK 3 popup commands do not use native ChiKareGo2"
validate_css_menu_font "$gtk3_theme_source" 'popover.menu,' ||
  die "GTK 3 menu-model popovers do not use native ChiKareGo2"
validate_css_menu_font "$gtk4_theme_source" 'menubar {' ||
  die "GTK 4 menubars do not use native ChiKareGo2"
validate_css_menu_font "$gtk4_theme_source" 'popover.menu,' ||
  die "GTK 4 menu-model popovers do not use native ChiKareGo2"

if grep -Fq '  outline: none;' "$gtk4_theme_source"; then
  die "GTK 4 theme suppresses a component focus outline"
fi
awk '
  $0 == "* {" { in_default = 1 }
  in_default && $0 == "  outline-color: @quartz_ink;" { default_color = 1 }
  in_default && $0 == "  outline-style: dotted;" { default_dotted = 1 }
  in_default && $0 == "  outline-width: 1px;" { default_width = 1 }
  in_default && $0 == "  outline-offset: -3px;" { default_inset = 1 }
  in_default && $0 == "}" { in_default = 0 }

  $0 == "entry:focus {" { in_entry = 1 }
  in_entry && $0 == "  outline-style: dotted;" { entry_dotted = 1 }
  in_entry && $0 == "}" { in_entry = 0 }

  $0 == "notebook:focus > header > tabs > tab:checked {" { in_tab = 1 }
  in_tab && $0 == "  outline-color: @quartz_ink;" { tab_color = 1 }
  in_tab && $0 == "  outline-style: dotted;" { tab_dotted = 1 }
  in_tab && $0 == "  outline-width: 1px;" { tab_width = 1 }
  in_tab && $0 == "  outline-offset: -3px;" { tab_inset = 1 }
  in_tab && $0 == "}" { in_tab = 0 }

  END {
    exit(default_color && default_dotted && default_width && default_inset &&
      entry_dotted && tab_color && tab_dotted && tab_width && tab_inset ? 0 : 1)
  }
' "$gtk3_theme_source" ||
  die "GTK 3 focus outlines are not consistently dotted"
awk '
  $0 == "*:focus-visible {" { in_default = 1 }
  in_default && $0 == "  outline-color: @quartz_ink;" { default_color = 1 }
  in_default && $0 == "  outline-style: dotted;" { default_dotted = 1 }
  in_default && $0 == "  outline-width: 1px;" { default_width = 1 }
  in_default && $0 == "  outline-offset: -3px;" { default_inset = 1 }
  in_default && $0 == "}" { in_default = 0 }

  $0 == "entry:focus-within," { in_entry = 1 }
  in_entry && $0 == "spinbutton:focus-within:not(.vertical) {" {
    entry_companion = 1
  }
  in_entry && $0 == "  outline-style: dotted;" { entry_dotted = 1 }
  in_entry && $0 == "}" { in_entry = 0 }

  $0 == "notebook:focus:focus-visible > header > tabs > tab:checked {" {
    in_tab = 1
  }
  in_tab && $0 == "  outline-color: @quartz_ink;" { tab_color = 1 }
  in_tab && $0 == "  outline-style: dotted;" { tab_dotted = 1 }
  in_tab && $0 == "  outline-width: 1px;" { tab_width = 1 }
  in_tab && $0 == "  outline-offset: -3px;" { tab_inset = 1 }
  in_tab && $0 == "}" { in_tab = 0 }

  END {
    exit(default_color && default_dotted && default_width && default_inset &&
      entry_companion && entry_dotted && tab_color && tab_dotted &&
      tab_width && tab_inset ? 0 : 1)
  }
' "$gtk4_theme_source" ||
  die "GTK 4 focus outlines are not consistently dotted"
validate_transparent_spin_text_surface() {
  local css_file="$1"
  local spin_text_selector="$2"

  awk -v selector="$spin_text_selector" '
    $0 == selector { in_spin_text = 1 }
    in_spin_text && $0 == "  background-color: transparent;" {
      transparent_surface = 1
    }
    in_spin_text && $0 == "}" { in_spin_text = 0 }
    END { exit(transparent_surface ? 0 : 1) }
  ' "$css_file" ||
    die "spin-button text surface can cover the rounded parent: $css_file"
}
validate_transparent_spin_text_surface \
  "$gtk3_theme_source" \
  'spinbutton entry {'
validate_transparent_spin_text_surface \
  "$gtk3_theme_source" \
  'spinbutton entry:disabled {'
validate_transparent_spin_text_surface \
  "$gtk4_theme_source" \
  'spinbutton:not(.vertical) > text {'
validate_pixel_frame_rule() {
  local css_file="$1"
  local frame_selector="$2"
  local expected_edge_inset="$3"
  local expected_radius="$4"

  awk \
    -v selector="$frame_selector" \
    -v expected_radius="$expected_radius" \
    -v expected_inset="$expected_edge_inset" '
      $0 == selector { in_frame = 1 }
      in_frame && $0 == "  border: 1px solid transparent;" { border = 1 }
      in_frame && $0 == "  border-image-source: none;" { no_border_image = 1 }
      in_frame && $0 == "  border-radius: 0;" { no_vector_clip = 1 }
      in_frame && /url\(/ { bitmap_layer = 1 }
      in_frame &&
        $0 ~ "^    url\\(\"../gtk-3.0/assets/frame-corner-v1-r" expected_radius "-[0-3]\\.xpm\"\\),$" {
        bitmap_corners++
      }

      in_frame && $0 == "  background-image:" {
        background = 1
        in_images = 1
        next
      }
      in_frame && $0 == "  background-size:" {
        in_images = 0
        sizes = 1
        in_sizes = 1
        next
      }
      in_frame && $0 == "  background-position:" {
        in_sizes = 0
        positions = 1
        in_positions = 1
        next
      }
      in_frame && $0 == "  background-repeat: no-repeat;" {
        in_positions = 0
        no_repeat = 1
      }

      in_images &&
        /^    repeating-linear-gradient\(to right, transparent 0, transparent [0-9]+px, @quartz_ink [0-9]+px,/ {
        pixel_rows++
        row = $0
        calc_stops = gsub(/calc\(100% - [0-9]+px\)/, "", row)
        if (calc_stops == 4 && $0 ~ /transparent 100%\),$/) {
          complete_pixel_rows++
        }
        if (pixel_rows % 2 == 1) {
          first_coverage_pass = $0
        } else if ($0 == first_coverage_pass) {
          coverage_pairs++
        }
      }
      in_images && $0 == "    repeating-linear-gradient(to right," {
        horizontal++
      }
      in_images && $0 == "    repeating-linear-gradient(to bottom," {
        vertical++
      }
      in_images &&
        $0 == "      @quartz_ink 0, @quartz_ink 1px, transparent 1px, transparent 2px)," {
        comma_stops++
      }
      in_images &&
        $0 == "      @quartz_ink 0, @quartz_ink 1px, transparent 1px, transparent 2px);" {
        final_stop = 1
      }

      in_sizes && $0 == "    100% 1px," { pixel_row_sizes++ }
      in_sizes &&
        $0 == "    " (expected_radius + 1) "px " (expected_radius + 1) "px," {
        bitmap_sizes++
      }
      in_sizes &&
        $0 == "    calc(100% - " expected_inset ") 1px," {
        horizontal_edge_sizes++
      }
      in_sizes &&
        $0 == "    1px calc(100% - " expected_inset ")," {
        vertical_edge_size = 1
      }
      in_sizes &&
        $0 == "    1px calc(100% - " expected_inset ");" {
        final_vertical_edge_size = 1
      }

      in_positions && /^    left [0-9]+px,$/ { top_positions++ }
      in_positions &&
        $0 == "    left top, right top, left bottom, right bottom," {
        bitmap_positions = 1
      }
      in_positions && /^    left calc\(100% - [0-9]+px\),$/ {
        bottom_positions++
      }
      in_positions &&
        $0 == "    center top, center bottom, left center, right center;" {
        edge_positions = 1
      }

      in_frame && $0 == "  background-origin: border-box;" { origin = 1 }
      in_frame && $0 == "  background-clip: border-box;" { clip = 1 }
      in_frame && $0 == "}" { in_frame = 0 }

      $0 == "frame > label {" { in_label = 1 }
      in_label && $0 == "  background-color: @quartz_chrome;" { label_face = 1 }
      in_label && $0 == "  padding-left: 3px;" { label_left = 1 }
      in_label && $0 == "  padding-right: 3px;" { label_right = 1 }
      in_label && $0 == "}" { in_label = 0 }

      END {
        # At radius 1 the arc is too short to contain a corner dot.
        generated_rows = (pixel_rows >= 4 || expected_radius == 1) &&
          pixel_rows % 4 == 0 &&
          complete_pixel_rows == pixel_rows &&
          coverage_pairs * 2 == pixel_rows &&
          pixel_row_sizes == pixel_rows &&
          top_positions == pixel_rows / 2 &&
          bottom_positions == pixel_rows / 2
        if (expected_radius >= 40) {
          corners = bitmap_layer && bitmap_corners == 4 && bitmap_sizes == 4 &&
            bitmap_positions && pixel_rows == 0
        } else {
          corners = !bitmap_layer && generated_rows && pixel_rows + 4 <= 128
        }
        complete = border && no_border_image && no_vector_clip &&
          corners && background && sizes && positions &&
          horizontal == 2 && vertical == 2 && comma_stops == 3 &&
          final_stop && horizontal_edge_sizes == 2 && vertical_edge_size &&
          final_vertical_edge_size && edge_positions && no_repeat &&
          origin && clip &&
          label_face && label_left && label_right
        exit(complete ? 0 : 1)
      }
    ' "$css_file"
}

validate_pixel_frame_rule \
  "$gtk3_theme_source" "frame > border {" "$frame_edge_inset_css" "$border_radius_px" ||
  die "GTK 3 frames do not use the pixel-dotted shared-radius container outline"
validate_pixel_frame_rule \
  "$gtk4_theme_source" "frame {" "$frame_edge_inset_css" "$border_radius_px" ||
  die "GTK 4 frames do not use the pixel-dotted shared-radius container outline"
awk '
  $0 == "statusbar frame," { in_status_frame = 1 }
  in_status_frame && $0 == "  background-image: none;" { cleared = 1 }
  in_status_frame && $0 == "}" { in_status_frame = 0 }
  END { exit(cleared ? 0 : 1) }
' "$gtk3_theme_source" ||
  die "GTK 3 statusbar layout frames inherit the generic dotted outline"
awk '
  $0 == ".statusbar frame {" { in_status_frame = 1 }
  in_status_frame && $0 == "  background-image: none;" { cleared = 1 }
  in_status_frame && $0 == "}" { in_status_frame = 0 }
  END { exit(cleared ? 0 : 1) }
' "$gtk4_theme_source" ||
  die "GTK 4 statusbar layout frames inherit the generic dotted outline"
awk '
  $0 == ".mate-panel-menu-bar PanelApplet > button.flat.toggle," {
    in_panel_menu_button = 1
  }
  in_panel_menu_button &&
    $0 == ".gnome-panel-menu-bar PanelApplet > button.flat.toggle {" {
      generic_panel_menu_button = 1
    }
  in_panel_menu_button && $0 == "  border-radius: 0;" { square = 1 }
  in_panel_menu_button && $0 == "}" { in_panel_menu_button = 0 }
  END { exit(generic_panel_menu_button && square ? 0 : 1) }
' "$gtk3_theme_source" ||
  die "GTK 3 main-panel menu buttons do not use square corners"
awk '
  $0 == "PanelToplevel.bottom button," { in_bottom_panel_buttons = 1 }
  in_bottom_panel_buttons && $0 == "#showdesktop-button," {
    show_desktop = 1
  }
  in_bottom_panel_buttons && $0 == "#tasklist-button {" { task_list = 1 }
  in_bottom_panel_buttons && $0 == "  border-radius: 0;" { square = 1 }
  in_bottom_panel_buttons && $0 == "}" { in_bottom_panel_buttons = 0 }
  END { exit(show_desktop && task_list && square ? 0 : 1) }
' "$gtk3_theme_source" ||
  die "GTK 3 bottom-panel buttons do not use square corners"
grep -Fq '@define-color quartz_paper @quartz_application;' "$gtk3_theme_source" ||
  die "GTK 3 paper surfaces do not use the application background"
grep -Fq "@define-color quartz_application $application_bg;" "$gtk3_theme_source" ||
  die "GTK 3 theme does not define the configured application background"
for gtk3_application_view_role in \
  '@define-color theme_base_color @quartz_application;' \
  '@define-color theme_unfocused_base_color @quartz_application;' \
  '@define-color insensitive_base_color @quartz_application;' \
  '@define-color content_view_bg @quartz_application;' \
  '@define-color text_view_bg @quartz_application;'; do
  grep -Fq "$gtk3_application_view_role" "$gtk3_theme_source" ||
    die "GTK 3 content views do not use the configured application background"
done
grep -Fq "@define-color quartz_menubar $menubar_bg;" "$gtk3_theme_source" ||
  die "GTK 3 theme does not define the configured menubar background"
grep -Fq "@define-color quartz_titlebar $titlebar_bg;" "$gtk3_theme_source" ||
  die "GTK 3 theme does not define the configured titlebar background"
grep -Fq '@define-color quartz_ink #000000;' "$gtk3_theme_source" ||
  die "GTK 3 theme does not define pure-black ink"
grep -Fq "@define-color quartz_disabled $application_disabled_fg;" "$gtk3_theme_source" ||
  die "GTK 3 disabled ink does not derive from the application color"
grep -Fq "@define-color quartz_pressed $application_shade;" "$gtk3_theme_source" ||
  die "GTK 3 pressed-button shade does not derive from the application color"
grep -Fq "@define-color quartz_disabled_bg $application_disabled_bg;" "$gtk3_theme_source" ||
  die "GTK 3 disabled-button face does not derive from the application color"
grep -Fq "@define-color quartz_toggle_on $application_shade;" "$gtk3_theme_source" ||
  die "GTK 3 toggle-on face does not derive from the application color"
grep -Fq '@define-color quartz_paper @quartz_application;' "$gtk4_theme_source" ||
  die "GTK 4 paper surfaces do not use the application background"
grep -Fq "@define-color quartz_application $application_bg;" "$gtk4_theme_source" ||
  die "GTK 4 theme does not define the configured application background"
grep -Fq '@define-color view_bg_color @quartz_application;' "$gtk4_theme_source" ||
  die "GTK 4 content views do not use the configured application background"
grep -Fq "@define-color quartz_menubar $menubar_bg;" "$gtk4_theme_source" ||
  die "GTK 4 theme does not define the configured menubar background"
grep -Fq "@define-color quartz_titlebar $titlebar_bg;" "$gtk4_theme_source" ||
  die "GTK 4 theme does not define the configured titlebar background"
grep -Fq "base_color:$application_bg" "$gtk2_theme_source" ||
  die "GTK 2 public base color does not use the configured application background"
grep -Fq "  base[NORMAL] = \"$application_bg\"" "$gtk2_theme_source" ||
  die "GTK 2 content views do not use the configured application background"
grep -Fq 'widget_class "*.<GtkTextView>" style "quartz-default"' "$gtk2_theme_source" ||
  die "GTK 2 text views do not use the shared application surface"
grep -Fq '@define-color quartz_ink #000000;' "$gtk4_theme_source" ||
  die "GTK 4 theme does not define pure-black ink"
grep -Fq "@define-color quartz_disabled $application_disabled_fg;" "$gtk4_theme_source" ||
  die "GTK 4 disabled ink does not derive from the application color"
grep -Fq "@define-color quartz_pressed $application_shade;" "$gtk4_theme_source" ||
  die "GTK 4 pressed-button shade does not derive from the application color"
grep -Fq "@define-color quartz_disabled_bg $application_disabled_bg;" "$gtk4_theme_source" ||
  die "GTK 4 disabled-button face does not derive from the application color"
grep -Fq "@define-color quartz_toggle_on $application_shade;" "$gtk4_theme_source" ||
  die "GTK 4 toggle-on face does not derive from the application color"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  awk '
    $0 == "button.flat:hover {" { in_flat_hover = 1 }
    in_flat_hover && $0 == "  background-color: @quartz_toggle_on;" {
      shaded_face = 1
    }
    in_flat_hover && $0 == "  border-color: transparent;" {
      borderless = 1
    }
    in_flat_hover && $0 == "  box-shadow: none;" { unraised = 1 }
    in_flat_hover && $0 == "}" { in_flat_hover = 0 }
    END { exit(shaded_face && borderless && unraised ? 0 : 1) }
  ' "$gtk_theme_source" ||
    die "GTK flat-button hover is not palette-shaded, borderless, and unraised: $gtk_theme_source"
done
awk '
  $0 == "button.flat:hover {" { in_flat_hover = 1 }
  in_flat_hover && $0 == "  margin-top: 0;" { stable_top = 1 }
  in_flat_hover && $0 == "  margin-bottom: 1px;" { stable_bottom = 1 }
  in_flat_hover && $0 == "}" { in_flat_hover = 0 }
  END { exit(stable_top && stable_bottom ? 0 : 1) }
' "$gtk3_theme_source" ||
  die "GTK 3 flat-button hover changes physical button position"
awk '
  $0 == "button.flat:hover {" { in_flat_hover = 1 }
  in_flat_hover && $0 == "  transform: none;" { stationary = 1 }
  in_flat_hover && $0 == "}" { in_flat_hover = 0 }
  END { exit(stationary ? 0 : 1) }
' "$gtk4_theme_source" ||
  die "GTK 4 flat-button hover changes physical button position"
awk '
  $0 == "headerbar button," { in_header_button = 1 }
  in_header_button && $0 == "windowcontrols button {" { window_controls = 1 }
  in_header_button && $0 == "  border: 0;" { borderless = 1 }
  in_header_button && $0 == "  box-shadow: none;" { unraised = 1 }
  in_header_button && $0 == "}" {
    complete = window_controls && borderless && unraised
    in_header_button = 0
  }
  END { exit(complete ? 0 : 1) }
' "$gtk4_theme_source" ||
  die "GTK 4 titlebar and window-control buttons are not flat and borderless"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  grep -Fq '@define-color quartz_suggested #ddf1d8;' "$gtk_theme_source" ||
    die "GTK theme does not define the pale-green suggested action: $gtk_theme_source"
  grep -Fq '@define-color quartz_suggested_pressed #cbdec7;' "$gtk_theme_source" ||
    die "GTK theme does not define the pressed suggested action: $gtk_theme_source"
  grep -Fq '@define-color quartz_destructive #f4d6d6;' "$gtk_theme_source" ||
    die "GTK theme does not define the pale-red destructive action: $gtk_theme_source"
  grep -Fq '@define-color quartz_destructive_pressed #e0c5c5;' "$gtk_theme_source" ||
    die "GTK theme does not define the pressed destructive action: $gtk_theme_source"
done
grep -Fq "bg_color:$application_bg" "$gtk2_theme_source" ||
  die "GTK 2 theme does not define the configured application background"
grep -Fq '  box-shadow: 0 1px 0 0 @quartz_ink;' "$gtk3_theme_source" ||
  die "GTK 3 buttons do not define a one-pixel downward shadow"
grep -Fq '  margin-top: 1px;' "$gtk3_theme_source" ||
  die "GTK 3 pressed buttons do not move down one pixel"
grep -Fq '  box-shadow: 0 1px 0 0 @quartz_ink;' "$gtk4_theme_source" ||
  die "GTK 4 buttons do not define a one-pixel downward shadow"
grep -Fq '  transform: translate(0, 1px);' "$gtk4_theme_source" ||
  die "GTK 4 pressed buttons do not move down one pixel"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  awk -v expected_radius="$border_radius_css" '
    $0 == "button:focus," || $0 == "button:focus-visible," {
      in_button_focus = 1
    }
    in_button_focus &&
      ($0 == "combobox button:focus {" ||
       $0 == "dropdown > button:focus-visible {") { companion = 1 }
    in_button_focus && $0 == "  outline-color: @quartz_ink;" { color = 1 }
    in_button_focus && $0 == "  outline-style: dotted;" { dotted = 1 }
    in_button_focus && $0 == "  outline-width: 1px;" { width = 1 }
    in_button_focus && $0 == "  outline-offset: -3px;" { inset = 1 }
    in_button_focus && $0 == "}" { in_button_focus = 0 }

    $0 == "button," { in_button_rule = 1 }
    in_button_rule && $0 == "  border-radius: " expected_radius ";" {
      radius = 1
    }
    in_button_rule && $0 == "}" { in_button_rule = 0 }

    END {
      exit(companion && color && dotted && width && inset && radius ? 0 : 1)
    }
  ' "$gtk_theme_source" ||
    die "GTK buttons do not use the radius-matched inset dotted focus contour: $gtk_theme_source"
  awk '
    $0 == "button:checked," { in_checked = 1 }
    in_checked && $0 == "  background-color: @quartz_toggle_on;" {
      shaded_face = 1
    }
    in_checked && $0 == "  box-shadow: none;" { unshadowed = 1 }
    in_checked && $0 == "}" {
      checked_complete = shaded_face && unshadowed
      in_checked = 0
    }

    $0 == "button.toggle:hover," { in_toggle_hover = 1 }
    in_toggle_hover && $0 == "button.radio:hover {" { radio_button = 1 }
    in_toggle_hover && $0 == "  background-color: @quartz_toggle_on;" {
      shaded_hover = 1
    }
    in_toggle_hover && $0 == "}" { in_toggle_hover = 0 }

    END {
      exit(checked_complete && radio_button && shaded_hover ? 0 : 1)
    }
  ' "$gtk_theme_source" ||
    die "GTK toggle buttons do not use application-shaded latches and hover: $gtk_theme_source"
done
awk '
  $0 == "button:checked," { in_checked = 1 }
  in_checked && $0 == "  margin-top: 1px;" { down = 1 }
  in_checked && $0 == "  margin-bottom: 0;" { shadow_row = 1 }
  in_checked && $0 == "}" { in_checked = 0 }
  END { exit(down && shadow_row ? 0 : 1) }
' "$gtk3_theme_source" ||
  die "GTK 3 checked toggle buttons do not occupy their former shadow row"
awk '
  $0 == "button:checked," { in_checked = 1 }
  in_checked && $0 == "  transform: translate(0, 1px);" { down = 1 }
  in_checked && $0 == "}" { in_checked = 0 }
  END { exit(down ? 0 : 1) }
' "$gtk4_theme_source" ||
  die "GTK 4 checked toggle buttons do not move into their former shadow row"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  awk -v expected_radius="$border_radius_css" '
    $0 == "levelbar.horizontal.discrete trough > block {" {
      in_horizontal_spacing = 1
    }
    in_horizontal_spacing && $0 == "  margin: 0 1px;" {
      horizontal_spacing = 1
    }
    in_horizontal_spacing && $0 == "}" { in_horizontal_spacing = 0 }

    $0 == "levelbar.vertical.discrete trough > block {" {
      in_vertical_spacing = 1
    }
    in_vertical_spacing && $0 == "  margin: 1px 0;" {
      vertical_spacing = 1
    }
    in_vertical_spacing && $0 == "}" { in_vertical_spacing = 0 }

    $0 == "levelbar.horizontal.discrete trough > block," {
      in_discrete_blocks = 1
    }
    in_discrete_blocks &&
      $0 == "levelbar.vertical.discrete trough > block," {
        both_orientations = 1
    }
    in_discrete_blocks && $0 == "  border-radius: 0;" {
      square_segments = 1
    }
    in_discrete_blocks && $0 == "}" { in_discrete_blocks = 0 }

    $0 == "levelbar.horizontal.discrete trough > block:first-child {" {
      in_first_segment = 1
    }
    in_first_segment &&
      $0 == "  border-radius: " expected_radius " 0 0 " expected_radius ";" {
        rounded_first_end = 1
    }
    in_first_segment && $0 == "}" { in_first_segment = 0 }

    $0 == "levelbar.horizontal.discrete trough > block:last-child {" {
      in_last_segment = 1
    }
    in_last_segment &&
      $0 == "  border-radius: 0 " expected_radius " " expected_radius " 0;" {
        rounded_last_end = 1
    }
    in_last_segment && $0 == "}" { in_last_segment = 0 }

    END {
      exit(horizontal_spacing && vertical_spacing && both_orientations &&
        square_segments && rounded_first_end && rounded_last_end ? 0 : 1)
    }
  ' "$gtk_theme_source" ||
    die "GTK discrete level bars do not keep separated square segments: $gtk_theme_source"
done
grep -Fq 'levelbar.horizontal:not(.discrete) trough > block:dir(ltr),' \
  "$gtk4_theme_source" ||
  die "GTK 4 continuous level-bar geometry is not isolated from discrete bars"
if grep -Fq 'levelbar.horizontal trough > block:dir(' "$gtk4_theme_source"; then
  die "GTK 4 continuous level-bar geometry leaks into discrete bars"
fi
grep -Fq 'GtkRange::trough-border = 0' "$gtk2_theme_source" ||
  die "GTK 2 theme does not keep scrollbar dither rails unframed"
grep -Fq 'GtkRange::trough-under-steppers = 0' "$gtk2_theme_source" ||
  die "GTK 2 scrollbar rail extends beneath its arrow steppers"
grep -Fq 'style "quartz-scrollbar"' "$gtk2_theme_source" ||
  die "GTK 2 theme does not define its dithered scrollbar renderer"
grep -Fq '../gtk-3.0/assets/arrow-up.xpm' "$gtk2_theme_source" ||
  die "GTK 2 dithered scrollbar does not preserve its arrow glyphs"
for disabled_scrollbar_arrow_rule in \
  'state = INSENSITIVE detail = "vscrollbar" arrow_direction = UP overlay_file = "../gtk-3.0/assets/arrow-up-disabled.xpm"' \
  'state = INSENSITIVE detail = "vscrollbar" arrow_direction = DOWN overlay_file = "../gtk-3.0/assets/arrow-down-disabled.xpm"' \
  'state = INSENSITIVE detail = "hscrollbar" arrow_direction = LEFT overlay_file = "../gtk-3.0/assets/arrow-left-disabled.xpm"' \
  'state = INSENSITIVE detail = "hscrollbar" arrow_direction = RIGHT overlay_file = "../gtk-3.0/assets/arrow-right-disabled.xpm"'; do
  grep -Fq "$disabled_scrollbar_arrow_rule" "$gtk2_theme_source" ||
    die "GTK 2 scrollbar does not use a palette-derived arrow for every insensitive stepper"
done
[[ "$(grep -Fxc '    <distance name="right_width" value="1"/>' "$marco_theme_source")" == "2" ]] ||
  die "Marco theme does not keep every right frame edge free of horizontal shadow"
[[ "$(grep -Fxc '    <distance name="right_titlebar_edge" value="1"/>' "$marco_theme_source")" == "1" ]] ||
  die "Marco titlebars still reserve space for a right-side shadow"
[[ "$(grep -Fxc '    <distance name="bottom_height" value="2"/>' "$marco_theme_source")" == "2" ]] ||
  die "Marco theme does not reserve one bottom shadow pixel for every frame geometry"
grep -Fq '<draw_ops name="window_border_and_shadow">' "$marco_theme_source" ||
  die "Marco theme does not define the hard window-frame shadow"
grep -Fq '<rectangle color="#000000" x="0" y="0" width="width-1" height="height-2" filled="false"/>' \
  "$marco_theme_source" ||
  die "Marco theme does not keep a one-pixel right outline above the bottom shadow"
grep -Fq '<rectangle color="#000000" x="0" y="height-1" width="width" height="1" filled="true"/>' \
  "$marco_theme_source" ||
  die "Marco theme does not draw its bottom-only shadow row"
if grep -Fq '<rectangle color="#000000" x="width-1" y="0" width="1" height="height" filled="true"/>' \
  "$marco_theme_source"; then
  die "Marco theme still draws a horizontal window shadow"
fi
[[ "$(grep -Fc 'draw_ops="window_border_and_shadow"' "$marco_theme_source")" == "3" ]] ||
  die "Marco theme does not apply the hard shadow to every frame style"
for gtk_theme_source in "$gtk3_theme_source" "$gtk4_theme_source"; do
  awk '
    $0 == "decoration {" { in_decoration = 1 }
    in_decoration && $0 == "  border: 0;" { transparent_frame = 1 }
    in_decoration &&
      $0 == "  box-shadow: 0 0 0 1px #000000, 0 1px 0 1px #000000;" {
        outlined_bottom_shadow = 1
    }
    in_decoration && $0 == "}" { in_decoration = 0 }
    END { exit(transparent_frame && outlined_bottom_shadow ? 0 : 1) }
  ' "$gtk_theme_source" ||
    die "GTK window decorations do not define their hard outline and bottom shadow: $gtk_theme_source"
  if grep -Fq 'box-shadow: 1px 1px 0 0 #000000;' "$gtk_theme_source"; then
    die "GTK window decorations still define a horizontal shadow: $gtk_theme_source"
  fi
done
awk \
  -v expected_radius="$border_radius_css" \
  -v expected_width="  border-width: ${border_slice_px}px ${border_slice_px}px ${border_shadow_slice_px}px ${border_slice_px}px;" \
  -v expected_slice="  border-image-slice: $border_slice_px $border_slice_px $border_shadow_slice_px $border_slice_px fill;" '
  $0 == "menu," { in_menu = 1 }
  in_menu && $0 == "  background-color: @quartz_paper;" { menu_fill = 1 }
  in_menu && $0 == "  background-clip: padding-box;" { menu_clip = 1 }
  in_menu && $0 == expected_width { menu_width = 1 }
  in_menu && /menu-frame-[[:xdigit:]]+\.xpm/ { menu_frame = 1 }
  in_menu && $0 == expected_slice { menu_slice = 1 }
  in_menu && $0 == "  border-radius: " expected_radius ";" {
    menu_radius_clip = 1
  }
  in_menu && $0 == "}" { in_menu = 0 }

  $0 == "menuitem:disabled {" { in_disabled_item = 1; disabled_item = 1 }
  in_disabled_item && $0 == "  background-color: transparent;" {
    disabled_item_clear = 1
  }
  in_disabled_item && $0 == "  background-image: none;" {
    disabled_item_image_clear = 1
  }
  in_disabled_item && $0 == "}" { in_disabled_item = 0 }

  $0 == "menu menuitem:disabled," {
    in_context_disabled_item = 1
    context_disabled_item = 1
  }
  in_context_disabled_item && $0 == "  background-color: transparent;" {
    context_disabled_item_clear = 1
  }
  in_context_disabled_item && $0 == "  background-image: none;" {
    context_disabled_item_image_clear = 1
  }
  in_context_disabled_item && $0 == "  opacity: 1;" {
    context_disabled_item_opaque = 1
  }
  in_context_disabled_item && $0 == "}" { in_context_disabled_item = 0 }

  $0 == "window.popup {" { in_popup = 1 }
  in_popup && $0 == "  background-color: transparent;" { popup_clear = 1 }
  in_popup && $0 == "  border-radius: " expected_radius ";" {
    popup_radius_clip = 1
  }
  in_popup && $0 == "}" { in_popup = 0 }

  $0 == "window.popup decoration {" { in_decoration = 1 }
  in_decoration && $0 == "  border-radius: " expected_radius ";" {
    decoration_radius_clip = 1
  }
  in_decoration && $0 == "  box-shadow: none;" { decoration_bare = 1 }
  in_decoration && $0 == "}" { in_decoration = 0 }

  $0 == "popover {" { in_popover = 1 }
  in_popover && $0 == "  border-radius: " expected_radius ";" { popover_radius = 1 }
  in_popover && $0 == "  box-shadow: 0 1px 0 0 #000000;" { popover_shadow = 1 }
  in_popover && $0 == "}" { in_popover = 0 }

  END {
    exit(menu_fill && menu_clip && menu_width && menu_frame && menu_slice &&
      menu_radius_clip && disabled_item && disabled_item_clear &&
      disabled_item_image_clear && context_disabled_item &&
      context_disabled_item_clear && context_disabled_item_image_clear &&
      context_disabled_item_opaque && popup_clear && popup_radius_clip &&
      decoration_radius_clip && decoration_bare && popover_radius &&
      popover_shadow ? 0 : 1)
  }
' "$gtk3_theme_source" ||
  die "GTK 3 menus do not preserve the rounded frame through disabled rows"
grep -Fq 'border-width: 1px 1px 2px 1px;' "$gtk3_theme_source" ||
  die "GTK 3 theme does not preserve the bottom-only shadow on clipped CSD windows"
grep -Fq '/* Dithered classic scrollbars:' "$gtk3_theme_source" ||
  die "GTK 3 theme does not define dithered scrollbars"
[[ "$(grep -Fxc '  background-image: url("assets/scrollbar-dither.xpm");' "$gtk3_theme_source")" == "1" ]] ||
  die "GTK 3 scrollbar does not use the shared dither rail background"
grep -Fq 'scrollbar:disabled trough {' "$gtk3_theme_source" ||
  die "GTK 3 theme does not target disabled scrollbar rails"
[[ "$(grep -Fxc '  background-image: url("assets/scrollbar-dither-disabled.xpm");' "$gtk3_theme_source")" == "1" ]] ||
  die "GTK 3 disabled scrollbar does not use the shared preblended dither"
grep -Fq 'scrollbar.vertical trough {' "$gtk3_theme_source" ||
  die "GTK 3 theme does not inset vertical scrollbar rails"
grep -Fq 'scrollbar.horizontal trough {' "$gtk3_theme_source" ||
  die "GTK 3 theme does not inset horizontal scrollbar rails"
grep -Fq '  padding: 0 4px;' "$gtk3_theme_source" ||
  die "GTK 3 vertical scrollbar rail is not 8px inside its 16px allocation"
grep -Fq '  padding: 4px 0;' "$gtk3_theme_source" ||
  die "GTK 3 horizontal scrollbar rail is not 8px inside its 16px allocation"
grep -Fq '  margin: 0 -2px;' "$gtk3_theme_source" ||
  die "GTK 3 vertical scrollbar thumb does not overhang its 8px rail"
grep -Fq '  margin: -2px 0;' "$gtk3_theme_source" ||
  die "GTK 3 horizontal scrollbar thumb does not overhang its 8px rail"
grep -Fq '  background-clip: content-box;' "$gtk3_theme_source" ||
  die "GTK 3 scrollbar dither is not clipped to its inset rail"
grep -Fq '  border-right-style: none;' "$gtk3_theme_source" ||
  die "GTK 3 linked controls do not collapse their internal border seam"
grep -Fq '.linked:not(.vertical):not(buttonbox) > button:first-child,' "$gtk3_theme_source" ||
  die "GTK 3 linked buttons do not preserve their outer left corners"
grep -Fq '.linked:not(.vertical):not(buttonbox) > button:last-child,' "$gtk3_theme_source" ||
  die "GTK 3 linked buttons do not preserve their outer right corners"
grep -Fq 'scrollbar.horizontal button.up,' "$gtk3_theme_source" ||
  die "GTK 3 horizontal scrollbar does not map its backward stepper to the left arrow"
grep -Fq 'scrollbar.horizontal button.down,' "$gtk3_theme_source" ||
  die "GTK 3 horizontal scrollbar does not map its forward stepper to the right arrow"
awk '
  $0 == "scrollbar button:disabled," { in_disabled_stepper = 1 }
  in_disabled_stepper && $0 == "  background-color: @quartz_paper;" {
    application_stepper = 1
  }
  in_disabled_stepper && $0 == "  margin: 0;" { zero_stepper_margin = 1 }
  in_disabled_stepper && $0 == "  box-shadow: none;" { no_stepper_shadow = 1 }
  in_disabled_stepper && $0 == "}" { in_disabled_stepper = 0 }
  END {
    exit(application_stepper && zero_stepper_margin && no_stepper_shadow ? 0 : 1)
  }
' "$gtk3_theme_source" ||
  die "GTK 3 disabled scrollbar stepper is not application-colored and shadowless"
for disabled_scrollbar_arrow in up down left right; do
  grep -Fq \
    "url(\"assets/arrow-$disabled_scrollbar_arrow-disabled.xpm\")" \
    "$gtk3_theme_source" ||
    die "GTK 3 scrollbar does not use its palette-derived $disabled_scrollbar_arrow arrow"
done
awk '
  $0 == "scale highlight," { scale_highlight_selectors++ }
  $0 == "scale fill," ||
    $0 ~ /^scale\.fine-tune (fill|highlight),$/ ||
    $0 == "scrollbar slider," {
      antialiased_range_override = 1
  }

  $0 == "scale fill {" { in_scale_fill = 1 }
  in_scale_fill && $0 == "  border-radius: 0;" { square_scale_clip = 1 }
  in_scale_fill && $0 == "}" { in_scale_fill = 0 }

  $0 == "scrollbar slider {" { in_scrollbar_thumb = 1 }
  in_scrollbar_thumb && $0 == "  border-radius: 0;" {
    square_scrollbar_clip = 1
  }
  in_scrollbar_thumb && $0 == "}" { in_scrollbar_thumb = 0 }

  END {
    exit square_scale_clip && square_scrollbar_clip &&
      scale_highlight_selectors == 1 && !antialiased_range_override ? 0 : 1
  }
' "$gtk3_theme_source" ||
  die "GTK 3 range masks are clipped by an antialiased CSS radius"
grep -Fq 'calc(100% - 2px) calc(100% - 3px)' "$gtk3_theme_source" ||
  die "GTK 3 scrollbar thumb does not define its pixel-curved shadow"
grep -Fq '@define-color quartz_scroll_thumb @quartz_application;' "$gtk3_theme_source" ||
  die "GTK 3 scrollbar thumb does not use the application background"
grep -Fq '/* Macintosh value rails:' "$gtk3_theme_source" ||
  die "GTK 3 theme does not define the Macintosh value rail"
grep -Fq 'background-image: url("assets/scale-dither.xpm");' "$gtk3_theme_source" ||
  die "GTK 3 scale does not use the dithered rail background"
grep -Fq 'min-height: 14px;' "$gtk3_theme_source" ||
  die "GTK 3 scale does not preserve its 13px body plus shadow"
grep -Fq 'opacity: 0;' "$gtk3_theme_source" ||
  die "GTK 3 scale does not hide its puck"
grep -Fq 'scale:disabled {' "$gtk3_theme_source" ||
  die "GTK 3 theme does not target disabled scales"
grep -Fq 'opacity: 0.5;' "$gtk3_theme_source" ||
  die "GTK 3 disabled scale is not 50-percent opaque"
grep -Fq 'popover > contents,' "$gtk4_theme_source" ||
  die "GTK 4 theme does not target popover contents"
awk -v expected_radius="$border_radius_css" '
  $0 == "popover > contents," { in_contents = 1 }
  in_contents && $0 == "  border-radius: " expected_radius ";" { radius = 1 }
  in_contents && $0 == "  box-shadow: 0 1px 0 0 #000000;" { shadow = 1 }
  in_contents && $0 == "}" { in_contents = 0 }
  END { exit(radius && shadow ? 0 : 1) }
' "$gtk4_theme_source" ||
  die "GTK 4 popovers do not use rounded corners with a bottom-only shadow"
grep -Fq 'border-width: 1px 1px 2px 1px;' "$gtk4_theme_source" ||
  die "GTK 4 theme does not preserve the bottom-only shadow on clipped CSD windows"
grep -Fq '/* Dithered scrollbars.' "$gtk4_theme_source" ||
  die "GTK 4 theme does not define dithered scrollbars"
[[ "$(grep -Fxc '  background-image: url("../gtk-3.0/assets/scrollbar-dither.xpm");' "$gtk4_theme_source")" == "1" ]] ||
  die "GTK 4 scrollbar does not use the shared dither rail background"
grep -Fq 'scrollbar:disabled > range > trough,' "$gtk4_theme_source" ||
  die "GTK 4 theme does not target disabled scrollbar rails"
[[ "$(grep -Fxc '  background-image: url("../gtk-3.0/assets/scrollbar-dither-disabled.xpm");' "$gtk4_theme_source")" == "1" ]] ||
  die "GTK 4 disabled scrollbar does not use the shared preblended dither"
grep -Fq 'scrollbar.vertical > range > trough,' "$gtk4_theme_source" ||
  die "GTK 4 theme does not inset vertical scrollbar rails"
grep -Fq 'scrollbar.horizontal > range > trough,' "$gtk4_theme_source" ||
  die "GTK 4 theme does not inset horizontal scrollbar rails"
grep -Fq '  padding: 0 4px;' "$gtk4_theme_source" ||
  die "GTK 4 vertical scrollbar rail is not 8px inside its 16px allocation"
grep -Fq '  padding: 4px 0;' "$gtk4_theme_source" ||
  die "GTK 4 horizontal scrollbar rail is not 8px inside its 16px allocation"
grep -Fq '  margin: 0 -2px;' "$gtk4_theme_source" ||
  die "GTK 4 vertical scrollbar thumb does not overhang its 8px rail"
grep -Fq '  margin: -2px 0;' "$gtk4_theme_source" ||
  die "GTK 4 horizontal scrollbar thumb does not overhang its 8px rail"
grep -Fq '  background-clip: content-box;' "$gtk4_theme_source" ||
  die "GTK 4 scrollbar dither is not clipped to its inset rail"
grep -Fq '.linked:not(.vertical) > button:dir(ltr):not(:last-child),' "$gtk4_theme_source" ||
  die "GTK 4 linked buttons do not target their internal right corners"
grep -Fq '  border-right-style: none;' "$gtk4_theme_source" ||
  die "GTK 4 linked controls do not collapse their internal border seam"
grep -Fq 'splitbutton > button:dir(ltr),' "$gtk4_theme_source" ||
  die "GTK 4 split buttons do not preserve their outer corner mask"
awk '
  $0 == "scrollbar > range > trough > slider," {
    scrollbar_thumb_selectors++
  }
  $0 == "scale > trough > highlight," { scale_highlight_selectors++ }
  $0 == "scale > trough > fill," ||
    $0 ~ /^scale\.fine-tune > trough > (fill|highlight),$/ {
      antialiased_range_override = 1
  }

  $0 == "scale > trough > fill {" { in_scale_fill = 1 }
  in_scale_fill && $0 == "  border-radius: 0;" { square_scale_clip = 1 }
  in_scale_fill && $0 == "}" { in_scale_fill = 0 }

  $0 == "scrollbar slider {" { in_scrollbar_thumb = 1 }
  in_scrollbar_thumb && $0 == "  border-radius: 0;" {
    square_scrollbar_clip = 1
  }
  in_scrollbar_thumb && $0 == "}" { in_scrollbar_thumb = 0 }

  END {
    exit square_scale_clip && square_scrollbar_clip &&
      scrollbar_thumb_selectors == 1 && scale_highlight_selectors == 1 &&
      !antialiased_range_override ? 0 : 1
  }
' "$gtk4_theme_source" ||
  die "GTK 4 range masks are clipped by an antialiased CSS radius"
awk '
  $0 == "scrollbar button:disabled," { in_disabled_stepper = 1 }
  in_disabled_stepper && $0 == "  background-color: @quartz_paper;" {
    application_stepper = 1
  }
  in_disabled_stepper && $0 == "  margin: 0;" { zero_stepper_margin = 1 }
  in_disabled_stepper && $0 == "  box-shadow: none;" { no_stepper_shadow = 1 }
  in_disabled_stepper && $0 == "}" { in_disabled_stepper = 0 }
  END {
    exit(application_stepper && zero_stepper_margin && no_stepper_shadow ? 0 : 1)
  }
' "$gtk4_theme_source" ||
  die "GTK 4 disabled scrollbar stepper is not application-colored and shadowless"
grep -Fq 'calc(100% - 2px) calc(100% - 3px)' "$gtk4_theme_source" ||
  die "GTK 4 scrollbar thumb does not define its pixel-curved shadow"
grep -Fq '@define-color quartz_scroll_thumb @quartz_application;' "$gtk4_theme_source" ||
  die "GTK 4 scrollbar thumb does not use the application background"
grep -Fq '/* Macintosh value rails:' "$gtk4_theme_source" ||
  die "GTK 4 theme does not define the Macintosh value rail"
grep -Fq 'background-image: url("../gtk-3.0/assets/scale-dither.xpm");' "$gtk4_theme_source" ||
  die "GTK 4 scale does not use the dithered rail background"
grep -Fq 'min-height: 14px;' "$gtk4_theme_source" ||
  die "GTK 4 scale does not preserve its 13px body plus shadow"
grep -Fq 'opacity: 0;' "$gtk4_theme_source" ||
  die "GTK 4 scale does not hide its puck"
grep -Fq 'scale:disabled {' "$gtk4_theme_source" ||
  die "GTK 4 theme does not target disabled scales"
grep -Fq 'opacity: 0.5;' "$gtk4_theme_source" ||
  die "GTK 4 disabled scale is not 50-percent opaque"

grep -Fqx 'Name=Quartz System 7 Cursors' "$cursor_theme_source/index.theme" ||
  die "cursor theme index has the wrong name"
grep -Fqx 'Inherits=Adwaita' "$cursor_theme_source/index.theme" ||
  die "cursor theme does not declare its modern-state fallback"
for index in "${!cursor_theme_base_files[@]}"; do
  cursor_theme_base_file="${cursor_theme_base_files[$index]}"
  cursor_theme_file="$cursor_theme_source/cursors/$cursor_theme_base_file"
  [[ -f "$cursor_theme_file" && ! -L "$cursor_theme_file" ]] ||
    die "native cursor is not a regular file: $cursor_theme_base_file"
  actual_cursor_sha256="$(sha256sum "$cursor_theme_file" | awk '{ print $1 }')"
  [[ "$actual_cursor_sha256" == "${cursor_theme_expected_sha256[$index]}" ]] ||
    die "unexpected $cursor_theme_base_file cursor checksum: $actual_cursor_sha256"
done
for cursor_theme_alias in "${cursor_theme_aliases[@]}"; do
  cursor_alias_name="${cursor_theme_alias%%:*}"
  cursor_alias_target="${cursor_theme_alias#*:}"
  cursor_alias_file="$cursor_theme_source/cursors/$cursor_alias_name"
  [[ -f "$cursor_alias_file" && ! -L "$cursor_alias_file" ]] ||
    die "cursor alias is not a regular file: $cursor_alias_name"
  cmp -s -- "$cursor_alias_file" "$cursor_theme_source/cursors/$cursor_alias_target" ||
    die "cursor alias $cursor_alias_name does not match $cursor_alias_target"
done

actual_icon_theme_sha256="$(sha256sum "$icon_theme_archive" | awk '{ print $1 }')"
[[ "$actual_icon_theme_sha256" == "$expected_icon_theme_sha256" ]] ||
  die "unexpected NineIcons48x.tar.gz checksum: $actual_icon_theme_sha256"
LC_ALL=C tar -tzf "$icon_theme_archive" | LC_ALL=C awk -v root="$icon_theme_name/" '
  index($0, root) != 1 || $0 ~ /(^|\/)\.\.(\/|$)/ { exit 1 }
' || die "icon theme archive contains an unsafe or unexpected path"
LC_ALL=C tar -tzf "$icon_theme_archive" |
  LC_ALL=C grep -Fx "$icon_theme_name/index.theme" >/dev/null ||
  die "icon theme archive does not contain $icon_theme_name/index.theme"

actual_font_sha256="$(sha256sum "$font_source" | awk '{ print $1 }')"
[[ "$actual_font_sha256" == "$expected_font_sha256" ]] ||
  die "unexpected ChiKareGo2.ttf checksum: $actual_font_sha256"

scanned_family="$(fc-scan --format '%{family[0]}' "$font_source")"
[[ "$scanned_family" == "$font_family" ]] ||
  die "font family is '$scanned_family', expected '$font_family'"

for index in "${!geneva_strike_sizes[@]}"; do
  strike_size="${geneva_strike_sizes[$index]}"
  native_pixels="${geneva_native_pixels[$index]}"
  geneva_source="$script_dir/fonts/system7-geneva-$strike_size.otb"
  expected_family="System 7 Geneva $strike_size"

  [[ -r "$geneva_source" ]] || die "Geneva strike not found: $geneva_source"

  actual_geneva_sha256="$(sha256sum "$geneva_source" | awk '{ print $1 }')"
  [[ "$actual_geneva_sha256" == "${geneva_expected_sha256[$index]}" ]] ||
    die "unexpected $(basename "$geneva_source") checksum: $actual_geneva_sha256"

  scanned_geneva="$(fc-scan --format '%{family[0]}|%{pixelsize}|%{scalable}|%{outline}' "$geneva_source")"
  IFS='|' read -r scanned_geneva_family scanned_geneva_pixels scanned_geneva_scalable scanned_geneva_outline <<<"$scanned_geneva"
  [[ "$scanned_geneva_family" == "$expected_family" ]] ||
    die "Geneva strike family is '$scanned_geneva_family', expected '$expected_family'"
  awk -v actual="$scanned_geneva_pixels" -v expected="$native_pixels" \
    'BEGIN { exit !(actual == expected) }' ||
    die "$expected_family has ${scanned_geneva_pixels}px cells, expected ${native_pixels}px"
  [[ "$scanned_geneva_scalable" == "False" || "$scanned_geneva_scalable" == "false" ]] ||
    die "$expected_family must be a non-scalable bitmap font"
  [[ "$scanned_geneva_outline" == "False" || "$scanned_geneva_outline" == "false" ]] ||
    die "$expected_family unexpectedly contains outlines"
done

terminal_font_match="$(fc-match \
  --format '%{family[0]}|%{pixelsize}|%{scalable}|%{outline}|%{antialias}' \
  "$monospace_font_family:size=$monospace_font_size_pt:dpi=96")"
IFS='|' read -r terminal_family terminal_pixels terminal_scalable terminal_outline terminal_antialias \
  <<<"$terminal_font_match"
[[ "$terminal_family" == "$monospace_font_family" ]] ||
  die "Terminus bitmap font is missing; install the fonts-terminus-otb package"
awk -v actual="$terminal_pixels" -v expected="$monospace_font_native_px" \
  'BEGIN { exit !(actual == expected) }' ||
  die "$terminal_family resolved at ${terminal_pixels}px, expected ${monospace_font_native_px}px"
[[ "$terminal_scalable" == "False" || "$terminal_scalable" == "false" ]] ||
  die "$terminal_family resolved to a scalable font"
[[ "$terminal_outline" == "False" || "$terminal_outline" == "false" ]] ||
  die "$terminal_family unexpectedly contains outlines"
[[ "$terminal_antialias" == "False" || "$terminal_antialias" == "false" ]] ||
  die "$terminal_family did not retain one-bit rendering"

schema_exists org.mate.Marco.general || die "MATE Marco settings are not installed"
schema_exists org.mate.interface || die "MATE interface settings are not installed"
schema_exists org.mate.peripherals-mouse || die "MATE mouse settings are not installed"
schema_exists org.mate.background || die "MATE desktop background settings are not installed"
schema_exists org.mate.caja.desktop || die "MATE desktop icon settings are not installed"
schema_exists org.mate.panel || die "MATE panel settings are not installed"
schema_exists org.mate.session.required-components ||
  die "MATE session component settings are not installed"
setting_is_writable org.mate.Marco.general titlebar-font ||
  die "org.mate.Marco.general titlebar-font is not writable"
setting_is_writable org.mate.Marco.general titlebar-uses-system-font ||
  die "org.mate.Marco.general titlebar-uses-system-font is not writable"
setting_is_writable org.mate.Marco.general theme ||
  die "org.mate.Marco.general theme is not writable"
setting_is_writable org.mate.Marco.general button-layout ||
  die "org.mate.Marco.general button-layout is not writable"
setting_is_writable org.mate.Marco.general compositing-manager ||
  die "org.mate.Marco.general compositing-manager is not writable"
setting_is_writable org.mate.session.required-components windowmanager ||
  die "org.mate.session.required-components windowmanager is not writable"
setting_is_writable org.mate.interface gtk-decoration-layout ||
  die "org.mate.interface gtk-decoration-layout is not writable"
setting_is_writable org.mate.interface gtk-theme ||
  die "org.mate.interface gtk-theme is not writable"
setting_is_writable org.mate.interface icon-theme ||
  die "org.mate.interface icon-theme is not writable"
setting_is_writable org.mate.peripherals-mouse cursor-theme ||
  die "org.mate.peripherals-mouse cursor-theme is not writable"
setting_is_writable org.mate.peripherals-mouse cursor-size ||
  die "org.mate.peripherals-mouse cursor-size is not writable"
setting_is_writable org.mate.interface font-name ||
  die "org.mate.interface font-name is not writable"
setting_is_writable org.mate.interface document-font-name ||
  die "org.mate.interface document-font-name is not writable"
setting_is_writable org.mate.interface monospace-font-name ||
  die "org.mate.interface monospace-font-name is not writable"
setting_is_writable org.mate.background show-desktop-icons ||
  die "org.mate.background show-desktop-icons is not writable"
setting_is_writable org.mate.caja.desktop font ||
  die "org.mate.caja.desktop font is not writable"

horizontal_panel_found=false
while IFS= read -r quoted_panel_id; do
  panel_id="${quoted_panel_id//\'/}"
  [[ -n "$panel_id" ]] || continue
  panel_schema="org.mate.panel.toplevel:/org/mate/panel/toplevels/$panel_id/"
  panel_orientation="$(gsettings get "$panel_schema" orientation)"
  if [[ "$panel_orientation" == "'top'" || "$panel_orientation" == "'bottom'" ]]; then
    setting_is_writable "$panel_schema" size ||
      die "MATE panel '$panel_id' height is not writable"
    panel_background_schema="org.mate.panel.toplevel.background:/org/mate/panel/toplevels/$panel_id/background/"
    for panel_background_key in type image fit stretch rotate opacity; do
      setting_is_writable "$panel_background_schema" "$panel_background_key" ||
        die "MATE panel '$panel_id' background key '$panel_background_key' is not writable"
    done
    horizontal_panel_found=true
  fi
done < <(gsettings get org.mate.panel toplevel-id-list | grep -o "'[^']*'")
[[ "$horizontal_panel_found" == true ]] || die "no top or bottom MATE panel was found"
global_antialias_before=""
if schema_exists org.mate.font-rendering; then
  global_antialias_before="$(gsettings get org.mate.font-rendering antialiasing)"
fi
terminal_use_system_font_before="$(gsettings get "$terminal_profile" use-system-font)"
terminal_font_before="$(gsettings get "$terminal_profile" font)"

install -d -m 0755 -- \
  "$icon_dir" "$cursor_dir" "$font_dir" "$fontconfig_dir" "$gtk3_dir" \
  "$gtk4_dir" "$theme_destination" "$quartz_config_dir"

[[ ! -L "$cursor_theme_destination" ]] ||
  die "cursor theme destination must not be a symbolic link: $cursor_theme_destination"
[[ ! -e "$cursor_theme_destination" || -d "$cursor_theme_destination" ]] ||
  die "cursor theme destination must be a directory: $cursor_theme_destination"
install -d -m 0755 -- "$cursor_theme_destination"
find "$cursor_theme_destination" -mindepth 1 -depth -delete
cp -R -- "$cursor_theme_source/." "$cursor_theme_destination/"
cmp -s -- "$cursor_theme_source/index.theme" "$cursor_theme_destination/index.theme" ||
  die "installed cursor theme index drifted"
for cursor_theme_base_file in "${cursor_theme_base_files[@]}"; do
  cmp -s -- \
    "$cursor_theme_source/cursors/$cursor_theme_base_file" \
    "$cursor_theme_destination/cursors/$cursor_theme_base_file" ||
    die "installed cursor drifted: $cursor_theme_base_file"
done
for cursor_theme_alias in "${cursor_theme_aliases[@]}"; do
  cursor_alias_name="${cursor_theme_alias%%:*}"
  cursor_alias_target="${cursor_theme_alias#*:}"
  cursor_alias_file="$cursor_theme_destination/cursors/$cursor_alias_name"
  [[ -f "$cursor_alias_file" && ! -L "$cursor_alias_file" ]] ||
    die "installed cursor alias is not a regular file: $cursor_alias_name"
  cmp -s -- "$cursor_alias_file" \
    "$cursor_theme_destination/cursors/$cursor_alias_target" ||
    die "installed cursor alias $cursor_alias_name drifted"
done

[[ ! -L "$icon_theme_destination" ]] ||
  die "icon theme destination must not be a symbolic link: $icon_theme_destination"
icon_theme_tmp="$(mktemp -d "$icon_dir/.$icon_theme_name.XXXXXX")"
tar --extract --gzip --file "$icon_theme_archive" \
  --directory "$icon_theme_tmp" --no-same-owner \
  --exclude="$icon_theme_name/Iconfactory" \
  --exclude="$icon_theme_name/apps/16/Internet Explorer_2_16x16x8.png"
icon_theme_staged="$icon_theme_tmp/$icon_theme_name"
install -m 0644 -- "$icon_theme_index_source" "$icon_theme_staged/index.theme"
# Small desktop toolbar slots use native 16px artwork on padded canvases.
for icon_slot in 22 24; do
  install -d -m 0755 -- "$icon_theme_staged/places/$icon_slot"
  install -m 0644 -- "$script_dir/icons/overrides/places/$icon_slot/user-desktop.svg" \
    "$icon_theme_staged/places/$icon_slot/user-desktop.svg"
done
[[ -r "$icon_theme_staged/index.theme" ]] ||
  die "installed icon theme index is missing"
grep -Fqx "Name=$icon_theme_name" "$icon_theme_staged/index.theme" ||
  die "installed icon theme has the wrong name"
cmp -s -- "$icon_theme_index_source" "$icon_theme_staged/index.theme" ||
  die "installed icon theme index drifted"
for icon_theme_probe in \
  apps/32/application-default-icon.png \
  mimes/48/text-x-generic.png \
  places/48/folder.png; do
  [[ -r "$icon_theme_staged/$icon_theme_probe" ]] ||
    die "installed icon theme is missing $icon_theme_probe"
done
gtk-update-icon-cache --force --quiet "$icon_theme_staged"
install -d -m 0755 -- "$icon_theme_destination"
cp -R -- "$icon_theme_staged/." "$icon_theme_destination/"
cmp -s -- "$icon_theme_index_source" "$icon_theme_destination/index.theme" ||
  die "installed icon theme index drifted after deployment"
gtk-update-icon-cache --validate "$icon_theme_destination"
find "$icon_theme_tmp" -depth -delete
icon_theme_tmp=""

if [[ ! -f "$font_destination" ]] || ! cmp -s -- "$font_source" "$font_destination"; then
  install -m 0644 -- "$font_source" "$font_destination"
fi

for strike_size in "${geneva_strike_sizes[@]}"; do
  geneva_source="$script_dir/fonts/system7-geneva-$strike_size.otb"
  geneva_destination="$font_dir/system7-geneva-$strike_size.otb"
  if [[ ! -f "$geneva_destination" ]] || ! cmp -s -- "$geneva_source" "$geneva_destination"; then
    install -m 0644 -- "$geneva_source" "$geneva_destination"
  fi
done

cp -R -- "$theme_source_dir/." "$theme_destination/"
install -m 0644 -- "$theme_source_dir/theme.conf" "$quartz_theme_config"
for obsolete_theme_relative_file in "${obsolete_theme_relative_files[@]}"; do
  obsolete_theme_file="$theme_destination/$obsolete_theme_relative_file"
  [[ ! -L "$obsolete_theme_file" ]] ||
    die "obsolete theme asset must not be a symbolic link: $obsolete_theme_file"
  [[ ! -e "$obsolete_theme_file" || -f "$obsolete_theme_file" ]] ||
    die "obsolete theme asset is not a regular file: $obsolete_theme_file"
  if [[ -f "$obsolete_theme_file" ]]; then
    unlink -- "$obsolete_theme_file"
  fi
done

make --no-print-directory -C "$settings_source_dir" clean
make --no-print-directory -C "$settings_source_dir" \
  PREFIX="$settings_prefix" install

fontconfig_tmp="$(mktemp "$fontconfig_dir/.99-quartz-chikarego2.conf.XXXXXX")"
cat >"$fontconfig_tmp" <<EOF
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
  <!-- ChiKareGo2 traces Chicago's single 12-point/16-pixel screen design.
       Pango represents an absolute 16px CSS size with a synthetic 16-point
       field as well as the authoritative 16-pixel field. Normalize that case
       before examining point sizes so native menu text remains ChiKareGo2. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="pixelsize" compare="more_eq"><double>$font_native_size_px_min</double></test>
    <test name="pixelsize" compare="less_eq"><double>$font_native_size_px_max</double></test>
    <edit name="size" mode="assign"><double>$font_native_size_pt</double></edit>
    <edit name="pixelsize" mode="assign"><double>$font_native_size_px</double></edit>
  </match>

  <!-- GTK 2 can express only the native 12pt request, then derive a slightly
       different pixel size from the live X display DPI. Treat that point size
       as authoritative too and preserve the same exact 16-pixel strike. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="size" compare="more_eq"><double>$font_native_size_pt_min</double></test>
    <test name="size" compare="less_eq"><double>$font_native_size_pt_max</double></test>
    <edit name="size" mode="assign"><double>$font_native_size_pt</double></edit>
    <edit name="pixelsize" mode="assign"><double>$font_native_size_px</double></edit>
  </match>

  <!-- Redirect every explicit non-native request to the Geneva alias before
       font matching, where it will snap to a genuine retained bitmap strike.
       The narrow ranges tolerate point/pixel conversion noise only. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="size" compare="less"><double>$font_native_size_pt_min</double></test>
    <edit name="family" mode="assign" binding="strong"><string>$geneva_family</string></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="size" compare="more"><double>$font_native_size_pt_max</double></test>
    <edit name="family" mode="assign" binding="strong"><string>$geneva_family</string></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="pixelsize" compare="less"><double>$font_native_size_px_min</double></test>
    <edit name="family" mode="assign" binding="strong"><string>$geneva_family</string></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <test name="pixelsize" compare="more"><double>$font_native_size_px_max</double></test>
    <edit name="family" mode="assign" binding="strong"><string>$geneva_family</string></edit>
  </match>

  <!-- A request still naming ChiKareGo2 is native-sized or omitted its size.
       Canonicalize both dimensions so DPI defaults cannot move it off-grid. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>$font_family</string></test>
    <edit name="size" mode="assign"><double>$font_native_size_pt</double></edit>
    <edit name="pixelsize" mode="assign"><double>$font_native_size_px</double></edit>
  </match>

  <!-- Keep desktop-wide rendering unchanged; rasterize only this pixel face
       without grayscale or subpixel smoothing. -->
  <match target="font">
    <test name="family" compare="eq" qual="any">
      <string>$font_family</string>
    </test>
    <edit name="antialias" mode="assign">
      <bool>false</bool>
    </edit>
    <edit name="hinting" mode="assign">
      <bool>false</bool>
    </edit>
    <edit name="autohint" mode="assign">
      <bool>false</bool>
    </edit>
    <edit name="rgba" mode="assign">
      <const>none</const>
    </edit>
  </match>
</fontconfig>
EOF
replace_file_if_changed "$fontconfig_tmp" "$fontconfig_file"
fontconfig_tmp=""

fontconfig_tmp="$(mktemp "$fontconfig_dir/.99-quartz-geneva.conf.XXXXXX")"
cat >"$fontconfig_tmp" <<'EOF'
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
  <!-- Ubuntu rejects non-scalable fonts by default. Whitelist only the six
       original System 7 Geneva strikes used by the Quartz application role. -->
  <selectfont>
    <acceptfont>
      <pattern><patelt name="family"><string>System 7 Geneva 9</string></patelt></pattern>
      <pattern><patelt name="family"><string>System 7 Geneva 10</string></patelt></pattern>
      <pattern><patelt name="family"><string>System 7 Geneva 12</string></patelt></pattern>
      <pattern><patelt name="family"><string>System 7 Geneva 14</string></patelt></pattern>
      <pattern><patelt name="family"><string>System 7 Geneva 18</string></patelt></pattern>
      <pattern><patelt name="family"><string>System 7 Geneva 24</string></patelt></pattern>
    </acceptfont>
  </selectfont>

  <!-- Geneva has regular one-bit strikes only. Never synthesize weight or
       slant, because either transformation changes the source pixels. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <edit name="weight" mode="assign"><const>regular</const></edit>
    <edit name="slant" mode="assign"><const>roman</const></edit>
    <edit name="width" mode="assign"><const>normal</const></edit>
  </match>

  <!-- Absolute pixel requests are dispatched first. The boundaries are the
       96-DPI pixel equivalents of the point-size boundaries below; output
       pixel sizes are always native BDF cells, never resampled values. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="less"><double>12.6666667</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 9</string></edit>
    <edit name="size" mode="assign"><double>9</double></edit>
    <edit name="pixelsize" mode="assign"><double>12</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="more_eq"><double>12.6666667</double></test>
    <test name="pixelsize" compare="less"><double>14.6666667</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 10</string></edit>
    <edit name="size" mode="assign"><double>9</double></edit>
    <edit name="pixelsize" mode="assign"><double>12</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="more_eq"><double>14.6666667</double></test>
    <test name="pixelsize" compare="less"><double>17.3333333</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 12</string></edit>
    <edit name="size" mode="assign"><double>11.25</double></edit>
    <edit name="pixelsize" mode="assign"><double>15</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="more_eq"><double>17.3333333</double></test>
    <test name="pixelsize" compare="less"><double>21.3333333</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 14</string></edit>
    <edit name="size" mode="assign"><double>13.5</double></edit>
    <edit name="pixelsize" mode="assign"><double>18</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="more_eq"><double>21.3333333</double></test>
    <test name="pixelsize" compare="less"><double>28</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 18</string></edit>
    <edit name="size" mode="assign"><double>16.5</double></edit>
    <edit name="pixelsize" mode="assign"><double>22</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="pixelsize" compare="more_eq"><double>28</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 24</string></edit>
    <edit name="size" mode="assign"><double>21</double></edit>
    <edit name="pixelsize" mode="assign"><double>28</double></edit>
  </match>

  <!-- Snap every point-sized Geneva request to the nearest original strike.
       Pixel sizes are the native BDF cells, not resampled point conversions. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="less"><double>9.5</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 9</string></edit>
    <edit name="size" mode="assign"><double>9</double></edit>
    <edit name="pixelsize" mode="assign"><double>12</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="more_eq"><double>9.5</double></test>
    <test name="size" compare="less"><double>11</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 10</string></edit>
    <edit name="size" mode="assign"><double>9</double></edit>
    <edit name="pixelsize" mode="assign"><double>12</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="more_eq"><double>11</double></test>
    <test name="size" compare="less"><double>13</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 12</string></edit>
    <edit name="size" mode="assign"><double>11.25</double></edit>
    <edit name="pixelsize" mode="assign"><double>15</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="more_eq"><double>13</double></test>
    <test name="size" compare="less"><double>16</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 14</string></edit>
    <edit name="size" mode="assign"><double>13.5</double></edit>
    <edit name="pixelsize" mode="assign"><double>18</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="more_eq"><double>16</double></test>
    <test name="size" compare="less"><double>21</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 18</string></edit>
    <edit name="size" mode="assign"><double>16.5</double></edit>
    <edit name="pixelsize" mode="assign"><double>22</double></edit>
  </match>
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <test name="size" compare="more_eq"><double>21</double></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 24</string></edit>
    <edit name="size" mode="assign"><double>21</double></edit>
    <edit name="pixelsize" mode="assign"><double>28</double></edit>
  </match>

  <!-- A family-only request has no size to compare; use Geneva 12. -->
  <match target="pattern">
    <test name="family" compare="eq" qual="any"><string>Geneva</string></test>
    <edit name="family" mode="assign" binding="strong"><string>System 7 Geneva 12</string></edit>
    <edit name="size" mode="assign"><double>11.25</double></edit>
    <edit name="pixelsize" mode="assign"><double>15</double></edit>
  </match>

  <!-- Preserve desktop-wide rendering. Only the selected Geneva bitmap gets
       the one-bit raster policy, and bitmap scaling remains at 1:1. -->
  <match target="font">
    <test name="family" compare="contains" qual="any"><string>System 7 Geneva </string></test>
    <edit name="antialias" mode="assign"><bool>false</bool></edit>
    <edit name="embeddedbitmap" mode="assign"><bool>true</bool></edit>
    <edit name="embolden" mode="assign"><bool>false</bool></edit>
    <edit name="hinting" mode="assign"><bool>false</bool></edit>
    <edit name="autohint" mode="assign"><bool>false</bool></edit>
    <edit name="rgba" mode="assign"><const>none</const></edit>
    <edit name="lcdfilter" mode="assign"><const>lcdnone</const></edit>
  </match>
</fontconfig>
EOF
replace_file_if_changed "$fontconfig_tmp" "$geneva_fontconfig_file"
fontconfig_tmp=""

gtk3_tmp="$(mktemp "$gtk3_dir/.gtk.css.XXXXXX")"
if [[ -f "$gtk3_css" ]]; then
  awk -v begin="$css_begin" -v end="$css_end" '
    $0 == begin { managed = 1; next }
    $0 == end   { managed = 0; next }
    !managed    { lines[++count] = $0 }
    END {
      while (count > 0 && lines[count] ~ /^[[:space:]]*$/) {
        count--
      }
      for (line = 1; line <= count; line++) {
        print lines[line]
      }
    }
  ' "$gtk3_css" >"$gtk3_tmp"
fi

if [[ -s "$gtk3_tmp" ]]; then
  printf '\n\n' >>"$gtk3_tmp"
fi

cat >>"$gtk3_tmp" <<EOF
$css_begin
/*
 * ChiKareGo2 uses a 16-pixel em grid. At 16 logical pixels (or 12 points
 * at the snapshot's 96 DPI), its outlines land exactly on the pixel grid.
 * Keep the family on panel descendants, but let an explicit smaller size
 * reach Fontconfig so it can select an unscaled Geneva bitmap strike.
 */
.mate-panel-menu-bar,
.mate-panel-menu-bar * {
  font-family: "$font_family";
  font-style: normal;
  font-weight: normal;
}

.mate-panel-menu-bar {
  font-size: ${panel_font_size_px}px;
}

/* Menubars and command-menu shells default to native Chicago. Descendant
 * relative sizes remain free to trigger the Geneva fallback policy. */
window.background menubar,
dialog.background menubar,
menu,
.menu,
.context-menu,
popover.menu,
popover.background.menu {
  font-family: "$font_family";
  font-size: ${panel_font_size_px}px;
  font-style: normal;
  font-weight: normal;
}

headerbar.titlebar label.title,
.titlebar .title {
  font-family: "$font_family";
  font-size: ${panel_font_size_px}px;
  font-style: normal;
  font-weight: normal;
}

/* Flat, square GTK client-side frames and shared dialog surfaces. Main
 * application windows receive their color from the selected theme provider;
 * this user provider does not override toolkit-owned desktop transparency. */
dialog.background,
messagedialog.background,
dialog.background > box,
messagedialog.background > box,
dialog.background .background,
messagedialog.background .background {
  background-color: @quartz_application;
  background-image: none;
}

/* Generic content canvases share the chosen application background. */
viewport,
textview text,
vte-terminal {
  background-color: @quartz_application;
  background-image: none;
}

window.background menubar,
dialog.background menubar {
  background-color: @quartz_menubar;
  background-image: none;
  box-shadow: none;
}

window.background toolbar,
dialog.background toolbar,
window.background .toolbar,
dialog.background .toolbar {
  background-color: @quartz_application;
  background-image: none;
  box-shadow: none;
}

decoration,
window decoration,
window.csd decoration,
window.ssd decoration {
  margin: 0;
  padding: 0;
  border: 0;
  border-radius: 0;
  box-shadow: 0 0 0 1px #000000, 0 1px 0 1px #000000;
}

window,
window.csd,
window.ssd,
headerbar,
headerbar.titlebar,
.titlebar {
  border-radius: 0;
  box-shadow: none;
}

tooltip,
.tooltip {
  color: #000000;
  background-color: #ffecb3;
  border: 1px solid #000000;
  border-radius: ${border_radius_css};
  box-shadow: none;
}

tooltip label,
.tooltip label {
  color: inherit;
}

/* GTK clips decoration shadows on non-composited CSD surfaces. Keep the
 * visible frame one pixel on the top/left/right and reserve the offset shadow
 * as a second solid pixel on the bottom. */
window.csd,
window.solid-csd,
dialog.csd,
messagedialog.csd {
  border-color: #000000;
  border-style: solid;
  border-width: 1px 1px 2px 1px;
}

/* Keep every GTK menu surface rounded with a hard bottom-only shadow. These
 * user-priority rules follow the general window reset because toolkits and
 * applications may install later providers for their popup windows. */
menu,
.menu,
.context-menu {
  /* The theme-owned frame supplies the one-bit outline and the shared radius
   * clips GTK's edge-strip overdraw. The named color is resolved again
   * whenever the shared theme is reloaded. */
  background-color: @quartz_paper;
  background-clip: padding-box;
  background-image: none;
  border-color: transparent;
  border-style: solid;
  border-width: ${border_slice_px}px ${border_slice_px}px ${border_shadow_slice_px}px ${border_slice_px}px;
  /* The selected shared theme owns the recolored frame image. Keeping the
   * palette-aware image out of this long-lived user provider lets a normal
   * theme-name refresh recompute its color without rewriting user CSS. */
  border-image-slice: $border_slice_px $border_slice_px $border_shadow_slice_px $border_slice_px fill;
  border-image-width: ${border_slice_px}px ${border_slice_px}px ${border_shadow_slice_px}px ${border_slice_px}px;
  border-image-repeat: stretch;
  border-radius: ${border_radius_css};
  box-shadow: none;
  padding: 0;
}

window.popup {
  background-color: transparent;
  border-radius: ${border_radius_css};
}

window.popup decoration {
  margin: 0;
  border: 0;
  padding: 0;
  border-radius: ${border_radius_css};
  box-shadow: none;
}

popover,
popover.background {
  /* A popover's arrow needs its native border, including on .menu nodes. */
  background-clip: border-box;
  border-image: none;
  border: 1px solid #000000;
  border-radius: ${border_radius_css};
  box-shadow: 0 1px 0 0 #000000;
  padding: 1px;
}

tooltip decoration,
window.popup.tooltip decoration,
.tooltip decoration {
  margin: 0;
  padding: 0;
  border: 0;
  box-shadow: none;
}

headerbar,
headerbar.titlebar,
.titlebar {
  background-color: @quartz_titlebar;
  background-image: none;
}

/* Match the measured 1 + ${menubar_interior_height_px} + 1 System 7 menubar
 * geometry even when an application installs a provider at GTK's
 * application priority. The frame supplies the upper separator, so reserve
 * the menubar's first pixel in its face color instead of drawing a second
 * black line. */
window.background menubar,
dialog.background menubar {
  border-top: 1px solid @quartz_menubar;
  border-bottom: 1px solid #000000;
  min-height: ${menubar_interior_height_px}px;
  padding: 0;
}

window.background menubar > menuitem,
dialog.background menubar > menuitem {
  min-height: ${menubar_interior_height_px}px;
  padding-top: 0;
  padding-bottom: 0;
}

/* Out-of-process panel applets can install a late local provider that clears
 * borders or omit MATE's transient custom-background class after a layout
 * reset. These stable shared-panel selectors keep every GtkPlug inside both
 * rules without importing a host-only theme path (which strict snaps cannot
 * read). */
window.background.horizontal.mate-panel-menu-bar.gnome-panel-menu-bar {
  border-top: 1px solid #000000;
  border-bottom: 1px solid #000000;
}

window.background.horizontal.mate-panel-menu-bar.gnome-panel-menu-bar menubar {
  border-top: 1px solid #000000;
  border-bottom: 1px solid #000000;
}

/* Expanding in-process applets live directly below the horizontal panel and
 * can obscure its outer rule rows. Give every such shared applet surface the
 * same two rules at user priority, above applet-local providers. */
PanelToplevel.horizontal PanelApplet {
  border-top: 1px solid #000000;
  border-bottom: 1px solid #000000;
  min-height: 0;
  padding: 0;
}

PanelToplevel menubar {
  background-color: @quartz_menubar;
  background-image: none;
  border-top: 1px solid #000000;
  border-right: 0;
  border-bottom: 1px solid #000000;
  border-left: 0;
  min-height: 0;
  padding-top: 0;
  padding-bottom: 0;
}

window.mate-panel-menu-bar.mate-panel-menu-bar button,
window.gnome-panel-menu-bar.gnome-panel-menu-bar button,
window.mate-panel-menu-bar button#showdesktop-button,
window.mate-panel-menu-bar button#tasklist-button {
  border-top: 1px solid #000000;
  border-right: 0;
  border-bottom: 1px solid #000000;
  border-left: 0;
  min-height: 0;
  min-width: 0;
  margin: 0;
  padding: 0 6px;
}

/* Select main-panel menu launchers without catching ordinary status controls. */
window.mate-panel-menu-bar PanelApplet > button.flat.toggle,
window.gnome-panel-menu-bar PanelApplet > button.flat.toggle {
  border-radius: 0;
}

/* Keep the bottom panel's button segments square at user priority too. */
PanelToplevel.bottom button,
button#showdesktop-button,
button#tasklist-button {
  border-radius: 0;
}
$css_end
EOF
if [[ ! -f "$gtk3_css" ]] || ! cmp -s -- "$gtk3_tmp" "$gtk3_css"; then
  gtk3_user_css_changed=true
fi
replace_file_if_changed "$gtk3_tmp" "$gtk3_css"
gtk3_tmp=""

awk \
  -v begin="$css_begin" \
  -v end="$css_end" \
  -v expected_radius="$border_radius_css" \
  -v expected_family="  font-family: \"$font_family\";" \
  -v expected_size="  font-size: ${panel_font_size_px}px;" \
  -v expected_menu_width="  border-width: ${border_slice_px}px ${border_slice_px}px ${border_shadow_slice_px}px ${border_slice_px}px;" \
  -v expected_menu_slice="  border-image-slice: $border_slice_px $border_slice_px $border_shadow_slice_px $border_slice_px fill;" '
    $0 == begin { managed = 1; next }
    $0 == end { managed = 0 }
    !managed { next }

    $0 == "decoration," { in_window_decoration = 1 }
    in_window_decoration && $0 == "  border: 0;" {
      window_outline_uses_extent = 1
    }
    in_window_decoration &&
      $0 == "  box-shadow: 0 0 0 1px #000000, 0 1px 0 1px #000000;" {
      window_shadow = 1
    }
    in_window_decoration && $0 == "}" { in_window_decoration = 0 }

    $0 == "window.csd," { in_csd = 1 }
    in_csd && $0 == "  border-width: 1px 1px 2px 1px;" { csd_border = 1 }
    in_csd && $0 == "}" { in_csd = 0 }

    $0 == "window.background menubar," { in_menu_font = 1 }
    in_menu_font && $0 == expected_family { menu_font_family = 1 }
    in_menu_font && $0 == expected_size { menu_font_size = 1 }
    in_menu_font && $0 == "}" { in_menu_font = 0 }

    $0 == "menu," { in_menu = 1 }
    in_menu && $0 == "  background-color: @quartz_paper;" { menu_fill = 1 }
    in_menu && $0 == "  background-clip: padding-box;" { menu_clip = 1 }
    in_menu && $0 == expected_menu_width { menu_width = 1 }
    in_menu && /border-image-source:/ { user_menu_image = 1 }
    in_menu && $0 == expected_menu_slice { menu_slice = 1 }
    in_menu && $0 == "  border-radius: " expected_radius ";" {
      menu_radius_clip = 1
    }
    in_menu && $0 == "}" { in_menu = 0 }

    $0 == "window.popup {" { in_popup = 1 }
    in_popup && $0 == "  background-color: transparent;" { popup_clear = 1 }
    in_popup && $0 == "  border-radius: " expected_radius ";" {
      popup_radius_clip = 1
    }
    in_popup && $0 == "}" { in_popup = 0 }

    $0 == "window.popup decoration {" { in_decoration = 1 }
    in_decoration && $0 == "  border-radius: " expected_radius ";" {
      decoration_radius_clip = 1
    }
    in_decoration && $0 == "  box-shadow: none;" { decoration_bare = 1 }
    in_decoration && $0 == "}" { in_decoration = 0 }

    $0 == "popover," { in_popover = 1 }
    in_popover && $0 == "  border-radius: " expected_radius ";" { popover_radius = 1 }
    in_popover && $0 == "  box-shadow: 0 1px 0 0 #000000;" { popover_shadow = 1 }
    in_popover && $0 == "}" { in_popover = 0 }

    $0 == "tooltip," { in_tooltip = 1 }
    in_tooltip && $0 == ".tooltip {" { tooltip_class = 1 }
    in_tooltip && $0 == "  color: #000000;" { tooltip_text = 1 }
    in_tooltip && $0 == "  background-color: #ffecb3;" { tooltip_face = 1 }
    in_tooltip && $0 == "  border: 1px solid #000000;" { tooltip_outline = 1 }
    in_tooltip && $0 == "  border-radius: " expected_radius ";" {
      tooltip_radius = 1
    }
    in_tooltip && $0 == "}" { in_tooltip = 0 }

    $0 == "tooltip decoration," { in_tooltip_decoration = 1 }
    in_tooltip_decoration && $0 == "window.popup.tooltip decoration," {
      tooltip_popup_decoration = 1
    }
    in_tooltip_decoration && $0 == ".tooltip decoration {" {
      tooltip_compatibility_decoration = 1
    }
    in_tooltip_decoration && $0 == "  border: 0;" {
      tooltip_bare_decoration = 1
    }
    in_tooltip_decoration && $0 == "  box-shadow: none;" {
      tooltip_no_decoration_shadow = 1
    }
    in_tooltip_decoration && $0 == "}" { in_tooltip_decoration = 0 }

    $0 == "window.mate-panel-menu-bar PanelApplet > button.flat.toggle," {
      in_panel_menu_button = 1
    }
    in_panel_menu_button &&
      $0 == "window.gnome-panel-menu-bar PanelApplet > button.flat.toggle {" {
        generic_panel_menu_button = 1
      }
    in_panel_menu_button && $0 == "  border-radius: 0;" {
      square_panel_menu_button = 1
    }
    in_panel_menu_button && $0 == "}" { in_panel_menu_button = 0 }

    $0 == "PanelToplevel.bottom button," { in_bottom_panel_buttons = 1 }
    in_bottom_panel_buttons && $0 == "button#showdesktop-button," {
      show_desktop_button = 1
    }
    in_bottom_panel_buttons && $0 == "button#tasklist-button {" {
      task_list_button = 1
    }
    in_bottom_panel_buttons && $0 == "  border-radius: 0;" {
      square_bottom_panel_buttons = 1
    }
    in_bottom_panel_buttons && $0 == "}" { in_bottom_panel_buttons = 0 }

    END {
      exit(window_outline_uses_extent && window_shadow && csd_border &&
        menu_font_family && menu_font_size && menu_fill && menu_clip && menu_width &&
        !user_menu_image && menu_slice && menu_radius_clip && popup_clear &&
        popup_radius_clip && decoration_radius_clip && decoration_bare &&
        popover_radius && popover_shadow && tooltip_class && tooltip_text &&
        tooltip_face && tooltip_outline && tooltip_radius &&
        tooltip_popup_decoration && tooltip_compatibility_decoration &&
        tooltip_bare_decoration && tooltip_no_decoration_shadow &&
        generic_panel_menu_button && square_panel_menu_button &&
        show_desktop_button && task_list_button &&
        square_bottom_panel_buttons ? 0 : 1)
    }
  ' "$gtk3_css" ||
  die "GTK 3 user-priority windows, popups, or panel-button geometry is incomplete"

gtk4_tmp="$(mktemp "$gtk4_dir/.gtk.css.XXXXXX")"
cat >"$gtk4_tmp" <<EOF
$gtk4_css_begin
/* User-priority import also themes GTK 4/libadwaita application chrome. */
@import url("file://$theme_destination/gtk-4.0/gtk.css");
$gtk4_css_end
EOF

if [[ -f "$gtk4_css" ]]; then
  awk -v begin="$gtk4_css_begin" -v end="$gtk4_css_end" '
    $0 == begin { managed = 1; next }
    $0 == end   { managed = 0; next }
    !managed    { lines[++count] = $0 }
    END {
      first = 1
      while (first <= count && lines[first] ~ /^[[:space:]]*$/) {
        first++
      }
      while (count >= first && lines[count] ~ /^[[:space:]]*$/) {
        count--
      }
      if (first <= count) {
        print ""
        for (line = first; line <= count; line++) {
          print lines[line]
        }
      }
    }
  ' "$gtk4_css" >>"$gtk4_tmp"
fi
if [[ ! -f "$gtk4_css" ]] || ! cmp -s -- "$gtk4_tmp" "$gtk4_css"; then
  gtk4_user_css_changed=true
fi
replace_file_if_changed "$gtk4_tmp" "$gtk4_css"
gtk4_tmp=""

gtk2_tmp="$(mktemp "$HOME/.gtkrc-2.0.XXXXXX")"
if [[ -f "$gtk2_rc" ]]; then
  awk -v begin="$gtk2_rc_begin" -v end="$gtk2_rc_end" '
    $0 == begin { managed = 1; next }
    $0 == end   { managed = 0; next }
    !managed    { lines[++count] = $0 }
    END {
      while (count > 0 && lines[count] ~ /^[[:space:]]*$/) {
        count--
      }
      for (line = 1; line <= count; line++) {
        print lines[line]
      }
    }
  ' "$gtk2_rc" >"$gtk2_tmp"
fi

if [[ -s "$gtk2_tmp" ]]; then
  printf '\n\n' >>"$gtk2_tmp"
fi
cat >>"$gtk2_tmp" <<EOF
$gtk2_rc_begin
gtk-theme-name = "$marco_theme_name"
gtk-font-name = "$geneva_family $geneva_default_size_pt"
gtk-icon-theme-name = "$icon_theme_name"
gtk-cursor-theme-name = "$cursor_theme_name"
gtk-cursor-theme-size = $cursor_theme_size_px
gtk-button-images = 1
gtk-menu-images = 1
gtk-enable-animations = 1
$gtk2_rc_end
EOF
replace_file_if_changed "$gtk2_tmp" "$gtk2_rc"
gtk2_tmp=""

# Xsession sources this shared environment before starting MATE and Marco.
# Marco reads this switch when its compositor starts; it disables generated
# shadows for every window type while leaving the theme's drawn edge intact.
xsession_tmp="$(mktemp "$HOME/.xsessionrc.XXXXXX")"
if [[ -f "$xsession_rc" ]]; then
  awk -v begin="$xsession_begin" -v end="$xsession_end" '
    $0 == begin { if (managed || seen++) exit 1; managed = 1; next }
    $0 == end { if (!managed) exit 1; managed = 0; next }
    !managed { print }
    END { if (managed) exit 1 }
  ' "$xsession_rc" > "$xsession_tmp" ||
    die "cannot safely update malformed managed block in $xsession_rc"
fi
cat >> "$xsession_tmp" <<EOF
$xsession_begin
# Keep compositing, but let the shared theme draw the only window shadow.
export META_DEBUG_NO_SHADOW=1
$xsession_end
EOF
sh -n "$xsession_tmp" || die "invalid session environment in $xsession_rc"
if [[ -f "$xsession_rc" ]] && cmp -s -- "$xsession_tmp" "$xsession_rc"; then
  unlink -- "$xsession_tmp"
else
  if [[ -f "$xsession_rc" ]]; then
    chmod --reference="$xsession_rc" "$xsession_tmp"
  fi
  mv -- "$xsession_tmp" "$xsession_rc"
fi
xsession_tmp=""
export META_DEBUG_NO_SHADOW=1

fc-cache -f "$font_dir" >/dev/null

runtime_dir="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}"
if [[ -S "$runtime_dir/bus" ]]; then
  export XDG_RUNTIME_DIR="$runtime_dir"
  export DBUS_SESSION_BUS_ADDRESS="unix:path=$runtime_dir/bus"
fi

gsettings set org.mate.Marco.general titlebar-uses-system-font false
gsettings set org.mate.Marco.general titlebar-font "$font_family $titlebar_font_size_pt"
gsettings set org.mate.Marco.general button-layout "$window_button_layout"
configured_window_manager="$(gsettings get \
  org.mate.session.required-components windowmanager)"
case "$configured_window_manager" in
  "'marco'"|"'marco-no-composite'")
    # The Cupertino, Mutiny, and Pantheon desktop layouts include a dock, and
    # MATE Tweak enables it only in a composited session. Standard Marco's
    # built-in compositor runs with generated shadows disabled by the shared
    # session environment above. Migrate only Marco's explicit
    # no-compositor personality; do not replace a different window manager.
    if [[ "$configured_window_manager" == "'marco-no-composite'" ||
          "$(gsettings get org.mate.Marco.general compositing-manager)" != "true" ]] ||
       ! marco_shadows_disabled; then
      marco_restart_required=true
    fi
    gsettings set org.mate.session.required-components windowmanager 'marco'
    gsettings set org.mate.Marco.general compositing-manager true
    ;;
  *)
    # Third-party compositor personalities own compositing independently.
    gsettings set org.mate.Marco.general compositing-manager false
    ;;
esac
gsettings set org.mate.Marco.general theme "$marco_theme_name"
gsettings set org.mate.interface gtk-decoration-layout "$window_button_layout"
select_shared_theme org.mate.interface gtk-theme gtk-3.0
gsettings set org.mate.interface icon-theme "$icon_theme_name"
gsettings set org.mate.peripherals-mouse cursor-theme "$cursor_theme_name"
gsettings set org.mate.peripherals-mouse cursor-size "$cursor_theme_size_px"
gsettings set org.mate.interface font-name "$geneva_family $geneva_default_size_pt"
gsettings set org.mate.interface document-font-name "$geneva_family $geneva_default_size_pt"
gsettings set org.mate.interface monospace-font-name \
  "$monospace_font_family $monospace_font_size_pt"
gsettings set org.mate.background show-desktop-icons true
gsettings set org.mate.caja.desktop font "$geneva_family $geneva_default_size_pt"
if setting_is_writable org.mate.interface use-custom-font; then
  gsettings set org.mate.interface use-custom-font true
fi
# GTK must allow functional spinner animations. Shared CSS still disables
# transitions and animations by default, opting in only active spinners.
if setting_is_writable org.mate.interface enable-animations; then
  gsettings set org.mate.interface enable-animations true
fi
if setting_is_writable org.mate.interface gtk-enable-animations; then
  gsettings set org.mate.interface gtk-enable-animations true
fi
if setting_is_writable org.mate.interface gtk-overlay-scrolling; then
  gsettings set org.mate.interface gtk-overlay-scrolling false
fi
gnome_gtk_theme_managed=false
gnome_icon_theme_managed=false
gnome_cursor_theme_managed=false
gnome_cursor_size_managed=false
gnome_overlay_scrolling_managed=false
gnome_button_layout_managed=false
gnome_monospace_font_managed=false
if schema_exists org.gnome.desktop.interface; then
  if setting_is_writable org.gnome.desktop.interface gtk-theme; then
    select_shared_theme org.gnome.desktop.interface gtk-theme gtk-4.0
    gnome_gtk_theme_managed=true
  fi
  if setting_is_writable org.gnome.desktop.interface icon-theme; then
    gsettings set org.gnome.desktop.interface icon-theme "$icon_theme_name"
    gnome_icon_theme_managed=true
  fi
  if setting_is_writable org.gnome.desktop.interface cursor-theme; then
    gsettings set org.gnome.desktop.interface cursor-theme "$cursor_theme_name"
    gnome_cursor_theme_managed=true
  fi
  if setting_is_writable org.gnome.desktop.interface cursor-size; then
    gsettings set org.gnome.desktop.interface cursor-size "$cursor_theme_size_px"
    gnome_cursor_size_managed=true
  fi
  if setting_is_writable org.gnome.desktop.interface font-name; then
    gsettings set org.gnome.desktop.interface font-name "$geneva_family $geneva_default_size_pt"
  fi
  if setting_is_writable org.gnome.desktop.interface document-font-name; then
    gsettings set org.gnome.desktop.interface document-font-name "$geneva_family $geneva_default_size_pt"
  fi
  if setting_is_writable org.gnome.desktop.interface monospace-font-name; then
    gsettings set org.gnome.desktop.interface monospace-font-name \
      "$monospace_font_family $monospace_font_size_pt"
    gnome_monospace_font_managed=true
  fi
  if setting_is_writable org.gnome.desktop.interface enable-animations; then
    gsettings set org.gnome.desktop.interface enable-animations true
  fi
  if setting_is_writable org.gnome.desktop.interface overlay-scrolling; then
    gsettings set org.gnome.desktop.interface overlay-scrolling false
    gnome_overlay_scrolling_managed=true
  fi
fi
if schema_exists org.gnome.desktop.wm.preferences &&
   setting_is_writable org.gnome.desktop.wm.preferences button-layout; then
  gsettings set org.gnome.desktop.wm.preferences button-layout "$window_button_layout"
  gnome_button_layout_managed=true
fi

# MATE Tweak replaces the complete panel settings tree when its layout menu is
# used. Apply the shared Quartz panel contract after all theme selection, then
# keep it synchronized for any horizontal panels created by later resets.
systemctl --user daemon-reload
systemctl --user enable "$settings_panel_service" >/dev/null
# A full install may replace the helper executable. Stop its old generation,
# perform one synchronous reconciliation, and then start exactly one watcher.
# This avoids racing a service-start apply against a second explicit refresh.
systemctl --user stop "$settings_panel_service" >/dev/null 2>&1 || true
"$settings_panel_helper" --refresh
systemctl --user start "$settings_panel_service"
matched_family="$(fc-match --format '%{family[0]}' "$font_family:pixelsize=$panel_font_size_px")"
matched_antialias="$(fc-match --format '%{antialias}' "$font_family:pixelsize=$panel_font_size_px")"
[[ "$matched_family" == "$font_family" ]] ||
  die "Fontconfig resolved '$font_family' to '$matched_family'"
[[ "$matched_antialias" == "False" || "$matched_antialias" == "false" ]] ||
  die "Fontconfig did not disable antialiasing for $font_family (value: $matched_antialias)"

font_probe_patterns=(
  "$font_family:dpi=96"
  "$font_family:size=8:dpi=96"
  "$font_family:size=10:dpi=96"
  "$font_family:size=11.98:dpi=96"
  "$font_family:size=12:dpi=96"
  "$font_family:size=12:pixelsize=15:dpi=90"
  "$font_family:size=12.02:dpi=96"
  "$font_family:size=13:dpi=96"
  "$font_family:size=18:dpi=96"
  "$font_family:size=24:dpi=96"
  "$font_family:pixelsize=10:dpi=96"
  "$font_family:pixelsize=12:dpi=96"
  "$font_family:pixelsize=13.3333:dpi=96"
  "$font_family:pixelsize=15:dpi=96"
  "$font_family:pixelsize=15.98:dpi=96"
  "$font_family:pixelsize=16:dpi=96"
  "$font_family:size=16:pixelsize=16:dpi=96"
  "$font_family:pixelsize=16.02:dpi=96"
  "$font_family:pixelsize=18:dpi=96"
  "$font_family:pixelsize=22:dpi=96"
  "$font_family:pixelsize=28:dpi=96"
)
font_probe_labels=(
  "unsized"
  "8pt" "10pt" "11.98pt" "12pt" "GTK 2 native 12pt" "12.02pt"
  "13pt" "18pt" "24pt"
  "10px" "12px" "13.3333px" "15px" "15.98px" "16px"
  "Pango absolute 16px" "16.02px" "18px" "22px" "28px"
)
font_probe_families=(
  "$font_family"
  "System 7 Geneva 9"
  "System 7 Geneva 10"
  "System 7 Geneva 12"
  "$font_family"
  "$font_family"
  "System 7 Geneva 12"
  "System 7 Geneva 14"
  "System 7 Geneva 18"
  "System 7 Geneva 24"
  "System 7 Geneva 9"
  "System 7 Geneva 9"
  "System 7 Geneva 10"
  "System 7 Geneva 12"
  "System 7 Geneva 12"
  "$font_family"
  "$font_family"
  "System 7 Geneva 12"
  "System 7 Geneva 14"
  "System 7 Geneva 18"
  "System 7 Geneva 24"
)
font_probe_pixels=(
  16
  12 12 15 16 16 15 18 22 28
  12 12 12 15 15 16 16 15 18 22 28
)
for index in "${!font_probe_patterns[@]}"; do
  requested_pattern="${font_probe_patterns[$index]}"
  request_label="${font_probe_labels[$index]}"
  expected_family="${font_probe_families[$index]}"
  expected_pixels="${font_probe_pixels[$index]}"

  configured_font="$(fc-pattern --config --default \
    --format '%{family[0]}|%{pixelsize}' "$requested_pattern")"
  IFS='|' read -r configured_family configured_pixels <<<"$configured_font"
  [[ "$configured_family" == "$expected_family" ]] ||
    die "$font_family $request_label mapped to '$configured_family', expected '$expected_family'"
  awk -v actual="$configured_pixels" -v expected="$expected_pixels" \
    'BEGIN { exit !(actual == expected) }' ||
    die "$font_family $request_label mapped to ${configured_pixels}px, expected ${expected_pixels}px"

  resolved_font="$(fc-match \
    --format '%{family[0]}|%{pixelsize}|%{scalable}|%{outline}|%{antialias}' \
    "$requested_pattern")"
  IFS='|' read -r resolved_family resolved_pixels resolved_scalable resolved_outline resolved_antialias <<<"$resolved_font"
  [[ "$resolved_family" == "$expected_family" ]] ||
    die "Fontconfig resolved $font_family $request_label to '$resolved_family', expected '$expected_family'"
  awk -v actual="$resolved_pixels" -v expected="$expected_pixels" \
    'BEGIN { exit !(actual == expected) }' ||
    die "$resolved_family resolved at ${resolved_pixels}px, expected ${expected_pixels}px"
  [[ "$resolved_antialias" == "False" || "$resolved_antialias" == "false" ]] ||
    die "$resolved_family did not retain one-bit rendering"

  if [[ "$expected_family" == "$font_family" ]]; then
    [[ "$resolved_scalable" == "True" || "$resolved_scalable" == "true" ]] ||
      die "$font_family did not retain its native outline at $request_label"
    [[ "$resolved_outline" == "True" || "$resolved_outline" == "true" ]] ||
      die "$font_family did not retain its native outline at $request_label"
  else
    [[ "$resolved_scalable" == "False" || "$resolved_scalable" == "false" ]] ||
      die "$resolved_family resolved to a scalable font"
    [[ "$resolved_outline" == "False" || "$resolved_outline" == "false" ]] ||
      die "$resolved_family resolved to an outline font"
  fi
done

geneva_probe_sizes=(8 10 11 13 16 21 72)
geneva_probe_strikes=(9 10 12 14 18 24 24)
geneva_probe_pixels=(12 12 15 18 22 28 28)
for index in "${!geneva_probe_sizes[@]}"; do
  requested_size="${geneva_probe_sizes[$index]}"
  expected_strike="${geneva_probe_strikes[$index]}"
  expected_pixels="${geneva_probe_pixels[$index]}"
  expected_family="System 7 Geneva $expected_strike"

  configured_geneva="$(fc-pattern --config --default \
    --format '%{family[0]}|%{pixelsize}|%{weight}|%{slant}' \
    "$geneva_family:size=$requested_size:dpi=96:weight=bold:slant=italic")"
  IFS='|' read -r configured_family configured_pixels configured_weight configured_slant <<<"$configured_geneva"
  [[ "$configured_family" == "$expected_family" ]] ||
    die "Geneva ${requested_size}pt mapped to '$configured_family', expected '$expected_family'"
  awk -v actual="$configured_pixels" -v expected="$expected_pixels" \
    'BEGIN { exit !(actual == expected) }' ||
    die "Geneva ${requested_size}pt mapped to ${configured_pixels}px, expected ${expected_pixels}px"
  [[ "$configured_weight" == "80" && "$configured_slant" == "0" ]] ||
    die "Geneva ${requested_size}pt retained a synthetic weight or slant"

  resolved_geneva="$(fc-match --format '%{family[0]}|%{pixelsize}|%{scalable}|%{outline}|%{antialias}' \
    "$geneva_family:size=$requested_size:dpi=96:weight=bold:slant=italic")"
  IFS='|' read -r resolved_family resolved_pixels resolved_scalable resolved_outline resolved_antialias <<<"$resolved_geneva"
  [[ "$resolved_family" == "$expected_family" ]] ||
    die "Fontconfig resolved Geneva ${requested_size}pt to '$resolved_family'"
  awk -v actual="$resolved_pixels" -v expected="$expected_pixels" \
    'BEGIN { exit !(actual == expected) }' ||
    die "$resolved_family resolved at ${resolved_pixels}px, expected ${expected_pixels}px"
  [[ "$resolved_scalable" == "False" || "$resolved_scalable" == "false" ]] ||
    die "$resolved_family resolved to a scalable font"
  [[ "$resolved_outline" == "False" || "$resolved_outline" == "false" ]] ||
    die "$resolved_family resolved to an outline font"
  [[ "$resolved_antialias" == "False" || "$resolved_antialias" == "false" ]] ||
    die "$resolved_family did not retain one-bit rendering"
done

[[ "$(gsettings get org.mate.interface font-name)" == "'$geneva_family $geneva_default_size_pt'" ]] ||
  die "MATE application font did not change to $geneva_family"
[[ "$(gsettings get org.mate.interface document-font-name)" == "'$geneva_family $geneva_default_size_pt'" ]] ||
  die "MATE document font did not change to $geneva_family"
[[ "$(gsettings get org.mate.interface monospace-font-name)" == "'$monospace_font_family $monospace_font_size_pt'" ]] ||
  die "MATE monospace font did not change to $monospace_font_family"
[[ "$(gsettings get org.mate.caja.desktop font)" == "'$geneva_family $geneva_default_size_pt'" ]] ||
  die "MATE desktop icon font did not change to $geneva_family"
[[ "$(gsettings get org.mate.interface gtk-theme)" == "'$marco_theme_name'" ]] ||
  die "MATE did not select the $marco_theme_name GTK theme"
[[ "$(gsettings get org.mate.Marco.general button-layout)" == "'$window_button_layout'" ]] ||
  die "Marco did not retain the selected $window_layout_label button layout"
[[ "$(gsettings get org.mate.interface gtk-decoration-layout)" == "'$window_button_layout'" ]] ||
  die "GTK did not retain the selected $window_layout_label button layout"
[[ "$(gsettings get org.mate.interface icon-theme)" == "'$icon_theme_name'" ]] ||
  die "MATE did not select the $icon_theme_name icon theme"
[[ "$(gsettings get org.mate.peripherals-mouse cursor-theme)" == "'$cursor_theme_name'" ]] ||
  die "MATE did not select the $cursor_theme_name cursor theme"
[[ "$(gsettings get org.mate.peripherals-mouse cursor-size)" == "$cursor_theme_size_px" ]] ||
  die "MATE did not select the native ${cursor_theme_size_px}px cursor size"
[[ "$(gsettings get org.mate.background show-desktop-icons)" == "true" ]] ||
  die "MATE did not enable the shared desktop surface"
if [[ "$gnome_gtk_theme_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface gtk-theme)" == "'$marco_theme_name'" ]] ||
    die "GNOME/GTK 4 did not select the $marco_theme_name GTK theme"
fi
if [[ "$gnome_icon_theme_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface icon-theme)" == "'$icon_theme_name'" ]] ||
    die "GNOME/GTK 4 did not select the $icon_theme_name icon theme"
fi
if [[ "$gnome_cursor_theme_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface cursor-theme)" == "'$cursor_theme_name'" ]] ||
    die "GNOME/GTK 4 did not select the $cursor_theme_name cursor theme"
fi
if [[ "$gnome_cursor_size_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface cursor-size)" == "$cursor_theme_size_px" ]] ||
    die "GNOME/GTK 4 did not select the native ${cursor_theme_size_px}px cursor size"
fi
if [[ "$gnome_overlay_scrolling_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface overlay-scrolling)" == "false" ]] ||
    die "GNOME/GTK 4 overlay scrolling remained enabled"
fi
if [[ "$gnome_button_layout_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.wm.preferences button-layout)" == "'$window_button_layout'" ]] ||
    die "GNOME-compatible clients did not retain the selected $window_layout_label button layout"
fi
if [[ "$gnome_monospace_font_managed" == "true" ]]; then
  [[ "$(gsettings get org.gnome.desktop.interface monospace-font-name)" == "'$monospace_font_family $monospace_font_size_pt'" ]] ||
    die "GNOME-compatible clients did not select $monospace_font_family for monospace text"
fi
[[ "$(gsettings get "$terminal_profile" use-system-font)" == "$terminal_use_system_font_before" ]] ||
  die "MATE Terminal's system-font selection changed unexpectedly"
[[ "$(gsettings get "$terminal_profile" font)" == "$terminal_font_before" ]] ||
  die "MATE Terminal's configured font changed unexpectedly"

for relative_theme_file in \
  index.theme theme.conf gtk-2.0/gtkrc gtk-3.0/gtk.css gtk-4.0/gtk.css \
  metacity-1/metacity-theme-1.xml; do
  cmp -s -- "$theme_source_dir/$relative_theme_file" "$theme_destination/$relative_theme_file" ||
    die "installed theme file drifted: $relative_theme_file"
done
for obsolete_theme_relative_file in "${obsolete_theme_relative_files[@]}"; do
  [[ ! -e "$theme_destination/$obsolete_theme_relative_file" &&
    ! -L "$theme_destination/$obsolete_theme_relative_file" ]] ||
    die "obsolete theme asset remains installed: $obsolete_theme_relative_file"
done
for theme_asset in \
  "$theme_source_dir"/gtk-2.0/assets/* \
  "$theme_source_dir"/gtk-3.0/assets/*; do
  relative_theme_file="${theme_asset#"$theme_source_dir"/}"
  cmp -s -- "$theme_asset" "$theme_destination/$relative_theme_file" ||
    die "installed theme asset drifted: $relative_theme_file"
done
[[ "$(grep -Fxc -- "$gtk2_rc_begin" "$gtk2_rc")" == "1" ]] ||
  die "GTK 2 managed settings are missing or duplicated"
[[ "$(grep -Fxc -- "$gtk2_rc_end" "$gtk2_rc")" == "1" ]] ||
  die "GTK 2 managed settings are malformed"
[[ "$(grep -Fxc -- "$gtk4_css_begin" "$gtk4_css")" == "1" ]] ||
  die "GTK 4 user-priority import is missing or duplicated"
[[ "$(grep -Fxc -- "$gtk4_css_end" "$gtk4_css")" == "1" ]] ||
  die "GTK 4 user-priority import is malformed"
grep -Fq "@import url(\"file://$theme_destination/gtk-4.0/gtk.css\");" "$gtk4_css" ||
  die "GTK 4 user-priority theme import is missing"

for settings_binary in \
  "$settings_launcher" \
  "$settings_apply_helper" \
  "$settings_panel_helper" \
  "$settings_libexec/quartz-gtk2-audit" \
  "$settings_libexec/quartz-gtk3-audit" \
  "$settings_libexec/quartz-gtk4-audit"; do
  [[ -x "$settings_binary" ]] ||
    die "Quartz Settings binary was not installed: $settings_binary"
done
[[ -r "$settings_panel_service_file" ]] ||
  die "Quartz panel synchronization service was not installed"
systemctl --user is-enabled --quiet "$settings_panel_service" ||
  die "Quartz panel synchronization service is not enabled"
systemctl --user is-active --quiet "$settings_panel_service" ||
  die "Quartz panel synchronization service is not active"
"$settings_panel_helper" --check ||
  die "Quartz panel synchronization service did not retain the shared panel theme"
for settings_theme_file in \
  render-theme.sh theme.conf \
  gtk-2.0/gtkrc.in gtk-3.0/gtk.css.in gtk-4.0/gtk.css.in \
  metacity-1/metacity-theme-1.xml; do
  [[ -r "$settings_theme_data/$settings_theme_file" ]] ||
    die "Quartz Settings theme source was not installed: $settings_theme_file"
done
[[ -x "$settings_theme_data/render-theme.sh" ]] ||
  die "Quartz Settings theme renderer is not executable"
cmp -s -- "$theme_source_dir/theme.conf" "$quartz_theme_config" ||
  die "Quartz Settings theme configuration drifted during installation"
[[ -r "$settings_desktop" ]] ||
  die "Quartz Settings desktop entry was not installed"
grep -Fqx "Name=Quartz Settings" "$settings_desktop" ||
  die "Quartz Settings desktop entry has the wrong name"
grep -Fqx "Exec=$settings_launcher" "$settings_desktop" ||
  die "Quartz Settings desktop entry has the wrong executable"
ldd "$settings_libexec/quartz-gtk2-audit" | grep -F "libgtk-x11-2.0.so.0" >/dev/null ||
  die "GTK 2 audit is not linked to native GTK 2"
ldd "$settings_libexec/quartz-gtk3-audit" | grep -F "libgtk-3.so.0" >/dev/null ||
  die "GTK 3 audit is not linked to native GTK 3"
ldd "$settings_libexec/quartz-gtk4-audit" | grep -F "libgtk-4.so.1" >/dev/null ||
  die "GTK 4 audit is not linked to native GTK 4"

if [[ -n "$global_antialias_before" ]]; then
  global_antialias_after="$(gsettings get org.mate.font-rendering antialiasing)"
  [[ "$global_antialias_after" == "$global_antialias_before" ]] ||
    die "the global antialiasing setting changed unexpectedly"
fi

# Install additions before refreshing Marco so its font map can see new faces.
# Compare the pinned collection, not timestamps, to avoid repeated restarts.
if ! cmp -s "$script_dir/fonts/community/outputs.json" \
    "$data_home/doc/quartz-community-fonts/outputs.json"; then
  marco_restart_required=true
fi
bash "$script_dir/fonts/install-pixel-fonts.sh"

marco_refresh_result="none"
if [[ "$marco_restart_required" == "true" ]]; then
  if restart_marco_if_running; then
    # The replaced process may still appear in /proc briefly after SIGTERM.
    for _attempt in {1..50}; do
      marco_shadows_disabled && break
      sleep 0.1
    done
    marco_shadows_disabled || die "live Marco did not disable compositor shadows"
    marco_refresh_result="restarted"
  fi
elif reload_marco_theme_if_running; then
  marco_refresh_result="reloaded"
fi

install_extras_if_present "$script_dir/quartz-extras"

printf 'Applied Quartz System 6 theme for %s.\n' "$(id -un)"
printf '  Panel/menu text: %s %spt/%spx; other sizes use Geneva strikes\n' \
  "$font_family" "$font_native_size_pt" "$font_native_size_px"
printf '  Application menus: %spx (face-color top reserve + %s + black bottom)\n' \
  "$menubar_height_px" "$menubar_interior_height_px"
printf '  Desktop panels:    %spx (black top + %s + black bottom)\n' \
  "$panel_height_px" "$menubar_interior_height_px"
printf '  Panel layout guard: active across MATE Tweak layout resets\n'
printf '  Desktop menu:      enabled through the shared MATE desktop surface\n'
printf '  Desktop icons:     %s %spt\n' "$geneva_family" "$geneva_default_size_pt"
printf '  Window titles:   %s %spt, antialiasing off\n' "$font_family" "$titlebar_font_size_pt"
printf '  Window layout:   %s\n' "$window_layout_label"
printf '  Application UI:  %s 12 (native 9/10/12/14/18/24 strikes)\n' "$geneva_family"
printf '  Monospace font:  %s %spt/%spx, one-bit bitmap rendering\n' \
  "$monospace_font_family" "$monospace_font_size_pt" "$monospace_font_native_px"
printf '  Terminal profile: unchanged (%s; configured %s); system-font profiles inherit Terminus\n' \
  "$terminal_use_system_font_before" "$terminal_font_before"
printf '  Window frames:   %s, 1px black, square; crisp bottom-only shadows\n' "$marco_theme_name"
printf '  Compositor:      generated shadows disabled globally in the MATE session\n'
printf '  Application bg:  %s across shared GTK 2/3/4 application surfaces\n' "$application_bg"
printf '  Menubar bg:      %s across application bars and MATE panels\n' "$menubar_bg"
printf '  Titlebar bg:     %s across Marco and GTK client-side titles\n' "$titlebar_bg"
printf '  GTK 2/3/4:       1px black, shadowed physical buttons and windows\n'
printf '  Control states:  application-derived shades; no neutral white/gray fallback\n'
printf '  Container frames: one-bit pixel-dotted curves in GTK 3/4; solid GTK 2 fallback\n'
printf '  Focus outlines:   one-pixel dotted contours across GTK 2/3/4\n'
printf '  Icons:            %s, including native 48px folders and file types\n' "$icon_theme_name"
printf '  Cursors:          %s at native %spx; modern-only states inherit Adwaita\n' \
  "$cursor_theme_name" "$cursor_theme_size_px"
printf '  Cursor fallback:  link/help, vertical text, drag/drop, resize/scroll, and zoom\n'
printf '  Rounded UI:      %s where base GTK is rounded; square base corners preserved\n' \
  "$border_radius_css"
printf '  Scrollbars:      inset 8px dither rails; application-colored shadowed thumbs\n'
printf '  Value rails:     13px body + 1px shadow; dithered remainder; no puck\n'
printf '  Quartz Settings: %s (3 layouts, 40 palettes, custom colors, and GTK audits)\n' "$settings_launcher"
if [[ -n "$global_antialias_before" ]]; then
  printf '  Font rendering:  Geneva/ChiKareGo2 one-bit; global antialiasing remains %s\n' "$global_antialias_before"
else
  printf '  Font rendering:  Geneva/ChiKareGo2 one-bit; global settings unchanged\n'
fi

case "$marco_refresh_result" in
  restarted)
    printf '  Live Marco:      restarted once to apply compositor settings without generated shadows\n'
    ;;
  reloaded)
    printf '  Live Marco:      theme reloaded in place\n'
    ;;
  *)
    printf '  Live Marco:      no active window manager refreshed; log out and back in once\n'
    ;;
esac

printf '  Live panel:      palette refreshed in place; active layout preserved\n'

if [[ "$gtk3_user_css_changed" == "true" || "$gtk4_user_css_changed" == "true" ]]; then
  printf '  Live GTK apps:   user CSS changed; restart open apps or log out and back in once\n'
elif [[ "$gtk_theme_setting_refreshed" == "true" ]]; then
  printf '  Live GTK apps:   shared theme setting refreshed after the in-place update\n'
else
  printf '  Live GTK apps:   shared theme selected; restart any app retaining an old stylesheet\n'
fi

# Restore explicitly chosen shared typography after default-font validation.
python3 "$settings_theme_data/typography.py" --restore
