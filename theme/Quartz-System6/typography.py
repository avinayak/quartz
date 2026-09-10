#!/usr/bin/env python3
"""Shared desktop typography; no application-private settings."""
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time
import xml.etree.ElementTree as ET

CONFIG = Path(os.environ.get('XDG_CONFIG_HOME', Path.home() / '.config'))
STATE = CONFIG / 'quartz-settings/typography.json'
BEGIN = '/* quartz-typography:begin */'
END = '/* quartz-typography:end */'


def catalog(include_legacy=False):
    result = {}
    rows = subprocess.check_output(['fc-list', '--format', '%{family[0]}|%{pixelsize}|%{scalable}\n'], text=True)
    for row in rows.splitlines():
        family, sizes, scalable = row.split('|')
        if not re.fullmatch(r'[\w .+-]+', family):
            continue
        if scalable == 'False':
            result.setdefault(family, set()).update(float(x) for x in sizes.split(',') if x)
        elif family == 'ChiKareGo2':
            result[family] = {16.0}
        elif family in ('Pixel Operator', 'Pixel Operator Mono', 'GNU Unifont', 'Unifont', 'Fixedsys Excelsior 3.01', 'PxPlus IBM VGA8'):
            result.setdefault(family, set()).update((16.0, 32.0, 48.0, 64.0))
    if not include_legacy and 'System 7 Geneva' in result:
        result = {family: sizes for family, sizes in result.items()
                  if not re.fullmatch(r'System 7 Geneva [0-9]+', family)}
    return {family: sorted(sizes) for family, sizes in sorted(result.items()) if sizes}


def migrate_fonts(values):
    available = catalog()
    migrated = []
    for value in values:
        old = re.fullmatch(r'System 7 Geneva ([0-9]+) [0-9.]+px', value)
        if old and 'System 7 Geneva' in available:
            size = min(available['System 7 Geneva'], key=lambda size: (abs(size - float(old[1])), size))
            value = f'System 7 Geneva {size:g}px'
        migrated.append(value)
    return migrated


def validate(values):
    available = catalog()
    if len(values) != 3:
        raise ValueError('Choose a menu, title, and application font.')
    parsed = []
    for value in values:
        match = re.fullmatch(r'(.+) ([0-9]+(?:\.[0-9]+)?)px', value)
        if not match or float(match[2]) not in available.get(match[1], []):
            raise ValueError('Font or native size is unavailable: ' + value)
        parsed.append((match[1], float(match[2])))
    return parsed


def managed(path, block, begin=BEGIN, end=END):
    text = path.read_text() if path.exists() else ''
    text = re.sub(re.escape(begin) + r'.*?' + re.escape(end) + r'\n?', '', text, flags=re.S)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.quartz-tmp')
    temporary.write_text(text.rstrip() + '\n' + begin + '\n' + block + '\n' + end + '\n')
    temporary.replace(path)


