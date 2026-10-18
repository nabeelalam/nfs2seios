"""Generate new uppercase HUD artwork from reviewed paths; no original assets."""
import argparse
import hashlib
import json
from pathlib import Path
from publication_audit import reviewed_timing_svg
from rasterize_hud_svg import rasterize

ROOT = Path(__file__).resolve().parents[1]


def generate():
    directory = ROOT / 'assets/hud'
    catalog = json.loads((directory / 'small-font.json').read_text())
    if catalog['version'] != 1 or len(catalog['entries']) != 26:
        raise ValueError('Unexpected uppercase font catalog')
    rgba = bytearray()
    sources = {}
    for index, entry in enumerate(catalog['entries']):
        character = chr(65 + index)
        width = 4 if character == 'I' else 8
        filename = 'small-' + character + '.svg'
        if (entry['character'] != character or entry['file'] != filename or
                entry['vectorSize'] != [width, 8] or entry['rasterSize'] != [width * 8, 64]):
            raise ValueError('Unexpected uppercase glyph layout')
        source = (directory / filename).read_bytes()
        digest = hashlib.sha256(source).hexdigest()
        if digest != entry['sha256'] or not reviewed_timing_svg('assets/hud/' + filename, source):
            raise ValueError('Unreviewed uppercase vector source')
        sources[filename] = digest
        pixels = rasterize(source, width * 8, 64)
        if len(pixels) != width * 8 * 64 * 4:
            raise ValueError('Unexpected uppercase raster size')
        # Keep a fixed tile; the narrow I owns only its first 32 columns.
        for y in range(64):
            rgba += pixels[y * width * 32:(y + 1) * width * 32]
            rgba += bytes((64 - width * 8) * 4)
    encoded = bytearray()
    position = 0
    while position < len(rgba):
        color = rgba[position:position + 4]
        count = 1
        while (count < 65535 and position + (count + 1) * 4 <= len(rgba) and
               rgba[position + count * 4:position + (count + 1) * 4] == color):
            count += 1
        encoded += bytes((count & 255, count >> 8)) + color
        position += count * 4

    def array(name, values):
        lines = [','.join('0x%02x' % b for b in values[i:i + 24]) for i in range(0, len(values), 24)]
        return 'static const unsigned char ' + name + '[]={\n' + ',\n'.join(lines) + '\n};\n'

    text = '// SPDX-License-Identifier: MIT\n// Generated from new vector paths only; no original game pixels.\n'
    text += array('nfs_small_font_raster_digest', hashlib.sha256(rgba).digest())
    text += array('nfs_small_font_pixels_rle', encoded)
    return text, dict(generatedFrom='New uppercase vector paths only', originalGamePixelsIncluded=False,
                     sourceHashes=sources, rasterSha256=hashlib.sha256(rgba).hexdigest(),
                     rasterBytes=len(rgba), rleBytes=len(encoded),
                     headerSha256=hashlib.sha256(text.encode()).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    text, manifest = generate()
    header = ROOT / 'src/SmallFontPixels.inc'
    meta = ROOT / 'assets/hud/small-generated.json'
    if args.check:
        if header.read_text() != text or json.loads(meta.read_text()) != manifest:
            raise SystemExit('Generated uppercase artwork differs')
    else:
        header.write_bytes(text.encode())
        meta.write_bytes((json.dumps(manifest, indent=2) + '\n').encode())
    print(json.dumps(manifest))


if __name__ == '__main__':
    main()
