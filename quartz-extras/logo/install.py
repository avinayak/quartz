#!/usr/bin/env python3
"""Select a shared icon theme overlay containing only Quartz desktop logos."""
import argparse
import configparser
import os
from pathlib import Path
import shutil
import subprocess
import sys

THEME = 'Quartz-Desktop-Logo'
NAMES = ('start-here', 'start-here-symbolic', 'distributor-logo',
         'distributor-logo-symbolic', 'mate-logo', 'quartz-logo')


def install_theme(destination, base):
    if not base or base == THEME or any(c in base for c in '\n\r,'):
        raise ValueError('Invalid inherited icon theme: ' + base)
    destination.mkdir(parents=True, exist_ok=True)
    icons = destination / 'scalable/places'
    icons.mkdir(parents=True, exist_ok=True)
    source = Path(__file__).with_name('quartz-logo.svg')
    for name in NAMES:
        shutil.copyfile(source, icons / (name + '.svg'))
    (destination / 'index.theme').write_text(
        '[Icon Theme]\nName=Quartz Desktop Logo\n'
        'Comment=Quartz diamond with the current desktop icons\n'
        f'Inherits={base}\nDirectories=scalable/places\n\n'
        '[scalable/places]\nSize=32\nMinSize=8\nMaxSize=512\n'
        'Context=Places\nType=Scalable\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh-if-enabled', action='store_true')
    args = parser.parse_args()
    config = Path(os.environ.get('XDG_CONFIG_HOME', Path.home() / '.config'))
    enabled = config / 'quartz-settings/desktop-logo-enabled'
    if args.refresh_if_enabled and not enabled.exists():
        return
    from gi.repository import Gio
    settings = Gio.Settings.new('org.mate.interface')
    if not settings.is_writable('icon-theme'):
        raise ValueError('The desktop icon theme setting is locked.')
    data = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share'))
    destination = data / 'icons' / THEME
    base = settings.get_string('icon-theme')
    if base == THEME:
        index = configparser.ConfigParser(interpolation=None)
        index.read(destination / 'index.theme')
        base = index.get('Icon Theme', 'Inherits')
    install_theme(destination, base)
    subprocess.run(['gtk-update-icon-cache', '--force', '--quiet',
                    str(destination)], check=True)
    if not settings.set_string('icon-theme', THEME):
        raise ValueError('Could not select the Quartz desktop logo.')
    schemas = Gio.SettingsSchemaSource.get_default()
    if schemas.lookup('org.gnome.desktop.interface', True):
        gnome = Gio.Settings.new('org.gnome.desktop.interface')
        if gnome.is_writable('icon-theme'):
            gnome.set_string('icon-theme', THEME)
    Gio.Settings.sync()
    enabled.parent.mkdir(parents=True, exist_ok=True)
    enabled.write_text('enabled\n')
    print('Quartz diamond selected for desktop menus.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, ImportError, configparser.Error,
            subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
