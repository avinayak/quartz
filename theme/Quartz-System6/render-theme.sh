#!/usr/bin/env bash

# Render the toolkit-native Quartz theme from the shared geometry, layout, and
# palette.

set -Eeuo pipefail
IFS=$'\n\t'

source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
readonly source_dir
readonly default_window_layout="cupertino"

if (( $# == 1 )); then
  config_file="$source_dir/theme.conf"
  output_dir="$1"
elif (( $# == 3 )) && [[ "$1" == "--config" ]]; then
  config_file="$2"
  output_dir="$3"
else
  printf 'error: usage: %s [--config CONFIG_FILE] OUTPUT_DIRECTORY\n' "$0" >&2
  exit 1
fi
readonly config_file output_dir

command -v sha256sum >/dev/null 2>&1 || {
  printf 'error: required command is missing: sha256sum\n' >&2
  exit 1
}

die() {
  printf 'error: %s\n' "$*" >&2
  exit 1
}

read_theme_config() {
  local config_line=""
  local config_key=""
  local config_value=""
  local border_radius_value=""
  local window_layout_value=""
  local menubar_bg_value=""
  local titlebar_bg_value=""
  local application_bg_value=""

  [[ -r "$config_file" ]] || die "theme configuration not found: $config_file"

  while IFS= read -r config_line || [[ -n "$config_line" ]]; do
    [[ "$config_line" =~ ^[[:space:]]*$ ]] && continue
    [[ "$config_line" =~ ^[[:space:]]*# ]] && continue
    [[ "$config_line" =~ ^([A-Z0-9_]+)=([^[:space:]]+)$ ]] ||
      die "invalid theme setting: $config_line"
    config_key="${BASH_REMATCH[1]}"
    config_value="${BASH_REMATCH[2]}"

    case "$config_key" in
      QUARTZ_BORDER_RADIUS_PX)
        [[ -z "$border_radius_value" ]] || die "duplicate theme setting: $config_key"
        border_radius_value="$config_value"
        ;;
      QUARTZ_WINDOW_LAYOUT)
        [[ -z "$window_layout_value" ]] || die "duplicate theme setting: $config_key"
        window_layout_value="$config_value"
        ;;
      QUARTZ_MENUBAR_BG)
        [[ -z "$menubar_bg_value" ]] || die "duplicate theme setting: $config_key"
        menubar_bg_value="$config_value"
        ;;
      QUARTZ_TITLEBAR_BG)
        [[ -z "$titlebar_bg_value" ]] || die "duplicate theme setting: $config_key"
        titlebar_bg_value="$config_value"
        ;;
      QUARTZ_APPLICATION_BG)
        [[ -z "$application_bg_value" ]] || die "duplicate theme setting: $config_key"
        application_bg_value="$config_value"
        ;;
      *) die "unknown theme setting: $config_key" ;;
    esac
  done <"$config_file"

  [[ "$border_radius_value" =~ ^(0|[1-9][0-9]?)$ ]] ||
    die "QUARTZ_BORDER_RADIUS_PX must be an integer from 0 through 64"
  (( 10#$border_radius_value <= 64 )) ||
    die "QUARTZ_BORDER_RADIUS_PX must be an integer from 0 through 64"
  [[ -n "$window_layout_value" ]] || window_layout_value="$default_window_layout"
  case "$window_layout_value" in
    cupertino)
      button_border_x="7"
      title_x="frame_x_center-title_width/2"
      title_mask_x="frame_x_center-title_width/2-6"
      ;;
    redmond)
      button_border_x="2"
      title_x="7"
      title_mask_x="1"
      ;;
    redmond-reversed)
      button_border_x="2"
      title_x="width-title_width-7"
      title_mask_x="width-title_width-13"
      ;;
    *)
      die "QUARTZ_WINDOW_LAYOUT must be cupertino, redmond, or redmond-reversed"
      ;;
  esac
  for config_value in \
    "$menubar_bg_value" "$titlebar_bg_value" "$application_bg_value"; do
    [[ "$config_value" =~ ^#[[:xdigit:]]{6}$ ]] ||
      die "theme colors must use opaque #RRGGBB values"
  done

  border_radius_px="$((10#$border_radius_value))"
  window_layout="$window_layout_value"
  menubar_bg="$(printf '%s' "$menubar_bg_value" | tr '[:upper:]' '[:lower:]')"
  titlebar_bg="$(printf '%s' "$titlebar_bg_value" | tr '[:upper:]' '[:lower:]')"
  application_bg="$(printf '%s' "$application_bg_value" | tr '[:upper:]' '[:lower:]')"
  readonly border_radius_px window_layout button_border_x title_x title_mask_x
  readonly menubar_bg titlebar_bg application_bg
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

render_template() {
  local template_file="$1"
  local rendered_file="$2"
  local rendered_radius_px="$3"
  local rendered_slice_px="$4"
  local rendered_gtk2_frame_inset_px="$5"
  local rendered_gtk2_entry_inner_x_px="$6"
  local rendered_gtk2_entry_inner_y_px="$7"
  local rendered_shadow_slice_px="$8"
  local rendered_frame_edge_inset_px="$((2 * (rendered_radius_px > 0 ? rendered_radius_px + 1 : 2)))"

  [[ -r "$template_file" ]] || die "theme template not found: $template_file"
  sed \
    -e "s/@QUARTZ_BORDER_RADIUS@/${rendered_radius_px}px/g" \
    -e "s/@QUARTZ_BORDER_RADIUS_PX@/${rendered_radius_px}/g" \
    -e "s/@QUARTZ_BORDER_SLICE_PX@/${rendered_slice_px}/g" \
    -e "s/@QUARTZ_GTK2_FRAME_INSET_PX@/${rendered_gtk2_frame_inset_px}/g" \
    -e "s/@QUARTZ_GTK2_ENTRY_INNER_X_PX@/${rendered_gtk2_entry_inner_x_px}/g" \
    -e "s/@QUARTZ_GTK2_ENTRY_INNER_Y_PX@/${rendered_gtk2_entry_inner_y_px}/g" \
    -e "s/@QUARTZ_BORDER_SHADOW_SLICE_PX@/${rendered_shadow_slice_px}/g" \
    -e "s|@QUARTZ_GTK3_MENU_FRAME_ASSET@|${menu_frame_asset}|g" \
    -e "s|@QUARTZ_SWITCH_ASSET_PREFIX@|${switch_asset_prefix}|g" \
    -e "s|@QUARTZ_SWITCH_SLICE@|${switch_slice_px}|g" \
    -e "s/@QUARTZ_FRAME_EDGE_INSET@/${rendered_frame_edge_inset_px}px/g" \
    -e "s/@QUARTZ_BUTTON_BORDER_X@/${button_border_x}/g" \
    -e "s|@QUARTZ_TITLE_X@|${title_x}|g" \
    -e "s|@QUARTZ_TITLE_MASK_X@|${title_mask_x}|g" \
    -e "s|@QUARTZ_MENUBAR_BG@|${menubar_bg}|g" \
    -e "s|@QUARTZ_TITLEBAR_BG@|${titlebar_bg}|g" \
    -e "s|@QUARTZ_APPLICATION_BG@|${application_bg}|g" \
    -e "s|@QUARTZ_APPLICATION_SHADE@|${application_shade}|g" \
    -e "s|@QUARTZ_APPLICATION_DISABLED_BG@|${application_disabled_bg}|g" \
    -e "s|@QUARTZ_APPLICATION_DISABLED_FG@|${application_disabled_fg}|g" \
    "$template_file" |
    awk -v radius="$rendered_radius_px" \
      -v asset_dir="$output_dir/gtk-3.0/assets" '
      function rounded(value) {
        return int(value + 0.5)
      }

      function remember(base_x, base_y, key) {
        if (base_x < 0 || base_x > radius ||
            base_y < 0 || base_y > radius) {
          return
        }

        if (radius > 0 &&
            ((base_x == radius && base_y == 0) ||
             (base_x == 0 && base_y == radius))) {
          return
        }

        key = base_y SUBSEP base_x
        if (!(key in seen)) {
          seen[key] = 1
          dot_x[++dot_count] = base_x
          dot_y[dot_count] = base_y
        }
      }

      function corner_name(corner) {
        return "frame-corner-v1-r" radius "-" corner ".xpm"
      }

      function write_corner(corner, file, x, y, source_x, source_y, row) {
        file = asset_dir "/" corner_name(corner)
        print "/* XPM */" > file
        print "static char *frame_corner[] = {" > file
        printf "\"%d %d 2 1\",\n", radius + 1, radius + 1 > file
        print "\"  c None\"," > file
        print "\". c #000000\"," > file
        for (y = 0; y <= radius; y++) {
          row = ""
          for (x = 0; x <= radius; x++) {
            source_x = (corner % 2 == 1) ? radius - x : x
            source_y = (corner >= 2) ? radius - y : y
            row = row (((source_y SUBSEP source_x) in seen) ? "." : " ")
          }
          printf "\"%s\"%s\n", row, (y == radius ? "};" : ",") > file
        }
        close(file)
      }

      BEGIN {
        pi = atan2(0, -1)

        if (radius == 0) {
          remember(0, 0)
        } else {
          arc_length = pi * radius / 2
          for (distance = 1;
               distance <= arc_length / 2 + 0.000001;
               distance += 2) {
            angle = distance / radius
            base_x = rounded(radius - radius * sin(angle))
            base_y = rounded(radius - radius * cos(angle))
            remember(base_x, base_y)
            remember(base_y, base_x)
          }
        }
      }

      $0 == "@QUARTZ_FRAME_DOT_BACKGROUND_IMAGES@" {
        # GTK 4 aborts when a CSS array exceeds 128 layers. Large radii use
        # four native-size one-bit corner sprites with the exact same dots;
        # straight edges still repeat independently at arbitrary frame sizes.
        if (dot_count * 4 + 4 > 128) {
          for (corner = 0; corner < 4; corner++) {
            write_corner(corner)
            printf "    url(\"../gtk-3.0/assets/%s\"),\n", corner_name(corner)
          }
          next
        }
        # GTK 4 can leave one unit of white coverage on a full-span gradient
        # at the frame boundary. Two identical passes compound to exact black
        # without widening the one-pixel sample.
        for (edge = 0; edge < 2; edge++) {
          for (dot_index = 1; dot_index <= dot_count; dot_index++) {
            for (coverage_pass = 0; coverage_pass < 2; coverage_pass++) {
              x = dot_x[dot_index]
              printf "    repeating-linear-gradient(to right, transparent 0, transparent %dpx, @quartz_ink %dpx, @quartz_ink %dpx, transparent %dpx, transparent calc(100%% - %dpx), @quartz_ink calc(100%% - %dpx), @quartz_ink calc(100%% - %dpx), transparent calc(100%% - %dpx), transparent 100%%),\n", x, x, x + 1, x + 1, x + 1, x + 1, x, x
            }
          }
        }
        next
      }

      $0 == "@QUARTZ_FRAME_DOT_BACKGROUND_SIZES@" {
        if (dot_count * 4 + 4 > 128) {
          for (corner = 0; corner < 4; corner++) {
            printf "    %dpx %dpx,\n", radius + 1, radius + 1
          }
          next
        }
        for (edge = 0; edge < 2; edge++) {
          for (dot_index = 1; dot_index <= dot_count; dot_index++) {
            for (coverage_pass = 0; coverage_pass < 2; coverage_pass++) {
              print "    100% 1px,"
            }
          }
        }
        next
      }

      $0 == "@QUARTZ_FRAME_DOT_BACKGROUND_POSITIONS@" {
        if (dot_count * 4 + 4 > 128) {
          print "    left top, right top, left bottom, right bottom,"
          next
        }
        for (dot_index = 1; dot_index <= dot_count; dot_index++) {
          for (coverage_pass = 0; coverage_pass < 2; coverage_pass++) {
            printf "    left %dpx,\n", dot_y[dot_index]
          }
        }
        for (dot_index = 1; dot_index <= dot_count; dot_index++) {
          for (coverage_pass = 0; coverage_pass < 2; coverage_pass++) {
            printf "    left calc(100%% - %dpx),\n", dot_y[dot_index]
          }
        }
        next
      }

      { print }
    ' >"$rendered_file"

  if grep -Eq '@QUARTZ_[A-Z0-9_]+@' "$rendered_file"; then
    die "unresolved theme variable in $template_file"
  fi
  chmod 0644 "$rendered_file"
}

generate_bar_xpm() {
  local output_file="$1"
  local bar_kind="$2"
  local face_color="$3"
  local symbol=""
  local row=0
  local pixel="+"

  case "$bar_kind" in
    application) symbol="menubar_app_xpm" ;;
    panel) symbol="panel_bar_xpm" ;;
    *) die "unknown bar asset kind: $bar_kind" ;;
  esac

  {
    printf '/* XPM */\n'
    printf 'static char * %s[] = {\n' "$symbol"
    printf '"1 25 2 1",\n'
    printf '". c #000000",\n'
    printf '"+ c %s",\n' "$face_color"
    for ((row = 0; row < 25; row++)); do
      pixel="+"
      if [[ "$bar_kind" == "panel" && "$row" == 0 ]] || (( row == 24 )); then
        pixel="."
      fi
      if (( row == 24 )); then
        printf '"%s"};\n' "$pixel"
      else
        printf '"%s",\n' "$pixel"
      fi
    done
  } >"$output_file"
  chmod 0644 "$output_file"
}