def install_pixel_rendering_policy(families):
    root = ET.Element('fontconfig')
    # Retire the original Chicago-to-Geneva size fallback. Typography now
    # retains the chosen family and selects its nearest native size.
    legacy = CONFIG / 'fontconfig/conf.d/99-quartz-chikarego2.conf'
    if legacy.exists():
        tree = ET.parse(legacy)
        changed = False
        for match in list(tree.getroot()):
            if (match.find("test[@name='family']/string") is not None
                    and match.find("test[@name='family']/string").text == 'ChiKareGo2'
                    and match.find("edit[@name='family']") is not None):
                tree.getroot().remove(match)
                changed = True
        if changed:
            tree.write(legacy, encoding='unicode', xml_declaration=True)
    for family in sorted(set(families)):
        if isinstance(families, dict):
            sizes = sorted(families[family])
            match = ET.SubElement(root, 'match', target='pattern')
            ET.SubElement(ET.SubElement(match, 'test', name='family', compare='eq', qual='any'), 'string').text = family
            edit = ET.SubElement(match, 'edit', name='pixelsize', mode='assign')
            node = edit
            for lower, upper in zip(sizes, sizes[1:]):
                branch = ET.SubElement(node, 'if')
                # Compare point requests against DPI-adjusted boundaries.
                # Do not multiply 'size': Fontconfig also represents it as a range.
                condition = ET.SubElement(branch, 'if')
                has_pixels = ET.SubElement(condition, 'more')
                ET.SubElement(has_pixels, 'name').text = 'pixelsize'
                ET.SubElement(has_pixels, 'double').text = '0'
                pixel_test = ET.SubElement(condition, 'less_eq')
                ET.SubElement(pixel_test, 'name').text = 'pixelsize'
                ET.SubElement(pixel_test, 'double').text = str((lower + upper) / 2)
                point_test = ET.SubElement(condition, 'less_eq')
                ET.SubElement(point_test, 'name').text = 'size'
                boundary = ET.SubElement(point_test, 'divide')
                ET.SubElement(boundary, 'double').text = str((lower + upper) * 36)
                dpi = ET.SubElement(boundary, 'if')
                positive = ET.SubElement(dpi, 'more')
                ET.SubElement(positive, 'name').text = 'dpi'
                ET.SubElement(positive, 'double').text = '0'
                ET.SubElement(dpi, 'name').text = 'dpi'
                ET.SubElement(dpi, 'double').text = '96'
                ET.SubElement(branch, 'double').text = str(lower)
                node = branch
            ET.SubElement(node, 'double').text = str(sizes[-1])
            if family == 'ChiKareGo2':
                # This outline revival has exactly one native grid.
                ET.SubElement(ET.SubElement(match, 'edit', name='size', mode='assign'), 'double').text = '12'
        if family == 'ChiKareGo2':
            native = ET.SubElement(root, 'match', target='pattern')
            ET.SubElement(ET.SubElement(native, 'test', name='family', compare='eq', qual='any'), 'string').text = family
            for name, value in (('pixelsize', '16'), ('size', '12')):
                ET.SubElement(ET.SubElement(native, 'edit', name=name, mode='assign'), 'double').text = value
        for target in ('pattern', 'font'):
            match = ET.SubElement(root, 'match', target=target)
            ET.SubElement(ET.SubElement(match, 'test', name='family', compare='eq', qual='any'), 'string').text = family
            for property_name in ('antialias', 'embolden'):
                ET.SubElement(ET.SubElement(match, 'edit', name=property_name, mode='assign'), 'bool').text = 'false'
            if target == 'font':
                # Fontconfig's bitmap scaling rule otherwise adds a fractional
                # matrix for headings and relative text sizes. Cairo filters
                # that transformed bitmap even when antialias is disabled.
                matrix = ET.SubElement(ET.SubElement(match, 'edit', name='matrix', mode='assign'), 'matrix')
                for value in (1, 0, 0, 1):
                    ET.SubElement(matrix, 'double').text = str(value)

    ET.indent(root)
    path = CONFIG / 'fontconfig/conf.d/99-quartz-typography-pixels.conf'
    text = '<?xml version="1.0"?>\n<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">\n' + ET.tostring(root, encoding='unicode') + '\n'
    if not path.exists() or path.read_text() != text:
        path.parent.mkdir(parents=True, exist_ok=True)
        temporary = path.with_suffix('.tmp')
        temporary.write_text(text)
        temporary.replace(path)


