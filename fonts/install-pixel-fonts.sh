#!/usr/bin/env bash
set -Eeuo pipefail
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
data_home="${XDG_DATA_HOME:-$HOME/.local/share}"
config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
destination="$data_home/fonts/quartz-system7-bitmap"
install -d "$destination" "$config_home/fontconfig/conf.d"
install -m 0644 "$source_dir"/system7-bitmap/*.otb "$destination/"
# Explicitly accept only our namespaced originals, even on distributions that
# disable bitmap fonts. Other desktop font matching remains unchanged.
python3 - "$destination" "$config_home/fontconfig/conf.d/99-quartz-system7-bitmap.conf" <<'PY'
import pathlib
import subprocess
import sys
import xml.etree.ElementTree as ET
families = set()
for path in pathlib.Path(sys.argv[1]).glob('*.otb'):
    family = subprocess.check_output(['fc-scan', '--format', '%{family[0]}', str(path)], text=True)
    if not family.startswith('System 7 '):
        raise SystemExit('Unexpected bitmap family: ' + family)
    families.add(family)
root = ET.Element('fontconfig')
accept = ET.SubElement(ET.SubElement(root, 'selectfont'), 'acceptfont')
for family in sorted(families):
    pattern = ET.SubElement(accept, 'pattern')
    ET.SubElement(ET.SubElement(pattern, 'patelt', name='family'), 'string').text = family
    match = ET.SubElement(root, 'match', target='font')
    ET.SubElement(ET.SubElement(match, 'test', name='family', compare='eq'), 'string').text = family
    for name in ('antialias', 'embolden'):
        ET.SubElement(ET.SubElement(match, 'edit', name=name, mode='assign'), 'bool').text = 'false'
ET.indent(root)
pathlib.Path(sys.argv[2]).write_text('<?xml version="1.0"?>\n<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">\n' + ET.tostring(root, encoding='unicode') + '\n')
PY
fc-cache -f "$destination"

# All additional bitmap choices are bundled with their original licenses.
python3 "$source_dir/install-community-fonts.py"
