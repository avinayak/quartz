#!/usr/bin/env python3
"""Namespace the source pipeline's OTBs without changing bitmap/metric tables.
Usage: python3 import-system7-bitmap.py SOURCE_BITMAP_DIRECTORY
Build-only dependency: fontTools.
"""
from pathlib import Path
import hashlib
import sys
from fontTools.ttLib import TTFont

output = Path(__file__).resolve().parent / 'system7-bitmap'
output.mkdir(exist_ok=True)
manifest = []
for source in sorted(Path(sys.argv[1]).glob('*.otb')):
    family, style, size = source.stem.rsplit('-', 2)
    family = 'System 7 ' + family.replace('-', ' ')
    if style != 'Regular':
        family += ' ' + style
    font = TTFont(source, recalcTimestamp=False)
    original = {tag: font.getTableData(tag) for tag in font.keys() if tag not in ('GlyphOrder', 'name', 'head')}
    for record in font['name'].names:
        if record.nameID in (1, 16):
            value = family
        elif record.nameID == 4:
            value = family + ' ' + size
        elif record.nameID == 6:
            value = family.replace(' ', '-') + '-' + size
        else:
            continue
        record.string = value.encode(record.getEncoding())
    destination = output / source.name
    font.save(destination)
    saved = TTFont(destination)
    assert all(saved.getTableData(tag) == data for tag, data in original.items())
    manifest.append(f'{hashlib.sha256(source.read_bytes()).hexdigest()}  source/{source.name}\n{hashlib.sha256(destination.read_bytes()).hexdigest()}  system7-bitmap/{source.name}')
(output.parent / 'System7-BITMAP-SHA256SUMS').write_text('\n'.join(manifest) + '\n')