def remove_user_font_overrides(path):
    if not path.exists():
        return
    text = path.read_text()
    text = re.sub(re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n?', '', text, flags=re.S)
    # The initial installer put fixed fonts in its shared user-provider block.
    # Theme providers cannot override those higher-priority declarations.
    text = re.sub(r'(/\* quartz-system7-fonts:begin \*/)(.*?)(/\* quartz-system7-fonts:end \*/)',
                  lambda m: m[1] + re.sub(r'^\s*font-(?:family|size|style|weight):[^;]+;\s*$', '', m[2], flags=re.M) + m[3],
                  text, flags=re.S)
    if text != path.read_text():
        path.write_text(text)


def refresh():
    for schema in ('org.mate.interface', 'org.gnome.desktop.interface'):
        current = subprocess.check_output(['gsettings', 'get', schema, 'gtk-theme'], text=True).strip().strip("'")
        if current != 'Quartz-System6':
            continue
        fallback = next((p.parent.name for root in (Path.home() / '.themes', Path('/usr/share/themes'))
                         for p in sorted(root.glob('*/gtk-3.0')) if p.parent.name != current), None)
        if fallback is None:
            raise ValueError('No alternate GTK theme is available to refresh the desktop.')
        subprocess.run(['gsettings', 'set', schema, 'gtk-theme', fallback], check=True)
        time.sleep(0.15)
        subprocess.run(['gsettings', 'set', schema, 'gtk-theme', current], check=True)


def pango_family(family):
    # A trailing comma prevents Pango from treating a family suffix such as
    # Bold, Italic, or Oblique as a request to synthesize another style.
    return family + ',' if family.split()[-1].lower() in ('bold', 'italic', 'oblique', 'light', 'medium', 'regular', 'book') else family


def pango_pixels(font):
    return f'{pango_family(font[0])} {font[1]:g}px'


def points_for_pixels(font, dpi):
    if dpi <= 0:
        dpi = 96.0
    return f'{pango_family(font[0])} {font[1] * 72.0 / dpi:g}'


def title_points(font):
    # Marco rescales its Pango description as points, even for an absolute-size
    # description. Supply points using the live GTK screen resolution.
    import gi
    gi.require_version('Gdk', '3.0')
    from gi.repository import Gdk
    screen = Gdk.Screen.get_default()
    dpi = screen.get_resolution() if screen is not None else 96.0
    return points_for_pixels(font, dpi)


def apply(values):
    values = migrate_fonts(values)
    menu, title, application = validate(values)
    install_pixel_rendering_policy(catalog(include_legacy=True))
    def css(font):
        family, size = font
        return f'font-family: "{family}"; font-size: {size:g}px; font-weight: normal; font-style: normal;'
    block = '* { ' + css(application) + ' }\n'
    block += ('menubar, menubar *, window.background menubar, window.background menubar *, '
              'dialog.background menubar, dialog.background menubar *, menu, menu *, .menu, .menu *, '
              '.context-menu, .context-menu *, popover.menu, popover.menu *, popover.background.menu, '
              'popover.background.menu *, .mate-panel-menu-bar, .mate-panel-menu-bar * { ' + css(menu) + ' }\n')
    block += 'headerbar.titlebar label.title, .titlebar .title, headerbar .title { ' + css(title) + ' }'
    for version in ('3.0', '4.0'):
        remove_user_font_overrides(CONFIG / f'gtk-{version}/gtk.css')
        managed(Path.home() / f'.themes/Quartz-System6/gtk-{version}/gtk.css', block)
    rc = f'gtk-font-name = "{pango_pixels(application)}"\n'
    for name, font in [('application', pango_pixels(application)), ('menu', pango_pixels(menu)), ('title', pango_pixels(title))]:
        rc += f'style "quartz-type-{name}" {{ font_name = "{font}" }}\n'
    rc += 'class "GtkWidget" style : highest "quartz-type-application"\n'
    rc += 'widget_class "*<GtkMenuItem>*" style : highest "quartz-type-menu"\n'
    rc += 'widget_class "*<GtkMenuBar>*" style : highest "quartz-type-menu"\n'
    managed(Path.home() / '.gtkrc-2.0', rc, '# quartz-typography:begin', '# quartz-typography:end')
    for schema, key, value in [
        ('org.mate.interface', 'font-name', pango_pixels(application)),
        ('org.mate.interface', 'document-font-name', pango_pixels(application)),
        ('org.mate.Marco.general', 'titlebar-uses-system-font', 'false'),
        ('org.mate.Marco.general', 'titlebar-font', title_points(title)),
    ]:
        subprocess.run(['gsettings', 'set', schema, key, value], check=True)
    STATE.parent.mkdir(parents=True, exist_ok=True)
    temporary = STATE.with_suffix('.tmp')
    temporary.write_text(json.dumps(values) + '\n')
    temporary.replace(STATE)
    refresh()


def main():
    if sys.argv[1:] == ['--catalog']:
        for family, sizes in catalog().items():
            print(family + '\t' + ','.join(f'{size:g}' for size in sizes))
    elif sys.argv[1:] == ['--current']:
        print('\n'.join(migrate_fonts(json.loads(STATE.read_text()) if STATE.exists() else ['ChiKareGo2 16px', 'ChiKareGo2 16px', 'System 7 Geneva 12px'])))
    elif sys.argv[1:] == ['--restore']:
        if STATE.exists():
            apply(json.loads(STATE.read_text()))
    elif len(sys.argv) == 5 and sys.argv[1] == '--apply':
        apply(sys.argv[2:])
    else:
        raise ValueError('Expected --catalog, --restore, or --apply MENU TITLE APPLICATION')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        sys.exit(str(error))
