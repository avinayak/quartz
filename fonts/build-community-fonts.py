#!/usr/bin/env python3
"""Build bundled bitmap fonts offline. Requires fonttosfnt, pcf2bdf, FontForge, fontTools."""
from pathlib import Path
import hashlib
import json
import subprocess
import tempfile
import re
from fontTools.ttLib import TTFont
from fontTools.ttLib.tables.DefaultTable import DefaultTable

ROOT = Path(__file__).resolve().parent / 'community'

def build():
    manifest = json.loads((ROOT / 'manifest.json').read_text())
    (ROOT / 'otb').mkdir(exist_ok=True)
    outputs = []
    with tempfile.TemporaryDirectory(prefix='quartz-community-build-') as directory:
        for item in manifest['fonts']:
            source = ROOT / item['input']
            print(item['input'], flush=True)
            assert hashlib.sha256(source.read_bytes()).hexdigest() == item['sha256'], source
            raw = Path(directory) / 'raw.otb'
            if source.suffix == '.otb':
                raw.write_bytes(source.read_bytes())
            elif source.suffix == '.sfd':
                code = 'import fontforge,sys; f=fontforge.open(sys.argv[1]); assert f.bitmapSizes; f.generate(sys.argv[2], bitmap_type="otb")'
                subprocess.run(['fontforge','-lang=py','-c',code,str(source),str(raw)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
            else:
                if source.name.endswith('.pcf.gz'):
                    bdf = Path(directory) / 'input.bdf'
                    subprocess.run(['pcf2bdf', '-o', str(bdf), str(source)], check=True, stderr=subprocess.PIPE)
                    # Original Proggy PCFs omit BDF charset/name properties.
                    # Recover those declared fields from their XLFD header;
                    # character codes, glyph rows, and metrics stay untouched.
                    text = bdf.read_text()
                    xlfd = re.search(r'^FONT (.+)$', text, re.M)[1].split('-')
                    properties = {'FAMILY_NAME': '"' + xlfd[2] + '"',
                                  'WEIGHT_NAME': '"' + xlfd[3] + '"',
                                  'SLANT': '"' + xlfd[4].upper() + '"',
                                  'PIXEL_SIZE': xlfd[7],
                                  'CHARSET_REGISTRY': '"' + xlfd[-2].upper() + '"',
                                  'CHARSET_ENCODING': '"' + xlfd[-1] + '"'}
                    additions = [key + ' ' + value for key, value in properties.items()
                                 if not re.search(r'^' + key + r' ', text, re.M)]
                    text = re.sub(r'^STARTPROPERTIES ([0-9]+)$', lambda m: 'STARTPROPERTIES ' + str(int(m[1]) + len(additions)), text, flags=re.M)
                    text = text.replace('ENDPROPERTIES', '\n'.join(additions) + '\nENDPROPERTIES')
                    if [part.lower() for part in xlfd[-2:]] != ['iso8859', '1']:
                        raise ValueError('Unexpected PCF encoding: ' + str(xlfd[-2:]))
                    # Latin-1 codepoints have identical Unicode values. Explicit
                    # Unicode metadata lets FreeType expose its Unicode charmap.
                    text = text.replace('CHARSET_REGISTRY "ISO8859"', 'CHARSET_REGISTRY "ISO10646"')
                    bdf.write_text(text)
                    source = bdf
                subprocess.run(['fonttosfnt','-c','-b','-g','2','-m','2','-o',str(raw),str(source)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
            font = TTFont(raw, recalcTimestamp=False, recalcBBoxes=False)
            original_tables = {tag:font.reader[tag] for tag in font.reader.keys() if tag not in ('name','head')}
            assert 'EBDT' in font and 'EBLC' in font, source
            assert all(s.bitmapSizeTable.bitDepth == 1 for s in font['EBLC'].strikes), source
            assert 'CFF ' not in font and 'CFF2' not in font, source
            if 'glyf' in font:
                assert all(font['glyf'][name].numberOfContours == 0 for name in font['glyf'].glyphs), source
            retained = original_tables
            family = item['family']
            # Surface actual bitmap styles as selectable families. Never
            # synthesize a bold/italic face or trace/rasterize an outline.
            for record in font['name'].names:
                if record.nameID in (1,4,16,21): value = family
                elif record.nameID == 6: value = family.replace(' ','-')
                else: continue
                record.string = value.encode(record.getEncoding())
            for tag, data in retained.items():
                table = DefaultTable(tag)
                table.data = data
                font[tag] = table
            font['head'].created = font['head'].modified = 2082844800
            target = ROOT / item['output']
            font.save(target)
            saved = TTFont(target)
            assert all(saved.getTableData(tag) == data for tag,data in retained.items()), source
            scanned = subprocess.check_output(['fc-scan','--format','%{family[0]}|%{pixelsize}|%{scalable}',str(target)],text=True).split('|')
            assert scanned[0] == family and scanned[2] == 'False', (source,scanned)
            outputs.append({'file':item['output'],'family':family,'sizes':[float(x) for x in scanned[1].split(',')],'sha256':hashlib.sha256(target.read_bytes()).hexdigest()})
    (ROOT / 'outputs.json').write_text(json.dumps(outputs,indent=2)+'\n')
    print(f'Built and validated {len(outputs)} one-bit bitmap files.')

if __name__ == '__main__':
    build()
