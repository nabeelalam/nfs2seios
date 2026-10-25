"""Author one finite modal word mask and verify its generated source.

The title reuses existing Controls word4. No font file, alphabet or original
pixels are embedded. Authoring requires a locally installed Haettenschweiler;
checking existing generated headers needs only the Python standard library.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import os
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
TITLE_SHA = '603163b4df5f7b5b02a26906c60827f681593d56a1ca6ca14159665302812bdc'
MESSAGE_SHA = '3a89c0f668882ded92427e466cd6f0b078d0183fa1516d33c7c7e1bba75faf88'
WIDTH, HEIGHT = 432, 128
NOTICE = '// SPDX-License-Identifier: MIT\n// One authored complete-word alpha mask; no font, alphabet or original pixels.\n'

def find_font(explicit=None):
    if explicit is not None:
        p = Path(explicit)
        if not p.is_file():
            raise ValueError('The supplied local font file does not exist')
        return p
    system = os.environ.get('WINDIR') or os.environ.get('SystemRoot')
    folders = [Path(system) / 'Fonts'] if system else []
    local = os.environ.get('LOCALAPPDATA')
    if local:
        folders.append(Path(local) / 'Microsoft' / 'Windows' / 'Fonts')
    for folder in folders:
        if folder.is_dir():
            for p in sorted(folder.iterdir()):
                if p.name.lower() == 'hatten.ttf':
                    return p
    raise ValueError('Supply the locally installed Haettenschweiler font with --font')

def runs_for(data):
    out = []
    at = 0
    while at < len(data):
        end = at + 1
        while end < len(data) and data[end] == data[at] and end - at < 65535:
            end += 1
        out.extend((end - at, data[at]))
        at = end
    return out

def decode(runs):
    pixels = WIDTH * HEIGHT
    if type(runs) is not list or not runs or len(runs) % 2 or len(runs) > pixels * 2:
        raise ValueError('Modal message run inventory changed')
    alpha = bytearray()
    for length, value in zip(runs[::2], runs[1::2]):
        if type(length) is not int or type(value) is not int or not 1 <= length <= 65535 or not 0 <= value <= 255 or length > pixels - len(alpha):
            raise ValueError('Modal message run overflow')
        alpha.extend(bytes((value,)) * length)
    if len(alpha) != pixels or not any(alpha):
        raise ValueError('Modal message alpha size/ink changed')
    for x in range(WIDTH):
        if alpha[x] or alpha[(HEIGHT - 1) * WIDTH + x]:
            raise ValueError('Modal message vertical padding changed')
    for y in range(HEIGHT):
        if alpha[y * WIDTH] or alpha[y * WIDTH + WIDTH - 1]:
            raise ValueError('Modal message horizontal padding changed')
    rgba = bytearray(b'\xff' * (pixels * 4))
    rgba[3::4] = alpha
    if hashlib.sha256(rgba).hexdigest() != MESSAGE_SHA:
        raise ValueError('Modal message authored identity changed')
    return bytes(rgba)

def outputs(runs):
    decode(runs)
    identity = NOTICE + '#ifndef NFS_FEEDBACK_MODAL_WORD_IDENTITY_H\n#define NFS_FEEDBACK_MODAL_WORD_IDENTITY_H\n'
    identity += 'static const unsigned char nfs_feedback_modal_word_sha256[2][32]={\n'
    identity += ',\n'.join('{' + ','.join(str(x) for x in bytes.fromhex(sha)) + '}' for sha in (TITLE_SHA, MESSAGE_SHA))
    identity += '\n};\n#endif\n'
    data = NOTICE + '#ifndef NFS_FEEDBACK_MODAL_WORD_DATA_H\n#define NFS_FEEDBACK_MODAL_WORD_DATA_H\n'
    data += 'static const unsigned short nfs_feedback_modal_word_message_alpha_rle[]={\n'
    data += ',\n'.join(','.join(map(str, runs[i:i + 24])) for i in range(0, len(runs), 24))
    data += '\n};\n#endif\n'
    return {'FeedbackModalWordIdentity.h': identity.encode('ascii'), 'FeedbackModalWordData.h': data.encode('ascii')}

def read_runs(directory=None):
    directory = ROOT / 'src/hd' if directory is None else Path(directory)
    raw = (directory / 'FeedbackModalWordData.h').read_bytes()
    if len(raw) > 1048576:
        raise ValueError('Modal authored data source too large')
    pattern = re.escape(NOTICE + '#ifndef NFS_FEEDBACK_MODAL_WORD_DATA_H\n#define NFS_FEEDBACK_MODAL_WORD_DATA_H\nstatic const unsigned short nfs_feedback_modal_word_message_alpha_rle[]={\n') + r'([0-9,\n]+)' + re.escape('\n};\n#endif\n')
    match = re.fullmatch(pattern, raw.decode('ascii'))
    if not match:
        raise ValueError('Unexpected modal authored data source')
    return [int(x) for x in match.group(1).replace('\n', '').split(',')]

def check(directory=None):
    directory = ROOT / 'src/hd' if directory is None else Path(directory)
    for name, data in outputs(read_runs(directory)).items():
        if (directory / name).read_bytes() != data:
            raise ValueError('Stale modal word source ' + name)

def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('Duplicate modal manifest key')
        result[key] = value
    return result


def generate(manifest, directory=None):
    document = json.loads(Path(manifest).read_text(encoding='utf-8'), object_pairs_hook=unique_object)
    expected = {'schema': 1, 'word': 2, 'text': 'device not found', 'width': WIDTH, 'height': HEIGHT,
                'rasterScale': 4, 'logicalAdvance': 108, 'logicalHeight': 32,
                'baselineOriginalUiPixels': 24, 'typeface': 'Haettenschweiler',
                'horizontalScale': .88, 'lighterMasterMinFilter': 9, 'rgbaSha256': MESSAGE_SHA,
                'inkBounds': [6, 29, 426, 93], 'fitX': 1.0}
    if type(document) is not dict or set(document) != set(expected) | {'alphaRuns'} or any(type(document[k]) is not type(v) or document[k] != v for k, v in expected.items()) or any(type(x) is not int for x in document['inkBounds']):
        raise ValueError('Modal word authoring manifest changed')
    encoded = outputs(document['alphaRuns'])
    directory = ROOT / 'src/hd' if directory is None else Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    for name, data in encoded.items():
        p = directory / name
        if p.exists() and p.read_bytes() != data:
            raise FileExistsError('Refusing to overwrite different modal word source ' + name)
        if not p.exists():
            p.write_bytes(data)

def author(output, font_path=None):
    from PIL import Image, ImageDraw, ImageFilter, ImageFont
    output = Path(output)
    if output.exists():
        raise FileExistsError('Choose a new private artwork output folder')
    font = ImageFont.truetype(str(find_font(font_path)), 192)
    if font.getname()[0].lower().replace(' ', '') != 'haettenschweiler':
        raise ValueError('The selected local typeface is not Haettenschweiler')
    canvas = Image.new('L', (2400, 512))
    text = 'device not found'
    ImageDraw.Draw(canvas).text((16, 200), text, font=font, fill=255, anchor='ls')
    ink = canvas.crop(canvas.getbbox())
    pad = Image.new('L', (ink.width + 32, ink.height + 32))
    pad.paste(ink, (16, 16))
    ink = pad.resize((round(pad.width * .88), pad.height), Image.Resampling.LANCZOS)
    ink = ink.filter(ImageFilter.MinFilter(9))
    ink = ink.crop(ink.getbbox())
    shaped = ink.resize((max(1, round(ink.width / 2)), max(1, round(ink.height / 2))), Image.Resampling.LANCZOS)
    fit_x = min(1.0, (WIDTH - 4) / shaped.width)
    if fit_x < 1:
        shaped = shaped.resize((max(1, round(shaped.width * fit_x)), shaped.height), Image.Resampling.LANCZOS)
    x = (WIDTH - shaped.width) // 2
    y = round(96 + font.getbbox(text, anchor='ls')[1] / 2)
    if x < 1 or x + shaped.width >= WIDTH or y < 1 or y + shaped.height >= HEIGHT:
        raise ValueError('Authored message ink exceeds the source allocation')
    alpha = Image.new('L', (WIDTH, HEIGHT))
    alpha.paste(shaped, (x, y))
    rgba = Image.new('RGBA', (WIDTH, HEIGHT), (255, 255, 255, 0))
    rgba.putalpha(alpha)
    record = dict(schema=1, word=2, text=text, width=WIDTH, height=HEIGHT, rasterScale=4,
                  logicalAdvance=108, logicalHeight=32, baselineOriginalUiPixels=24,
                  typeface='Haettenschweiler', horizontalScale=.88, lighterMasterMinFilter=9,
                  rgbaSha256=hashlib.sha256(rgba.tobytes()).hexdigest(),
                  alphaRuns=runs_for(alpha.tobytes()), inkBounds=list(alpha.getbbox()), fitX=fit_x)
    output.mkdir()
    rgba.save(output / 'device-not-found-4x.png')
    (output / 'word-art.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    return record

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--check', action='store_true')
    group.add_argument('--author', type=Path)
    group.add_argument('--from-art', type=Path)
    parser.add_argument('--font', type=Path)
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    if args.author:
        record = author(args.author, args.font)
        print(json.dumps({k: v for k, v in record.items() if k != 'alphaRuns'}, indent=2))
    elif args.from_art:
        generate(args.from_art, args.output_dir)
        print('Modal message source generated; title artwork shared')
    else:
        check(args.output_dir)
        print('Modal word source identities verified; title artwork shared')