recolor_xpm_palette() {
  local asset_root="$1"
  local xpm_file=""
  local recolored_file=""

  while IFS= read -r -d '' xpm_file; do
    recolored_file="${xpm_file}.quartz-paper"
    awk \
      -v paper="$application_bg" \
      -v shade="$application_shade" \
      -v disabled_bg="$application_disabled_bg" \
      -v disabled_fg="$application_disabled_fg" '
      {
        # Each XPM palette line defines one color. Select its original role
        # once, so a custom gray cannot be substituted again as another role.
        if (/#[Ff][Ff][Ff][Ff][Ff][Ff]/) {
          gsub(/#[Ff][Ff][Ff][Ff][Ff][Ff]/, paper)
        } else if (/#[Ee][Bb][Ee][Bb][Ee][Bb]/) {
          gsub(/#[Ee][Bb][Ee][Bb][Ee][Bb]/, shade)
        } else if (/#[Dd]9[Dd]9[Dd]9/) {
          gsub(/#[Dd]9[Dd]9[Dd]9/, disabled_bg)
        } else if (/#777777|#7[Ff]7[Ff]7[Ff]/) {
          gsub(/#777777|#7[Ff]7[Ff]7[Ff]/, disabled_fg)
        }
        print
      }
    ' "$xpm_file" >"$recolored_file"
    chmod 0644 "$recolored_file"
    mv -- "$recolored_file" "$xpm_file"
  done < <(find "$asset_root" -type f -name '*.xpm' -print0)
}

compute_gtk2_frame_inset() {
  local rendered_radius_px="$1"
  local rendered_slice_px="$2"

  # GtkEntry's text-area child window is inset by xthickness/ythickness. Find
  # the deepest pixel used by the generated one-pixel curve so that child
  # window cannot paint over any part of the frame.
  awk \
    -v radius="$rendered_radius_px" \
    -v slice="$rendered_slice_px" '
    function inside(px, py, left, top, right, bottom, curve, dx, dy) {
      if (px < left || px >= right || py < top || py >= bottom) {
        return 0
      }
      if (curve == 0) {
        return 1
      }

      if (px < left + curve && py < top + curve) {
        dx = left + curve - px
        dy = top + curve - py
        return dx * dx + dy * dy <= curve * curve
      }
      return 1
    }

    BEGIN {
      size = 2 * slice + 1
      deepest = 0
      for (y = 0; y <= slice; y++) {
        for (x = 0; x <= slice; x++) {
          px = x + 0.5
          py = y + 0.5
          outer = inside(px, py, 0, 0, size, size, radius)
          inner = inside(px, py, 1, 1, size - 1, size - 1,
            radius > 0 ? radius - 1 : 0)
          if (outer && !inner) {
            depth = x < y ? x : y
            if (depth > deepest) {
              deepest = depth
            }
          }
        }
      }
      print deepest + 1
    }
  '
}

validate_css_template() {
  local template_file="$1"

  [[ -r "$template_file" ]] || die "theme template not found: $template_file"
  awk '
    /border-((top|bottom)-(left|right)-)?radius:[[:space:]]*/ {
      value = $0
      sub(/^[^:]*:[[:space:]]*/, "", value)
      sub(/[[:space:]]*;.*/, "", value)
      count = split(value, corners, /[[:space:]]+/)
      for (corner = 1; corner <= count; corner++) {
        if (corners[corner] != "0" &&
            corners[corner] != "0px" &&
            corners[corner] != "@QUARTZ_BORDER_RADIUS@") {
          exit 1
        }
      }
    }
  ' "$template_file" ||
    die "CSS radius bypasses QUARTZ_BORDER_RADIUS_PX: $template_file"
  grep -Fq 'border-radius: @QUARTZ_BORDER_RADIUS@;' "$template_file" ||
    die "CSS template does not use QUARTZ_BORDER_RADIUS_PX: $template_file"
}

validate_css_tooltip() {
  local template_file="$1"

  grep -Fqx '@define-color quartz_tooltip #ffecb3;' "$template_file" ||
    die "GTK tooltip palette does not define the amber face: $template_file"
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
  ' "$template_file" ||
    die "GTK tooltip is not amber with black text and outline: $template_file"
}

validate_transparent_spin_text_surface() {
  local template_file="$1"
  local spin_text_selector="$2"

  awk -v selector="$spin_text_selector" '
    $0 == selector { in_spin_text = 1 }
    in_spin_text && $0 == "  background-color: transparent;" {
      transparent_surface = 1
    }
    in_spin_text && $0 == "}" { in_spin_text = 0 }
    END { exit(transparent_surface ? 0 : 1) }
  ' "$template_file" ||
    die "spin-button text surface can cover the rounded parent: $template_file"
}

generate_rounded_xpm() {
  local output_file="$1"
  local asset_kind="$2"
  local rendered_radius_px="$3"
  local rendered_slice_px="$4"

  case "$asset_kind" in
    frame | white | black | menu-shadow) ;;
    button-white-raised | button-black-raised | button-disabled-raised | \
      button-shade-raised | button-shade-pressed) ;;
    *) die "unknown rounded asset kind: $asset_kind" ;;
  esac

  awk \
    -v kind="$asset_kind" \
    -v radius="$rendered_radius_px" \
    -v slice="$rendered_slice_px" \
    -v paper="$application_bg" \
    -v shade="$application_shade" \
    -v disabled_bg="$application_disabled_bg" \
    -v disabled_fg="$application_disabled_fg" '
    function inside(px, py, left, top, right, bottom, curve, dx, dy) {
      if (px < left || px >= right || py < top || py >= bottom) {
        return 0
      }
      if (curve == 0) {
        return 1
      }

      if (px < left + curve && py < top + curve) {
        dx = left + curve - px
        dy = top + curve - py
        return dx * dx + dy * dy <= curve * curve
      }
      if (px >= right - curve && py < top + curve) {
        dx = px - (right - curve)
        dy = top + curve - py
        return dx * dx + dy * dy <= curve * curve
      }
      if (px < left + curve && py >= bottom - curve) {
        dx = left + curve - px
        dy = py - (bottom - curve)
        return dx * dx + dy * dy <= curve * curve
      }
      if (px >= right - curve && py >= bottom - curve) {
        dx = px - (right - curve)
        dy = py - (bottom - curve)
        return dx * dx + dy * dy <= curve * curve
      }
      return 1
    }

    BEGIN {
      size = 2 * slice + 1
      is_button = index(kind, "button-") == 1
      is_menu = kind == "menu-shadow"
      is_raised = is_menu || kind == "button-white-raised" || \
        kind == "button-black-raised" || kind == "button-disabled-raised" || \
        kind == "button-shade-raised"
      is_pressed = kind == "button-shade-pressed"
      is_disabled = kind == "button-disabled-raised"
      has_light_fill = kind == "white" || is_menu || \
        kind == "button-white-raised" || kind == "button-shade-raised" || \
        kind == "button-shade-pressed" || is_disabled
      canvas_height = size + ((is_button || is_menu) ? 1 : 0)

      if (kind == "frame") {
        symbol = "frame_rounded_xpm"
      } else if (kind == "white") {
        symbol = "box_white_rounded_xpm"
      } else if (kind == "black") {
        symbol = "box_black_rounded_xpm"
      } else if (kind == "menu-shadow") {
        symbol = "menu_shadow_xpm"
      } else if (kind == "button-white-raised") {
        symbol = "button_white_raised_xpm"
      } else if (kind == "button-black-raised") {
        symbol = "button_black_raised_xpm"
      } else if (kind == "button-disabled-raised") {
        symbol = "button_disabled_raised_xpm"
      } else if (kind == "button-shade-raised") {
        symbol = "button_shade_raised_xpm"
      } else {
        symbol = "button_shade_pressed_xpm"
      }
      colors = has_light_fill ? 3 : 2
      fill = has_light_fill ? "+" : (kind == "frame" ? " " : ".")
      fill_color = (is_pressed || kind == "button-shade-raised") ? shade : \
        (is_disabled ? disabled_bg : paper)
      edge_color = is_disabled ? disabled_fg : "#000000"

      print "/* XPM */"
      print "static char * " symbol "[] = {"
      print "\"" size " " canvas_height " " colors " 1\","
      print "\"  c None\","
      print "\". c " edge_color "\","
      if (has_light_fill) {
        print "\"+ c " fill_color "\","
      }

      for (y = 0; y < canvas_height; y++) {
        row = ""
        for (x = 0; x < size; x++) {
          px = x + 0.5
          body_y = is_pressed ? y - 1 : y
          if (body_y >= 0 && body_y < size) {
            py = body_y + 0.5
            outer = inside(px, py, 0, 0, size, size, radius)
            inner = inside(px, py, 1, 1, size - 1, size - 1,
              radius > 0 ? radius - 1 : 0)
          } else {
            outer = 0
            inner = 0
          }

          shadow = 0
          if (is_raised && y > 0) {
            shadow = inside(px, y - 0.5, 0, 0, size, size, radius)
          }
          row = row (outer ? (inner ? fill : ".") : (shadow ? "." : " "))
        }
        suffix = y + 1 == canvas_height ? "};" : ","
        print "\"" row "\"" suffix
      }
    }
  ' >"$output_file"
  chmod 0644 "$output_file"
}

generate_switch_xpm() {
  local output_file="$1" size="$2" radius="$3" edge="$4" face="$5"
  local outside="${6:-None}"
  # Sample pixel centers, as for the shared GTK 2 frame. Only complete ink,
  # paper, or transparent pixels are emitted; no gray coverage at corners.
  awk -v size="$size" -v radius="$radius" -v edge="$edge" -v face="$face" -v outside="$outside" '
    function inside(x, y, inset, r, lo, hi, dx, dy) {
      lo = inset; hi = size - inset
      if (x < lo || y < lo || x >= hi || y >= hi) return 0
      dx = x < lo + r ? lo + r - x : (x > hi - r ? x - hi + r : 0)
      dy = y < lo + r ? lo + r - y : (y > hi - r ? y - hi + r : 0)
      return dx * dx + dy * dy <= r * r
    }
    BEGIN {
      print "/* XPM */"
      print "static char *quartz_switch[] = {"
      printf "\"%d %d 3 1\",\n", size, size
      print "\"  c " outside "\","
      print "\". c " edge "\","
      print "\"+ c " face "\","
      for (y = 0; y < size; y++) {
        row = ""
        for (x = 0; x < size; x++) {
          outer = inside(x + 0.5, y + 0.5, 0, radius)
          inner = inside(x + 0.5, y + 0.5, 1, radius > 0 ? radius - 1 : 0)
          row = row (outer ? (inner ? "+" : ".") : " ")
        }
        printf "\"%s\"%s\n", row, y == size - 1 ? "};" : ","
      }
    }
  ' >"$output_file"
  chmod 0644 "$output_file"
}

generate_notebook_tab_xpm() {
  local output_file="$1"
  local tab_side="$2"
  local rendered_radius_px="$3"
  local rendered_slice_px="$4"

  case "$tab_side" in
    top | bottom | left | right) ;;
    *) die "unknown notebook tab side: $tab_side" ;;
  esac

  # Build each tab by cropping the page-facing half of the same rounded box
  # used elsewhere in GTK 2. The cropped edge stays open, while the two outer
  # corners retain the configured radius and a one-pixel black outline.
  awk \
    -v side="$tab_side" \
    -v radius="$rendered_radius_px" \
    -v slice="$rendered_slice_px" \
    -v paper="$application_bg" '
    function inside(px, py, left, top, right, bottom, curve, dx, dy) {
      if (px < left || px >= right || py < top || py >= bottom) {
        return 0
      }
      if (curve == 0) {
        return 1
      }

      if (px < left + curve && py < top + curve) {
        dx = left + curve - px
        dy = top + curve - py
        return dx * dx + dy * dy <= curve * curve
      }
      if (px >= right - curve && py < top + curve) {
        dx = px - (right - curve)
        dy = top + curve - py
        return dx * dx + dy * dy <= curve * curve
      }
      if (px < left + curve && py >= bottom - curve) {
        dx = left + curve - px
        dy = py - (bottom - curve)
        return dx * dx + dy * dy <= curve * curve
      }
      if (px >= right - curve && py >= bottom - curve) {
        dx = px - (right - curve)
        dy = py - (bottom - curve)
        return dx * dx + dy * dy <= curve * curve
      }
      return 1
    }

    BEGIN {
      size = 2 * slice + 1
      depth = slice + 1
      horizontal = side == "top" || side == "bottom"
      width = horizontal ? size : depth
      height = horizontal ? depth : size
      symbol = "notebook_tab_" side "_xpm"

      print "/* XPM */"
      print "static char * " symbol "[] = {"
      print "\"" width " " height " 3 1\","
      print "\"  c None\","
      print "\". c #000000\","
      print "\"+ c " paper "\","

      for (output_y = 0; output_y < height; output_y++) {
        row = ""
        for (output_x = 0; output_x < width; output_x++) {
          x = side == "right" ? output_x + slice : output_x
          y = side == "bottom" ? output_y + slice : output_y
          px = x + 0.5
          py = y + 0.5
          outer = inside(px, py, 0, 0, size, size, radius)
          inner = inside(px, py, 1, 1, size - 1, size - 1,
            radius > 0 ? radius - 1 : 0)
          row = row (!outer ? " " : (inner ? "+" : "."))
        }
        suffix = output_y + 1 == height ? "};" : ","
        print "\"" row "\"" suffix
      }
    }
  ' >"$output_file"
  chmod 0644 "$output_file"
}

