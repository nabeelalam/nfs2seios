# HD interface and texture work

Development branch: `codex/hd-ui`. Integration branch: `main`.

This branch incorporates main through the build 19 session-recorder delivery.
It is an independent workstream. Performance, battery work and session recording
continue on main; no unfinished visual changes are required there. This document
establishes the work, not a claim that replacement artwork is implemented.

## Scope and order

1. Inspect the existing HUD and frontend rendering locally. Identify which paths
   draw speedometer, lap information, map, record text and starting lights. Keep
   source texture resolution distinct from final framebuffer resolution: raising
   the output resolution alone does not add detail to low-resolution artwork.
2. Make a small optional replacement proof for one HUD element. Preserve its
   layout, transparency, palette behavior, animation and split-screen placement.
   Decide the replacement boundary from the inspected draw path before changing
   shared renderer interfaces. Do not start with a broad renderer rewrite.
3. Compare original and replacement output in the Linux pixel harness and an
   actual original-data race. Check both players, multiple output sizes, menus,
   track transitions and context recreation. Keep originals untouched.
4. Expand to the remaining HUD and text only after that proof works. Preserve
   existing timing, game rules, input and accessibility controls. Start lights
   must retain the original sequence and timing.
5. Request one Apple/device test when a meaningful visual milestone passes host
   validation. Measure frame time and memory at native resolution in a busy
   split-screen race before proposing integration into main.

## Keep main easy to integrate

- Keep visual helpers, tests and documentation in dedicated files where possible.
  Make necessary existing-renderer hooks small and explicit. Do not duplicate the
  engine, build system or performance fixes into a separate implementation.
- Keep the current main version of diagnostics, recording, frame pacing, memory
  ownership and input behavior. Resolve conflicts around these contracts rather
  than taking one entire file or assuming the branch version is newer.
- Keep feature commits separate from maintenance merges. Regularly merge
  `origin/main` into this shared branch after main advances. This preserves
  published history. Rebase is an option for unpublished local commits; do not
  force-push the shared branch as routine maintenance.
- After an integration, inspect the diff against main, run relevant existing
  host tests plus the visual checks, and verify that no stale copied performance
  code or unrelated changes remain. Record failures rather than hiding them.
- Keep replacement loading optional with a complete original-art fallback.
  Missing or invalid replacements must preserve the working game. Do not enable
  unfinished visual changes on main. Merge a reviewed feature only after visual,
  performance and actual-device acceptance.

Normal integration procedure, from this branch's separate checkout:

```sh
git fetch origin
git merge origin/main
git diff origin/main...HEAD
```

Before merging, require a clean working tree or preserve unfinished changes in
scoped local commits. Do not reset, clean or overwrite active work. A branch
reduces interference; it cannot guarantee that future edits never conflict.

## Privacy and build boundaries

Original assets, extracted images, game profiles, captures and local diagnostic
logs stay in ignored local storage. Do not publish game data, derivative texture
packs without a separate rights review, private paths or device information.
Original-data tests use copies; original game folders remain unchanged. All
published commits must pass the existing policy and independent secret scans.

Keep existing manual-only CI behavior. Creating, updating or publishing this
branch does not justify a cloud build. Use local host and pixel checks first;
an Apple build must answer a specific unresolved SDK or device question. The
current main-only cloud policy remains unchanged until a branch device test is
actually needed and reviewed.

## Acceptance

- Sharper intended interface elements, with unchanged game behavior and usable
  layout in single-player and split-screen at supported resolutions.
- Correct transparent edges, palette changes and start-light animation; no
  stale replacements after a track or graphics-context change.
- Original-art fallback verified with absent and malformed replacements.
- No meaningful frame-time regression, unbounded resource growth or continuing
  background work. Host measurements do not establish iPad power consumption.
- Physical iPad screenshots and play test before calling the work finalized.

## Current status

Main through the delivered build 19 recorder and its documentation has been
merged without conflicts. The difference against main remains dedicated visual
work and its plan; no HD runtime changes have been merged back into main.

