"""Verify installed community fonts through Pango/Cairo without changing settings."""
from pathlib import Path
import hashlib
import importlib.util
import json
import gi
import cairo

gi.require_version('Pango', '1.0')
gi.require_version('PangoCairo', '1.0')
from gi.repository import Pango, PangoCairo

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('typography', root / 'theme/Quartz-System6/typography.py')
typography = importlib.util.module_from_spec(spec)
spec.loader.exec_module(typography)
outputs = json.loads((root / 'fonts/community/outputs.json').read_text())
families = {}
for item in outputs:
    path = root / 'fonts/community' / item['file']
    assert hashlib.sha256(path.read_bytes()).hexdigest() == item['sha256'], path
    families.setdefault(item['family'], set()).update(item['sizes'])
count = 0
for family, sizes in sorted(families.items()):
    for size in sorted(sizes):
        for requested in (size, size * 1.2):
            description = Pango.FontDescription(typography.pango_pixels((family, requested)))
            assert description.get_family() == family, (family, description.to_string())
            surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 1500, 150)
            cr = cairo.Context(surface)
            cr.set_source_rgb(1, 1, 1)
            cr.paint()
            cr.set_source_rgb(0, 0, 0)
            layout = PangoCairo.create_layout(cr)
            layout.set_font_description(description)
            loaded = layout.get_context().load_font(description)
            assert loaded.describe().get_family() == family, (family, loaded.describe().to_string())
            layout.set_text('Quartz ABC abc 0123456789', -1)
            PangoCairo.show_layout(cr, layout)
            surface.flush()
            colors = set(bytes(surface.get_data()))
            assert colors == {0, 255}, (family, requested, sorted(colors))
            count += 1
print(f'Passed {count} native/heading raster checks across {len(families)} added families/styles; every sample is strictly one-bit.')
