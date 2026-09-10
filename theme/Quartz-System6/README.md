# Quartz theme source

`theme.conf` is the shared source for geometry, window layout, and the three
user-facing background roles. `QUARTZ_BORDER_RADIUS_PX` controls rounded
corners in GTK 2, GTK 3, and GTK 4; use an integer from `0` through `64`
pixels. A value of `0` makes every theme-controlled rounded corner square.
`QUARTZ_WINDOW_LAYOUT` accepts:

- `cupertino`: the System 7 arrangement, with close on the left, a centered
  title, and maximize/zoom on the right. This is the default.
- `redmond`: a left-aligned title and minimize, maximize, close on the right.
- `redmond-reversed`: close, minimize, maximize on the left and a right-aligned
  title.

The two Redmond layouts use compact six-pixel gaps between the visible control
boxes.

The palette keys are:

- `QUARTZ_MENUBAR_BG` for application menu bars and MATE panels.
- `QUARTZ_TITLEBAR_BG` for Marco and GTK client-side title bars.
- `QUARTZ_APPLICATION_BG` for general application chrome and content surfaces,
  including generic icon, list, tree, text, viewport, sidebar, and notebook
  views across GTK 2, GTK 3, and GTK 4.

Colors use opaque `#RRGGBB` values. Quartz Settings exposes the three roles as
color pickers, supplies 20 flat presets plus 20 mixed presets, and persists the
selected layout and palette in `~/.config/quartz-settings/theme.conf`.

The toolkit files are templates because GTK generations do not share one
configuration syntax. `render-theme.sh` turns them into a loadable theme and
also generates the palette-matched GTK 2 menu-bar bitmap, GTK 3 menu-frame
nine-slice, and MATE panel bitmap. GTK 3 menu frames and panel bars receive
content-addressed copies, so a changed palette always has a new asset URI in a
long-running process. The renderer also derives the GTK 2 entry paint inset
from the same radius so the entry's text surface cannot cover the curved
frame. The top-level installer and Quartz Settings both use this renderer.

MATE panel layout loaders reset the relocatable toplevel settings, including
panel size and background. The installed shared `quartz-panel-theme.service`
observes those settings and restores the generated bitmap plus 25-pixel
fully opaque geometry to every horizontal panel created by a layout change.
Layout-owned panel placement and applet composition remain untouched; the
service becomes inert whenever `Quartz-System6` is not the selected GTK theme.
It coalesces each layout loader's dconf notification burst after one second of
quiet, then waits for the panel PID and toplevel list to remain stable before
reconciling the completed tree. All watcher and synchronous refresh writes use
one shared lock, so palette and layout events cannot interleave two GSettings
passes.
Palette refreshes use a content-addressed, valid panel bitmap path so
mate-panel reloads changed pixels without briefly removing the background from
hosted applets, while an unchanged palette remains idempotent. Neither the
installer nor Quartz Settings replaces the panel for a palette change. Quartz
Settings also leaves an active watcher process running rather than restarting
it on every Apply. Generic
panel, in-process applet, menubar, and GtkPlug nodes carry the same background
and outer rules, so expanding applets cannot uncover a stale strip after a
layout reset. The installer also enables MATE's shared desktop surface, which
preserves the desktop context menu independently of the chosen layout.

Palette changes ask Marco to reload the shared frame theme in place. This
updates existing title bars without dropping the compositor or replacing the
window-manager process, so dock-backed layouts remain mapped during Apply.

