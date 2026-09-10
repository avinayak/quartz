import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('typography', Path(__file__).resolve().parents[2] / 'theme/Quartz-System6/typography.py')
typography = importlib.util.module_from_spec(spec)
spec.loader.exec_module(typography)


class TypographyTests(unittest.TestCase):
    def test_pixel_policy_covers_each_family_without_global_antialias_changes(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(typography, 'CONFIG', Path(directory)):
                typography.install_pixel_rendering_policy(['Terminus', 'ChiKareGo2', 'System 7 Helvetica'])
                path = Path(directory) / 'fontconfig/conf.d/99-quartz-typography-pixels.conf'
                root = typography.ET.fromstring(path.read_text())
                self.assertEqual(len(root.findall('match')), 7)
                for match in root.findall("match"):
                    if match.find("edit[@name='antialias']") is None:
                        continue
                    self.assertIn(match.find('test/string').text, ['Terminus', 'ChiKareGo2', 'System 7 Helvetica'])
                    self.assertEqual(match.find("edit[@name='antialias']/bool").text, 'false')
                for match in root.findall("match[@target='font']"):
                    matrix = match.find("edit[@name='matrix']/matrix")
                    self.assertEqual([int(v.text) for v in matrix], [1, 0, 0, 1])
                original = path.read_bytes()
                typography.install_pixel_rendering_policy(['ChiKareGo2', 'Terminus', 'System 7 Helvetica'])
                self.assertEqual(path.read_bytes(), original)

    def test_real_style_family_is_not_parsed_as_synthetic_style(self):
        self.assertEqual(typography.pango_pixels(('Tamzen Bold', 14)), 'Tamzen Bold, 14px')
        self.assertEqual(typography.points_for_pixels(('Scientifica Italic', 12), 96), 'Scientifica Italic, 9')
        self.assertEqual(typography.pango_pixels(('Spleen', 16)), 'Spleen 16px')

    def test_marco_uses_points_at_live_dpi(self):
        self.assertEqual(typography.points_for_pixels(('ChiKareGo2', 16), 96), 'ChiKareGo2 12')
        self.assertEqual(typography.points_for_pixels(('ChiKareGo2', 16), 192), 'ChiKareGo2 6')
        self.assertEqual(typography.points_for_pixels(('ChiKareGo2', 16), -1), 'ChiKareGo2 12')

    def test_migration_removes_only_managed_font_overrides(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'gtk.css'
            path.write_text('label { color: red; }\n/* quartz-system7-fonts:begin */\nmenubar {\n font-family: "Old Font";\n font-size: 16px;\n border-width: 1px;\n}\n/* quartz-system7-fonts:end */\n' + typography.BEGIN + '\n* { font-size: 15px; }\n' + typography.END)
            typography.remove_user_font_overrides(path)
            self.assertNotIn('font-', path.read_text())
            self.assertIn('color: red', path.read_text())
            self.assertIn('border-width: 1px', path.read_text())
            original = path.read_text()
            typography.remove_user_font_overrides(path)
            self.assertEqual(path.read_text(), original)

    def test_geneva_sizes_are_grouped_and_saved_choices_migrate(self):
        rows = 'System 7 Geneva|9,10,12,14,18,20,24|False\nSystem 7 Geneva 12|15|False\nSystem 7 Geneva Italic|9|False\n'
        with patch.object(typography.subprocess, 'check_output', return_value=rows):
            fonts = typography.catalog()
            self.assertNotIn('System 7 Geneva 12', fonts)
            self.assertIn('System 7 Geneva Italic', fonts)
            self.assertEqual(typography.migrate_fonts(['System 7 Geneva 12 15px']), ['System 7 Geneva 12px'])
            self.assertIn('System 7 Geneva 12', typography.catalog(include_legacy=True))

    def test_catalog_keeps_every_bitmap_strike(self):
        rows = 'System 7 Geneva 9|12|False\nTerminus|12,14,16,32|False\nTerminus|16,24|False\nChiKareGo2||True\nDejaVu Sans||True\n'
        with patch.object(typography.subprocess, 'check_output', return_value=rows):
            fonts = typography.catalog()
        self.assertEqual(fonts['Terminus'], [12, 14, 16, 24, 32])
        self.assertEqual(fonts['System 7 Geneva 9'], [12])
        self.assertNotIn('DejaVu Sans', fonts)

    def test_apply_preserves_unmanaged_content_and_replaces_previous_roles(self):
        with tempfile.TemporaryDirectory() as directory:
            home = Path(directory)
            config = home / '.config'
            gtk = config / 'gtk-3.0/gtk.css'
            gtk.parent.mkdir(parents=True)
            gtk.write_text('/* existing user settings */\n')
            fonts = {'ChiKareGo2': [16], 'Terminus': [12, 24], 'System 7 Geneva 12': [15]}
            with patch.object(typography, 'CONFIG', config), patch.object(typography, 'STATE', config / 'quartz-settings/typography.json'), patch.object(typography.Path, 'home', return_value=home), patch.object(typography, 'catalog', return_value=fonts), patch.object(typography.subprocess, 'run') as run, patch.object(typography, 'refresh') as refresh, patch.object(typography, 'title_points', return_value='ChiKareGo2 12'):
                typography.apply(['ChiKareGo2 16px', 'Terminus 12px', 'System 7 Geneva 12 15px'])
                typography.apply(['Terminus 24px', 'ChiKareGo2 16px', 'Terminus 12px'])
                self.assertNotIn(typography.BEGIN, gtk.read_text())
                gtk = home / '.themes/Quartz-System6/gtk-3.0/gtk.css'
                self.assertEqual(gtk.read_text().count(typography.BEGIN), 1)
                self.assertIn('existing user settings', (config / 'gtk-3.0/gtk.css').read_text())
                self.assertIn('font-size: 24px', gtk.read_text())
                self.assertIn('gtk-font-name = "Terminus 12px"', (home / '.gtkrc-2.0').read_text())
                self.assertIn('Terminus 24px', typography.STATE.read_text())
                self.assertEqual(run.call_count, 8)
                self.assertEqual(refresh.call_count, 2)
                before = gtk.read_text()
                with self.assertRaises(ValueError):
                    typography.apply(['Terminus 13px'] * 3)
                self.assertEqual(gtk.read_text(), before)


if __name__ == '__main__':
    unittest.main()
