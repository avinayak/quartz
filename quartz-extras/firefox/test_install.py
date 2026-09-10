import importlib.util
import tempfile
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('installer', Path(__file__).with_name('install.py'))
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


class InstallationTests(unittest.TestCase):
    def test_install_refresh_remove_preserves_customizations(self):
        with tempfile.TemporaryDirectory() as temporary:
            profile = Path(temporary)
            (profile / 'chrome').mkdir()
            original = '@namespace url("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul");\n/* existing */\n'
            (profile / 'chrome/userChrome.css').write_text(original)
            (profile / 'user.js').write_text('user_pref("custom.pref", true);\n')
            pref = f'user_pref("{installer.PREF}", false);\n'
            (profile / 'prefs.js').write_text(pref)
            installer.update(profile, '/* theme */')
            first = (profile / 'user.js').read_text()
            installer.update(profile, '/* refreshed */')
            self.assertEqual(first, (profile / 'user.js').read_text())
            self.assertTrue((profile / 'chrome/userChrome.css').read_text().endswith(original))
            self.assertEqual((profile / 'quartz-firefox-backup/userChrome.css').read_text(), original)
            (profile / 'prefs.js').write_text(pref.replace('false', 'true') + 'user_pref("new.pref", 42);\n')
            installer.update(profile, '', remove=True)
            self.assertEqual((profile / 'chrome/userChrome.css').read_text(), original)
            self.assertEqual((profile / 'user.js').read_text(), 'user_pref("custom.pref", true);\n')
            self.assertIn(pref, (profile / 'prefs.js').read_text())
            self.assertIn('new.pref', (profile / 'prefs.js').read_text())
            self.assertFalse((profile / 'chrome/quartz-firefox.css').exists())
            installer.update(profile, '/* reinstall */')
            installer.update(profile, '', remove=True)

    def test_fresh_profile_and_malformed_block(self):
        with tempfile.TemporaryDirectory() as temporary:
            profile = Path(temporary)
            installer.update(profile, '/* theme */')
            installer.update(profile, '', remove=True)
            self.assertFalse((profile / 'user.js').exists())
            self.assertFalse((profile / 'chrome/userChrome.css').exists())
            (profile / 'user.js').write_text('// Quartz Firefox begin\ncustom')
            with self.assertRaises(ValueError):
                installer.update(profile, '/* theme */')
            self.assertEqual((profile / 'user.js').read_text(), '// Quartz Firefox begin\ncustom')

    def test_native_snap_flatpak_profiles_and_absolute_paths(self):
        with tempfile.TemporaryDirectory() as temporary:
            home = Path(temporary)
            expected = []
            for location in ('.mozilla/firefox', 'snap/firefox/common/.mozilla/firefox',
                             '.var/app/org.mozilla.firefox/.mozilla/firefox'):
                root = home / location
                profile = root / 'profile with spaces'
                profile.mkdir(parents=True)
                (root / 'profiles.ini').write_text('[Profile0]\nPath=profile with spaces\nIsRelative=1\n')
                expected.append(profile.resolve())
            self.assertEqual(installer.profiles(home), sorted(expected))
            (home / '.mozilla/firefox/profiles.ini').write_text(
                '[Profile0]\nIsRelative=0\nPath=' + str(expected[0]) + '\n')
            self.assertEqual(installer.profiles(home), sorted(expected))


if __name__ == '__main__':
    unittest.main()