read_theme_config
# These factors preserve the original System 7 neutral values for Classic
# white (#ebebeb, #d9d9d9, and #777777), while retaining the hue of every
# colored application surface. They are deliberately rendered to opaque
# pixels so GTK 2, GTK 3, and GTK 4 produce the same state colors.
application_shade="$(shade_hex_color "$application_bg" 235)"
application_disabled_bg="$(shade_hex_color "$application_bg" 217)"
application_disabled_fg="$(shade_hex_color "$application_bg" 119)"
readonly application_shade application_disabled_bg application_disabled_fg
border_slice_px="$((border_radius_px > 0 ? border_radius_px : 1))"
readonly border_slice_px
border_shadow_slice_px="$((border_slice_px + 1))"
readonly border_shadow_slice_px
gtk2_frame_inset_px="$(
  compute_gtk2_frame_inset "$border_radius_px" "$border_slice_px"
)"
readonly gtk2_frame_inset_px
gtk2_entry_inner_x_px="$((4 - gtk2_frame_inset_px))"
(( gtk2_entry_inner_x_px >= 0 )) || gtk2_entry_inner_x_px=0
readonly gtk2_entry_inner_x_px
gtk2_entry_inner_y_px="$((2 - gtk2_frame_inset_px))"
(( gtk2_entry_inner_y_px >= 0 )) || gtk2_entry_inner_y_px=0
readonly gtk2_entry_inner_y_px

