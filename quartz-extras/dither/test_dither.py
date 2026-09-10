import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('dither', Path(__file__).with_name('install.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class Settings:
    def __init__(self, image):
        self.values = {'picture-filename': str(image), 'picture-options': 'zoom'}
        self.locked = False
        self.reject = False

    def get_string(self, key):
        return self.values[key]

    def is_writable(self, key):
        return not self.locked

    def set_string(self, key, value):
        if self.reject:
            return False
        self.values[key] = value
        return True


class DitherTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.source = self.base / "original ' wallpaper.png"
        image = module.GdkPixbuf.Pixbuf.new(module.GdkPixbuf.Colorspace.RGB, True, 8, 17, 13)
        image.fill(0x789abc80)
        image.savev(str(self.source), 'png', [], [])
        self.digest = hashlib.sha256(self.source.read_bytes()).digest()
        self.root = self.base / 'outputs'
        self.settings = Settings(self.source)

    def tearDown(self):
        self.assertEqual(hashlib.sha256(self.source.read_bytes()).digest(), self.digest)

    def test_copy_repeat_restore(self):
        module.apply(self.settings, self.root)
        output = Path(self.settings.get_string('picture-filename'))
        self.assertNotEqual(output, self.source)
        image = module.GdkPixbuf.Pixbuf.new_from_file(str(output))
        self.assertEqual((image.get_width(), image.get_height()), (17, 13))
        pixels = image.get_pixels()
        self.assertEqual(set(pixels[3::4]), {128})
        self.assertEqual(set(pixels[0::4]), {0x78})
        module.apply(self.settings, self.root)
        self.assertEqual(Path(self.settings.get_string('picture-filename')).read_bytes(), output.read_bytes())
        module.apply(self.settings, self.root, restore=True)
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_color_counts(self):
        # A broad RGB gradient exercises each palette, including all endpoints.
        data = bytes(v for r in range(0, 256, 17) for g in range(0, 256, 17)
                     for b in range(0, 256, 17) for v in (r, g, b))
        image = module.GdkPixbuf.Pixbuf.new_from_bytes(
            module.GLib.Bytes.new(data), module.GdkPixbuf.Colorspace.RGB,
            False, 8, 64, 64, 192)
        for count in module.COLOR_COUNTS:
            with self.subTest(colors=count):
                result = module.dither(image, count)
                pixels = result.get_pixels()
                palette = {tuple(pixels[i:i + 3]) for i in range(0, len(pixels), 3)}
                self.assertLessEqual(len(palette), count)
                self.assertGreater(len(palette), 1)
                source_colors = {tuple(data[i:i + 3]) for i in range(0, len(data), 3)}
                self.assertTrue(palette <= source_colors)
                self.assertEqual(palette, set(module.extract_palette(image, count)))

    def test_palette_follows_source_even_at_two_colors(self):
        for base in ((130, 30, 10), (10, 30, 130)):
            data = bytes(v for y in range(16) for x in range(16)
                         for v in (base[0] + x, base[1] + y, base[2] + x))
            image = module.GdkPixbuf.Pixbuf.new_from_bytes(
                module.GLib.Bytes.new(data), module.GdkPixbuf.Colorspace.RGB,
                False, 8, 16, 16, 48)
            source_colors = {tuple(data[i:i + 3]) for i in range(0, len(data), 3)}
            palette = set(module.extract_palette(image, 2))
            self.assertEqual(len(palette), 2)
            self.assertTrue(palette <= source_colors)
            for size in (1, 4):
                pixels = module.dither(image, 2, size).get_pixels()
                self.assertTrue({tuple(pixels[i:i + 3]) for i in range(0, len(pixels), 3)} <= palette)

    def test_transparent_colors_do_not_enter_palette(self):
        data = bytes((210, 80, 30, 255, 0, 255, 0, 0))
        image = module.GdkPixbuf.Pixbuf.new_from_bytes(
            module.GLib.Bytes.new(data), module.GdkPixbuf.Colorspace.RGB,
            True, 8, 2, 1, 8)
        self.assertEqual(module.extract_palette(image, 2), [(210, 80, 30)])
        self.assertEqual(module.dither(image, 2).get_pixels()[3::4], data[3::4])

    def test_change_count_uses_original(self):
        module.apply(self.settings, self.root, colors=2)
        module.apply(self.settings, self.root, colors=256)
        output = module.GdkPixbuf.Pixbuf.new_from_file(self.settings.get_string('picture-filename'))
        original = module.GdkPixbuf.Pixbuf.new_from_file(str(self.source))
        self.assertEqual(output.get_pixels(), module.dither(original, 256).get_pixels())
        module.apply(self.settings, self.root, restore=True)
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_invalid_count_preserves_selection(self):
        for count in (0, 3, 257):
            with self.assertRaises(ValueError):
                module.apply(self.settings, self.root, colors=count)
        self.assertFalse(self.root.exists())
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_pixel_sizes_and_edge_blocks(self):
        # Odd dimensions and varying alpha catch stride and edge-block mistakes.
        width, height = 17, 13
        data = bytes(v for y in range(height) for x in range(width)
                     for v in (x * 15, y * 20, (x * 7 + y * 9) % 256, (x + y) * 8))
        image = module.GdkPixbuf.Pixbuf.new_from_bytes(
            module.GLib.Bytes.new(data), module.GdkPixbuf.Colorspace.RGB,
            True, 8, width, height, width * 4)
        for size in (1, 2, 3, 8, 32):
            with self.subTest(pixel_size=size):
                result = module.dither(image, 64, size)
                self.assertEqual((result.get_width(), result.get_height()), (width, height))
                pixels = result.get_pixels()
                self.assertEqual(pixels[3::4], data[3::4])
                for y in range(height):
                    for x in range(width):
                        offset = (y * width + x) * 4
                        anchor = ((y // size * size) * width + x // size * size) * 4
                        self.assertEqual(pixels[offset:offset + 3], pixels[anchor:anchor + 3])

    def test_pixel_size_change_uses_original(self):
        module.apply(self.settings, self.root, pixel_size=8)
        module.apply(self.settings, self.root, pixel_size=2)
        output = module.GdkPixbuf.Pixbuf.new_from_file(self.settings.get_string('picture-filename'))
        original = module.GdkPixbuf.Pixbuf.new_from_file(str(self.source))
        self.assertEqual(output.get_pixels(), module.dither(original, 64, 2).get_pixels())
        module.apply(self.settings, self.root, restore=True)
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_invalid_pixel_size_preserves_selection(self):
        for size in (0, -1, 33, 1.5):
            with self.assertRaises(ValueError):
                module.apply(self.settings, self.root, pixel_size=size)
        self.assertFalse(self.root.exists())
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_rejected_setting_removes_copy(self):
        self.settings.reject = True
        with self.assertRaises(ValueError):
            module.apply(self.settings, self.root)
        self.assertEqual(list(self.root.iterdir()), [])
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_write_failure_preserves_selection(self):
        with patch.object(module.GdkPixbuf.Pixbuf, 'savev', side_effect=OSError('disk full')):
            with self.assertRaises(OSError):
                module.apply(self.settings, self.root)
        self.assertEqual(list(self.root.iterdir()), [])
        self.assertEqual(self.settings.get_string('picture-filename'), str(self.source))

    def test_invalid_and_locked(self):
        self.settings.locked = True
        with self.assertRaises(ValueError):
            module.apply(self.settings, self.root)
        self.settings.locked = False
        with self.assertRaises(ValueError):
            module.apply(self.settings, self.root, restore=True)
        self.settings.values['picture-filename'] = str(self.base / 'missing.png')
        with self.assertRaises(module.GLib.Error):
            module.apply(self.settings, self.root)
        self.assertFalse(self.root.exists())

    def test_changed_selection_is_not_overwritten(self):
        real_dither = module.dither
        def changed(image, colors=64, pixel_size=1):
            self.settings.values['picture-filename'] = '/another-wallpaper.png'
            return real_dither(image, colors, pixel_size)
        with patch.object(module, 'dither', side_effect=changed):
            with self.assertRaises(ValueError):
                module.apply(self.settings, self.root)
        self.assertEqual(self.settings.get_string('picture-filename'), '/another-wallpaper.png')
        self.assertEqual(list(self.root.iterdir()), [])


if __name__ == '__main__':
    unittest.main()
