"""Author finite Setup-controls word graphics privately, never a font alphabet.

Requires Pillow and a locally installed Haettenschweiler font. This command only
writes the requested output folder; it does not publish or activate game assets.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import os
from pathlib import Path

# Exact observed English labels. Resource numbers are routing hints only; the
# consuming scene must match its immutable command text, size and vertex span.
LABELS = (
    (1, 'steer left', (275, 315), (18, 16), (59, 50), 'right'),
    (2, 'steer right', (276, 316), (18, 16), (67, 57), 'right'),
    (3, 'accelerate', (277, 317), (18, 16), (68, 59), 'right'),
    (4, 'brake', (278, 318), (18, 16), (36, 31), 'right'),
    (5, 'handbrake', (279, 319), (18, 16), (68, 59), 'right'),
    (6, 'shift up', (280, 320), (18, 16), (49, 42), 'right'),
    (7, 'shift down', (281, 321), (18, 16), (70, 60), 'right'),
    (8, 'camera view', (282, 322), (18, 16), (83, 72), 'right'),
    (9, 'look behind', (283, 323), (18, 16), (75, 66), 'right'),
    (10, 'horn', (284, 324), (18, 16), (29, 25), 'right'),
    (11, 'reset', (287,), (18,), (33,), 'right'),
    (12, 'DONE', (285,), (18,), (35,), 'right'),
    (13, 'CANCEL', (286,), (18,), (51,), 'left'),
    (14, 'SETUP', (274, 312), (18,), (42,), 'center'),
    (15, 'PLAYER ONE', (313,), (16,), (69,), 'left'),
    (16, 'PLAYER TWO', (314,), (16,), (71,), 'left'),
    (17, 'press ENTER to re-map', (0, 311), (18,), (146,), 'left'),
    (18, 'enter new control', (309,), (18,), (116,), 'left'),
)


def find_font(explicit):
    if explicit:
        candidate = Path(explicit)
        if not candidate.is_file():
            raise ValueError('The supplied local font file does not exist')
        return candidate
    windows = os.environ.get('WINDIR') or os.environ.get('SystemRoot')
    folders = [Path(windows) / 'Fonts'] if windows else []
    local = os.environ.get('LOCALAPPDATA')
    if local:
        folders.append(Path(local) / 'Microsoft' / 'Windows' / 'Fonts')
    for folder in folders:
        if folder.is_dir():
            for candidate in sorted(folder.iterdir()):
                if candidate.name.lower() == 'hatten.ttf':
                    return candidate
    raise ValueError('Haettenschweiler was not found; supply --font with its local font file')


def alpha_runs(alpha):
    data = alpha.tobytes()
    runs = []
    start = 0
    while start < len(data):
        end = start + 1
        while end < len(data) and data[end] == data[start] and end - start < 65535:
            end += 1
        runs.extend((end - start, data[start]))
        start = end
    return runs


def generate(output, font_path):
    from PIL import Image, ImageDraw, ImageFilter, ImageFont

    # Same lighter .88 style as the accepted complete-word artwork: draw at
    # twice final raster size, narrow, erode four master pixels, then downsample.
    font = ImageFont.truetype(str(font_path), 192)
    family, _ = font.getname()
    if family.lower().replace(' ', '') != 'haettenschweiler':
        raise ValueError('The selected font is not Haettenschweiler')
    rendered = []
    reviews = []
    images = []
    for word, text, resources, sizes, advances, alignment in LABELS:
        canvas = Image.new('L', (2400, 512))
        ImageDraw.Draw(canvas).text((16, 200), text, font=font, fill=255, anchor='ls')
        bounds = canvas.getbbox()
        if not bounds:
            raise ValueError('An expected complete word has no ink')
        ink = canvas.crop(bounds)
        pad = Image.new('L', (ink.width + 32, ink.height + 32))
        pad.paste(ink, (16, 16))
        ink = pad.resize((round(pad.width * .88), pad.height), Image.Resampling.LANCZOS)
        ink = ink.filter(ImageFilter.MinFilter(9))
        ink = ink.crop(ink.getbbox())
        for source_size, advance in zip(sizes, advances):
            scale = source_size / 18.0
            shaped = ink.resize((max(1, round(ink.width * scale / 2)),
                                 max(1, round(ink.height * scale / 2))), Image.Resampling.LANCZOS)
            width, height = advance * 4, 128
            fit_x = min(1.0, (width - 4) / shaped.width)
            if fit_x < 1:
                shaped = shaped.resize((max(1, round(shaped.width * fit_x)), shaped.height),
                                       Image.Resampling.LANCZOS)
            x = width - 2 - shaped.width if alignment == 'right' else 2 if alignment == 'left' else (width - shaped.width) // 2
            # Presentation baseline follows the established word-art band, while
            # source16 receives 8/9 ink size. Physical alignment remains a preview
            # acceptance item; source glyph metrics are not replaced by guesses.
            y = round(96 + font.getbbox(text, anchor='ls')[1] * scale / 2)
            if x < 1 or x + shaped.width >= width or y < 1 or y + shaped.height >= height:
                raise ValueError('Authored ink exceeds its original allocation')
            alpha = Image.new('L', (width, height))
            alpha.paste(shaped, (x, y))
            rgba = Image.new('RGBA', (width, height), (255, 255, 255, 0))
            rgba.putalpha(alpha)
            image_id = len(rendered) + 1
            filename = 'word-%02d-size-%d.png' % (word, source_size)
            record = dict(image=image_id, word=word, text=text, sourceSize=source_size,
                          width=width, height=height, logicalAdvance=advance,
                          logicalHeight=32, alignment=alignment, resourceHints=list(resources),
                          rgbaSha256=hashlib.sha256(rgba.tobytes()).hexdigest(),
                          alphaRuns=alpha_runs(alpha), file=filename)
            rendered.append(record)
            reviews.append(dict(image=image_id, word=word, text=text, sourceSize=source_size,
                                inkBounds=list(alpha.getbbox()), fitX=fit_x,
                                alignment=alignment, baselineRasterPixels=96))
            images.append((filename, rgba))
    if len(LABELS) != 18 or len(rendered) != 28:
        raise ValueError('The finite label inventory changed')
    manifest = dict(schema=1, scope='Eighteen complete English Setup-controls labels in twenty-eight size-specific images; no font file, alphabet or original game pixels.',
                    typeface='Haettenschweiler', rasterScale=4, horizontalScale=.88,
                    lighterMasterMinFilter=9, baselineOriginalUiPixels=24,
                    sourceSizes=[16, 18], wholeWordGraphicsOnly=True,
                    originalDynamicBindingTextUnchanged=True,
                    resourceHintsSupplyNoSourceAuthority=True,
                    actualGamePlacementVerified=False, source16BaselinePreviewRequired=True,
                    words=rendered, review=reviews)
    # Validate all rendering before creating output. Never overwrite previous art.
    output = Path(output)
    if output.exists() and any(output.iterdir()):
        raise ValueError('Output folder must be new or empty to preserve previous artwork')
    output.mkdir(parents=True, exist_ok=True)
    for filename, rgba in images:
        rgba.save(output / filename)
    encoded = (json.dumps(manifest, separators=(',', ':'), ensure_ascii=True) + '\n').encode('ascii')
    (output / 'setup-controls-word-art.json').write_bytes(encoded)
    board = Image.new('RGB', (1100, len(images) * 160), (30, 30, 35))
    draw = ImageDraw.Draw(board)
    for index, ((_, rgba), row) in enumerate(zip(images, reviews)):
        draw.text((10, index * 160 + 4), '%s | source%d | %s' % (row['text'], row['sourceSize'], row['alignment']), fill='white')
        board.paste(rgba, (10, index * 160 + 25), rgba)
    board.save(output / 'setup-controls-word-review.png')
    return dict(labels=18, images=28, manifestSha256=hashlib.sha256(encoded).hexdigest(),
                privateOutputOnly=True, gameAssetsActivated=False)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True, help='New or empty private output directory')
    parser.add_argument('--font', type=Path, help='Optional locally installed Haettenschweiler font')
    args = parser.parse_args()
    print(json.dumps(generate(args.output, find_font(args.font)), sort_keys=True))