grep -Fqx '  GtkWidget::tooltip-radius = @QUARTZ_BORDER_RADIUS_PX@' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 template does not use QUARTZ_BORDER_RADIUS_PX"
grep -Fq 'tooltip_bg_color:#ffecb3\ntooltip_fg_color:#000000' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
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
' "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 tooltip is not amber with black text and outline"
grep -Fq '@QUARTZ_BORDER_SLICE_PX@' "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 template does not use the configured rounded-asset slice"
grep -Fq '@QUARTZ_BORDER_SHADOW_SLICE_PX@' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 buttons do not reserve the one-pixel shadow slice"
grep -Fqx '  xthickness = @QUARTZ_GTK2_FRAME_INSET_PX@' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 entry does not protect the generated rounded frame"
grep -Fq '@QUARTZ_GTK2_ENTRY_INNER_X_PX@' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 entry does not compensate horizontal frame inset"
grep -Fq '@QUARTZ_GTK2_ENTRY_INNER_Y_PX@' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 entry does not compensate vertical frame inset"
grep -Fqx '  GtkWidget::focus-line-pattern = "\1\1"' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 template does not define the one-pixel dotted focus pattern"
grep -Fqx '  GtkWidget::focus-line-width = 1' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 template does not enable the shared focus line"
grep -Fqx '  GtkButton::focus-line-width = 1' \
  "$source_dir/gtk-2.0/gtkrc.in" ||
  die "GTK 2 template does not enable the button focus line"
