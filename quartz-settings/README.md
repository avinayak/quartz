# Quartz Settings

## Typography

The **Typography** tab independently selects **Menu bar**, **Window titles**,
and **Application** fonts, with a live text sample for each. **Apply Typography
System-Wide** updates shared GTK 2/3/4 and Marco font settings and refreshes the shared
theme in running desktop windows and panels. Marco receives a point size
converted from the chosen pixels at the live screen DPI. When upgrading from
the initial Typography implementation, reopen existing applications once to
clear their cached user-stylesheet overrides.

Choices are discovered from installed fonts: all bitmap families and every
reported native strike, plus ChiKareGo2 at its native 16-pixel size. The full original System 7 bitmap collection is installed by the top-level
installer: Athens, Cairo, Chicago, Courier, Geneva, Helvetica, London,
Los Angeles, Monaco, New York, Palatino, San Francisco, Symbol, Times, and Venice.
Every source bitmap strike is included, including Geneva Italic. Typography
fonts always use one-bit rendering: a shared family-scoped Fontconfig policy
disables antialiasing, synthetic emboldening, and fractional bitmap scaling
for every offered family. Relative heading sizes use a native bitmap strike
instead of stretching and filtering its pixels. Pixel and point requests round
to the nearest native size (using the request DPI); ties choose the smaller
size. This shared Fontconfig policy applies wherever applications use the
selected fonts through the system font stack, including relative headings.
Applications with private rendering engines may bypass system font matching.
After installing new font families, older processes may need reopening to
clear their cached fallback faces. Each family has one entry with its native
sizes in the size selector; Geneva Italic is a genuine separate style. The
legacy size-named Geneva entries are hidden and saved selections migrate to
the matching original strike in the unified Geneva family. Terminus
provides nine sizes from 12 to 32 pixels on the project VM. Supported installed
pixel outline families also appear at whole-grid multiples. Install additional
bitmap fonts through the system font installer, then reopen Quartz Settings.

The installer also bundles [39 community font families and real style variants](../fonts/community/README.md):
Spleen, Tamzen, Tamzen Powerline, Cozette, Scientifica, Creep, Cherry, Kirsch,
Gohufont, ProFont, X11 Fixed, and the original Proggy/Webby variants. Every
family keeps its native sizes together. Fonts and redistribution notices are
bundled offline; your selected menu, title, and application fonts are preserved.

Selections are stored in `~/.config/quartz-settings/typography.json` separately
from the palette and survive palette changes and full theme reinstalls. The
managed font blocks live in the shared theme and preserve other user GTK settings. This does not change
application-private preferences. Very large fonts can exceed the fixed Quartz
19-pixel title and 25-pixel desktop panel geometry; those native sizes remain
available, but may be clipped in those surfaces.

## Appearance and Extras

In **Extras**, **Add quartz theme to firefox** installs the optional Quartz
browser theme using your saved palette. Close Firefox before clicking, then
reopen it. See [Firefox installation and removal](../quartz-extras/README.md)
for supported profiles, backups, and compatibility limits.

**Use Quartz logo instead of MATE** in Extras selects a shared desktop icon
theme with a transparent diamond logo for menus, retaining the current icon
set for everything else. The choice survives full theme reinstalls.

Quartz Settings has **Appearance**, **Typography**, and **Extras** tabs. Extras offers
**Download Recommended Wallpapers**, which runs the installed Quartz Extras
installer asynchronously and registers the System7 collection in MATE's wallpaper
chooser. It shows download activity, completion, and errors; the current desktop
background stays selected. The full Extras folder is installed with Settings so
this action works without the source checkout.

Quartz Settings configures Quartz's shared desktop theme and hosts its native
toolkit compatibility galleries. The main window provides:

- A **Window layout** menu with **Cupertino** (the System 7 arrangement: close
  left, centered title, zoom right), **Redmond** (title left, compact controls
  right), and **Redmond reversed** (compact controls left, title right).
  Cupertino is the default; explicitly saved layouts remain unchanged.