Pressed close, maximize/restore, and minimize titlebar controls use the same
native 11×11 System 7 burst sprite in Marco and GTK 3/4 client-side decorations.
The pixels follow `GoAwayHiliteData` / `ZoomHiliteData` in the original
[StandardWDEF source](https://github.com/elliotnunn/supermario/blob/master/base/SuperMarioProj.1994-02-09/Toolbox/WindowMgr/StandardWDEF.a#L2523).
System 7 shared this highlight between close and zoom; Quartz extends it to
minimize. Transparent paper pixels reveal the selected titlebar color. The
sprite appears only while the pointer is held inside the control and clears
on release or drag-out, with no animation or scaling. GTK 2 windows receive
it through Marco. Custom window controls outside these shared components
remain a compatibility limitation.

The top-level installer migrates only Marco's explicit no-compositor session
personality to standard Marco and enables its built-in compositor. This common
desktop behavior lets the dock-backed Cupertino, Mutiny, and Pantheon MATE
Tweak layouts run while retaining Quartz's hard frame geometry. A different
configured window manager is left in place. The dock surface itself remains
owned by the layout's Plank theme; Quartz does not modify that application's
private theme preference.

The metatheme selects the separate shared `Quartz-System7-cursors` pointer
theme at its native 16-pixel size. Its five original Macintosh cursor types
and all deliberately unsupported modern states are documented in
`cursors/Quartz-System7-cursors/SOURCE.md`; the top-level installer deploys and
selects it for MATE and GTK.

GTK 3 and GTK 4 spinners cycle through eight native 12×12 symbolic frames
while active. A moving gap provides visible motion without rotated pixels or
crossfades; stopped spinners are blank and disabled active spinners are static.
The installer enables GTK's animation setting so functional busy indicators can
run. Shared CSS continues to disable animations and transitions by default,
with only active spinners opting in.

Scale value fills and scrollbar thumbs in GTK 2, GTK 3, and GTK 4 cast their
hard one-pixel shadow straight down. Both side outlines remain one pixel wide;
the shadow follows the pixel-cut corners without a rightward offset, including
vertical and inverted ranges. GTK 2 uses shared pixmaps and GTK 3/4 use hard
gradient masks for the same geometry.

Raised content buttons in all three GTK generations use a one-pixel black
shadow whose silhouette is derived from that same radius. GTK 4 titlebar and
window-control buttons remain borderless and unraised. Linked GTK 4 entries
in header bars retain their complete outline and outer corners because the
adjacent borderless buttons cannot supply a shared frame. While a momentary
content button is pressed, its surface and contents move down one pixel into
the shadow position and the shadow disappears. Pressed and latched faces use
an opaque shade calculated from the selected application color (235/255 per
channel), identically in GTK 2, GTK 3, and GTK 4. Disabled faces use a deeper
217/255 shade and disabled ink uses 119/255, so colored palettes never fall
back to neutral white or gray controls. Classic white retains the historical
`#ebebeb`, `#d9d9d9`, and `#777777` System 7 values by construction. GTK 2
exposes the persistent latch and pointer-down through the same `ACTIVE` theme
state, so its general `GtkToggleButton` style retains its stable raised
fallback. Flat GTK 3/4 buttons remain borderless and unraised; hover adds only
the application-derived shade without moving the control.
Suggested and destructive actions use ordinary button geometry with pale
`#ddf1d8` green and `#f4d6d6` red faces respectively.
Hover tooltips use a pale amber `#ffecb3` face with black text and a single
one-pixel black outline across GTK 2, GTK 3, and GTK 4.
GTK 3/4 switches use a compact 32×16 track with an outlined paper 12×12 thumb and
two-pixel vertical margins. The renderer samples the shared radius into
one-bit artwork, clamping it to each component's size. Native nine-slice
corners keep the track crisp when its allocation changes; the thumb is
unscaled. Off places the thumb on the left of a plain paper track. On moves
the same thumb right and exposes the scrollbar's native 4×2 dither tile in
the left half. The dither repeats at its original size instead of stretching.
Disabled ink derives from the application palette. Toolkit I/O glyphs are
hidden, leaving position and fill to indicate state. Palette/radius-specific
asset names allow running processes to reload the new pixels in place.
Every GTK 3 and GTK 4 focus outline uses a one-pixel dotted contour three
pixels inside the focused surface. Buttons retain their normal outer geometry,
and the contour follows the control's final shared or per-corner radius instead
of drawing a second solid rectangular frame. GTK 2 widgets that expose the
native focus primitive use the equivalent one-pixel-on, one-pixel-off pattern.
On MATE panels, direct flat-toggle menu launchers and bottom-panel button
segments use square corners. Ordinary top-panel status-applet buttons retain
the shared theme radius.
Horizontal GTK 3/4 spin buttons leave their internal text surface transparent,
so the rounded parent remains the sole owner of every outer corner pixel.
Vertical GTK 3/4 spin buttons use fully outlined upper and lower steppers
joined by a text surface with side borders. Disabled stacks retain the same
geometry with the shared disabled palette. Both native audit galleries include
enabled and disabled vertical examples.

Generic GTK 3 and GTK 4 frames use explicit one-pixel black/paper stops around
all four edges, so the outline remains visibly pixel-dotted instead of merging
into GTK's antialiased round-dot stroke. The renderer derives each corner's
one-bit stair-step pixels from the shared radius and uses full-span hard-stop
rows instead of a vector radius clip, preventing gray coverage pixels at the
turn. At radii 40–64, four native-size one-bit corner sprites carry the same
dot coordinates, keeping GTK 4 below its 128-layer CSS-array limit. Straight
edges remain repeating gradients; the sprites are not stretched or nine-sliced.
Their geometry-versioned, radius-specific names remain stable across palettes
because they contain only black ink and transparency.
An opaque three-pixel label margin preserves the intentional frame gap.
GTK 2 retains its solid rounded frame: its stock pixmap engine stretches a
nine-slice edge instead of repeating it, so a dotted bitmap would turn into
size-dependent bars rather than an even outline. Notebook tabs use the shared
radius on the two exposed corners and leave the page-facing edge open.
Across GTK 2, GTK 3, and GTK 4, pages have a one-pixel outline on all sides,
with an opening beside the selected tab. The frame follows top, bottom, left,
or right tab placement. Tabs retain the inset dotted keyboard-focus contour.
GTK 3 scrolled views that request a native frame receive a solid one-pixel
enclosure, including tables, icon views, and text views. Unframed scroll
containers remain unframed. Flat column headers provide single internal
separators without adding a second outer outline or moving on press.
Lists directly inside a framed scroll container (including GTK's viewport
adapter) defer their outline to that container, avoiding a double border.
Standalone lists retain their own outline.
Progress cell renderers use GTK's tree-view `.trough` and `.progressbar`
classes, with an inset fill and contrasting percentage text. Selected rows
invert the meter's paper/ink colors, including when the window loses focus;
disabled meters use the application-derived disabled ink.
GTK 4 column views own a single outer frame; their internal list view is
unframed so rows align with the headers. Column titles add only internal
dividers and stay flat when pressed. Standalone list views retain their frame.
Toolkit menu and popup surfaces across GTK 2, GTK 3, and GTK 4 use the shared
radius with a hard one-pixel shadow along the bottom edge only. GTK 3 combines
a renderer-generated one-bit nine-slice with the same shared CSS radius. The
radius clips GTK's stretched top and bottom border-image strips before they can
show through the bitmap's transparent corners; the content-addressed frame URI
and live palette fill let an already-running process recolor cleanly on a
shared-theme refresh. This includes GTK 2 combo popups and generic GTK 3 popup
windows used by desktop-panel dropdowns.
GTK 3 popovers use a normal CSS border instead of the menu nine-slice so GTK
can join their pointing arrow to the outline. Their model-button rows share
the 20-pixel minimum height of ordinary menu items.
Those same shared menu components use ChiKareGo2 at its native 12-point,
16-pixel design size for ordinary command text. Relative or explicitly
smaller descendants retain that family request so the installed Fontconfig
policy can substitute a native Geneva bitmap strike instead of scaling
Chicago off-grid. GTK 2 adds internal menubar padding to retain the common
25-pixel bar height with Chicago's shorter line metric.
Marco and GTK client-side window frames likewise keep their one-pixel outline
and add a hard one-pixel shadow only beneath the window, with no shadow along
either vertical edge.

The selected window-layout preset controls Marco's title position and button
arrangement plus GTK's desktop-wide client-side button arrangement. GTK
applications that leave `GtkHeaderBar:decoration-layout` unset inherit the
shared layout; an explicit application override remains authoritative. GTK 3
and GTK 4 `GtkHeaderBar` center their title widget as part of the toolkit's
layout algorithm and expose no theme-level title-alignment setting, so their
client-side titles remain centered in all three presets. This is a
shared-toolkit compatibility limitation; the theme does not patch individual
header bars to imitate Marco's left- or right-aligned title.

GTK 3 reads `$XDG_CONFIG_HOME/gtk-3.0/gtk.css` once into a process-local user
style provider. If the installer changes that file, already-running GTK
processes retain the previous provider even when the selected theme is
reapplied. The installer therefore reports when open GTK applications need to
be restarted, or when one logout/login is required to refresh the whole
session. This is a toolkit-wide lifecycle constraint; the installer does not
restart or special-case individual applications.

A custom undecorated window that merely advertises an X11 popup-menu hint but
does not expose a GTK `menu`, `popover`, or `.popup` CSS node cannot be selected
by a general toolkit rule. Such a surface retains its own window shape. The
theme intentionally does not name an application-specific CSS class or apply
an antialiased compositor mask as a workaround.
Brisk Menu's classic window is one such surface: its category and session
areas do not acquire an outer popup frame. Its framed results list still
receives the general scrolled-list styling above.

To inspect a rendered theme without installing it:

```bash
./theme/Quartz-System6/render-theme.sh /tmp/Quartz-System6
```

To render an alternate validated configuration without changing the source:

```bash
./theme/Quartz-System6/render-theme.sh --config /path/to/theme.conf /tmp/Quartz-System6
```

Scrollbar dither rails occupy the central 8 pixels of their 16-pixel lane
in GTK 2/3/4, with symmetric four-pixel gutters. The 12-pixel thumb overhangs
the rail by two pixels per side. Scrollbars retain the original sparse 4×2 staggered dither tile.

Progress bars use the stock baseline thickness: four pixels including the
outline in GTK 3/4, with a two-pixel fill. GTK 2 uses the baseline six-pixel
minimum and native text spacing; embedded text can increase its allocation.

Level bars also follow their toolkit baselines in both orientations: GTK 3
uses a 9px frame for continuous and discrete meters; GTK 4 uses 11px for
continuous meters and 4px for discrete meters.
