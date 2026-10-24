"""Author the four classic steering bands as new vector artwork.

This reconstructs a gold capsule housing, inset red range and raised divider.
It reads no original bitmap pixels. Outputs are private previews until a genuine
original chart packet and final-frame join authorize renderer integration.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path

WIDTH, HEIGHT = 356, 49
LEVELS = ((0, 'full', 1.0), (1, 'three quarter', .75),
          (2, 'half', .5), (3, 'one quarter', .25))


def svg(level):
    if type(level) is not int or not 0 <= level < len(LEVELS):
        raise ValueError('Expected an original steering level from 0 through 3')
    _, label, fraction = LEVELS[level]
    # Intentional reconstruction: range and divider share the housing centre.
    # Original low-resolution edge noise and slight asymmetry are not enlarged.
    left, right, centre = 18.0, 330.0, 174.0
    span = (right-left)*fraction
    start = centre-span/2
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="1424" height="196" viewBox="0 0 356 49">
<title>Classic steering sensitivity: {label}</title>
<defs>
 <linearGradient id="rim" x2="0" y2="1"><stop stop-color="#fff0ad"/><stop offset=".16" stop-color="#efc45c"/><stop offset=".53" stop-color="#b67b26"/><stop offset=".82" stop-color="#dfac46"/><stop offset="1" stop-color="#7a440e"/></linearGradient>
 <linearGradient id="gold" x2="0" y2="1"><stop stop-color="#85501a"/><stop offset=".22" stop-color="#ad7a28"/><stop offset=".61" stop-color="#c2953e"/><stop offset="1" stop-color="#8a581b"/></linearGradient>
 <linearGradient id="red" x2="0" y2="1"><stop stop-color="#650c07"/><stop offset=".16" stop-color="#d51a0c"/><stop offset=".43" stop-color="#ff2911"/><stop offset=".85" stop-color="#f31c08"/><stop offset="1" stop-color="#ac1008"/></linearGradient>
 <linearGradient id="divider" x2="1" y2="0"><stop stop-color="#e73a22"/><stop offset=".18" stop-color="#c82315"/><stop offset=".76" stop-color="#a8150e"/><stop offset="1" stop-color="#6f0e09"/></linearGradient>
 <linearGradient id="cap" x2="1" y2="0"><stop stop-color="#ffe99c"/><stop offset=".42" stop-color="#efbb53"/><stop offset="1" stop-color="#a66a1d"/></linearGradient>
 <filter id="housingShadow" x="-8%" y="-40%" width="120%" height="180%"><feDropShadow dx="2.2" dy="4" stdDeviation="2.2" flood-color="#000" flood-opacity=".58"/></filter>
 <filter id="dividerShadow" x="-60%" y="-25%" width="240%" height="170%"><feDropShadow dx="1.6" dy="2" stdDeviation="1.1" flood-color="#000" flood-opacity=".6"/></filter>
 <clipPath id="housing"><path d="M19 6H329C338 6 343 12 343 21S338 36 329 36H19C10 36 5 30 5 21S10 6 19 6Z"/></clipPath>
 <clipPath id="window"><rect x="18" y="8.5" width="312" height="25" rx=".35"/></clipPath>
 <pattern id="brushed" patternUnits="userSpaceOnUse" width="31" height="7"><path d="M0 .7H24M5 3.8H31M0 6.1H15" stroke="#fff3bc" stroke-width=".24" opacity=".11"/><path d="M11 1.8H31M0 4.9H23" stroke="#4e2c09" stroke-width=".2" opacity=".1"/></pattern>
</defs>
<path d="M19 6H329C338 6 343 12 343 21S338 36 329 36H19C10 36 5 30 5 21S10 6 19 6Z" fill="url(#rim)" stroke="#6e4317" stroke-width=".65" filter="url(#housingShadow)"/>
<g clip-path="url(#housing)">
 <path d="M19 6.6C10.6 6.6 5.7 12.4 5.7 21S10.6 35.3 19 35.3V6.6Z" fill="url(#cap)"/>
 <rect x="18" y="8.5" width="312" height="25" fill="url(#gold)"/>
 <rect x="18" y="8.5" width="312" height="25" fill="url(#brushed)"/>
 <g clip-path="url(#window)"><rect x="{start:.2f}" y="8.5" width="{span:.2f}" height="25" fill="url(#red)"/><path d="M{start:.2f} 9.4H{start+span:.2f}" stroke="#ffb485" stroke-width=".45" opacity=".55"/></g>
 <path d="M18 33.6V8.3H330" fill="none" stroke="#64340e" stroke-width=".85"/>
 <path d="M18 33.5H330V8.5" fill="none" stroke="#ffe49a" stroke-width=".75" opacity=".8"/>
 <path d="M19 6.8H329C335.8 6.8 340.1 10.9 341.7 16" fill="none" stroke="#fff0b7" stroke-width=".7" opacity=".85"/>
 <path d="M18 35.6H329C337.1 35.6 342.2 29.4 342.5 23" fill="none" stroke="#79430f" stroke-width=".7"/>
</g>
<g filter="url(#dividerShadow)"><path d="M170 4H177L178 5V37H170Z" fill="url(#divider)"/><path d="M170.4 4.5H176.8" stroke="#f45c3c" stroke-width=".65"/><path d="M170.5 5V36.5" stroke="#ef5030" stroke-width=".45" opacity=".65"/><path d="M177.5 5V37H170" fill="none" stroke="#73100a" stroke-width=".7"/></g>
</svg>'''


def generate(output):
    output.mkdir(parents=True, exist_ok=False)
    images = []
    for level, label, fraction in LEVELS:
        data = svg(level).encode('utf-8')
        name = 'steering-band-%u.svg' % level
        (output/name).write_bytes(data)
        images.append(dict(level=level, label=label, fraction=fraction,
                           file=name, sha256=hashlib.sha256(data).hexdigest()))
    manifest = dict(schema=1, logicalSize=[WIDTH, HEIGHT], rasterSize=[1424, 196],
                    source='New vector reconstruction; no original pixel input',
                    identity='Gold capsule, inset symmetric red range, raised red divider',
                    rangeWindow=[18, 8.5, 330, 33.5], dividerCentre=174,
                    rendererActivated=False, originalSourceProofRequired=True,
                    images=images)
    (output/'steering-bar-art.json').write_text(json.dumps(manifest, indent=2)+'\n')
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = generate(args.output)
    print(json.dumps(dict(images=len(result['images']), rendererActivated=False)))


if __name__ == '__main__':
    main()
