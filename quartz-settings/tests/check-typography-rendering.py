# Run after installing the shared typography policy; requires python3-cairo
# and python3-gi. No desktop settings are modified.
import gi
import cairo
gi.require_version('Pango', '1.0')
gi.require_version('PangoCairo', '1.0')
from gi.repository import Pango, PangoCairo
for size in (12, 15, 18, 21, 24):
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 450, 50)
    cr = cairo.Context(surface)
    cr.set_source_rgb(1, 1, 1)
    cr.paint()
    cr.set_source_rgb(0, 0, 0)
    layout = PangoCairo.create_layout(cr)
    layout.set_font_description(Pango.FontDescription(f'System 7 Geneva {size}px'))
    layout.set_text('Shared Quartz appearance', -1)
    PangoCairo.show_layout(cr, layout)
    surface.flush()
    colors = set(bytes(surface.get_data()))
    print(size, sorted(colors))
    assert colors <= {0, 255}, (size, colors)
print('Native Pango/Cairo rendering is one-bit at native and relative heading sizes.')