validate_css_template "$source_dir/gtk-3.0/gtk.css.in"
validate_css_template "$source_dir/gtk-4.0/gtk.css.in"
validate_css_tooltip "$source_dir/gtk-3.0/gtk.css.in"
validate_css_tooltip "$source_dir/gtk-4.0/gtk.css.in"
for css_template in \
  "$source_dir/gtk-3.0/gtk.css.in" \
  "$source_dir/gtk-4.0/gtk.css.in"; do
  for frame_placeholder in \
    '@QUARTZ_FRAME_DOT_BACKGROUND_IMAGES@' \
    '@QUARTZ_FRAME_DOT_BACKGROUND_SIZES@' \
    '@QUARTZ_FRAME_DOT_BACKGROUND_POSITIONS@'; do
    grep -Fqx "$frame_placeholder" "$css_template" ||
      die "CSS template omits the generated frame-corner layer: $css_template"
  done
  grep -Fq '@QUARTZ_FRAME_EDGE_INSET@' "$css_template" ||
    die "CSS template omits the generated frame-edge inset: $css_template"
done
validate_transparent_spin_text_surface \
  "$source_dir/gtk-3.0/gtk.css.in" \
  'spinbutton entry {'
validate_transparent_spin_text_surface \
  "$source_dir/gtk-3.0/gtk.css.in" \
  'spinbutton entry:disabled {'
