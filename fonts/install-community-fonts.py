#!/usr/bin/env python3
"""Install the pinned, offline bitmap collection and its redistribution notices."""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parent / 'community'

def install():
    manifest = json.loads((ROOT / 'manifest.json').read_text())
    outputs = json.loads((ROOT / 'outputs.json').read_text())
    expected = {f['output']: f for f in manifest['fonts']}
    if len(outputs) != len(expected) or {f['file'] for f in outputs} != set(expected):
        raise ValueError('Community font outputs do not match the manifest.')
    for item in outputs:
        if item['family'] != expected[item['file']]['family']:
            raise ValueError('Unexpected font family in ' + item['file'])
        if hashlib.sha256((ROOT / item['file']).read_bytes()).hexdigest() != item['sha256']:
            raise ValueError('Community font checksum mismatch: ' + item['file'])
    for collection in manifest['collections']:
        if not (ROOT / collection['license']).is_file():
            raise ValueError('Missing font license: ' + collection['id'])
    data = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share'))
    config = Path(os.environ.get('XDG_CONFIG_HOME', Path.home() / '.config'))
    destination = data / 'fonts/quartz-community'
    destination.mkdir(parents=True, exist_ok=True)
    for item in outputs:
        shutil.copyfile(ROOT / item['file'], destination / Path(item['file']).name)
    docs = data / 'doc/quartz-community-fonts'
    shutil.copytree(ROOT / 'licenses', docs / 'licenses', dirs_exist_ok=True)
    for name in ('README.md', 'manifest.json', 'outputs.json'):
        shutil.copyfile(ROOT / name, docs / name)
    root = ET.Element('fontconfig')
    accept = ET.SubElement(ET.SubElement(root, 'selectfont'), 'acceptfont')
    for family in sorted({item['family'] for item in outputs}):
        ET.SubElement(ET.SubElement(ET.SubElement(accept, 'pattern'), 'patelt', name='family'), 'string').text = family
        match = ET.SubElement(root, 'match', target='font')
        ET.SubElement(ET.SubElement(match, 'test', name='family', compare='eq'), 'string').text = family
        for name in ('antialias', 'embolden'):
            ET.SubElement(ET.SubElement(match, 'edit', name=name, mode='assign'), 'bool').text = 'false'
        matrix = ET.SubElement(ET.SubElement(match, 'edit', name='matrix', mode='assign'), 'matrix')
        for number in (1, 0, 0, 1):
            ET.SubElement(matrix, 'double').text = str(number)
    ET.indent(root)
    policy = config / 'fontconfig/conf.d/99-quartz-community-bitmaps.conf'
    policy.parent.mkdir(parents=True, exist_ok=True)
    text = '<?xml version="1.0"?>\n<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">\n' + ET.tostring(root, encoding='unicode') + '\n'
    temp = policy.with_suffix('.tmp')
    temp.write_text(text)
    temp.replace(policy)
    subprocess.run(['fc-cache', '-f', str(destination)], check=True)
    print(f'Installed {len(outputs)} community bitmap font files ({len({f["family"] for f in outputs})} families/styles).')

if __name__ == '__main__':
    install()
