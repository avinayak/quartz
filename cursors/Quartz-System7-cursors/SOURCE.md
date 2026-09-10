# Quartz System 7 cursor sources

This theme preserves the five black-and-white cursors that Apple documented as
the standard Macintosh cursor set: the ROM arrow and the System-file I-beam,
crosshair, plus, and wristwatch resources. Every image retains its original
16-by-16 grid and hotspot; no tracing, resampling, smoothing, or antialiasing is
used.

The period *Inside Macintosh: Imaging With QuickDraw* documentation names the
standard arrow in
[About the Cursor](https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-370.html)
and identifies the other four common cursor resources in
[Using Cursor Utilities](https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-371.html).

The arrow data and mask occur at byte offset `124240` in the Macintosh II ROM
image whose SHA-256 is
`cc6d754cfa7841644971718ada1121bc5f94ff954918f502a75abb0e6fd90540`.
The same 68-byte cursor appears as `CURDATA` in Apple's original QuickDraw
`GrafAsm.a` source, published by the Computer History Museum at
<https://computerhistory.org/blog/macpaint-and-quickdraw-source-code/>. The
linked `102658076_quickdraw_acc.zip` has SHA-256
`fd609319e4a530995673440dadf0a6b75fc8322f2b96fec1d37ed890c2275b46`.
The arrow's decoded-resource SHA-256 is
`f7861225708284b299d2bbf5bc659472df5c8708b3789c3c5c2d341d78313e7d`.

The other four cursors are `CURS` resources 1 through 4 in the System file on
the Install 1 disk of Apple's North American System 7.0.1 installer. The pinned
installer is `System_7.0.1.smi.bin`, SHA-256
`d78e1c52a6f16ba14907cbe816a22a43f8aa8e1a9c4f664d3b7d5d982a9baab2`,
from <https://ftp.zx.net.nz/pub/micro/macintosh/System_7.0.1.smi.bin>.
The decoded resources are:

| Cursor | Resource | Hotspot (x, y) | Decoded-resource SHA-256 |
| --- | ---: | ---: | --- |
| I-beam | 1 | 7, 4 | `a99ec0e60771ada5a580bbddf4e57924375ccf51250d8c48831de5c0ee0f7908` |
| Crosshair | 2 | 5, 5 | `43587b642a37018ff30b6b3a6cc14c9b3c226facea7c4186e117607ed0ff02ec` |
| Plus | 3 | 8, 8 | `0ea7f1a945fdf23f8ef332097182c337aeab8d2780ecc26c4daef6caf11d5e98` |
| Wristwatch | 4 | 8, 8 | `3226d4d77426a9da25fbc0a74920b6242d83c6b9805ca04103c741f872c48829` |

Classic QuickDraw combined one data bit and one mask bit per pixel. A set data
bit becomes black, a mask-only bit becomes white, and an empty pair becomes
transparent in the Xcursor conversion. The original I-beam and crosshair used
XOR pixels so they inverted whatever was underneath. Xcursor's ARGB format has
no backdrop-inversion operation, so those exact pixels are opaque black here;
their silhouette, grid, and hotspot remain unchanged.

## Desktop mappings and unavailable states

The arrow covers `default`, `left_ptr`, and `top_left_arrow`; the I-beam covers
`text` and `xterm`; the crosshair covers the standard X crosshair aliases; and
the plus covers `cell`. The wristwatch covers `wait`, `watch`, `progress`, and
`left_ptr_watch`. `progress` and `left_ptr_watch` are modern distinctions, so
they deliberately reuse the sole original Macintosh busy cursor rather than
inventing new artwork.

System 7 did not provide shared standard artwork for these modern desktop
states:

- link/pointer and help/context-menu;
- vertical text;
- drag copy, alias, move, no-drop, not-allowed, grab, and grabbing;
- directional, row, column, and all-direction resize/scroll;
- zoom in and zoom out.

Those names are intentionally absent and inherit from Adwaita so applications
retain truthful pointer feedback. The additional `CURS` -5760 found in the
System file belongs to the matching color-picker dialog resource group, not to
the documented shared cursor set, so it is not repurposed as a desktop state.
An invisible `none` cursor is synthesized by the toolkit and needs no theme
artwork.

Run `./build-cursors.sh` to regenerate the checked-in Xcursor binaries. It
requires ImageMagick and `xcursorgen`; neither tool is required when installing
the prebuilt theme.

The original cursor pixels are copyright Apple and their original designers.
No license grant from Apple is implied by their inclusion here.
