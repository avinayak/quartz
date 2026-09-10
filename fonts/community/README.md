# Quartz community bitmap fonts

This offline bundle contains 89 bitmap font files: 39 selectable families and
real style variants from 11 upstream collections. Native sizes share one family
entry. Real bold/italic faces remain distinct choices; no simulated styles,
outline tracing, or rasterized outline fonts are included.

| Collection | Included faces | Upstream license |
| --- | --- | --- |
| [Spleen](https://github.com/fcambus/spleen) | All six native sizes | BSD-2-Clause |
| [Tamzen](https://github.com/sunaku/tamzen-font) | Regular, bold, and Powerline versions at all seven sizes | Tamsyn/Tamzen permissive license |
| [Cozette](https://github.com/the-moonwitch/Cozette) | Regular and crossed-seven variant | MIT |
| [Scientifica](https://github.com/oppiliappan/scientifica) | Regular, bold, italic | SIL OFL 1.1 |
| [Creep](https://github.com/romeovs/creep) | Original bitmap strike | MIT |
| [Cherry](https://github.com/turquoise-hexagon/cherry) | Regular and bold at four sizes | ISC-style permissive license |
| [Kirsch](https://github.com/molarmanful/kirsch) | Original bitmap strike | SIL OFL 1.1 |
| [Proggy](https://github.com/bluescan/proggyfonts) | Original Proggy and Webby bitmap variants | MIT |
| [Gohufont](https://github.com/hchargois/gohufont) | Unicode regular and bold at both native sizes | WTFPL v2 |
| [ProFont](https://tobiasjung.name/downloadfile.php?file=profont-otb-2.zip) | All seven native strikes in the OTB distribution | MIT |
| [X11 Fixed](https://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts.tar.gz) | Unicode regular, bold, oblique, narrow and wide variants | Public domain |

`manifest.json` records the exact upstream repository revisions or archive
hashes, source paths and SHA-256 checksums, display families, and license paths.
`inputs/` preserves those source files. `licenses/` retains the original notices
and the X11 authors list. `outputs.json` pins each built OTB's checksum and
native sizes. These notices are installed under
`~/.local/share/doc/quartz-community-fonts/` with the fonts.

Build on Ubuntu with `python3 ../build-community-fonts.py` from this directory,
after installing `fonttosfnt`, `pcf2bdf`, `fontforge-nox`, and `python3-fonttools`.
Normal installation uses the committed OTBs; it needs no build tools or network.

PCF inputs are decoded losslessly by pcf2bdf. Missing Proggy BDF name and
charset properties are recovered from the original XLFD; Latin-1 character
codes are retained unchanged and identified as their identical Unicode values. BDF inputs are wrapped using fonttosfnt with no cropping and byte-aligned
bitmap data. FontForge exports existing SFD bitmap strikes directly; it never
generates new strikes. Existing OTB files are reused. fontTools changes only
names and timestamp/checksum fields. The build verifies that all other tables,
including bitmap pixels and metrics, remain byte-identical to the wrapped
input. It also checks one-bit bitmap depth, absence of visible outlines, and
Fontconfig's non-scalable classification.

The one deliberate naming adjustment places genuine style variants in their
own families so Quartz's family chooser can select them. Pango descriptions
quote ambiguous style-suffixed family names with a trailing comma. All added
fonts use the shared nearest-native-size and no-smoothing policy.