- A **Color theme** menu with 40 presets. Classic, Platinum, Graphite,
  Blueberry, Sky, Aqua, Teal, Mint, Sage, Lime, Lemon, Sand, Tangerine, Coral,
  Strawberry, Rose, Lavender, Grape, Iris, and Mocha are flat palettes: each
  uses one medium pastel for all three surfaces. A corresponding set of 20
  **Mixed - …** palettes uses that same pastel for the application, a richer
  midpoint for menus, and the strongest tint for titles.
- Independent color pickers for the menu bar, title bar, and application
  background. The application role covers shared window chrome and standard
  content views such as icon, list, tree, text, viewport, sidebar, and notebook
  surfaces. Editing any picker selects **Custom**.
- A one-bit, one-logical-pixel miniature window preview and an **Apply
  System-Wide** action. Its square frame, 19-pixel Marco title bar, 25-pixel
  GTK menu bar, selected title/control arrangement, bitmap fonts, stripes,
  separators, lower shadow, and application-colored button are drawn from the
  same geometry and palette roles as the installed shared theme.
- `GTK 2 Audit`, `GTK 3 Audit`, and `GTK 4 Audit` compatibility launchers.

Applying a theme saves `~/.config/quartz-settings/theme.conf` and renders the
selected layout and palette into GTK 2/3/4, Marco, and MATE panels. Existing
windows and panels refresh in place. If an installer changes the managed user
stylesheet itself, already-running applications may need one restart.

The `quartz-panel-theme.service` restores 25-pixel height, full opacity, and the
palette bitmap after MATE panel-layout changes. It preserves panel placement
and applet composition, and acts only on horizontal panels while Quartz is
selected. Refreshes share a lock and wait for layout-reset notifications to
settle. Panel and menu bitmaps use content-based filenames so changed pixels
reload without discarding a valid background.

The installer enables standard Marco compositing for dock-based layouts and
disables generated shadows through `META_DEBUG_NO_SHADOW=1` in
`~/.xsessionrc`. The theme retains its sharp one-pixel bottom shadow. Marco
may restart once to inherit these settings; palette changes reload in place.
The shared desktop surface remains enabled for the desktop context menu.

See the [theme source documentation](../theme/Quartz-System6/README.md) for
geometry, palette, and toolkit details.

Interfaces painted outside those shared layers (for example, an application's
own canvas or private color preference) are compatibility limitations. Quartz
Settings does not patch or imitate them with application-specific rules.

Each button starts a separately linked native audit program. The GTK 2 audit
links to `libgtk-x11-2.0.so.0`, the GTK 3 audit links to `libgtk-3.so.0`, and
the GTK 4 audit links to `libgtk-4.so.1`. Nothing is emulated across toolkit
versions.

The settings app and audits do not write application-private CSS or
preferences. Palette changes are rendered only through the repository's
shared GTK 2, GTK 3, GTK 4, Marco, and panel theme layers. The audits render
ordinary toolkit widgets so every visual correction remains system-wide.

## Build and install

Ubuntu MATE 24.04 provides the target GTK 2.24, GTK 3.24, and GTK 4.14
libraries. Install the development dependencies once:

```bash
sudo apt install build-essential pkg-config libgtk2.0-dev libgtk-3-dev libgtk-4-dev
```

Then build and install for the current user:

```bash
make -C quartz-settings clean check
make -C quartz-settings PREFIX="$HOME/.local" install
systemctl --user daemon-reload
systemctl --user enable --now quartz-panel-theme.service
```

The top-level `apply-ubuntu-mate-theme.sh` builds and installs the programs
and enables the panel synchronization service automatically. The desktop
entry is installed as `org.quartz.Settings.desktop`, and the menu item is named
**Quartz Settings**.
It can also be started from a terminal with:

```bash
~/.local/bin/quartz-settings
```

`make -C quartz-settings check` includes renderer edge cases, managed-config
validation, and optional-setting refresh guards without changing the desktop.
Additional checks are available:

```bash
# Requires a display; xvfb-run can supply one for headless testing.
make -C quartz-settings check-ui
# Requires ImageMagick and xcursorgen.
make -C quartz-settings check-cursors
```

UI checks verify repeated activation reuses one Settings window and parse
the shared GTK 3/4 CSS at every radius from 0 through 64. Cursor checks rebuild
all 19 payloads twice, compare them with the pinned binaries, and verify that
unrelated output files survive and symlink destinations are rejected.

## Audit coverage