The read-only `scripts/inspect_hud_archive.py` inventories uncompressed FSH
directory and entry headers with bounded reads, exact archive-size checks and
entry-span validation. Synthetic tests cover malformed directories, offsets,
truncation and raw signed metadata. It neither exports nor changes artwork.
Private inspection of nine copied HUD archives succeeded and verified the copies
were unchanged. The shared HUD archive contains 194 bitmap entries, with maximum
dimensions 209 by 135; map-marker images are at most 9 by 9. Dimensions alone do
not identify which visible element an entry draws.

The format layout was checked against the upstream
[FSH structures](https://github.com/OpenNFS/LibOpenNFS/blob/3ebd326c1ee3ac0858bd7fa522d077978cc1f016/Shared/FSH/FshTypes.h)
and [archive reader](https://github.com/OpenNFS/LibOpenNFS/blob/3ebd326c1ee3ac0858bd7fa522d077978cc1f016/Shared/FSH/FshArchive.cpp).
No upstream code is executed or incorporated into this utility. Original-data
inventories and source research copies remain in ignored local storage.

Next: decode and visually identify selected entries locally, then trace the
actual race draw path. Determine whether an element reaches Glide textures or a
software framebuffer before selecting the replacement hook. Palette identity,
transparent pixels and both split-screen views must be preserved. Changing
filtering alone does not constitute HD artwork. No HD renderer, replacement
artwork, FPS improvement or device behavior is claimed yet.

The next inspection stage now has a bounded indexed8 decoder and native-size PNG
exporter for private review. Three synthetic fixtures check exact RGBA/PNG pixels,
transparent palette entries, local-palette precedence, unsupported formats and
malformed dimensions/indices/attachment spans. All 246 indexed8 entries in the
nine copied HUD archives decoded without a rejected entry; copies were unchanged.
Only three native previews have been visually reviewed so far: a grey HUD panel,
a grid panel and a shaded map dot. This does not establish complete visual parity,
runtime usage or any HD replacement. Images and decoded inventories stay ignored.

The directory contains repeated entry names, so the inspection tools select by
entry index instead of silently overwriting duplicates. Runtime replacement
identity must also account for actual pixels, dimensions and palette/context
lifetime; a filename, short entry name or reused texture address alone is unsafe.
Static source inspection connects HUD named-entry lookup to the original layout
setup. A subsequent bounded original-data split-screen trace identified a packed
256-square RGBA5551 HUD atlas: 15 source-entry pixel matches, with 22 same-frame
triangle pairs using the same UVs and geometry shifted by one viewport height.
The atlas includes HUD fonts and icons and is used in both views; an icon being
present in this shared atlas does not mean that icon was drawn during the race.
The trace itself adds overhead and provides no FPS or energy measurement.

## Optional atlas upload boundary

The first experimental hook accepts one privately generated atlas pack. It
matches every original converted RGBA5551 byte, format and size at texture
download/context restoration. It changes the GPU image dimensions and RGBA8
pixels while retaining original CPU pixels, UVs, screen geometry and filtering.
Different data at a reused texture address follows the original upload path.
Other texture formats and palette-generation behavior remain unchanged.

The app looks for `gamedata/hud-hd.nfsatlas` within its selected/imported asset
root, once before the game session starts. A valid explicit pack takes priority.
If it is absent or invalid, this branch enables the built-in recipe below;
unrecognized original textures retain their original art. Configuration changes
are permitted before a session starts; clearing takes
place after its renderer and workers stop. CPU pack data survives context
recreation and is released at session end, including exceptions. There is no
per-frame file polling. Packs are private original-data derivatives and ignored
by Git; they are never included in the application package or source publication.

The development format is a 32-byte little-endian header:

| Offset | Field |
| --- | --- |
| 0 | Eight bytes: `NFSHDA1` followed by zero |
| 8 | Original square edge, power of two, at most 256 |
| 12 | Replacement square edge, power of two, larger, at most 2048 and 8x original |
| 16 | Original byte count, exactly edge squared times two |
| 20 | Replacement byte count, exactly edge squared times four |
| 24, 28 | Reserved, both zero |

Then come the exact original GPU RGBA5551 bytes and straight RGBA8 replacement
bytes, without trailers. All sizes are checked before allocation. Short files,
unknown flags, unsupported dimensions and extra data are rejected. GPU maximum
texture size is checked on a match; unsupported sizes retain original art.

Optimized and ASan/UBSan parser fixtures exercise full-content identity, all
truncation points, invalid headers, trailing data, allocation bounds, alpha and
ownership. Actual GLES fixtures verify larger image pixels/dimensions, exact
transparent RGBA, same-address content changes, original fallback and context
recreation, alongside the existing palette and resolution checks. This is an
upload-boundary proof, not finished HD artwork or device acceptance. A subsequent
20-second original-data race uses a conspicuously colored synthetic 512-square
replacement in both split-screen HUDs, with both native controllers active,
reset, simulated background/resume and clean shutdown. The temporary pack is
removed and copied configuration restored afterward. The test retains 64 game
objects and changes only the renderer and app lifetime. This 480-line host run
does not establish HD visual quality, device performance or energy efficiency.
Timing numeral candidates follow below. Native-display review, track transitions
and device frame-time/memory checks remain required before final acceptance.

## Timing digit artwork candidate

The first ten hand-drawn timing numerals are in `assets/hud/timing-0.svg` through
`timing-9.svg`, with a source-hash/placement catalog in `timing-font.json`.
These are new path/gradient artwork, not embedded original images. The style is
provisional. Only these specifically named, bounded path/gradient SVGs are allowed
by publication checks; embedded images, scripts, external resources, text,
metadata/comments, unknown attributes and other SVG locations remain rejected.

Each 12x27 vector canvas renders at 48x108 pixels. Only the first 104 raster rows
replace a 12x26 original rectangle: original atlas row47 is shared with the speed
font and must remain untouched. The private assembled atlas is 1024-square.
Every pixel outside the ten rectangles, including the shared row, is checked
against original nearest expansion. Colon/dot punctuation and other HUD elements
are still original.

A bounded original-data split race with both native controllers passes 480 to
960 to 480 render-height changes, reset, simulated background/resume and clean
shutdown. The visual test first recognizes unchanged player indicators, captures
the candidate HUD as its own reference, then requires its clock to advance. An
earlier original-font template timeout is retained as a test-harness failure,
not accepted as a game pass. Screenshots still present at 640x480 while the game
framebuffer reaches 1280x960; native-display visual quality is not established.

For developers, `scripts/build_timing_atlas.py` accepts a private captured
256-square GPU RGBA5551 atlas, its independently known SHA-256 and a new output
filename. It verifies the capture, vector source catalog and layout, then uses
locally installed librsvg/Cairo to create the optional pack without modifying
originals. The pack contains original data and must remain private. This is an
offline development path. The automatic runtime path below removes this manual
step for the recognized original atlas. No artwork is added to main by this
prototype; physical iPad acceptance remains required.

## Automatic built-in generation

The HD branch enables built-in timing artwork when no explicit private override
pack is supplied. `TimingFontPixels.inc` contains only the new glyph pixels,
run-length encoded from the reviewed vector sources. The generator reads no
original game images. Original-atlas metadata contains only content fingerprints.
Apple builds compile the generated header directly and need no SVG library,
downloaded font, extra game assets, developer-captured atlas or private pack.

At a 256-square converted RGBA5551 upload, a cheap 512-byte sampled fingerprint
rejects unrelated textures. A match must then pass SHA-256 over every original
byte; the sampled fingerprint never determines replacement identity. Unknown or
changed textures use original graphics. A match allocates a private original
reference and builds the 1024-square image from the user's own uploaded texture,
then overlays only the ten new glyph rectangles. Shared atlas row47 and all
unrelated pixels remain original. The image stays resident across GL recreation
and is freed after the session stops. There is no per-frame file polling or
dependency on a developer-generated private file.

`TextureDigest.h` uses portable unsigned SHA-256 operations. Optimized and
ASan/UBSan fixtures cover standard known vectors, padding boundaries, alignment,
canaries, a million-byte message and 96 independent Python-hashlib comparisons.
An unsampled one-byte mutation passes the cheap prefilter but is rejected by the
full digest. New-art RLE fixtures check counts, truncation, output capacity,
exact glyph bytes and every untouched pixel. Actual original-data automatic
composition equals the previously accepted private pack byte for byte. Real
GLES readback checks every resulting RGBA pixel, context restoration and disabled
fallback, alongside the existing renderer tests. These host checks do not prove
Apple compilation, physical display quality, frame time or battery efficiency.

After vector changes, regenerate with `python scripts/generate_timing_builtin.py`
on a development host with librsvg/Cairo, review the artwork and generated diff,
then use `--check` to verify reproducibility. The generated manifest records
source/pixel/header hashes. Future cloud runs include the small portable parser,
fingerprint and compositor fixtures in the existing sequential host job; this
does not add a runner or automatically dispatch builds.

The automatic path also passes a bounded 20-second original split-screen race
with no private atlas file present before or after the run. Both native players
are controlled, one automatic upload is observed, and 480/960/480 render-height
transitions, reset, simulated background/resume and clean shutdown pass. The
copied profile is restored afterward. This remains local host integration:
physical iPad sharpness, native-display appearance, frame time, memory and energy
must be checked before final integration into main.

## Native-size review: sampling correction required

A subsequent private host harness matches the actual SDL drawable and render
buffer at 640x480, 1280x960 and 2186x1640, then returns to 640x480. It captures
only the exact game executable's PID-verified window at one-to-one pixel scale.
Both native controllers, reset and simulated resume after returning to 480 pass.
A same-binary original-font control also passes; a synthetic constant-transparent
explicit override disables the automatic recipe without changing visible pixels.
The temporary control is removed and the copied profile restored afterward.

Full-size crops show sharper timing/lap digits, but also expose an unintended
change to a round player indicator that was not redrawn: it becomes more
pixelated. A separate unchanged LAP label region is byte-identical in the two
runs. Retaining original texel values in a nearest-expanded atlas does not retain
the original texture's linear-filter footprint. Source-array identity and whole
uploaded-image identity therefore do not establish unchanged sampled output.

This candidate is not ready for an HD iPad release. Keep main unchanged and first
isolate numeric glyph sampling from original atlas sampling. Add actual sampled
non-target pixel comparisons, verify context/texture state and cleanup, and
assess added CPU/GPU work before any Apple build. Native host appearance is now
observed, but physical iPad display precision, performance and energy acceptance
remain open. Original game screenshots and comparison artifacts stay private.

## Isolated numeral sampling correction

The current local correction retains the original GPU atlas and its filtering.
Only recognized complete timing numerals use an auxiliary HD sampler, selected
through the third texture-coordinate component. Other sprites, constant-color
UV primitives and unknown or clipped regions retain their original samples.
Explicit developer atlas overrides retain their existing whole-image behavior.
The original texture snapshot also survives context recreation when overlapping
texture memory has since been overwritten by another download.

Actual shader sampling at 2186 by 1640 now gives zero changed non-target pixels
and a positive changed-numeral result in the same executable. Optimized and
sanitized project-unit checks cover line storage reuse, texture unit restoration,
suspend/resume, overlapping texture memory and context recreation. Permanent
background shutdown follows the session's actual queue-retirement order and
issues no intercepted HD texture/sampler/uniform operations without a context.
An initial fixture that tried to dispatch another graphics command after
suspension was rejected and corrected to match the actual session lifecycle.

The bounded original split-screen run passed with both native controllers,
480/960/1640/480 drawable sizes, reset, simulated suspend/resume and clean exit.
The previously affected lower round indicator and an unchanged LAP-label region
are now byte-identical to the earlier original-font reference. This comparison
uses the same engine and harness with a changed renderer; it is distinct from
the same-executable sampled-pixel fixture. Private captures remain unpublished.

Four alternating-order pairs replaying 128 recorded HUD triangles at native
render size used identical draw and texture-bind counts with zero per-frame
uploads. The HD path adds shader work: the local software renderer showed about
0.35 milliseconds more elapsed time per sampled frame. This is a partial HUD
geometry replay with fixed modulation, not a complete race benchmark or a
measurement of the iPad GPU, energy consumption or sustained frame rate.

Main is unchanged. Before any HD integration, measure actual device frame time,
memory and energy with the existing session recorder. Physical appearance,
performance, shader precision and battery acceptance are still unverified.
