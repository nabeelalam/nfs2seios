"""Author three finite classic gold tier badges as explicit vector artwork.

This generator reads no original images, font files or other inputs. The shared
coupe/material geometry and three whole letter paths are authored in this file.
Outputs are review artwork, not an installed replacement or accepted remaster.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path

LETTERS = {
    'A': 'M14.7 34.1 17.5 30.8 24.1 13.2 22.5 11.1 23.0 9.9 38.9 9.9 39.4 11.3 37.9 13.1 42.4 30.7 45.1 32.3 44.8 34.1 30.7 34.1 30.7 32.4 33.0 30.8 31.7 25.5 24.0 25.5 22.2 30.7 24.7 32.3 24.2 34.1 Z M25.2 22.2 31.0 22.2 29.2 14.9 Z',
    'B': 'M15.9 9.9 30.5 9.9 C38.9 9.9 42.8 12.0 42.5 16.4 C42.3 19.0 40.3 20.7 36.5 21.5 C41.0 22.2 43.1 24.1 42.6 27.3 C42.0 32.0 37.9 34.1 29.8 34.1 L15.0 34.1 15.4 32.1 18.2 30.8 19.9 13.2 15.6 12.0 Z M27.5 13.7 27.5 19.1 29.3 19.1 C31.9 19.1 33.1 18.3 33.1 16.4 C33.1 14.5 31.8 13.7 29.3 13.7 Z M27.5 23.7 27.5 30.1 29.5 30.1 C32.4 30.1 33.8 29.1 33.8 26.8 C33.8 24.7 32.5 23.7 29.5 23.7 Z',
    'C': 'M40.6 10.6 40.6 18.9 33.1 18.9 33.1 16.5 Q33.1 13.9 29.9 13.9 H28.2 Q24.3 13.9 24.3 18.4 V26.5 Q24.3 30.2 28.2 30.2 H30.0 Q33.1 30.2 33.1 27.7 V25.4 H40.1 V32.5 Q35.8 34.5 30.0 34.5 H26.4 Q15.2 34.5 15.2 22.4 Q15.2 9.4 28.9 9.4 H31.1 Q36.3 9.4 40.6 10.6 Z',
}

DEFS = '''<defs>
 <linearGradient id="gold" x1="5" y1="11" x2="119" y2="32" gradientUnits="userSpaceOnUse"><stop stop-color="#fff15a"/><stop offset=".16" stop-color="#ffdb2b"/><stop offset=".43" stop-color="#eaa31b"/><stop offset=".76" stop-color="#ad6c15"/><stop offset="1" stop-color="#62451d"/></linearGradient>
 <linearGradient id="bevel" x1="0" y1="6" x2="0" y2="38" gradientUnits="userSpaceOnUse"><stop stop-color="#ffe995"/><stop offset=".24" stop-color="#8e5a09"/><stop offset=".62" stop-color="#a5680d"/><stop offset="1" stop-color="#33250d"/></linearGradient>
 <linearGradient id="glass" x1="70" y1="12" x2="102" y2="23" gradientUnits="userSpaceOnUse"><stop stop-color="#724b11"/><stop offset=".58" stop-color="#2d220c"/><stop offset="1" stop-color="#0c0d07"/></linearGradient>
 <linearGradient id="rim" x1="0" y1="24" x2="0" y2="36" gradientUnits="userSpaceOnUse"><stop stop-color="#f6c33c"/><stop offset=".48" stop-color="#a96b12"/><stop offset="1" stop-color="#d69b27"/></linearGradient>
 <linearGradient id="metal-sheen" x1="0" y1="6" x2="0" y2="38" gradientUnits="userSpaceOnUse"><stop stop-color="#fff8bf" stop-opacity=".40"/><stop offset=".20" stop-color="#fff8bf" stop-opacity=".10"/><stop offset=".46" stop-color="#fff8bf" stop-opacity="0"/><stop offset=".78" stop-color="#49320b" stop-opacity=".19"/><stop offset="1" stop-color="#ffe174" stop-opacity=".19"/></linearGradient><clipPath id="capsule"><path d="M21 6.1H102.7C112.8 6.1 118.8 11.7 118.8 21.6S113.1 37.9 102.7 37.9H21C10.8 37.9 5.1 32.1 5.1 22S10.8 6.1 21 6.1Z"/></clipPath>
</defs>'''

# One authored low, long-nosed coupe in all three badges. The rear haunch,
# swept cabin, two open wheel arches, short deck and rounded right-facing nose
# define the shared subject; the fine lines are metal/glass relief, not a grille
# of unrelated marks or an enlarged low-resolution pixel contour.
# Alpha is semantic: letter and coupe are empty relief cut through the metal,
# rather than black paint. Only thin metal/window/wheel details remain visible.
COUPE_CUTOUT = 'M33.3 31.9 34.5 28.4 41.3 24.5 50.5 21.0 61.1 17.5 69.8 12.9Q76.2 10.2 84.1 10.5L95.2 10.9Q101.9 11.2 106.8 15.1L111.3 18.3Q114.2 20.7 114.2 25.0L112.4 30.9 109.5 34.6 98.7 35.3 86.4 35.9 78.2 35.5 62.6 35.5 49.0 35.1 39.5 33.8Z'
CAR_RELIEF = '''<g id="coupe-metal-relief" clip-path="url(#capsule)" fill="none" stroke-linecap="round" stroke-linejoin="round">
 <!-- Restrained roof/glass and headlamp hints inside a bold empty silhouette. -->
 <path d="M68.3 16.6Q75.7 12.3 83.2 12.4L94.5 12.9Q99.0 13.4 102.3 15.9M91.8 13.1 95.0 16.6" stroke="#9c711f" stroke-width=".65"/>
 <path d="M107.0 19.9 110.5 20.7 110.0 21.8 107.8 22.0" stroke="#bf8e28" stroke-width=".75"/>
 <ellipse cx="69.7" cy="30.7" rx="4.65" ry="6.0" stroke="url(#rim)" stroke-width="2.15"/>
 <ellipse cx="105.2" cy="29.7" rx="3.0" ry="5.05" stroke="#ac7c22" stroke-width="1.15"/>
</g>'''

def svg(letter):
    mask = '''<defs><mask id="negative-relief" maskUnits="userSpaceOnUse" x="0" y="0" width="123" height="44"><path d="M0 0H123V44H0Z" fill="white"/><path d="%s" fill="black" fill-rule="evenodd"/><path d="%s" fill="black"/></mask></defs>''' % (LETTERS[letter], COUPE_CUTOUT)
    return '''<svg xmlns="http://www.w3.org/2000/svg" width="123" height="44" viewBox="0 0 123 44">
<title>Classic gold tier %s negative-relief badge, authored review</title>
%s
%s
<g mask="url(#negative-relief)">
<path d="M21 5.7H102.7C113.0 5.7 119.2 11.5 119.2 21.6S113.3 38.3 102.7 38.3H21C10.5 38.3 4.7 32.3 4.7 22S10.5 5.7 21 5.7Z" fill="url(#bevel)" stroke="#261e0b" stroke-width=".7"/>
<path d="M21 6.7H102.7C112.4 6.7 118.2 12.0 118.2 21.6S112.7 37.3 102.7 37.3H21C11.2 37.3 5.7 31.7 5.7 22S11.2 6.7 21 6.7Z" fill="url(#gold)"/>
<path d="M21 6.7H102.7C112.4 6.7 118.2 12.0 118.2 21.6S112.7 37.3 102.7 37.3H21C11.2 37.3 5.7 31.7 5.7 22S11.2 6.7 21 6.7Z" fill="url(#metal-sheen)"/>
<path d="M9.7 14.3Q13.1 7.5 21.7 7.4H96.1M9.6 29.6Q12.7 36.5 22.1 36.5H45" fill="none" stroke="#fff294" stroke-opacity=".68" stroke-width=".55" stroke-linecap="round"/>
</g>
%s
</svg>
''' % (letter, DEFS, mask, CAR_RELIEF)

def generate(output_dir):
    output = Path(output_dir)
    if output.exists() and any(output.iterdir()):
        raise ValueError('Output directory must be new or empty; prior reviews are preserved')
    rendered = [(letter, svg(letter).encode('utf-8')) for letter in ('A', 'B', 'C')]
    output.mkdir(parents=True, exist_ok=True)
    manifest = {'schema': 1, 'viewBox': [0, 0, 123, 44],
                'scope': 'Three finite authored negative-relief tier badges; shared classic coupe and gold material',
                'explicitPathsOnly': True, 'letterAndCoupeAreTransparentCutouts': True, 'fontOrOriginalPixelsEmbedded': False,
                'accepted': False, 'nativeBinding': False, 'badges': []}
    for letter, source in rendered:
        filename = 'tier-%s.svg' % letter.lower()
        with (output / filename).open('xb') as handle:
            handle.write(source)
        manifest['badges'].append({'tier': letter, 'svg': filename,
                                   'sha256': hashlib.sha256(source).hexdigest()})
    with (output / 'tier-badge-art.json').open('x', encoding='utf-8') as handle:
        json.dump(manifest, handle, indent=2)
        handle.write('\n')
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(generate(args.output_dir), sort_keys=True))
