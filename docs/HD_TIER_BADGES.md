# Car class badge artwork candidates

The A, B and C class badges share an authored gold capsule and coupe silhouette. Letters and the main car silhouette use negative relief: their interiors are transparent, with limited gold counters, wheel rings and highlights. They are not opaque black lettering.

`python scripts/generate_tier_badge_art.py --output-dir OUTPUT` writes three deterministic SVGs and a manifest into an empty directory. The generator reads no original game images or font files. The vectors contain explicit paths and local material/mask definitions; no embedded raster assets, external references or text/font dependencies are used. Original game data remains separately supplied.

The source tests check deterministic generation, finite vector structure, local references, shared material/car construction, mask structure and refusal to overwrite existing output. They do not prove raster appearance, runtime selection, physical iPad rendering or final artistic acceptance.

Local loader and native presentation checks are tracked separately. All three designs remain review candidates. Do not describe the badges, the eight track emblems or the complete HD interface as finalized until their remaining visual and device checks are satisfied.
