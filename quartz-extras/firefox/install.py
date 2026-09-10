#!/usr/bin/env python3
"""Explicit, reversible Firefox opt-in. Never run by the wallpaper installer."""
import argparse
import configparser
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

PREF = 'toolkit.legacyUserProfileCustomizations.stylesheets'
PREF_RE = re.compile(r'^user_pref\("' + re.escape(PREF) + r'",\s*(?:true|false)\);\s*$', re.M)
IMPORT = '/* Quartz Firefox begin */\n@import url("quartz-firefox.css");\n/* Quartz Firefox end */\n'
USER = '// Quartz Firefox begin\nuser_pref("' + PREF + '", true);\n// Quartz Firefox end\n'


def read(path):
    return path.read_text() if path.exists() else ''


def atomic_write(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix='.quartz-', dir=path.parent)
    try:
        with os.fdopen(fd, 'w') as stream:
            stream.write(content)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def profiles(home):
    found = set()
    for root in (home / '.mozilla/firefox', home / 'snap/firefox/common/.mozilla/firefox',
                 home / '.var/app/org.mozilla.firefox/.mozilla/firefox'):
        ini = configparser.ConfigParser(interpolation=None)
        ini.read(root / 'profiles.ini')
        for section in ini.sections():
            if section.startswith('Profile') and ini.has_option(section, 'Path'):
                path = Path(ini.get(section, 'Path'))
                if ini.get(section, 'IsRelative', fallback='1') == '1':
                    path = root / path
                if path.is_dir():
                    found.add(path.resolve())
    return sorted(found)


def render(home):
    palette = dict.fromkeys(('QUARTZ_APPLICATION_BG', 'QUARTZ_TITLEBAR_BG',
                             'QUARTZ_MENUBAR_BG'), '#ffffff')
    config = Path(os.environ.get('XDG_CONFIG_HOME', home / '.config')) / 'quartz-settings/theme.conf'
    for line in read(config).splitlines():
        key, sep, value = line.partition('=')
        if key in palette and sep:
            if not re.fullmatch(r'#[0-9a-fA-F]{6}', value):
                raise ValueError('Invalid Quartz color: ' + key)
            palette[key] = value
    css = Path(__file__).with_name('userChrome.css.in').read_text()
    for key, value in palette.items():
        css = css.replace('@' + key + '@', value)
    return css


def update(profile, css, remove=False):
    chrome = profile / 'chrome/userChrome.css'
    theme = profile / 'chrome/quartz-firefox.css'
    user = profile / 'user.js'
    prefs = profile / 'prefs.js'
    backup = profile / 'quartz-firefox-backup'
    state_path = backup / 'state.json'
    if remove and not state_path.exists():
        return
    original_chrome, original_user = read(chrome), read(user)
    # Refuse to overwrite a manually installed file or damaged managed block.
    for content, block, marker in ((original_chrome, IMPORT, 'Quartz Firefox'),
                                   (original_user, USER, 'Quartz Firefox')):
        if marker in content and (content.count(block) != 1 or content.count(marker) != 2):
            raise ValueError('Damaged Quartz block in ' + str(profile))
    if not state_path.exists():
        if backup.exists() or theme.exists() or IMPORT in original_chrome or USER in original_user:
            raise ValueError('Unmanaged Quartz files already exist in ' + str(profile))
        backup.mkdir(mode=0o700)
        state = {'pref': PREF_RE.findall(read(prefs)),
                 'chrome_existed': chrome.exists(), 'user_existed': user.exists()}
        for path in (chrome, user, prefs):
            if path.exists():
                shutil.copy2(path, backup / path.name)
        atomic_write(state_path, json.dumps(state))
    state = json.loads(state_path.read_text())
    clean_chrome = original_chrome.replace(IMPORT, '')
    clean_user = original_user.replace(USER, '')
    if remove:
        for path, content, existed in ((chrome, clean_chrome, state['chrome_existed']),
                                       (user, clean_user, state['user_existed'])):
            if content or existed:
                atomic_write(path, content)
            elif path.exists():
                path.unlink()
        if prefs.exists():
            restored = PREF_RE.sub('', read(prefs))
            if state['pref']:
                restored += '\n' + '\n'.join(state['pref']) + '\n'
            atomic_write(prefs, restored)
        theme.unlink(missing_ok=True)
        # Keep the original backup for recovery, but permit a fresh opt-in.
        archive = backup.with_name('quartz-firefox-backup-removed-' + str(time.time_ns()))
        backup.rename(archive)
    else:
        atomic_write(theme, css)
        # Imports must precede the user's existing rules and @namespace.
        atomic_write(chrome, IMPORT + clean_chrome)
        atomic_write(user, clean_user + ('' if not clean_user or clean_user.endswith('\n') else '\n') + USER)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--remove', action='store_true')
    args = parser.parse_args()
    running = subprocess.run(['pgrep', '-u', str(os.getuid()), '-x', 'firefox|firefox-bin'],
                             stdout=subprocess.DEVNULL, check=False)
    if running.returncode == 0:
        raise ValueError('Close Firefox completely, then click the button again.')
    if running.returncode != 1:
        raise ValueError('Could not check whether Firefox is running.')
    home = Path.home()
    targets = profiles(home)
    if not targets:
        raise ValueError('Open Firefox once to create a profile, then close it and try again.')
    css = '' if args.remove else render(home)
    for profile in targets:
        update(profile, css, args.remove)
    print(('Removed' if args.remove else 'Added') + f' Quartz theme in {len(targets)} Firefox profile(s). Open Firefox to see the change.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, configparser.Error) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