validate_transparent_spin_text_surface \
  "$source_dir/gtk-4.0/gtk.css.in" \
  'spinbutton:not(.vertical) > text {'
for css_template in \
  "$source_dir/gtk-3.0/gtk.css.in" \
  "$source_dir/gtk-4.0/gtk.css.in"; do
  if grep -Fq 'outline-style: solid;' "$css_template"; then
    die "CSS template contains a solid focus outline: $css_template"
  fi
done
if grep -Fq '  outline: none;' "$source_dir/gtk-4.0/gtk.css.in"; then
  die "GTK 4 template suppresses a component focus outline"
fi

if [[ -e "$output_dir" ]]; then
  [[ -d "$output_dir" && ! -L "$output_dir" ]] ||
    die "output path is not a directory: $output_dir"
  [[ -z "$(find "$output_dir" -mindepth 1 -maxdepth 1 -print -quit)" ]] ||
    die "output directory is not empty: $output_dir"
else
  install -d -m 0755 -- "$output_dir"
fi

install -d -m 0755 -- \
  "$output_dir/gtk-2.0/assets" \
  "$output_dir/gtk-3.0/assets" \
  "$output_dir/gtk-4.0" \
  "$output_dir/metacity-1"

install -m 0644 -- "$source_dir/index.theme" "$output_dir/index.theme"
printf 'QUARTZ_BORDER_RADIUS_PX=%s\n' "$border_radius_px" >"$output_dir/theme.conf"
printf 'QUARTZ_WINDOW_LAYOUT=%s\n' "$window_layout" >>"$output_dir/theme.conf"
printf 'QUARTZ_MENUBAR_BG=%s\n' "$menubar_bg" >>"$output_dir/theme.conf"
printf 'QUARTZ_TITLEBAR_BG=%s\n' "$titlebar_bg" >>"$output_dir/theme.conf"
printf 'QUARTZ_APPLICATION_BG=%s\n' "$application_bg" >>"$output_dir/theme.conf"
chmod 0644 "$output_dir/theme.conf"
cp -R -- "$source_dir/gtk-2.0/assets/." "$output_dir/gtk-2.0/assets/"
cp -R -- "$source_dir/gtk-3.0/assets/." "$output_dir/gtk-3.0/assets/"
install -m 0644 -- "$source_dir/gtk-3.0/assets/titlebutton-pressed-system7.xpm" \
  "$output_dir/metacity-1/titlebutton-pressed-system7.xpm"
