"""Emit only eight complete authored steering-word images; no reusable font."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path

LABELS = ('sensitivity', 'DONE', 'CANCEL', 'STEERING SENSITIVITY',
          'full', 'three quarter', 'half', 'one quarter')
ADVANCES = (66, 35, 51, 149, 21, 85, 25, 75)


def generate(manifest, output):
    document = json.loads(Path(manifest).read_text(encoding='utf-8'))
    words = document['words']
    if (len(words) != 8 or document['baselineOriginalUiPixels'] != 24
            or not document['wholeWordGraphicsOnly']):
        raise ValueError('Unexpected finite authored manifest')
    identities = ['// SPDX-License-Identifier: MIT',
                  '#ifndef NFS_CONTROLS_STEERING_WORD_IDENTITY_H',
                  '#define NFS_CONTROLS_STEERING_WORD_IDENTITY_H', '#include <stdint.h>']
    data = ['// SPDX-License-Identifier: MIT',
            '// Generated finite complete-word alpha images from approved authored manifest.',
            '#pragma once']
    hashes = []
    for index, word in enumerate(words):
        width, height = 4 * ADVANCES[index], 128
        if (word['image'] != index + 1 or word['word'] != index + 1
                or word['text'] != LABELS[index] or word['sourceSize'] != 18
                or word['logicalAdvance'] != ADVANCES[index]
                or word['width'] != width or word['height'] != height):
            raise ValueError('Unexpected authored word identity')
        runs = word['alphaRuns']
        if not runs or len(runs) % 2:
            raise ValueError('Invalid alpha RLE')
        alpha = bytearray()
        for count, value in zip(runs[::2], runs[1::2]):
            if not 1 <= count <= 65535 or not 0 <= value <= 255 or len(alpha) + count > width * height:
                raise ValueError('Out-of-range alpha RLE')
            alpha.extend(bytes([value]) * count)
        if len(alpha) != width * height:
            raise ValueError('Incomplete alpha RLE')
        rgba = bytearray(b'\xff\xff\xff\xff' * len(alpha))
        rgba[3::4] = alpha
        digest = hashlib.sha256(rgba).hexdigest()
        if digest != word['rgbaSha256']:
            raise ValueError('Authored image digest mismatch')
        if any(alpha[:width]) or any(alpha[-width:]) or any(alpha[::width]) or any(alpha[width - 1::width]):
            raise ValueError('Missing transparent original-allocation padding')
        hashes.append('{' + ','.join(map(str, bytes.fromhex(digest))) + '}')
        data.append('static const unsigned short nfs_controls_steering_word_alpha_rle_%d[]={%s};'
                    % (index + 1, ','.join(map(str, runs))))
    for name, values in (('width', (4 * value for value in ADVANCES)),
                         ('advance', ADVANCES), ('source_size', (18,) * 8)):
        identities.append('static const unsigned nfs_controls_steering_word_%s[8]={%s};'
                          % (name, ','.join(map(str, values))))
    identities.append('static const char*const nfs_controls_steering_word_text[8]={%s};'
                      % ','.join(json.dumps(label) for label in LABELS))
    identities.append('static const unsigned char nfs_controls_steering_word_sha256[8][32]={%s};'
                      % ','.join(hashes))
    identities.append('#endif')
    output = Path(output)
    if not output.is_dir():
        raise ValueError('Output must be an existing source directory')
    for suffix, lines in (('Identity.h', identities), ('Data.h', data)):
        (output / ('ControlsSteeringWord' + suffix)).write_text('\n'.join(lines) + '\n', encoding='utf-8')
    return {'images': 8, 'wholeWordsOnly': True, 'nativeActivation': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    arguments = parser.parse_args()
    print(json.dumps(generate(arguments.manifest, arguments.output), sort_keys=True))
