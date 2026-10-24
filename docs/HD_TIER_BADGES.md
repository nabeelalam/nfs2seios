# Car class badge artwork candidates

The A, B and C class badges share an authored gold capsule and coupe silhouette. Letters and the main car silhouette use negative relief: their interiors are transparent, with limited gold counters, wheel rings and highlights. They are not opaque black lettering.

`python scripts/generate_tier_badge_art.py --output-dir OUTPUT` writes three deterministic SVGs and a manifest into an empty directory. The generator reads no original game images or font files. The vectors contain explicit paths and local material/mask definitions; no embedded raster assets, external references or text/font dependencies are used. Original game data remains separately supplied.

The source tests check deterministic generation, finite vector structure, local references, shared material/car construction, mask structure and refusal to overwrite existing output. They do not prove raster appearance, runtime selection, physical iPad rendering or final artistic acceptance.

Local loader and native presentation checks are tracked separately. All three designs remain review candidates. Do not describe the badges, the eight track emblems or the complete HD interface as finalized until their remaining visual and device checks are satisfied.

Local native-window tests now cover ordinary main-menu B -> C -> B -> A changes through the original Opponents controls. A separate instrumented run compared every original native badge payload byte and required fresh draw selections. The normal renderer run used two fresh class-letter image witnesses per choice and the expected authored texture acceptance earlier in that run. Its acceptance log is emitted once per texture/height; it does not prove a new upload at each return. See `evidence/hd-tier-badge-runtime.json` for the precise scope and limitations.

These checks do not establish complete badge material/fade/context recovery, split-screen behavior, iPad rendering or final artistic acceptance. Original captures, reference masks and packs remain private. The final normal rerun retains eight uniquely named visual witnesses and verifies reference hashes before and after execution. Independent recomputation reproduces every recorded class score. All eight logged native artwork textures created during that session were retired on shutdown; this is a session-scoped observation, not general leak or context-recovery coverage.