Every audit is a scrollable, multi-page gallery. Controls are interactive, so
hover, pressed, focus, keyboard navigation, open-menu, drag, selection, and
resizing states can be inspected in addition to the states shown initially.

### GTK 2.24

- Application chrome: server-side window frame, accelerator-aware menubar,
  normal/image/check/radio/inconsistent/disabled menu items, separators,
  submenus, tearoff menu, popup menu, toolbar, detachable handle box, and
  status bars.
- Buttons: normal, mnemonic, stock image, custom child, default, flat,
  insensitive, link, toggle on/off, check on/off/inconsistent/disabled, radio
  selected/unselected/disabled, and start/end button-box layouts.
- Text and input: plain/markup/selectable/wrapped/ellipsized/mnemonic/disabled
  labels, accelerator label, editable/read-only/disabled/password/selected
  entries, entry icons, entry progress, tooltip, integer and decimal spin
  buttons, combo box, editable combo, option menu, file/font/color chooser
  buttons, volume button, and multiline tagged text view.
- Ranges and feedback: horizontal/vertical/marked/inverted/disabled scales,
  determinate/inverted/pulsing/disabled progress bars, horizontal and vertical
  scrollbars, active and disabled spinners, all four info-bar message types,
  calendar with marked dates, stock image, arrow widgets, separators, and a
  drawing area.
- Data views: sortable/resizable tree-view columns with pixbuf, text, toggle,
  and progress cell renderers; hierarchical tree store; multi-select icon
  view; selected rows; wrapping text view; and scrollable viewports.
- Containers: all five frame shadow types, expanded/collapsed expanders,
  notebook, horizontal and vertical panes, scrolled window, viewport, layout,
  fixed positioning, alignment, aspect frame, event box, tables, and boxes.
- Dialogs: information/warning/question/error messages, custom dialog,
  file chooser, recent chooser, color selection, font selection, about dialog,
  assistant, and legacy file selection.
- GTK 2 legacy surfaces: `GtkCombo`, `GtkOptionMenu`, `GtkList`, `GtkCList`,
  `GtkHandleBox`, `GtkRuler`, `GtkAlignment`, `GtkArrow`, `GtkFileSelection`,
  stock buttons/images, and classic button/menu/tool-button families.

The recent chooser is created on demand through its dialog launcher. Keeping
the GTK 2 recent chooser permanently embedded triggers an upstream GTK 2/GLib
source-removal diagnostic on current Ubuntu, independent of theme CSS or RC
rules.

### GTK 3.24

- Application chrome: server-side main window, traditional menubar and popup
  menus, image/check/radio/inconsistent/disabled menu states, toolbar,
  menu-tool button, status bar, header bar, client-side dialog, action bar,
  model buttons, menu buttons, content popovers, and context menus.
- Buttons and state classes: normal, mnemonic, image, circular, flat, linked,
  suggested action, destructive action, insensitive, link, toggle, check,
  radio, switch, and disabled variants.
- Text and input: label variants, editable/read-only/password/search/selected/
  disabled entries, placeholder, primary and secondary icons, entry progress,
  tooltip, spin buttons, combo and editable combo, file/font/color/app chooser
  buttons, and volume button.
- Ranges and feedback: horizontal/vertical/marked/inverted/disabled scales,
  determinate/inverted/pulsing/disabled progress, continuous low/middle/full
  level bars, discrete level bar, native scrollbars, scale button, spinners,
  four info-bar types, calendar, image, and drawing area.
- Data views: flat and hierarchical tree models, pixbuf/text/toggle/progress
  cell renderers, grid lines, sortable/resizable columns, icon view, selected
  list-box rows, multi-select flow box, and tagged/selected text view.
- Containers: frame shadow types, separators, expanders, notebook, paned,
  grid, overlay, revealer, search bar, stack, stack switcher, stack sidebar,
  centered box layout, action bar, and scrolled windows.
- Dialogs and special windows: all message types, custom and CSD dialogs,
  file/recent/color/font/application choosers, about, assistant, and keyboard
  shortcuts window.
- Specialized widgets: embedded recent/app/file/font/color choosers and the
  file-system places sidebar.

### GTK 4.14