# GTK icon allocations are even-sized. An even canvas prevents centering the
# odd 11px bitmap on half pixels and filtering its one-bit edges. Keep the
# original sprite unscaled, adding transparent padding at the right/bottom.
awk '
  /^"11 11 2 1"/ { print "\"12 12 2 1\","; next }
  /^"[.X]+"/ {
    sub(/"[,]?$/, ".\",")
    print
    if (++rows == 11) print "\"............\""
    next
  }
  { print }
' "$source_dir/gtk-3.0/assets/titlebutton-pressed-system7.xpm" \
  >"$output_dir/gtk-3.0/assets/titlebutton-pressed-system7-gtk.xpm"
recolor_xpm_palette "$output_dir/gtk-2.0/assets"
recolor_xpm_palette "$output_dir/gtk-3.0/assets"

# GtkImage caches border-image payloads by URI in long-running GTK 3
# processes. Give every palette/radius-specific frame its own stable URI so a
# normal theme-name refresh cannot retain corner pixels from the old palette.
generate_rounded_xpm \
  "$output_dir/gtk-3.0/assets/menu-frame.xpm" \
  menu-shadow "$border_radius_px" "$border_slice_px"
menu_frame_digest="$(sha256sum -- \
  "$output_dir/gtk-3.0/assets/menu-frame.xpm" | awk '{ print $1 }')"
menu_frame_asset="menu-frame-$menu_frame_digest.xpm"
readonly menu_frame_digest menu_frame_asset
cp -- \
  "$output_dir/gtk-3.0/assets/menu-frame.xpm" \
  "$output_dir/gtk-3.0/assets/$menu_frame_asset"

