# Full System 7 bitmap collection

These 54 OTB files were imported from the local Quartz 3 System 7.0.1 font
pipeline (`ui/assets/fonts/built/bitmap`). Its canonical source is Apple's
North American System 7.0.1 Fonts disk, using the same pinned disk image hash
recorded in `Geneva-SOURCE.md`.

The source pipeline packages original NFNT strikes using Bits'N'Picas 2.2,
preserves original glyphs and metrics, and packs its binary EBDT samples to one
bit for XRender compatibility. No outline fonts or synthesized strikes are
included here. `System7-BITMAP-SHA256SUMS` records the upstream and imported
artifact hashes.

`import-system7-bitmap.py` prefixes internal family names with `System 7` to
avoid replacing installed outline families or the existing Geneva alias. It
verifies that all tables other than naming and the checksum-bearing `head`
table remain byte-identical. Geneva's source italic strike receives a distinct
family so the chooser can expose it. Rebuilding requires Python fontTools;
installation uses only Python's standard library and Fontconfig.

The original glyphs are copyright Apple and their original designers. No
license grant from Apple is implied by their inclusion here.
