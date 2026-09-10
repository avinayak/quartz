#!/usr/bin/env python3
"""Create and select a separate ordered-color-dithered MATE wallpaper."""
from collections import Counter
from functools import lru_cache
import argparse
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile

import gi
gi.require_version('GdkPixbuf', '2.0')
from gi.repository import GdkPixbuf, Gio, GLib

BAYER = ((0, 8, 2, 10), (12, 4, 14, 6), (3, 11, 1, 9), (15, 7, 13, 5))


COLOR_COUNTS = (2, 4, 8, 16, 32, 64, 128, 256)


def extract_palette(image, colors):
    """Weighted median-cut sampling with representatives taken from source pixels."""
    width, height = image.get_width(), image.get_height()
    stride, channels = image.get_rowstride(), image.get_n_channels()
    pixels = image.get_pixels()
    histogram = Counter()
    # Bound palette work for large wallpapers; sample original pixels without
    # resizing/interpolation, which would introduce colors absent from the source.
    step = 1
    while ((width + step - 1) // step) * ((height + step - 1) // step) > 65536:
        step += 1
    for y in range(0, height, step):
        for x in range(0, width, step):
            offset = y * stride + x * channels
            weight = pixels[offset + 3] if channels == 4 else 255
            if weight:
                histogram[tuple(pixels[offset:offset + 3])] += weight
    if not histogram:
        # Fully transparent samples have no visible palette to approximate.
        return [tuple(pixels[:3])]
    boxes = [list(histogram)]
    while len(boxes) < colors:
        candidates = [(i, max(max(c[k] for c in box) - min(c[k] for c in box)
                              for k in range(3)))
                      for i, box in enumerate(boxes) if len(box) > 1]
        if not candidates:
            break
        index, _ = max(candidates, key=lambda item: item[1])
        box = boxes.pop(index)
        axis = max(range(3), key=lambda k: max(c[k] for c in box) - min(c[k] for c in box))
        box.sort(key=lambda c: (c[axis], c))
        total = sum(histogram[c] for c in box)
        weight = 0
        split = 1
        for split, color in enumerate(box[:-1], 1):
            weight += histogram[color]
            if weight * 2 >= total:
                break
        boxes.extend((box[:split], box[split:]))
    palette = []
    for box in boxes:
        total = sum(histogram[c] for c in box)
        center = tuple(sum(c[k] * histogram[c] for c in box) / total for k in range(3))
        palette.append(min(box, key=lambda c: (sum((c[k] - center[k]) ** 2 for k in range(3)), c)))
    return palette


def palette_mapper(palette):
    exact = {bytes(color): bytes(color) for color in palette}

    @lru_cache(maxsize=32768)
    def choices(red, green, blue):
        color = (red * 8 + 4, green * 8 + 4, blue * 8 + 4)
        nearest = sorted(palette, key=lambda p: sum((p[k] - color[k]) ** 2 for k in range(3)))[:2]
        first = nearest[0]
        second = nearest[-1]
        direction = tuple(second[k] - first[k] for k in range(3))
        distance = sum(v * v for v in direction)
        fraction = (sum((color[k] - first[k]) * direction[k] for k in range(3)) / distance
                    if distance else 0)
        fraction = max(0, min(1, fraction))
        return tuple(bytes(second if fraction > (threshold + 0.5) / 16 else first)
                     for threshold in range(16))

    def map_color(rgb, threshold):
        if rgb in exact:
            return exact[rgb]
        return choices(rgb[0] // 8, rgb[1] // 8, rgb[2] // 8)[threshold]
    return map_color


def dither(image, colors=64, pixel_size=1):
    """Ordered dithering to a bounded palette; preserve alpha and dimensions."""
    if colors not in COLOR_COUNTS:
        raise ValueError('Color count must be one of: ' + ', '.join(map(str, COLOR_COUNTS)))
    if not isinstance(pixel_size, int) or not 1 <= pixel_size <= 32:
        raise ValueError('Pixel size must be an integer from 1 to 32.')
    width, height = image.get_width(), image.get_height()
    channels, stride = image.get_n_channels(), image.get_rowstride()
    pixels = bytearray(image.get_pixels())
    map_color = palette_mapper(extract_palette(image, colors))
    for y in range(0, height, pixel_size):
        block_height = min(pixel_size, height - y)
        for x in range(0, width, pixel_size):
            block_width = min(pixel_size, width - x)
            # Sample each block's center, then expand without interpolation.
            # Edge blocks are cropped; the original dimensions and alpha stay intact.
            offset = (y + block_height // 2) * stride + (x + block_width // 2) * channels
            threshold = BAYER[(y // pixel_size) % 4][(x // pixel_size) % 4]
            rgb = map_color(bytes(pixels[offset:offset + 3]), threshold)
            if pixel_size == 1:
                pixels[offset:offset + 3] = rgb
            else:
                for row in range(y, y + block_height):
                    start = row * stride + x * channels
                    for channel in range(3):
                        pixels[start + channel:start + block_width * channels:channels] = (
                            bytes((rgb[channel],)) * block_width)
    return GdkPixbuf.Pixbuf.new_from_bytes(
        GLib.Bytes.new(bytes(pixels)), GdkPixbuf.Colorspace.RGB,
        image.get_has_alpha(), 8, width, height, stride)


def original_for(current, root):
    path = Path(current)
    if path.name != 'wallpaper.png' or path.parent.parent != root:
        return None
    metadata = path.parent / 'original.json'
    if not metadata.is_file():
        return None
    value = json.loads(metadata.read_text())
    if not isinstance(value, str) or not value:
        raise ValueError('The saved original wallpaper reference is invalid.')
    return value


def local_path(value):
    if value.startswith('file:'):
        return Path(GLib.filename_from_uri(value)[0])
    if not os.path.isabs(value):
        raise ValueError('Select a local wallpaper image in Appearance → Background first.')
    return Path(value)


def apply(settings, root, restore=False, colors=64, pixel_size=1):
    current = settings.get_string('picture-filename')
    if not settings.is_writable('picture-filename'):
        raise ValueError('The desktop wallpaper setting is locked.')
    original = original_for(current, root)
    if restore:
        if original is None:
            raise ValueError('The current wallpaper is not a Quartz dithered copy.')
        # Validate before switching, including when the original was moved/deleted.
        GdkPixbuf.Pixbuf.new_from_file(str(local_path(original)))
        if not settings.set_string('picture-filename', original):
            raise ValueError('Could not restore the original wallpaper.')
        Gio.Settings.sync()
        return
    if settings.get_string('picture-options') == 'none':
        raise ValueError('Select a wallpaper image in Appearance → Background first.')
    # Repeated clicks always start from the original, never compound the effect.
    source = original or current
    image = GdkPixbuf.Pixbuf.new_from_file(str(local_path(source)))
    result = dither(image, colors, pixel_size)
    root.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix='dither-', dir=root))
    selected = False
    try:
        output = directory / 'wallpaper.png'
        result.savev(str(output), 'png', [], [])
        check = GdkPixbuf.Pixbuf.new_from_file(str(output))
        if (check.get_width(), check.get_height()) != (image.get_width(), image.get_height()):
            raise ValueError('The generated wallpaper could not be verified.')
        (directory / 'original.json').write_text(json.dumps(source))
        if settings.get_string('picture-filename') != current:
            raise ValueError('The wallpaper changed while processing. Please try again.')
        if not settings.set_string('picture-filename', str(output)):
            raise ValueError('Could not select the dithered wallpaper.')
        selected = True
        Gio.Settings.sync()
    finally:
        if not selected:
            shutil.rmtree(directory)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--restore', action='store_true')
    parser.add_argument('--colors', type=int, choices=COLOR_COUNTS, default=64,
                        help='Maximum colors extracted from the original image (default: 64).')
    parser.add_argument('--pixel-size', type=int, choices=range(1, 33), default=1,
                        metavar='1–32', help='Dither block size in image pixels (default: 1).')
    args = parser.parse_args()
    data = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share'))
    if not data.is_absolute():
        raise ValueError('XDG_DATA_HOME must be an absolute path.')
    apply(Gio.Settings.new('org.mate.background'),
          data / 'backgrounds/quartz-extras/dithered', args.restore, args.colors, args.pixel_size)


if __name__ == '__main__':
    try:
        main()
    except Exception as error:
        print(f'error: {error}', file=sys.stderr)
        sys.exit(1)