switch_radius_px="$((border_radius_px < 7 ? border_radius_px : 7))"
switch_thumb_radius_px="$((border_radius_px < 6 ? border_radius_px : 6))"
switch_slice_px="$((switch_radius_px + 1))"
switch_tile_size="$((2 * switch_slice_px + 1))"
switch_asset_digest="$(printf 'switch-v2:%s:%s:%s' \
  "$border_radius_px" "$application_bg" "$application_disabled_fg" | sha256sum | awk '{ print $1 }')"
switch_asset_prefix="switch-$switch_asset_digest"
switch_asset_path="$output_dir/gtk-3.0/assets/$switch_asset_prefix"
generate_switch_xpm "$switch_asset_path-track-off.xpm" "$switch_tile_size" "$switch_radius_px" '#000000' "$application_bg"
# The checked frame exposes a repeating dither underneath. Opaque paper
# outside its outline masks the tile at the native pixel corners.
generate_switch_xpm "$switch_asset_path-track-on.xpm" "$switch_tile_size" "$switch_radius_px" '#000000' None "$application_bg"
generate_switch_xpm "$switch_asset_path-track-disabled-off.xpm" "$switch_tile_size" "$switch_radius_px" "$application_disabled_fg" "$application_bg"
generate_switch_xpm "$switch_asset_path-track-disabled-on.xpm" "$switch_tile_size" "$switch_radius_px" "$application_disabled_fg" None "$application_bg"
generate_switch_xpm "$switch_asset_path-thumb.xpm" 12 "$switch_thumb_radius_px" '#000000' "$application_bg"
generate_switch_xpm "$switch_asset_path-thumb-disabled.xpm" 12 "$switch_thumb_radius_px" "$application_disabled_fg" "$application_bg"
cp -- "$output_dir/gtk-3.0/assets/scale-dither.xpm" "$switch_asset_path-dither.xpm"
cp -- "$output_dir/gtk-3.0/assets/scale-dither-disabled.xpm" "$switch_asset_path-dither-disabled.xpm"

render_template \
  "$source_dir/gtk-2.0/gtkrc.in" \
  "$output_dir/gtk-2.0/gtkrc" \
  "$border_radius_px" "$border_slice_px" \
  "$gtk2_frame_inset_px" \
  "$gtk2_entry_inner_x_px" "$gtk2_entry_inner_y_px" \
  "$border_shadow_slice_px"
render_template \
  "$source_dir/gtk-3.0/gtk.css.in" \
  "$output_dir/gtk-3.0/gtk.css" \
  "$border_radius_px" "$border_slice_px" \
  "$gtk2_frame_inset_px" \
  "$gtk2_entry_inner_x_px" "$gtk2_entry_inner_y_px" \
  "$border_shadow_slice_px"
render_template \
  "$source_dir/gtk-4.0/gtk.css.in" \
  "$output_dir/gtk-4.0/gtk.css" \
  "$border_radius_px" "$border_slice_px" \
  "$gtk2_frame_inset_px" \
  "$gtk2_entry_inner_x_px" "$gtk2_entry_inner_y_px" \
  "$border_shadow_slice_px"
render_template \
  "$source_dir/metacity-1/metacity-theme-1.xml" \
  "$output_dir/metacity-1/metacity-theme-1.xml" \
  "$border_radius_px" "$border_slice_px" \
  "$gtk2_frame_inset_px" \
  "$gtk2_entry_inner_x_px" "$gtk2_entry_inner_y_px" \
  "$border_shadow_slice_px"

generate_bar_xpm \
  "$output_dir/gtk-2.0/assets/menubar-app.xpm" \
  application "$menubar_bg"
generate_bar_xpm \
  "$output_dir/gtk-3.0/assets/panel-bar.xpm" \
  panel "$menubar_bg"
panel_bitmap_digest="$(sha256sum -- \
  "$output_dir/gtk-3.0/assets/panel-bar.xpm" | awk '{ print $1 }')"
cp -- \
  "$output_dir/gtk-3.0/assets/panel-bar.xpm" \
  "$output_dir/gtk-3.0/assets/panel-bar-$panel_bitmap_digest.xpm"

generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/frame-rounded.xpm" \
  frame "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/box-white-rounded.xpm" \
  white "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/box-black-rounded.xpm" \
  black "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/button-white-raised.xpm" \
  button-white-raised "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/button-black-raised.xpm" \
  button-black-raised "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/button-disabled-raised.xpm" \
  button-disabled-raised "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/button-shade-raised.xpm" \
  button-shade-raised "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/button-shade-pressed.xpm" \
  button-shade-pressed "$border_radius_px" "$border_slice_px"
generate_rounded_xpm \
  "$output_dir/gtk-2.0/assets/menu-shadow.xpm" \
  menu-shadow "$border_radius_px" "$border_slice_px"
for notebook_tab_side in top bottom left right; do
  generate_notebook_tab_xpm \
    "$output_dir/gtk-2.0/assets/notebook-tab-$notebook_tab_side.xpm" \
    "$notebook_tab_side" "$border_radius_px" "$border_slice_px"
done
