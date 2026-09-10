# System 7 Geneva bitmap strikes

Quartz uses the original regular Geneva 9, 10, 12, 14, 18, and 24 bitmap
strikes for application UI. Each `.otb` is a bitmap-only SFNT wrapper around
the corresponding one-bit NFNT data from Apple's North American System 7.0.1
Fonts disk; no outline tracing, interpolation, hinting, or resampling is used.

The strikes were imported from the Quartz System 7 font pipeline. Its pinned
inputs are Apple's `System_7.0.1.smi.bin` image (SHA-256
`d78e1c52a6f16ba14907cbe816a22a43f8aa8e1a9c4f664d3b7d5d982a9baab2`)
and the `lapfelix/classic-mac-bitmap-fonts` parser at commit
`5eabbf8a3970a802d265a177808f340ee31a2cc9`. The pipeline transfers glyph rows
and metrics to BDF, wraps one native strike per OTB family, and verifies the
resulting advances. The separate internal family names are intentional:
Geneva 9 and Geneva 10 both occupy 12-pixel cells but contain different source
pixels.

The application-facing `Geneva` alias is implemented by the theme script's
Fontconfig policy. It maps arbitrary point- or absolute-pixel-size requests to
the nearest retained point size, forces the matching native cell size,
disables synthetic bold/italic, and disables antialiasing only for the selected
Geneva face. The same alias receives every `ChiKareGo2` request outside that
font's native 12-point, 16-pixel size, so relative small text cannot rasterize
Chicago off its design grid. A native request in either unit is recognized
before conflicting converted fields and canonicalized to a 16-pixel match:
GTK 2 may derive a non-16px value from its live display DPI, while Pango may
attach a non-12pt field to an absolute 16px CSS request. Other font families
retain the desktop's global rendering policy.

The original Geneva bitmap glyphs are copyright Apple and their original
designers. No license grant from Apple is implied by their inclusion here.