- Application chrome: native client-side header bar and title buttons,
  popover menubar, stateful model menus, nested menus, check and radio action
  states, menu buttons, content popover, context popover menu, emoji chooser,
  action bar, standalone window controls, and title/subtitle surfaces.
- Buttons and controls: normal, mnemonic, icon, circular, flat, linked,
  suggested, destructive, insensitive, link, toggle, check, grouped check
  (radio role), inconsistent, switch, and disabled variants.
- Modern input: normal/read-only/disabled/search/password-with-reveal entries,
  editable label, placeholders, icons, entry progress, selected text, tooltip,
  spin buttons, standard/searchable/disabled drop-downs, font/color/application
  chooser buttons, and volume button.
- Ranges and feedback: marked horizontal and vertical scales, value positions,
  inversion and insensitive states, determinate/inverted/pulsing/disabled
  progress, continuous and discrete level bars, native scrollbars, vertical
  progress/level variants, spinners, four info-bar types, calendar, image,
  drawing area, separators, picture, empty video, and media controls.
- GTK 4 data model: signal list-item factories, `GtkStringList`, single
  selection, `GtkListView`, `GtkGridView`, `GtkColumnView`, resizable columns,
  row/column separators, list box, flow box, and tagged/selected text view.
- Containers: frame, expander, notebook, paned, grid, stack, stack switcher,
  stack sidebar, overlay, revealer, search bar, center box, action bar, fixed
  layout, aspect frame, scrolled windows, and boxes.
- Dialogs and specialized widgets: all message types, header-bar custom
  dialog, file/color/font/application choosers, about, assistant, keyboard
  shortcuts, plus embedded file/font/color/application chooser widgets.
- Rendering surfaces: picture, drawing area, video, and media controls are
  embedded. `GtkGLArea` is created by the **Open GL area** button so machines
  without DRI/EGL support are not forced to create an additional GL context
  merely by opening the audit. The GTK 4 renderer itself may still report the
  host's missing acceleration support.

## Scope boundaries

The galleries cover visible GTK widget families and their themeable states.
Nonvisual infrastructure such as clipboard ownership, accessibility buses,
input-method protocols, application actions themselves, socket/plug process
embedding, and print backends is outside a theme audit. File, recent,
application, font, and color surfaces can vary with installed providers and
host data; the audits still use the native shared GTK widgets for them.

Widgets removed by a later toolkit are represented by that toolkit's native
replacement rather than recreated. In particular, GTK 4 uses drop-downs and
factory-backed list/grid/column views instead of GTK 3 combo boxes,
tree views, icon views, and cell renderers; its popover menu model replaces
the removed traditional menu widgets; and GTK 4 scrollbars expose no native
stepper buttons.

**Dither current wallpaper** in Extras applies a retro ordered dither. Choose
**Color count**: 2, 4, 8, 16, 32, 64 (default),
128, or 256. The palette is extracted from representative original-image pixels,
including at 2 and 4 colors. This is the maximum palette size; simple images may
use fewer colors. Fully transparent sampled pixels do not influence the palette.
**Pixel size** accepts 1–32 image pixels (default: 1). Larger values create
chunkier square blocks, with partial blocks at image edges. Desktop scaling
may change their on-screen size. Changing either control reprocesses the original.
The action creates and selects a new PNG under `~/.local/share/backgrounds/quartz-extras/dithered`
(or `$XDG_DATA_HOME/backgrounds/quartz-extras/dithered`). The source image is
never modified. Image dimensions, transparency, and desktop placement settings
are preserved. **Restore original wallpaper** returns to the source, including
after restarting Settings. Repeated dithering starts from that original.
Local static images supported by GdkPixbuf are accepted; slideshow XML and
missing images report an error without changing the desktop. This action is
opt-in and does not run during theme or Extras installation.

Dithering regression tests (requires Python GObject/GdkPixbuf, included in MATE):
`python3 -m unittest discover -s ../quartz-extras/dither -v`.

Check actual native/heading raster pixels after installing typography:
`python3 quartz-settings/tests/check-typography-rendering.py` (from the repository root).

Verify midpoint and point/pixel size matching after installation:
`python3 quartz-settings/tests/check-typography-sizes.py`.

`make -C quartz-settings check-font-rendering` validates every bundled community
face through native Pango/Cairo and checks nearest-size matching for all fonts.
