# HD interface and texture work

Development branch: `codex/hd-ui`. Integration branch: `main`.

This branch shares the performance and session-recording infrastructure with main.
It is an independent workstream. Performance, battery work and session recording
continue on main; no unfinished visual changes are required there. This document
establishes the work, not a claim that replacement artwork is implemented.

## Current execution checkpoint — 6 November

Finish the complete UI/artwork batch before one grouped iPad package. The user
is unavailable; make routine decisions without approval questions. After that
package, resume performance and battery work without waiting for device feedback.

- All98 slideshow photographs source-reviewed; all15 shared footer candidates
  authored. One grouped native ASan/UBSan load passes all15 images and98 exact
  original-identity routes, retaining15 immutable image owners (180MiB decoded).
- Eight Jaguar and16 Lotus reviewed clean plates are selected, with former
  drafts preserved. Their cleaned inputs are in the new lossless photo resource.
- GT90 sill, Tombstone wording and GT1 showroom wall corrections are selected.
  Their exact per-slide layers join15 family footers in17 immutable canvases,
  98 exact routes,6,229,762compressed bytes and204MiB decoded (256MiB cap).
- Final photo input resource is240,423,232bytes, lossless versus source RGBA;
  98 source-reviewed compositions, with final catalog accepted for local
  integration. User/device/runtime acceptance remains separate and unverified.
- All36 CarInfo technical images source-reviewed; six corrected insets plus
  genuine Mustang title assembled once in the selected48-image resource.
  All41 other image records and all48 layout metadata remain exact. One native
  provider check passes all36car/tab pairs, dividers, ownership and clearing.
- All10 final resources are staged as a hash-verified set,468,289,521bytes total.
  Local/personal Apple bundling and IPA injection now include the matched
  slideshow pair, wide menu and Results companions. The grouped Release app
  builds and ordinary-input Showcase/Slideshow routes publish remastered artwork.
- Footer variants may use up to98 source slots, still under the unchanged256MiB
  decoded budget. A variant must preserve paired source identity and ownership.
- Corrected main background is now selected with four matched source pin files,
  verified private resources and preserved rollback. Actual Main publication
  passes at1280x720 and1180x820; old v5 selection drift is corrected.
- History text is HD, but its photographic collage remains original and needs
  a separate source-bound artwork pass before full visual completion.
- Broad UI routes, true aspect ratios, matching ARM64 package and physical iPad
  acceptance remain incomplete. No cloud build or public artwork upload here.

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

## Device comparison control

In the HD branch, four-finger tap shows diagnostics; tap its FPS label to open
settings. The Timing digits action switches between Original and HD immediately
without restarting, changing resolution, reloading textures or importing files.
It flushes pending geometry in its old mode, rejects background/worker calls,
and keeps cached artwork for later comparisons. The preference resets to HD
when a new game session starts. Unknown atlases still render their originals;
the control is unavailable with an explicit developer pack override.

Session JSONL samples include `hd_timing_requested` as true, false or null.
Changing modes resets FPS/engine timing windows and invalidates the recorder's
cached FPS. The summary utility separates Original/HD observations within each
Low Power Mode and excludes the first interval after each art-mode transition.
Older recordings without this additive field remain readable as unavailable.
Compare the same race section and allow a few seconds after changing modes.

The renderer switch has optimized/sanitized sampled-pixel, pending-batch,
context restoration and no-reupload coverage. Build 20 passed Apple compilation
and independent ARM64/iOS 16 Release package inspection, including 37 Apple host
checks and the actual UIKit/recorder integration. Physical testing remains open.
The explicitly named HD-branch workflow exception permits this test without
changing main.

Experimental download: [Build 20 unsigned ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39153713916-1/NFSIISEGame-development-unsigned.zip).
Runtime source: `583ee201fdf41ebca3f32d29fb19d1f2a02b74d0`.
This is a timing-digit prototype, not a complete HD interface or a performance
improvement claim. Update the existing app through SideStore with the same Apple
Account; keep the separately supplied game folder. Main's build 19 remains
separate from this experimental source.

For device comparison, use Native resolution, full graphics and the 30 FPS lock
off in North Country split-screen. Make one recording in normal power mode and
one in Low Power Mode. Within each recording compare Original and HD on similar
sections and allow a few seconds after switching. Stop and share each JSONL log;
no manual FPS/CPU transcription is needed. Changing system power settings may
interrupt recording, so set the power mode before starting each recording.

## Post-build-20 coordinate correction

Build 20's timing classifier misses digits 7-9 because the original game scales
texel coordinates by 255/256. Their lower bounds lie slightly before the integer
cell boundaries. The next source revision reverses that bias for classification
only and requires three distinct complete cell corners. Sampling coordinates
remain untouched; constant-UV, clipped and cross-cell primitives retain original
sampling. The expanded native-size GPU fixture verifies every digit 0-9 in both
triangles and Original/HD modes with no changed pixels outside the target glyph.
Optimized and sanitized project-unit checks pass. An otherwise identical old-
selector negative control passes digits 0-6 and fails at digit 7, proving this
fixture covers the regression. This correction is not included in the build 20
download above; no additional Apple build has been started for it.

## Gold speed numerals: next device candidate

Build 21's source adds ten new hand-drawn gold speedometer numerals. The existing
Original/HD control is labelled HUD digits and switches timing and speed art
together. The recorded `hd_timing_requested` field keeps its existing name for
compatibility. Other HUD textures and text remain original.

New vectors and their generated pixels contain no original game pixels. The
generator reads only the ten fixed vector filenames and verifies their hashes
and the strict SVG publication policy. A bounded decoder validates the complete
stream before writing reserved cells in the existing auxiliary texture. The
original GPU texture, shared timing gutters and game geometry remain intact.
Full glyph-corner matching respects the original per-digit trims. Relocated
gold sampling uses fragment high precision; the fallback retains original gold
pixels while keeping HD timing numerals available.

Local validation covers all ten timing and speed digits at native render size,
Original/HD switching without reuploads, context recreation, suspended cleanup,
exact whole-atlas generation, malformed-stream atomicity and memory-error checks.
The integrated branch matches the accepted private prototype's sampled pixels.
A bounded original split-screen run selected every gold digit in both views,
with both players controlled, braking, reset and simulated suspend/resume. It
ended cleanly; this was not a complete-race or physical-device acceptance test.

One sequential standard-runner device build is justified to check the actual
Apple compiler, package and shader path, then compare appearance and recorded
frame time in normal and Low Power Mode. Main remains unchanged. No device
performance, battery improvement or visual acceptance is claimed by these local
results. The inspected build 21 download below now supersedes build 20 for the
next physical comparison.

## Verified build 21 delivery

Experimental download: [Build 21 unsigned ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39161478449-1/NFSIISEGame-development-unsigned.zip).
Runtime source: `aa276622b62e06baef9df001a92b40daf684fbc9`.

One sequential standard public run passed Linux gates and 37 Apple host checks. Independent compiler-log and package inspection confirms Release ARM64 for iOS 16, both bounded ThinLTO links, actual renderer settings, all generated gold artwork embedded, and an allowlisted five-file app. Original assets, signing material, private directories, private test probes and sanitizer runtimes are excluded. No Probe, cache or Actions artifact upload was used.

Update the existing app through SideStore with the same Apple Account and keep the separately supplied game folder. Compare HUD digits Original/HD in matched North Country split-screen sections at Native resolution, full graphics and the 30 FPS lock off. Record normal power and Low Power Mode separately, stop and share the JSONL recordings. Physical appearance, stability, frame time and battery impact remain open. Main is unchanged, and other HUD artwork is still original. This documentation update requires no additional build.


## Small uppercase labels on the HD branch

The branch now adds 26 newly drawn uppercase glyphs for recognized original
race HUD textures, including the narrow I. They share the existing auxiliary
texture with timing and speed numerals. Original GPU textures remain intact;
exact content matching and complete-corner selection leave other or clipped
artwork unchanged. Runtime-owned source snapshots preserve matching letters
across overlapping guest texture RAM and context recreation. Limited fragment
precision retains the original letters and gold digits while HD timing remains
available. The diagnostics switch is labelled HUD text; its recording field
remains unchanged for existing session-analysis tools.

Optimized and sanitizer GPU checks cover all 26 letters, both display modes,
source mutations, malformed triangles, exact original pixels, texture-memory
overlap, context recovery and suspended cleanup. New-art generation matches the
independently tested raster, with bounded decoding and strict vector-source
publication rules. The original game can request a yellow HUD panel: a local
input/framebuffer trace reproduced the same color in Original and HD modes.
This color is not produced by the glyph replacement.

Build 21 does not contain these uppercase or record-time glyphs. Main remains separate, and physical
appearance, frame time and energy acceptance remain open. Other HUD elements,
including maps, logos and starting lights, remain original. This source update
does not require another cloud build by itself.


Narrow glyph bounds are verified through the original compiled shape and UI-quad
routines. A four-pixel-wide I still has an eight-pixel source height: the
horizontal and vertical corner offsets differ. Detection and sampling now use
those independent bounds; the graphics regression fixture uses the original
geometry. The same original-method check covers the ten small record numerals
and dot/colon. Original zero and O share identical converted pixels and geometry,
so the reviewed O artwork is deliberately shared with zero. The small record numerals and punctuation are now integrated on this branch;
physical acceptance remains open.


## Small record-time numerals and punctuation

Twelve visible numeric characters now use reviewed new vectors: zero through
nine, dot and colon. Original zero and O have the same converted pixels and
geometry, so zero deliberately shares O's existing cell. Only eleven additional
tiles and source identities are stored. The same 1024-pixel auxiliary texture
and sampler serve timing, speed, letters and record text. Original GPU pixels
remain intact, with owned numeric source snapshots for context recovery.

Small-cell coordinates now use arithmetic indexing of the shared layout rather
than extending a per-character shader branch chain. The GPU tests prove that
all earlier letter, timing and speed pixels remain byte-identical. This is a
source/layout improvement; no iPad speed or battery gain is inferred.

Local optimized, address/undefined-behavior and limited-precision matrices cover
38 visible glyph cases, exact original GPU textures, the zero/O alias, source
mutations and invalid triangles, live switching without uploads, both owned
caches under overlapping guest RAM/context recovery, flat panels and suspended
cleanup. Separate decoder tests verify all eleven new tiles, untouched pixels,
canaries, malformed-stream atomicity and reproducible vector generation. The
limited-precision path retains original record text and letters. Physical
appearance, stability, frame time and energy acceptance remain separate.

A bounded local original-game split-screen run also exercised both virtual
controllers, resolution changes, reset and simulated suspension/context recovery.
New record characters were observed in both views; shutdown was clean and the
copied profile was restored. This was a short driving check, not a complete race
or iPad visual, performance or energy acceptance.

## Maintained panel, arc and gear sources

The reviewed surface artwork now lives in the source pipeline alongside the
timing, speed, pause and brand artwork. Smooth vectors retain the original rising
RPM arc and tapered end; its transparent interior leaves animation to the game.
Shared panel shading removes the old opaque sprite-background mismatch. A
classic circular badge and Bebas gear lettering retain the original placements,
with a common optical baseline. Reverse, neutral and gears 1 through 6 were
observed for both players in a bounded local manual-transmission run.

Regeneration reads authored vectors and pinned fonts, with no original game
pixel input. It reuses the existing auxiliary atlas without additional textures
or uploads on gear changes. The maintained renderer reproduces all 83 scoped
GPU outputs of the tested candidate in three configurations. The SVG publication
exception remains tied to exact reviewed digests and a bounded XML schema.

Remaining interface fonts, full layout review and physical
iPad acceptance are unfinished. Keep these visual changes on this branch while
main continues performance work; merge them only after the remaster is finalized.

The small gold labels and record numerals now regenerate from the pinned,
unmodified Oxanium font, with its license included in application and release
resources. The source ledger covers 78 SVGs. A wide W uses a spare strip in the
same auxiliary atlas; the narrow 1 gets its natural cell width. Glyph placement
uses copied vertices and an active full-shader capability check, preserving
original guest attributes and the basic-shader fallback.

The record plaque keeps the classic embossed frame and uses a more readable
body material. Its caption has tighter tracking; the time retains the original
whole-string advance measurement with corrected centering and a common font
baseline. Exact original POS/LAP cells have tighter spacing around their original
centers. These changes add no GPU textures or draw calls. Remaining menus/icons
and complete single-player/split-screen visual acceptance
still need work; this is not final artwork or physical-device acceptance.

Clock punctuation and position/lap slashes now regenerate from two further
reviewed vectors, bringing the pinned SVG ledger to 78 sources. The clock uses
the numeral font and silver finish, with optical alignment to the actual numeral
row. The slash follows the classic angle with a thinner outline and smooth
coverage. Both use disjoint spare regions of the existing auxiliary atlas.

Exact original UV/geometry/color gates select these replacements. Guest inputs
stay immutable, unsupported cases retain original rendering, and the basic
shader path remains unchanged. Fresh source generation reproduces 25 tested
pixel/identity arrays and all 85 GPU regression images in three configurations.
The equivalent private candidate completed a bounded original split-screen run;
both clocks and all four separators were observed. This is local evidence,
not physical iPad appearance, frame time, battery or full-race acceptance.

The countdown lights now use a newly authored sphere with smooth edges and a
restrained highlight. The three original circles, equal spacing, game colors and
countdown timing are preserved. Two overlapping banks fit the existing auxiliary
atlas; this adds no GPU textures or draw calls. The reviewed ledger has79 SVGs.

Source generation reproduces26 tested pixel/identity arrays and adds a digest of
the authored sphere raster. A fresh maintained renderer reproduces all91 local
GPU reference images. The equivalent prototype also passed sanitizer and basic
shader matrices; the basic path remains original. Permanent decoder/selector
checks cover atlas boundaries, shared seam rows, all24 triangle orientations,
unchanged input and unsupported UV/depth cases.

The equivalent private candidate completed bounded original single-player and
split-screen observations with native controls, all countdown colors, clean
shutdown and restored copied settings. Actual countdown placement was reviewed.
These runs do not establish full-race completion or physical iPad acceptance.
Remaining menus, map artwork and complete layout/device/energy review stay open.

Frontend font discovery now has a bounded metadata inspector:
`python scripts/inspect_frontend_font.py /path/to/an/external/font.ffn`.
It reports glyph advances, bearings, line metrics and bitmap bounds without
exporting bitmap pixels. Five synthetic tests cover metrics identity, malformed
structures, payload bounds and the original numeric-font length discrepancy.

The original compiled font initialization, lookup and layout were exercised with
seven external fonts, 1,568 lookups and 45 strings. Actual frontend observation
identified a sprite font page and mapped all 128 captured text triangles to
complete glyph cells. Software prepares the font page; accelerated draws use
size-specific sampling offsets. An optional replacement must preserve those
contracts and the original fallback. The optional main-menu lettering uses the
exact-source and placement guards described below.

The main-menu remaster retains tall, consistently condensed lettering and
complete-word spacing. A maintained default-off capability bridge now isolates
renderer availability from engine linkage. The prepared original hit routine
extends only the left edges of the exact eight-row English main menu, then
restores them before returning or unwinding. Unsupported forms retain their
original targets, and the disabled hook makes no extra guest table reads.
The original compiled routine passes 1,008 strict-edge, cursor, register, stack
and restoration cases through this prepared hook. Optimized/sanitized helper
checks and C linkage also pass without SDL or a renderer.

The reviewed condensed word atlas is now integrated with renderer lifecycle
publication. All eight new vectors use the pinned project fonts, and every
visible ink pixel fits its matching interactive target. The separate 1024-square
atlas uploads once per owned context; switching between it and the HUD adds no
triangles or per-draw uploads. Unsupported sources, placements and basic shaders
retain original lettering and click targets.

Optimized, sanitized and basic-shader graphics matrices preserve all 91 earlier
HUD outputs per variant and reproduce six prototype menu images per variant.
The maintained renderer and prepared engine hook also pass an actual original
frontend run at 640x480 and 2186x1640: the newly widened Location edge selects and
opens its form, Escape returns and native controller Down selects Player One.
Shutdown is clean and copied settings are restored. Local images were reviewed.
This remains local frontend evidence, not full-race or physical iPad acceptance.

Next remaster the other frontend forms and fonts, then review remaining artwork
and complete UI spacing. The main performance branch stays separate; no new
cloud build or IPA was used for this source checkpoint.

Changing labels require complete-string identity as well as glyph coordinates:
otherwise a different track name can accidentally match only some replacement
letters. The prepared text entry now hashes bounded printable ASCII input, and
the existing owned triangle command snapshots that identity beside its vertices.
Renderer-thread scope restoration protects both normal and exceptional exits.
No text pointer is retained or text content logged. Unsupported strings use zero.
This adds no renderer command, barrier or GPU operation.

The private Location consumer prototype verifies recognized text, unknown-text
fallback and the second-row colon. An actual run changes North Country to Pacific
Spirit (original font throughout), then back to remastered North Country, with
working Done/Escape/controller navigation at both reviewed resolutions. Its
settings are restored. This is not physical iPad or performance acceptance.
That initial single-value prototype has now been expanded and integrated below;
the maintained main-menu allocations remain unchanged.

The existing Linux/Apple host workflow target lists now include all registered
UI tests, including the generated deferred bridge witness. A small local policy
test checks build/run coverage to catch omissions before Apple compilation.
No new cloud jobs or builds were started.

The Location form now has 18 reviewed authored words: static labels/buttons,
seven track names and lap counts 2/4/8 observed in the copied Single Race profile.
This does not establish every unlock or game mode. Unrecognized complete strings
retain the original font, rather than partially matching a known track name.
The dense form uses condensed Oxanium with Bebas buttons, preserving original
row baselines, one-pixel shadow offsets and horizontal text extents. Done's
interactive behavior was checked; text bounds alone do not prove every hit box.

The generator packs cropped new ink into unused space in the same 1024-square
atlas. It preserves all eight main-menu allocations and reproduces the tested
atlas exactly. Seven bounded RLE literal parts retain the existing 1 MiB public
file limit; no original pixels or private captures enter the source repository.
Per-word geometry uses the existing queued complete-text identity, including
independent identities for labels whose ellipsis is a separate original draw.

Local evidence covers both reviewed resolutions, all observed track/lap cycles,
Done/Escape/controller navigation, clean shutdown and copied-settings restoration.
Three GPU configurations preserve 103 prior images each and exercise 636 paired
original triangles/full-label contexts, plus missing/wrong-context fallback.
Other forms, remaining artwork, complete layout review and physical iPad
performance/energy acceptance remain open. Main performance work stays separate.

Setup is now integrated with 22 authored labels/choices and 346 guarded glyph
placements across its six option rows. Nineteen new vectors reuse the same dense
Oxanium/Bebas roles; colon, Done and Cancel explicitly share Location sources.
Observed game/race/style and toggle cycles include the original automatic
catch-up change when selecting split screen. Rendering a legacy modem/serial/
network label does not establish functioning connection modes.

Measured artwork did not fit the old square atlas in the bounded packing trials.
To preserve font size and proportions, the single cached menu atlas is now
1024x2048 RGBA (8 MiB, 4 MiB more than the earlier Location checkpoint). Its
accepted main/Location top half remains byte-exact. Generated dimensions drive
decoder bounds, allocation, GL capability checks and upload dimensions; the
vertical UV factor changes to one half. Eleven bounded literal parts retain the
unchanged 1 MiB publication gate. There are no new per-draw uploads or texture
objects, and unsupported source/capability/shader paths retain the original font.
This is not evidence of unchanged physical iPad battery use or performance.

The private tested candidate passes 2,904 sanitized triangle cases with 28
ambiguous placements resolved by complete draw-string identity. Actual original
game runs cover every observed Setup choice, Done and Escape at both resolutions
with clean shutdown and copied-settings restoration. Native coverage requires
each recognition screenshot to be 2186x1640, not just a density request. Three
GPU variants preserve 109 prior images each and replay 692 Setup/context pairs.
Maintained generation reproduces that candidate's artwork, geometry and runtime.
Remaining forms/artwork and physical iPad acceptance remain open.

Player One and Player Two typography now has a maintained source generator:
34 authored words, including 12 observed car choices, the legacy accented
Italdesign Cala name, transmission, graph labels and the two screen headings.
Twenty-nine new vectors explicitly reuse five Location/Setup word sources.
The original backgrounds and car artwork remain separate, unfinished work.

The same single 8 MiB menu texture now holds 82 logical words / 74 unique images.
Bounded deterministic packing relocates earlier words without changing their
source pixels, font proportions or original compatibility geometry. Nineteen
literal include parts remain below the unchanged 1 MiB publication limit.
Two independently owned original font pages share that texture. The accented
whole word retains the original font until both pages have been observed and
a complete frame has committed; unknown strings/pages keep the original font.
Queued renderer commands still own the complete text identity.

Maintained generation matches the exercised prototype's pixels and geometry;
the prepared C renderer matches after newline normalization and compiles with the actual optimized
project settings. Optimized and ASan/UBSan tests cover 5,068 glyph triangles,
138 ambiguous placements, immutable inputs, bounded text identity and fallback.
Equivalent-prototype original-game runs cover both players at 640x480 and
2186x1640, all 12 observed cars, transmission/graph cycles, Done/reopen/Escape,
clean shutdown and copied-profile restoration. This is not a newly executed
maintained-binary game run or physical-device acceptance.

Three equivalent-prototype graphics variants replay 1,082 observed triangles.
HUD/original/fallback images remain exact. Atlas relocation changes interpolation
rounding in 12 HD text images: at most 2/255 per RGB channel and 363 channel
values per image, with exact framebuffer alpha. The strict exact/one-level
failed attempts and immutable references are retained privately; the accepted
bounded review is explicit, not a blanket pixel-exact claim.

Remaining forms, map/artwork, full alignment review and physical iPad visual,
performance and energy checks remain open. No UI IPA has been produced here.

Opponents now has a maintained authored-source generator for 30 words: the
observed car/class choices, grid sizes, skill and traffic rows, buttons and
opponent list. Nine new vectors explicitly share 21 existing word sources.
Disabled/selected tints, original positions and complete-string fallback stay
with the original draw path. Coverage is for the observed English profile,
not every possible unlock, language or game mode.

A fixed, reviewed 83-image packing priority fits all 112 logical words in the
same single 8 MiB menu texture. Its two-pixel transparent right/bottom gutters
are tested with the existing linear sampler. All 74 earlier authored source
rectangles and 1,267 earlier glyph geometries, ink metrics and identities are
preserved exactly; only atlas UV addresses move. Cropping main-menu quads was
rejected after framebuffer comparisons, so it is absent from this integration.
There are no random packing trials during generation. Twenty-one bounded
literal include parts retain the unchanged 1 MiB public-file gate.

Maintained source reproduces the exercised prototype's atlas and all 1,933
glyph placements, and its prepared C renderer matches after newline
normalization and compiles with the actual optimized project settings.
Optimized and ASan/UBSan checks cover 7,732 triangles and 294 ambiguous positions.
Twelve asset-policy cases include 23 malformed catalog mutations. The equivalent
prototype completed observed car/grid/skill/traffic cycles and Done/reopen/Escape
at per-capture-verified 640x480 and 2186x1640, with clean shutdown and restored
copied settings. This is not a newly executed maintained-binary game test.

Three equivalent-prototype graphics configurations replay 1,332 observed
Opponents triangles and 1,082 Player triangles. Earlier 115-image limits remain:
original/HUD images exact; relocated HD text at most 2/255 per RGB channel and
500 changed channel values, with exact framebuffer alpha. The six newly added
dense Player references use a separately reviewed 0.01 percent RGB-channel
occupancy cap, still at most 2/255 and exact alpha. The uniform 500-channel
attempt failed at 763 changed values in that larger fixture and is retained
privately. Normal/sanitized runs have 106/121 byte-exact images; forced basic
has 121/121 exact. This is explicitly scoped sampling review, not blanket
pixel identity or proof of iPad battery/performance.

Remaining Options/submenus, map/artwork, full alignment review and physical
visual, performance and energy acceptance remain open. Main stays separate;
no UI IPA was built for this checkpoint.


## Required artwork scope and acceptance - 21 October

Modernize the original artwork rather than treating scaler output as a finished remaster. Preserve recognizable silhouettes, layout, padding, material character, source animation, opacity and control behavior. The full checklist includes:

- Cursor: faithful arrow, hotspot, scale, clipping, transparency and original fallback.
- Small NFS logo: preserve the actual logo lettering and its animated colored inset layers.
- Large translucent background wordmark: preserve the source proportions, blur variants, movement and opacity.
- Background: world map/coastlines, colored regions, gold panel, grid and decorative geometry.
- Volume bars: authored tracks, fills, caps and buttons, preserving exact value mapping, selected/held states, hit bounds and original blend behavior.
- Remaining HUD and all menu fonts: retain the classic gauge/italic yellow styling, condensed menu lettering and correct logo artwork; finish alignment and padding.

Private local cursor candidates have passed source/queue/GLES fixtures and actual menu navigation at logical and larger host resolutions. These are host integration observations, not Apple package or physical-device acceptance. The background and volume assets remain private review candidates pending actual scoped integration. Source-derived material and extracted references are not approved for publication by this checklist.

Host Audio-control checkpoint: the actual original game produced eight sampled states with changed volume and both held directions. Independent original-bar replay matches all307200 pixels per state exactly. This validates original transparency/order for those states, not authored-bar activation. Endpoint values, other selected rows, a source-proven fresh background and the same-pass text join remain gates. Original PC addresses differ from packed guest addresses; form identity must come from the compiled mapped layout. Source-selected item100 is the original no-hit sentinel, not a valid indexed item.


Actual background/compositor checkpoint (private host, 21 October): run18 supplies five source-proven exact reset-plus-overlay records and exact same-pass original bar joins. The independent C++ verifier is tested with an explicitly analysis-only lifetime; production needs its own current surface lifetime. The authored atlas getter, new additive renderer bridge and immutable Audio packet compile and pass their separate tests. Continuous per-redraw producer, combined game activation, complete value/row coverage and physical-device visual/performance/energy acceptance remain open. Cursor/logo/background/volume bars remain explicitly in scope, including the original colored/blurred animation variants.


Actual authored Audio host integration (21 October): local prototype14 renders remastered bars with HD text and cursor in the running original game at640x480 and2186x1640. Current-pass source/background/bar/text/lifetime proof continues beyond diagnostic caps with binary exporters disabled. Both directions on all3volume rows are confirmed by normalized-layout values logged only after successful GPU presentation. Both runs shut down cleanly and restore copied settings. Host Audio remains slow, also seen before authored activation; no same-workload performance/battery conclusion or iPad acceptance follows. Full-frame comparison and maintained integration remain open.

All12 vertical wordmark candidates now have faithful canvases/hotspots/blur families and subtlealpha, with V3 pale ivory materials after visual contrast review. FOR/SPEED contour differences remain; no animated background candidate is yet runtime activated. Proper small logo with all four source-driven inset colors is the next ordered-group integration, requiring original clean underlay/replay and both indexed-full/crossfade callbacks. No source-derived references are approved for publication by this checkpoint.


Small-logo observation checkpoint (21 October): eight actual animated original groups and eight final copies replay exactly on local host; original source and copied settings preserved. Prepared five-role authored atlas getter remains unlinked. Continuous owned provider and GPU composition still open; direct status groups are multiplayer readiness and stay on original fallback. ObserverV1 unsampled-shutdown failure is preserved, V2 scope fix narrowly validated. Brighter blk6 reuses geometry with separate material treatment; no scaler-only remaster acceptance. No maintained activation, public/cloud/IPA or physical-energy claim.


21 October - Native logo preparation and original-path checkpoint25: pure C++ verifier, continuous owned collector and additive GPU core pass standalone optimized/sanitized tests; all remain private, unlinked components. Original local run25 supplies6exact group/copy replays and one preliminarily complete300upload/300quad/order6 native chain; a stale prior attempt declines. Final main-menu harness assertion fails because screenshot shows original attract movie after90seconds idle, while game exits cleanly and copied config restores. Native GL witness accepts0records, errorMask68; exact query/readback cause is not yet captured, so actual GL-state/transparency acceptance remains open. No authored logo activation, maintained source activation, public/cloud/IPA or physical energy claim. Cursor, both background logo families, full background/map/panel/grid, volume bars and HUD/fonts remain required faithful redrawing scope. Component receipts and failures indexed in evidence/hd-ui-live-software-compositor-foundation.json.


21 October - Resumed local native logo diagnostic work (runs26-27): two fresh local links and two bounded original-game runs completed, with clean game shutdown, copied config restoration and no owned game processes left. Run26 identifies the diagnostic bug exactly: querying OBJECT_NAME on an absent stencil attachment is invalid in GLES2. V3 queries OBJECT_TYPE first; focused actual ES2 fixtures reproduce the old failure and pass the corrected path. Run27 independently strict-decodes three complete before-base/after-base/after-text GL stages for request43/context1, with original native text visible. Original3group/copy replays exact. Whole harness remains FAIL because acknowledged one-pixel keepalive did not prevent attract footage and a later original UI-frame barrier timed out; do not conflate scoped GL success with a full idle or authored composition pass. Original run25/run26 negatives retained. Native owned compositor activation, maintained integration and physical device visual/performance/energy acceptance remain open. Parallel faithful V4 background wordmark candidate corrects FOR/SPEED R/O/P/S contours, preserves NEED/material/blur/opacity/placement, parent-viewed and accepted only for further integration. No new IPA, cloud build or publication.


21 October - Actual authored native logo activation (private runs28-30): V1 renderer declines safely due an invented check on unused/uninitialized TMU oow. V2 validates consumed vertex fields and publishes38 authored replacements in run29, all300 original tile selections and600 triangles each, with HD labels/cursor visible. Independent real ES2 component tests verify exact original replay and outside-paint/depth preservation across black/nonblack and logical/native targets. CPU2 retains upload binding after successful publication, passes optimized/sanitized cached lifetime fixtures and three coherent optimized compiles. Actual LOCAL22/run30 remains a partial result:39 authored publications against265 CPU attempts; no GPU refusal, clean shutdown/profile restore/private display cleanup, but cached-frame continuity still unresolved and under exact source lifetime audit. No full UI, maintained activation, iPad visual/performance/energy, cloud or public publication claim. Background/map/panel/grid/wordmarks, volume and HUD/fonts remain required faithful redraw scope.


21 October - Cached logo lifetime correction (LOCAL23/run31): CPU3 moves original-iteration invalidation to40BC20 before any art mutation, preserving original timer-only returns. Independent source review, optimized/sanitized lexical no-redraw fixtures and3coherent project compiles pass. Actual run31 has245attempts:135initial no-proof then all110 CPU begins queued. Renderer2 publishes38, including1cached-content repeat, but72begins refuse target before staging; finish counters300/600 on those refusals are stale prior-pass counters and FBO0/size0 are trace placeholders. Both traces stay below cap. Game exits0 in23.433s, profile restored and owned display/processes closed. Native target validation currently shares a software-frame helper that requires unrelated prior texture binding; original presentation unbinds texture0. Renderer3 will prove native FBO/dimensions independently and verify sampling from actual300base textures, with precise actual diagnostics and exact stage-cache reuse. No long workload or whole coverage claim yet. Faithful background plan now pins2mapvariants, panel and12wordmarks; original panel/word/tail observer is the next parallel integration step. No cloud/public/device claim.


21 October - Native cached authored logo acceptance (LOCAL24, runs33/37): renderer3 separates actual native FBO/dimension/projection/attachment proof from the unrelated preselected texture that original presentation unbinds. Existing all300 native texture/source and600triangle guards remain. Actual640run33 publishes all111queued requests,0refusals,38ownedimage builds+73exactcachehits;72accepted requests have texture0/oldhelpergatefalse, confirming cause. Actual2186x1640run37 also exits cleanly with every eligible request published and25cachehits; nativecapture parent-viewed sharp logo/text/cursor. Original36nativepreflight failure retained: wrong640-onlyrunner refused BEFOREgame; fresh37 uses nativegeometry-only sibling with current strict PID/starttime proof. Both successful runs restore canonical copied config and close owned processes/display; no iPad/energy claim. Long34originalRESET-confirmed180s workload is active, traces bounded/capped. Whole background original-only panel/word/tail observer compiling in parallel; UI remains unfinished, main/public/Apple build untouched.


21 October - Sustained authored logo and original background step: long34 preserves a harnessFAIL at private pointer65 because linked test driver permits<=64; game clean166.911s,993GPUcommits0refusals, all4colorpairs/all80coefficients observed. Fresh38 uses5s keepalive cadence and explicit64budget without changing the game. Full180.328s main observation PASS;840GPUcommits0refusals/326cachedimages, all4pairs/all80coefficients observed,34originalRESET proofs/39totalpointerrequests. Game exits0 in197.348s, copied config restored, finalmain independently viewed, owned display/processes closed. CPUtrace capped2048; GPUtrace1688uncapped; no all-window CPU coverage or benchmark/battery claim (LOCAL25 link overlapped). Native37independently splits2initial640 and71native2186commits,25nativehits/no refusals. LOCAL25 original-only background observer run35 exitsclean30.616s,103logo commits/norefusal, exports4boundedscenes. Strict background capture FAIL preserved: V1 requires a tail that main form legitimately skips, and misses default4A14D4 side bytecopy. Independent source replay reports actual4side images +16word draws exact; V2 recorder now adds explicit original branch-skip and proper byte-count callback witnesses. Whole map/panel/wordmark remaster is not yet activated; UI/main/public/Apple/device acceptance remains open.


21 October - Whole-background integration checkpoint (LOCAL26/27, runs39/40): both bounded original-game runs exit0, restore copied canonical profile and close owned game/display. Native authored logo publishes all97 and105queued requests respectively,0GPUrefusals. BGv2 fixes actual main-form tail skip but strict capture stillFAIL: all4side draws lack callback rows; original content replay is not promoted to complete route proof. Frozen normal background-art getter packages2maps/panel/12words as5immutable PM textures, no perframe IO/allocation. New full-base observer actual40 captures normalblk6 source and original307200pixelcallback; all640x480 POST and following343copy PRE equal original source exactly, frozen strictparserPASS; stronger independent form/owner/callback review pending. Oldlogo exporter40 reports error16 and remains unaccepted; clean39 original-logo/content chain separately reviewed. Whole authored background not activated, maintained HD branch/main/public/cloud untouched. No new iPad or battery claim.


21 October - Whole-background CPU/GPU integration underway: independent actual40 review now accepts fixedmappedmain form, original483438callback, fulltargetowner/LUTs and all307200 map/POST/copyPRE words; separate indexedsideowner binary rederivation limitation remains explicit. Shared sceneABI V1 frozen; pure current-scene verifier and authored wholebackground GPUcore being implemented in parallel. Parent isolated renderer bridge reuses exact300nativeupload/source and600triangle guards, passes optimized/sanitized mockadapter16modes (existinglogo regression + newbackground) and fullrenderer C compile with54baselinewarnings. This is preliminary, not frozen/GPU/game/device acceptance; newfeature temporarily exclusive pending final arbitration/otherforms. Originaldrawing code preserved apart from single additive include; existing activegame/main/public/cloud unchanged.


21 October - Whole-background renderer component and direct palette route: actual41 proves side uses4A6AA0 indexedbyte-to565 lookup (164640PIXELS), not presumed483438/4A14D4; original4side/16word/logo/copy pixels replayexact,90nativeauthoredpub/norefusal,gameclean34.347s/profile/displayrestored. FrozenGPUcore passes optimized/sanitized6cases208checks each, state/depth/cache/failure checks, native component image reviewed. Parentrenderer coherent149input Ccompile0/54baselinewarnings,16adaptermodes+independentsourcereview; receiptc549e0d9450bbdcc1c7a62dd00c66d028b1cccbec4c898d12d25730435a3fc7c. Actual42 clean28.729s/106nativepub0refusal, indexedmetadata shows4correctcapturedrows, but fullscene exportFAIL: legacy serializer permits329280flatbytes and rejects164640indexedbytes. V2exportnamecollision avoided beforelaunch usingV3distinctflags; V4new scopedserializer now adds complete combinedexport fixture without alteringsharedinlineBG2. PureV1 synthetic common-event verifier frozen; actualroute/base-lifetimeV2 +continuousliveprovider integration under development. No new IPA/cloud/public/device/battery claim.


21 October - Original background capture43 ACCEPTED (LOCAL30): V4new scopedsceneexport emits15,878,241B SHA7039fe14617c2079895a6ea8f879de6b545bf4cb0a25f3ae14bbb13d3a946726; oldBG_FILEunset leaves legacyinline definitionsunchanged. Frozenindependentreader matches all4complete scenes:4actual4A6AA0 indexed164640bulkdraws,256u16paletteBEFORE/AFTER each,16worddraws/fullside boundaries andmainSkipno-op exact. Independentreceipt43bdab6d4ea618823ab662639ed536b64522e2cc4789d76d7edd266a9a9d00cb; oldfailures35/39/41/42 preserved. Gameclean39.203s/copiedprofile/owneddisplayrestored; native105begins104publications0refusal, sole105explicitlycanceledwithoutpublicationduringterminalcleanup (cancelstop0 is not actualSTOPflagproof). Fullbase/oldlogoexports also clean, but independentrecorder contentmatches do not establish uniquewhole-scene event/lifetime join. Continuouslivehooks nowfocusedopt/san52checks each, coherentsharedheader/01/03 ready and10/13 compiling; parentfrozenGPUrenderer ready fornextcombinedcandidate. No cloud/public/IPA/device/energy orfullUIcompletion claim.


22 October - First combined whole background run44 (LOCAL31): coherent5replacements+1art,73.o+1SDLarchive/74uniqueinputs, binary3fd854889a1e7d99a2cbadb55ee5f00cb87192c26d7009eb2148dd6d52dab999. Exporterfreebothfeatureflags1 gameexits0in31.507s/copiedprofile/owneddisplayrestored; BUT0BGCPU/0BGGPUtraces andoriginalbackgroundretained, so wholeactivationFAIL preserved. Independentc02cfecf68e4b8b5703e365680f8fae3031175bbff44b129ad83fddb71b50f45 verifiesall2383pins/flags/objects; no linkmix. Firstactualsilentfailureunobserved. Source-shapedfixturefinds prebase40BBE0warmup postGroup/tail/copy poisoningV1state beforeloc40D63E; V2adds armedgates andboundedfirstfailuretrace, coherent4TU rebuild32/run45next. Right-side marksaudit identifies separate% s5/128texture path40B6C4->40DC40/4276F0 afterbackground/text, ind5FordIndigo; otherexactliveassociationspending. ExistingAudio6currentwidgetshapescovered. No public/cloud/IPA ornewdevice/energyclaim.

22 October - Corrected whole-background actual45 (LOCAL32):
V2 provider and row hooks compiled coherently; exact 74 link inputs. Binary SHA256 020c756cc3527be74bcb3e6c230333964e917e82a32056173dbbbfb48c7bc8a0. Actual local640 game records103 BG NATIVE begin-accepted and103 finish-published; clean24.954-second shutdown, copied profile restored and owned display closed. Parent viewed original game screenshot: authored map contours, beveled gold panel and ghost words coexist with native labels/cursor/right marks. Independent receipt pending. Historical44 negative preserved. Native-density run and physical iPad acceptance remain next; no public/cloud/device/battery/full-UI completion claim.

22 October - Whole-background native actual46 and independent45 review:
Actual45 independently accepted by GPU review7be6ddee61f8f25689f2dfd98c6d6ddbe4e8a5c4252f38da201ded31969f9649 and provider review64f6cc1ec1ea6e158134aed9dd494e8a97e857ac59075eac01405fbbb2e11e3d. Exact103 paired publications,0 recorded refusals; detailed CPU trace256 cap excludes complete late-state claim. Actual46 sameLOCAL32 binary:127 matching begins/publications,14initial640 and113native2186x1640. Clean39.387s shutdown, canonical copied profile restored/owneddisplay closed. Parent inspected exactnativePNG: authored map/panel/ghostwords/logo and original labels/cursor/rightmarks visible. Native dimensions are host acceptance, not iPad FPS/energy proof. Longer actual47 animation test now active; independent46 receipt pending.

22 October - Sustained authored background47 and maintained integration foundation:
Independent47 review b404cb196852a284502aab2727a7b5c82280d3aabadbd8e341459c69cc17ead7:908matching GPU publications/0refusal over180.364s observed main;599rebuild309reuse. Clean193.128s terminal/config/display restored. CPU regular2048/detail256 caps limit late CPU coverage and role/scalar endpoints are not in BG GPU trace; no all-fades or device/performance claim.41pointercommands<64budget;35verified idle entries/36attempts include one missingreset then successful bounded retry.
MaintainedHD source now has additive owned renderer acceptance API plus12meaningful queue/STOP/lifetime cases and legacy tests:26optimized/sanitized executionsPASS,51pins parentverified; existing bridge lines unchanged. Receipt f2f1ec7d9593aba5052480d6d0889e6bd17e3a72f95007524fc8d66c5c71bf04. Uncommitted/no runtime callers yet. Further shared ABI/pure verifier extraction must use a caller-supplied trusted synthetic-test catalog, keeping actual original-asset hashes/data private.
Actual48 native main-options-audio-options-graphics-options-main test clean80.264s/config/display restored; parent viewed audio/graphics and authored main restored. Other menu backgrounds remain original; independent receipt pending. Right-side original identities corrected by private d44c1bd1f476e1461e2174931849f15b08abd700f00903d26003f8fc9c7bea24 audit; SIN5+RAC5 top, NOR5crest, IND5,A5. Private vector candidate not accepted and no runtime integration. No public/cloud/iPad/energy/fullUI completion.

22 October - Independent native menu lifecycle48 accepted:
Receipt a612e85d133326c7bfd35c96f736bc31c840857f793e03a149c1c7c9cf8137f4:43matchingCPU/GPU publications/0refusal,19logical before exit and24native after freshowner2 base;19rebuild5reuse onreturn, firstreturn rebuild. No backgroundscene publication inside Audio/Graphics; parent viewed original submenu backgrounds alongside HDtext/audio bars/cursor and authorednative main restored. DetailedCPU256cap retained; normalOwnerexit stale reason field is not firstFailure. All2429pins/coherentbinary verified by independentreview. Parent game/link terminal; next maintained source neutral/verifier extraction +private original-shape symbol refinement active. No new build needed for unchanged binary; device/energy/fullUI gates remain open.

22 October - Maintained HD portable foundation code checkpoint219344a:
Local codex/hd-ui commit219344ac5acfb4ab683e092df3b95a5e28a301ea contains exact11source/test/buildfiles, no artwork/privatecatalog/diagnosticexporters. Additive ownedrenderer API and sharedABI/ownedrecords/pureverifiers now maintained; sourcecatalog is explicit trusted immutable caller input (13scene+5logo+2map), boundsvalidation is not provenance. Frozenalgorithms restored by reverseaudit aftercatalogparameterization; originalproof/table/palette/pixel/order guards retained. Portable receiptf7fcf797918d1f69e294f305546fc0b4abb3007fbcaa92df8f8ac4545bca1d31/32pins and independentreview98fd97fc00e232da9e038d222cc52961a7372995a27096daf4961e01e509d576. Ownedqueue independentreview12087a74c3c028bb99c26f26611626901eba89c7cac8dd2864453afa85585251 documents submitting-worker destruction for unaccepted/postretired discard; no ownerdestructor blanket guarantee.
Direct optimized/sanitized synthetic full307200pixel opaqueoracle+15negativecases and strictCABI pass. Parent additionally configured actualmaintainedCMake, built onlythree focusedtargets, CTest14/14PASS receipt9cef84c5ec4c60997573df4478ce6b1b18a764729e8f025dceb58b53c640c00b. Two ignored read-only aliases reuse existing generated prerequisites; no engine/game build or originals changed. Staged policy+independent pinnedsecret scanner and commit hookPASS; no push/cloud. HD_UI_PLAN edits remain unstaged. Remaining production integration: continuous liveguestcollector/hooks +renderer/core +optional trustedprivateart target; no active new runtimefeature from foundationcommit alone.
Private symbolartV2 receipt d9459940a5ee93075f44d8310ead99c5c77948daa693f019156f37649b8c881b retained after parentvisualreview: authoredSVG, source-transparentholes/opaque rims confirmed; S lowerstroke/window closer, uppercurve/bevel need actualplacementreview. Candidate inactive/unacceptedfinal. Main actualnative screenshot exists run46; originalsubmenu backgrounds/right-side textures still incomplete. FulliPad/energy/UI completion unproven, goal remains active.


22 October - Maintained full HD-source integration and first actual game acceptance:
Continuous guest hooks, immutable owned queue, native renderer/core and separately loaded trusted artwork pack now compile in the maintained HD branch. Optional pack defaults off without a trusted build pin; missing/damaged/mismatched artwork preserves original rendering. Private artwork and original asset catalogue stay outside public source. Source/component optimized/sanitized tests plus independent reviews passed; combined maintained CMake focused checks36/36. Fresh complete Linux game build uses checked-copy bundle matching the existing workflow, two build workers and two ThinLTO workers; binary SHA150c5d3144a4cc127505c96b070ae5533b800f22a281cd010abfad8c018214d2. Independent review verifies453 source/generated pins and maintained link membership, no prototype replacements. First actual game run uses normal X11 input, separate copied assets and owned display:46matching native publications/0refusal at640, each300selections/600triangles; two stable menu observations, parent viewed screenshot, WM_DELETE clean0, copied config restored and display closed. Main artwork accepted at640 only; native maintained run next. No cloud/push/Apple/device/energy/fullUI claim. Historical unsuccessful configure/target attempts retained. Goal active.


22 October - Local integration checkpoint7e46f39 and retained native-desktop negative:
Exact42 audited code/test files committed locally as7e46f39369b83920d15e162fee1924610bb3e464 oncodex/hd-ui; staged publication policy, independent secret scan and commit hookPASS. No artwork/catalog data/private output committed, no push/cloud. Source bytepins unchanged. Second maintained app test enlarged its actual owned window to2186x1640 but renderer continued640x480; native-sized publication requirement correctlyFAILed. Game exited cleanly with WM_DELETE, copiedconfig restored and owneddisplay closed. Source diagnosis: SDLWindow dimensions seeded onlyatcreation, no pixel-size event refresh, plus desktopfixed framebuffer default640; iOS resolution path is separate. This is a hostpreview integration limitation, not newphysicaldevice evidence. Preserve failedrun; native desktop support and otherUI artwork remain open.


22 October - User correction to country-map artwork:
Prior functional renderer acceptance does not establish final visual approval. The background is a stack of overlapping country cutouts with soft cast shadows on lower layers and intentional soft-focus atmosphere. Corrected user-specified TOP-to-BOTTOM order is RED -> GREEN -> YELLOW -> BLUE. Flat collage interpretation and heavy outlined/mottled material-only revision are set aside. Existing artwork/pack/reference evidence retained unchanged; new separate layered candidate being authored privately, not activated.


22 October - Maintained desktop native-size fix and original-game acceptance:
Local checkpoint91797c988f8bd4690d12f2386ff5d0ccd79d25bf contains exact5source/test files; policy/secret scanner/commit hookPASS, no push/cloud. Main-owner actualpixel refresh handles matching live SDL windows only, leaves invalid/foreign/stale/same-size requests unchanged; desktop uses dynamic/resizable mode, iOS fixed/native selection retained. Real SDL/GLES optimized+ASan/UBSan focused tests verify fixed/native FBOdepth and five independent RGB samples across swap, with ownership checks; no forcedqueryfailure/highDPI/iOS execution claimed. Coherent incremental fullapp build73.172s/453pinnedinputs; binary065e2b5776f1d9505b1edd04076afd766bcc7f5cf2dfc0e6861b0ea58d4a3c94. Actual maintained game run03:85matchingGPU publications,38initial640 and47native2186x1640;300selections/600triangles each,0refusal/unpaired. Two actualnativeBMPs and parent/independent visualreview, cleanWMdelete0, canonicalcopiedconfig restored/owneddisplay closed. Independent runtime7600bf1bb293a1fbd0e27109af350d08cb02347f2113822fd8d65aee015bb6f6. This proves host technicalnative rendering, not map art approval or iPad/battery performance.
User prefers rich authored mapmaterial over sterileclean revision. Firststackcandidate corrects red>green>yellow>blue/castshadow ordering but omitted fullreflectionpools/changedfilterbounds; inactive. New privatecandidate must preserve complete currentvector material, prove unshadowed/unblurred interior equality, then add correctedstackshadows/softfocus. Currentpack and acceptedreferences unchanged.


22 October - Rich-material country stack V2 private preview:
Full original-authored vector paint subtree incl reflection pools/ellipses/striations retained through four country layer clips. Before added shadows/softness,85,840 eroded visible interior pixels match frozen baseline exactly acrossallRGBAchannels; no allpixelidentity claim. Corrected RED>GREEN>YELLOW>BLUE ordering/yellow aperture retained, finalsoftness explicitlysRGB avoids darkgreen/blue quantization. Parent viewed original/current/V2 logical andnative detail. Receipt927bf0fdfdcca4c2037b09fcc7ddef7f773794ff666c8df04181d5e9de6c4dbf. Closerdepth/atmosphere but original more luminous/irregular; hidden geometry/lighting inferred. Candidate remains private/inactive/notfinalapproved. Currentpack, gameassets and originalreferences unchanged. No additionalappbuild/cloud/device request.


22 October - Actual stacked-map host preview and efficient artwork-only builds:
Private alternate71,520,760byte pack04ea92e4cb74b9e9d50694dc1785b7e0a3c5814b3ec23c7515bfbca9f23f511c changes ONLYtag2/role1 texture forblk6;23othersections/sourcecatalogue remain byteexact. Independentcontainer comparison andunchangedC++ optimized/ASan+UBSan realpack loaderPASS. CurrentmapstackV2 ispreview only, notfinalapproved.
Local3file codecheckpoint3dca7c1e4024b824419fa27f861cd849cf3cda15 isolates privateartwork compilepin into generatedheader includedonlybyArtworkActivation.cpp. Actual nonemptyA->B pinchange rebuilds exactly1object andkeeps54others unchanged,6startupregressionsPASS. Renamedbyteidentical source template toArtworkConfiguration.template.h underexistingpublicsourceallowlist; finalconfigure/noopbuild changes0objects/binary. No publicationpolicyexception. Stagedpolicy/secret scanner/commithookPASS, no push/cloud. Empty/nonempty transitionstillchangesflags; relink stillneeded. Privatebinaryc4fcd100e5e97b8b6ecbd133e0f17423f945b68865737f4036b860ed02464245,454finalsourcepins verified.
Actualpreview run01 controlled originalapp usingnormalX11input:90pairedGPU commits,44initial640 and46native2186x1640,300selections/600triangles each,0refusal/unpaired. Two actualnativeBMPs; parent/independentviewed softerstackmap withsharptext/panel. Independent e4a11a61950d3d15e129c43ddbc8e2861b549d1e24189306d1afa728c67c4a59 alsoverifies176500staticlogo/upperpanel pixels unchanged,228636of300000mapinterior pixels changed. CleanWMDelete0; actualcanonicalcopiedconfiga678 andbaselinepack4500 restored, owneddisplayterminal. Preview pack staysprivate/frozen, notactivated shipping; no iPad/performance/energy/styleapprovalclaim.
Remainingright-native marks:35decodedidentities+2missingreferences;2SIN/RACcandidates andnewprivateIND5V1/V2 remaininactive. ParentviewedIndigoV2:closerwidth/counters butgold flatter/cleanerthanoriginal; nofinalartapproval. Sourcefeasibility keepsnativeupload/draw selection/fade/UV/context path, requires completeprepared128texturebinding observations andexplicit straightalpha/PMA handling beforeactivation. Source/prototypeguarddesign only, no actualreplacementyet. Currentartwork/sourceoriginals unchanged. Goalactive; nativehardwareUI/energy andremainingmenu/HUD artwork gatesopen.

22 October - Native menu-mark identity and sampler evidence:
Private renderer observation captured five exact prepared128 canvases SIN5/RAC5/NOR5/IND5/A5; all16384 words each verified independently against original source conversion and postupload rotation. Original game CPU/native draws unchanged. Actual owned-X11 run clean exit0; copied config and baseline pack restored. Trace cap exhausted before native1640 geometry, so no native geometry or suppressed-player2 proof claimed. Separate real GLES sampler component passed11 cases/46 checks each optimized and ASan+UBSan, original shader reverse-byteexact; does not prove runtime lifecycle. Canonical512 PMA SIN/RAC assets frozen for private runtime preview, not maintained/shipping activation. User preference remains rich authored map texture/reflections with corrected RED>GREEN>YELLOW>BLUE shadows/soft focus; clean variant rejected. No cloud/device test needed now; goal active.

22 October - Two authored menu marks actual native preview:
Private replacement renderer object8ffd6b89 links with454maintainedsource/73productionlink inputs into binaryc883d503. Independent strict15addition reverseaudit restores original renderer byteexact. Real owned-X11 game reached2186x1640; SIN/RAC activated at480and1640,2auxtextures createdonce/reused/retired,0loggedfallbacks;70backgroundpairedcommits including45native,300selections/600triangles each. CleanWMDelete0, canonicalcopiedconfig andbaselinepack restored, displayterminal. Independentreview92ed21427ab1d12ccedf50d7b5476c991951c53538f6b739a4149a3f91cdcf89. StableopaqueLogo35700px/goldbar2860/mapinterior300000 remainexact; broaderpanelanimation phases differ, no wholepanelidentityclaim. ActualnativePNG underhd-native-mark-preview-v1/run-01-native. PrivatePOC only; maintainableexternalcatalog/provider integration stillneeded, no iPad/performance/battery/artapprovalclaim. NorthCountrycrestV1 rejected for changed creature anatomy; refine original distinctive geometry with semantic/material judgment, not pixel enlargement. Goal active.

22 October - General remastering rule, explicit user requirement:
For EVERY artwork asset, identify the depicted subject/anatomy/structure and intended material, bevel, lighting/shadows BEFORE drawing. Guide reconstruction by semantic understanding; original pixel contours are measurement/reference, not automatic anatomy or material generators. Preserve intended original identity/proportions/composition while resolving low-resolution artifacts, not amplifying them or replacing with generic symbols. NorthCountry crest is a WINGED UNICORN: taperedhorn, equinehead/neck, shoulder-attached layeredfeathers and curled lower silhouette; raised beveled gold with recessed/contact shadows. Prior dragon/spine-like studies are negative references. This applies to all UI/HUD/logo/cursor/background work; user should not repeat it perimage. User explicitly requested persistent memory, saved as one ad-hoc extension note.

22 October - Maintained native-mark pack/provider foundation checkpoint:
Local10source/test/documentationfiles committed86eb823b31ca15284a73a0a4218179d49fc9507a after stagedpublicationpolicy+independentsecret+commithookPASS, no push/cloud. ExplicitNFSHDPK2 retains strict24legacysection schemas, boundednativecatalogue64bindings/8textures, same96MiBtrustedwholefilecap; v1unchanged/missingprovider usesoriginalmarks. ImmutableoptionalCprovider uses same retainedauthenticatedpack; unavailable/sealednativeprovider doesnotdisablevalidbackground. Independent8opt/sanitizerexecutionsPASS incl40malformedpackcases and16000concurrentproviderlookups; integratedCTest14/14PASS includesexistingstartup/provider/v1regressions. Private73,618,376bytepack2e7cd185 keeps24priorpayloads byteexact and adds2canonicalPMAtextures;4actualpackloader/startupopt+ASanUBSan runs PASS fullpreparedkey/artpayload equality, normal andalreadysealednativeprovider. This is loader/providerproof ONLY; maintainedgenericGPU/generatorintegration underway, no maintainedv2game/device/performanceclaim. Semanticwingedunicorn study remainsseparate/unapproved. Goal active.

22 October - Maintained optional native-mark renderer actual native acceptance:
Local6source/testfiles committed07fed64aa362165a8753de2810e4a1febf646ce1 afterpublicationpolicy/independentsecret/commithookPASS, no push/cloud. Puredefaultgenerator adds15strictlyreverse-byteexact blocks andcopies4generichelpers withoutregeneratingengine. Generictrustedprovider replaces privatePOCkeys/fileIO; boundedcontextcache preserves nativegeometry/UV/tint/fog/blend, unknownfallback, ownerretire/unboundabandon. RealSDL/GLES84checks PASS each optimized andASan+UBSan helper/provider test (existingSDLstaticunsanitized, driverleakdetectionoff). Firstoffscreenbackendsegfault andinitialwrapperstartupfailure preserved; correctedprocess-ownedX11 wrapperpassesregisteredCTest andsanitizedexecution. Coherentmaintainedhostbuild71.87s/466sourcepins binarycd7bbb1019bb0b7d3bf7341d1ba80f768c0f4fdd2be7247efc3cf32d3c1cd471. Actualnative2186x1640 menu run84pairedBGcommits/47native;2authoredresources createonce/reuse66originaluploads each/4logical-nativefirstaccepts/2retire. CleanWMDelete0, canonicalcopiedconfig+baselinepack restored andowneddisplayterminal. Source/pack/generation/currentstate independentlyreviewedseparately. ActualPNG hd-native-marks-maintained-live-v1/run-01-native/main-menu-native-marks-maintained.png. OnlySIN/RAC candidateart currentlyinprivatepack; NORsemanticwingedunicorn remainsseparateunapprovedstudy. No physicaliPad/performance/battery/finalartcompletionclaim; remainingmenu/HUD scope andgoal stayactive.

22 October - Semantic unicorn actual in-game placement preview:
Privatev3pack69cc242a/74,667,160bytes addsNORpreparedsourcea8f03f1c andexactsemantic512PMAcf17849a toprevious2marks;24legacybodies/prior2bindingrecords/old2art bodies unchanged independently. No sourcecodechanges; onlyArtworkActivation.cpp.o rebuilt/55othersunchanged, full466sourcepins unchanged. Binary221df188 hostbuild64.68s, actualnative2186x1640 run116pairedBGcommits/46native;3resourcescreateonce/reuse77each/6logical-nativeaccepts/3retire,0fallback. CleanWMDelete0/canonicalconfig+baselinepack+owneddisplayrestored. Independentactualea719f9b verifies prior2mark107040/map300000/opaqueLogo35700pixels unchanged;crest44583pixelschanged inchecked105600ROI. Parentviewednativeoriginal/candidate: recognizablewingedunicorn/raisedfeathers, buthead/beard/curlproportions andsubtlegoldrelief stillnotfinalartapproved. Previewonly, no iPad/performance/batteryclaim.
INDsemanticmaterialstudy preserves10customFORD/INDIGOletterpaths, newwarmbeveledrelief;inactive. Rightmainlabels are nativefrontendglyphs through405CF0/426C50 andexistingword-awarefontroute, not128emblemcanvases. ExistingSetupwordartdoesnotmatchrightmainlabelgeometry. Nextboundedreadonlycapture exactfullstringcontext/cell/UV/quad/color beforeanyreplacementmapping; maintainoriginalshadow/fade/draws/unknownfallback. Goalactive.

22 October - Actual right-side label geometry captured, HD typography next:
PrivateobserveroneCobject32fc9c2b linkedwith466maintainedsource/74productioninputs, binaryc84ab18c; topOpenGL/CPU/drawsunchanged,2taggeometryinsertsreversebyteexact. NormalownedX11input actualmenu/native2186x1640 cleanWMDelete0/config+baselinepack+displayrestored.168uniqueglyphtriangletuples=84each480/1640,41tagall,0legacycellmiss,finitebits/printedvalueschecked. Complete-stringFNV matcheslowercaseoneplayer6874bccb60242658/singleracea89c9ae889f238d6;42pairedquadsperheight, originalmain/shadowRGBA24420440/255 and008/255, shadow+1,+1. Identicallogicalgeometrybothheights. Mainwords49/50wide,y73..110, sourcequadheight37. ExistingSetupauthoredatlaswords66/68wide/51sourceheight doNOTmatchnewgeometry; reuseshouldkeepUVwidth/inkHeight intactandaddoutput-onlyscale,notcroptexture. Privatefixture/review298325b8 underhd-right-label-observation-review-v1. Countsbelowcapsbutnodroppedcounter/unobservedlocales/statesproof. No maintainedlabelreplacementyet; nextnarrowgeometrymapping+actualvisualreview. INDreliefcanonicalcandidate readyprivate/inactive. Goalactive/no cloud/iPadtestrequest.


22 October - Research-led track artwork requirement and verified reference set:
User requires research for every track/emblem before reconstruction. Proving Grounds is the specific Pioneer Productions logo (explicit wiki statement), not a generic silhouette; Monolithic Studios references the 2001 monolith. Retrieved and visually inspected all eight 1024x768 route-screen images via the user-selected wiki, with page text, exact source URLs and SHA256 receipts retained privately under .build/hd-track-reference-research-v1. These are clearer screenshot references, not proof of original high-resolution textures. Per-track RESEARCH.md separates verified identity, observed visual structure and unresolved attribution; colored route artwork must be compared with gold main-menu variant before replacement. North Country higher-definition reference confirms symmetric three-tower shield/window openings and unicorn ears, beard and shorter muzzle; prior semantic drafts remain unapproved. Original/reference images stay outside public Git. No artwork activation/build/cloud/device claim from research. Goal remains active.


22 October - Reference-led North Country crest candidate:
Private editable SVG and native/4x/8x previews under .build/hd-map-art-agent/right-marks-nor-reference-v3 replace guessed frame with symmetric three-tower architecture, three isolated arched window cutouts, distinct ears/chin beard, shorter muzzle and curved feather fan guided by authenticated1024x768 wiki reference bd6360ab. Darker bevel edges/directional highlights added; original and earlier candidates retained. Structural alpha connectivity proves three separate enclosed window holes, not hanging bars. Symmetric vector construction has minor rasterizer coverage differences (max19/255;32threshold pixels) and is not claimed pixel-mirror-exact. Parent viewed original/previous/new comparison: closer identity, still private/unapproved; proportions/material and actual in-game appearance remain open. Receipt 6b82eacfb1f022a1a0aea2a02c5f2a4925aba41cc575957517d4e1607463f5bb. Current game pack/source/gameassets unchanged; no new cloud/device build. Goal active.


22 October - User-directed hybrid crest:
User prefers face/castle of third displayed draft, wings/body of second. Private .build/hd-map-art-agent/right-marks-nor-hybrid-v4 combines exact ten wing paths (base/eight blades/coverts) from second with its original rounded lower-body curves; third head/ears/beard/jaw suffix, horn and symmetric three-window castle preserved. Native/4x/8x SVG rasters and three-way comparison retained; parent viewed4x. Receipt 56d06dca9a7771ca1cff6be3c6a0cc8faec5ce013bfcd54a7d275277cecdfb2e. Separate candidate only, not activated/finalapproved; original/previous drafts intact. UI technicallabel integration still pending; no sourcecode/push/cloud change. Goal active.


22 October - Maintained right-side menu caption integration:
Local8source/testfiles committed 34ea359c8e329d170173b412cdcdca7e6f01cb09 after publication policy + independent secret scanner + commit hook PASS, no push/cloud. Shared semanticcatalog extends original1933rows by42 byteexact to prior actual-preview rows. Output-only scale preserves sourceUV widths66/68 while fitting49/50wide by37high originalquads and+1,+1shadow. All22fontpixelheaders hashes ANDmtimes unchanged; existing1933rowbase restored byteexact. Fullgenerator and authenticated layout-only path agree; layout-only checks source/catalog/recipe/header/basehashes and packedregions, refusesstaleinputs, writesnothing on repeatednochange. Seven newPython tests,12existingartreviewtests PASS; optimized+ASan/UBSan each7900triangles/294ambiguouspositions with immutableattributes/fallbackguards PASS. Normal maintained incremental appbuild61.75s/twoworkers/466sourcepins; onlyGlide2x.c.o changed, binary8e610126b6c85aa771b3f4c98201f319a5268919b7d85a380e23eaad376f4740. ActualownedX11native2186x1640menu captures PASS; cleanWMDelete0/config+baselinepack restored/displayterminal. Five namedregions570600pixels including rightlabels/map/logo/crest exactlymatch earlieractualpreview. Existingletterweight/materialstilldraft; no physicaliPad/performance/batteryclaim or newcrestactivation. Checkpoint 000c53b5e14f6bddfd97370023e72918872df3f344d668fbefbc6fe12e2b9b10 under .build/hd-right-label-maintained-adoption-v1. Goalactive; furtherUI/HUDwork remains.


22 October - Hybrid crest actual maintained menu placement:
Exact hybridSVG nested at original(25,0) inside128canvas;512PMA42790198. Private74,667,160bytepackc01997ce changes onlyNORtexture+digest;27othersectionbodies/catalog/identities unchanged. Maintained artwork-pin-only localbuild87.91s/twoworkers/466sourcepins unchanged; onlyArtworkActivation.cpp.o changed, binaryefea7f1904e669733d09e90c338c9a31824a5b20a6cf3d42ac9f1ad7c70da17a; previous8e610126binary retained. ActualownedX11native2186x1640menu preview PASS,46matchingnativepublications atfinalcapture; cleanWMDelete0/config+baselinepack+owneddisplayrestored. Parentviewedfullmenu andnativecrestcomparison: preferredhead/castle+wing/body fitwithoutclipping, originaldrawmodulation makesgold darkerthanalonePNG; finalmaterial/visualapproval open. CorrectedtextROI excludescresttoweroverlap; fourstableROIs450650pixels exactlyunchanged,crest29773changed. Rejected wholeframe-equality assumption retained: broadleftpanel637916differentpixels betweenruns, cause/phase notasserted. Privatecheckpoint 8a4c1f7991cf49bf6e532457855f01ad18b021c87046e8bf762a381cb084d502 under .build/hd-hybrid-crest-maintained-live-v1. No maintainedsourcechanges/push/cloud/physicaliPadclaim. Goalactive; nextremainingfrontendemblems/cursor/backgroundlogo/volumebars andfinaltypography/HUD validation.


22 October - Proving Grounds Pioneer identity research and embedded-palette inspection fix:
Wiki explicitlyidentifiesPioneerProductions; historicallogo description corroborateshattedfigureovercircle. Exactoriginalcir5entry89x89decodedfromunchangedmainarchive2beb04bd afterfixingofflineinspectionutility: resolveembeddedpalette before requiringuniqueglobalfallback. LocalRGB24/BGRA32 withoutglobal andlocalwithambiguousglobals nowwork; absent/ambiguousfallback andunsupportedlocal remainrejected. Eightdecodertests+threeinventorytests PASS; twofilesprivacy-audited/localcommithookPASS, commit 6e2eb84fc50b6aac5933e4616c7aacda18a5345c; no push/cloud/runtimecodechanges. Originalfigure/outside shareindex255, index0absent; rejectedindex0transparent assumption retained. Explicit255negative-space view reveals goldfield aroundhattedprofile, notsolidgoldcowboy; runtimeconversion/bindingstillunverified. Private firstvectorstudy .build/hd-pioneer-authored-v1 preservescircle/hatcrown/brim/coatnegative space; measuredmaskoverlap0.707withprovisionaloriginalkeymask, therefore NOTexact/finalapproved/adopted. Clearercoloredreference andrawgoldoriginal retainedhashed; material/contourrefinement andactualruntimebinding next. Privatecheckpoint e4bd74b0560e3a3c2be85c7348569b172982af2bc1d09bc3e372cfb06d2dbe2f under .build/hd-pioneer-reference-study-v1. Goalactive; originalassets unchanged.


22 October - Verified RGB source alpha, eight gold references and actual Indigo relief preview:
Currentoriginalengine499CF8->4ADBB4 explicitlysetsRGBpaletteindex255alpha0, others255;32bitBGRAusesprovidedalpha. Maintainedofflineinspectionreader nowmatches,10decodertests+3inventorytests PASS. Twofileslocalcommit 6aecae5aeef0f275f7c9b50e69eab1d5fa2c24fd afterpolicy/independentsecret/commithookPASS. All8originalgoldtrackentries exportedprivately withcorrectsourcealpha, unchangedmainarchive2beb04bd. Staticrowblit/nativeconversion evidence hashed; Pioneer128canvas offset/preparedidentity andnativeblackquantization mask remainunverified, firstdraftunapproved.
Privatefourmarkpackc4682062/75,715,944bytes addsIndigo512PMA6cbee27b via verifiedconverted03key865917d1;24legacysections/prior3artbodies/prior3bindings unchanged. Initialbuildsourceguard correctlyrejected twojustcommittedofflineinspectionfilechanges; retainedrejection, matchedexactexpectedhashes andrefroze466inputs. Native runtimecodeunchanged; onlyArtworkActivation.cpp.o rebuilt/twoworkers87.54s, binarya3c2bc1d68999b9a7b9f09fd04a2266b4a576ef90778f889de1d9d7b9d93fc16; previousefeabinarypreserved. ActualownedX11native2186x1640menu PASS,49nativepublications atlastcapture;4resourcescreate/retire,8identity-heightfirstacceptpairs480/1640, cleanWMDelete0/config+baselinepack+displayrestored. ParentviewednativeFordIndigooriginal-vs-relief: sharpercustomletters/raisedgold, originalplacementfits; fontweight/proportionsstillfinalartreview. Fiveothernamedregions556250pixels exact;Indigo48818changed. Checkpoint a91e62e1d3398ea33f52e4aa79080e4244cf236aae2955d5d47ddce4d0b661bb under .build/hd-indigo-maintained-live-v1. No push/cloud/iPadclaim; UI/HUD work and goal remainactive.


22 October - Maintained cursor integration and actual native menu validation:
Local13file commit ee5d9c25e6b78b023d2642d443da56a27240d1fa after publication policy/independent secret scanner/commit hook PASS; no push/cloud. Exact4444/32/2048 source maps to128PMA65536 cursor alongside5551/128/512 marks. Version3 mixed packs validate encoding/edge/digest tuples and cross-profile references; v1/v2 remain strict. Optimized+ASan/UBSan pack/provider tests PASS including55 malformed cases each; actualGLES143 checks each mode and3 hook tests PASS. First actual run safely failed cursor activation due stale generated renderer; clean exit/restoration retained. Reapplied maintained generator15 blocks with every other renderer byte identical, copied helpers, rebuilt locally63.18s/twoworkers. Second actual2186x1640menu PASS:5resources created/retired,10identity-height pairs480/1640, cleanWMDelete0/config+baselinepack restored/display stopped. Six namedregions733330pixels exact vs originalcursor run; cursor crop visibly sharper. XTest motion did not move menu cursor, so actual corner/hotspot tracking NOTclaimed; originalgeometry/UV and syntheticclipping evidence separate. Privatecheckpoint e1fa35817ae1585f5cd43b75a7fd13d9caa74a4569ec8e8202de8f67ff70802b in .build/hd-cursor-maintained-live-v1. No iPad/performance/battery acceptance. Goalactive.


22 October - All eight original track emblem bindings, desktop typing fix and Pioneer refinement:
Read-only copied-renderer capture v2 safelyexited/restored all runs; first Escape loop cancelled changes, second retained changes and independently matched7sourceentries. Normal keyboard hollywood initially failed because SDL3 committed text was neverenabled ondesktop. Onefile src/SDLWindow.cpp fix starts text fordesktop platforms withoutscreenkeyboard support onmainowner, stops beforewindowdestroy; iOSexplicitkeyboardtoggle retained. Existingkeyboardmessage tests PASS includingcommittedtextonce/controllerisolation. Localmaintainedbuild65.03s onlySDLWindow.cpp.o changed, binary77c9f629. Coherent readonlyobserver relink49.78s, actualnormaltyping nowunlocksMonolithic withsamestartcopiedprofile. All8(cir5/out5/nor5/pac5/med5/him5/mon5/Lrs5) independentRGBquantization/canvasreconstruction matchescomplete32768bytes andoriginalARGB1555->RGBA5551 conversion, actualselection+pairedtriangles. Sourceoffsetsmeasured notguessed; originalpertracktopY88..97 invalidatescommonY92 assumption. Actualevents4335, zero dropped; cleanWMDelete0/copiedconfig+baselinepack restored/displaystopped. Checkpoint 4bd43dc097b82023022fddf13d522f6329d8e6cd54991e14012f82fafbd9872b under .build/hd-track-mark-capture-v3. Sourcecommit 6781914958e16546828e88b6f05deda4e9278321 afterpublicationpolicy+independentsecret+hookPASS; no push/cloud.
Pioneerfirstdraft512PMA placedatexactcir5(19,0)inside128 canvas; private6markpackfdad91da preserves24legacy/prior5art/prior5bindings. Artwork-pin-onlybuild84.86s onlyArtworkActivation.cpp.o changed. Actualpairedoriginal/v1native2186x1640runs PASS;6resourcescreate/retire,11firstacceptidentity-heightpairs;5namedregions627730pixels exact. Privatecheckpoint 693d1bcfcf71a2a97da5cf38e4cca652c32b683d946fc30b65c79a5e9901b04e in .build/hd-pioneer-maintained-preview-v1. V1 placementverified, contour NOTfinal. Independentv2 semanticreviewcorrectsface-directiontext(left-facingprofile, rightcollar), refineshat/profile/embossedgold; parentviewedcomparison. V2receipt 3de9bd451e3a69e793d9c7f4fc59b7879293c63687b96f548d5d9945ff403981 under .build/hd-pioneer-authored-v2; nativealphameasurement70.45->72.38percentIoU isfeedbackonly, notsemanticacceptance. V2 remainsseparate/unactivated.
Remaining: othertrack/caremblems andfrontendforms, background/volume maintainedintegration/finalmaterialreview, HUD/fonts andphysicaliPad visual/performance/energyacceptance. Current8texturepool/packlimit mustbereviewed before all8trackmarks plusmode/car/cursorartcoexist; do notsilentlyskipartorcallfullUIcomplete. Source8bindingproofs do notproveauthoredartcompletion. Goalactive.


22 October - Eight-resource maintained menu integration and bounded full-menu capacity:
Local source commit 0c7911d338e5226a58a63f471675cfce2f7861d7 raises V3 native artwork capacity to40 and128MiB; V1/V2 retain96MiB and V2eight textures. Exact5551/128 and4444/32 source profiles,64bindings and shader/draw/UV unchanged. Parser opt+ASan/UBSan58 malformed cases pass; actual synthetic SDL/GLES2805 checks each optimized/sanitized pass, all source tags within production2MiB. No full-game sanitizer or device claim.
Private eight-art pack34640048/78,928,040bytes integrates PioneerV2, MonolithicV2 and OutbackV3 with existing hybrid crest/cursor/mode/Indigo. Maintained local two-worker build84.82s passed seven legacy tests;469sourcepins rechecked, binary98cecfbc. Actual owned2186x1640 selector cycle confirms all8track positions;8authored resources created/retired,14identity-height pairs. Preserved six-resource baseline also cleanly passes. Both WM_DELETE exit0, copied config/baselinepack restored, owned displays stopped and lease absent. Parent viewed actual selector labels and comparison: all fit; exact contours/material remain drafts. Four other track emblems and Location backdrop/text remain original, not completed remasters. Mode/car crops231920pixels exact; broad left/logo/cursor crops differ, cause/phase unestablished, no whole-frame equality claim. Private checkpoint d6f476cc0ba049806f8a6f6f6b6d84188c4b09cbf9446d8cc623e8a0f1b6a57a in .build/hd-native-menu-maintained-live-v1. No push/cloud/physical iPad or performance/battery acceptance. PacificSpirit and Mediterraneo vector candidates delegated privately; goal remains active.


22 October - All eight authored track emblem candidates in actual maintained menu:
Privately reviewed PacificSpiritV2, MediterraneoV1, MysticPeaksV2 and LastResortV2 join prior4track emblems. Semantic reference study and side-by-side critique; no scaler or embedded raster. Pacific revised fan/eye/frame/darker material; Mystic rounded individual skulls with prior outer mountain silhouette retained; LastResort distinguished genuine transparent seams from opaque near-black relief using original RGBA, corrected temple positions and removed unsupported face marks. These are reference-led candidates, not exact/final artwork.
Private12-resource pack89a350ee/83,123,176bytes/37sections preserves prior8art/bindings and24legacy payloads byteexact. First local pack-pin-only build83.17s/binary0f018c1c activated all12 but logger40event cap hid final2retirements: retained original12create/18accept/10retire observation and rejected stronger lifecycle claim. Scoped source fix a556ddb5e531bec5cf416be3e98c895280d4b250 scales private diagnostic bound to400, no rendering change;2805 optimized actualGLES checks and all40 synthetic retirements logged, policy/independentsecrets/commithookPASS. Follow-up local helper-only build60.95s/twoworkers, binary2408509e;469pins checked, onlymatching source/generated log-helper differ frombaseline. Actual owned2186x1640 menu rerun PASS:12resources created and all12retired,18identity-height pairs, cleanWMDelete0/copiedconfig+baselinepack+display restored/lease absent. Earlier8-resource actualrun reused by exact frozen binary98cecfbc/pack34640048/manifest/result pins, avoiding duplicate baseline. Parent reviewed final8track crops and four-emblem comparison; no clipping, material/exact contour still review, other frontend forms/HUD incomplete. Checkpoint 48f4a2e06f6c82e3e741da6ff905af201326a64c8f02febc12123b99cdb9fdaf in .build/hd-native-menu-twelve-trace-live-v1. No cloud/push/newiPad/performance/battery acceptance. Goalactive.


22 October - Maintained owned software text core and next-form observation:
Read-only audits confirm Audio/Graphics compositor remains private, Location has a different real tail contract and cannot reuse MAIN_SKIP. MostHUDfonts/gauge/Omega/record/pause/countdown already maintained; race minimap/markers remain uncovered. Portable software core now local8file commit3fa51b3ef8814d232cebf927a4f810177388b0fa: immutable copied FontCatalog, exact shaping/clipping/color expansion, original store replay/full-plane verification and queued font ownership. No original font payload/guest reads/render hooks embedded. Opt+ASan/UBSan527495synthetic checks each,5130representative real-authored font layouts exact; registeredCTestPASS, independent review no actionable finding; policy/independentsecrets/commithookPASS. Currentgame binary2408509e unchanged; runtimeactivation/device/renderperformance unproven. Private checkpoint 14bf51078bb106e76b7307c9f78b0e7a23217090683717cfd21746f7c57c8d1e under .build/hd-software-core-maintained-v1.
Bounded readonly LocationV1 observer3units/27reversibleblocks locally linked60.10s; actualnative menu cycle clean/copiedconfig+pack+owneddisplay restored, BUT zeroobserver records. This is a retained diagnostic negative, not Locationproof. V2 adds bounded raw owner/iteration gate data before choosing correctform contract, pendingactualrun. Separately2pad hostfixture inputfilter56SDL/X11 checksPASS, oldwatchduplicatedkeyboardeventnegative retained; no currentHUD race yet. All private captures/candidate artwork preserved, no cloud/push/paidservice. Goalactive.


22 October - Actual Location observer gate diagnosis:
Private V2 local build56.75s, exact27reversibleblocks and all74baseinputs unchanged. Actual native selector cycle clean exit0/copiedconfig+baselinepack+owneddisplay restored. Zero normal records retained as negative, but bounded raw header proves verified main owner,173entry calls,27nonmain40BBE0redraws EAX0x404abd vs main0x405f61, zero readerrors and global4D243C zero throughout. All1891collector rejections came from wrong nonzero-global assumption; metadata also overwrites iteration identity. Correct next readonlyobserver to track actual redraw argument under owned entry; do not relax production main-only/tail verification. Separate TLS same-stem overwrite risk identified, not observed cause. Checkpoint e9df70873ecf0ebbbab683c4c9deeac4334ff3285f84f85d4c37be01e25f5584 under .build/hd-location-observer-v2. No cloud/push/device/renderacceptance; goalactive.


22 October - Actual Location caption route and compact software text core:
V3 native observation closed/restored cleanly but zeroevents/277110owner-gate rejections; original40F674dispatcher proves main40D3D0 andLocation40C160 are siblings, not nested. Corrected read-onlyV4 binds seenMain+sameGame+claimedthread and exactLocationlexicalowner;27sourceinsertions reverseexact, independentreview+opt/sanPASS. Local3unitbuild58.75s binary97a46c3b preserves all74baseinputs and production2408509e. Actualnative2186x1640cycle cleanWM_DELETE0/config+baselinepack+display restored:224records,6passes,22validplanes,16sources,84droppedcapture requests. Five valid same-target343x480/pitch686 tailpairs change705-709pixels at[117,167,168,186]; fourpairs directlycontain location via405CF0->486A30 mode0. Warmup pair0hasnorecordedtext andfinalpass6text hasnopairedtail. Unsupportedupload/redrawplanes andteardownmetadatareaderror preserved; no wholepass/store-replay orreplacementacceptance claim. Privatecheckpoint c8d410978c023d5c7d09bcd5bd9126538b2f02c5ecc6f2da06c01b8107948f66 under .build/hd-location-observer-v4.
Maintained3file localcommit40f3d6312061bb2dd7d52577e780e78f14afafa5 supports exactcompact343 andexisting640 softwaretextsurfaces, compactownedbuffers/actualpitch/LUTendpoints/clip/countbounds withoutpadding. FullLUT/header/owner/GPU authority remainscaller. Existing640andnew343boundary/retirement/mutation/clip tests pass optimized+ASanUBSan527620checks each; registeredCTestPASS, publicpolicy+independentsecrets+commithookPASS. DocumentationLFpreserved. Currentgamebinaryunchanged; no runtimeactivation/cloud/push/deviceclaim. Corecheckpoint 9cbfc670699794590d9c994381ad0d2f743ab80aa15fd558981b7330b55a5ae3.
Two-pad private main-onlyfixture built51.87s/f5a3c2ca; actualSetup/split-screen mode/Donehighlight observations passed cleanly. Third Player-form observation hit Xlib screenshotBadMatch atnative resize afterP1manualtransmission screen; helperfailure terminatedharness beforefinally, not observedenginecrash. Exactgame normalWM_DELETE/pidfdterminal andSDLstatus0; copiedconfig/pack restored, exactdisplayterminal,leasegone. Negative+RECOVERY.json preserved; child-isolated screenshot correction andP2/race validation pending. Goalactive.


22 October - Both current split-screen Player forms actually observed after screenshot isolation fix:
Retained firststage3 XlibBadMatchnegative+exactprocessrecovery. Child-isolated r2 screenshot wrapper authenticatesexactPID/exe/startticks andBMPdimensions; nativewindowmoveResize(0,0) waits3stablegeometryobservations, preventingoffscreenread. Rootactualr2passed normalWM_DELETEexit0/copiedconfig+baselinepack+owneddisplayrestored/leasegone. BothPlayer1640nativecaptures andnormal640forms viewed: P1FordIndigo/manual, P2NazcaC2/manual; transmissionrowselectedinboth. NormalSetupsplitmode/Donecommit andindependentP1/P2formnavigation proven; controllerbindings/automatictransmission changes/actualrace stillpending. No productioninput/physicalHUD/performanceclaim. PrivateCURRENT-CHECKPOINT 272011ec3b0bc05aad72f5a7e052d6bfcde90c34c004b7d3ecc51c29920cd3c3 under .build/hd-current-hud-regression-v1; next configurebothcontrollersnormally andnativeHUD race. Goalactive.


22 October - Concrete Location adoption scope established from actual source equality:
Private analysis95b09609 under .build/hd-location-adoption-analysis-v1 independentlymatches all8base-source captures to existingblk4, four640basepostplanes to fullsourceRGB565payload, side+nedbfullhashes, spds/forbexactarchiveprefixes with2/17observerunderspanbytes. Existingauthoredpack alreadyhasnecessarybackground/titleassets. Nextmaintainedwork is exactLocationowner/baseproducer +genuine343softwaretailpacket, narrowexisting405CF0StringScope/fiveoriginal4A5830/4B3EE0stores preservingoriginalwrites, startupauthenticatedownedFontCatalog, explicitLocationGPUadmission preservingmain/laterwidgets/marks/cursor. Precise missingauthorityisactualfont/state/origin/advance/individualstore replay; validateinsideadapter withboundedfallback, noanotherbroadobserverneeded. No completeLocationreplacement/physicalacceptanceclaimed. Handoff 82fbbb4e14bf460691113c239bf9755007fdae8d09fbedcba79651898e421119. Goalactive.

22 October - User art direction for the North Country winged-unicorn crest: use version 3 as the reference for the horse head/face and crest; use version 2 for the wings and horse-body silhouette. Preserve those specific strengths in the combined revision. This is guidance for the next art revision, not approval of an existing candidate or permission to replace the current accepted asset.

22 October - Normal Location entry diagnosed; original preview layer is next:
Local commits b0a4ea2 and c60747b save the guarded Location/font/GPU foundation and the exact normal-entry correction. Both commit hooks pass publication policy and independent secret scanning; nothing pushed. Final focused registered suite passes19 tests. Private actual-state probe compiles one isolated object and links without changing the maintained executable; game exits0, returns to stable Main and restores copied config/artwork/display/lease. Five real samples prove correct Location ownership but no captured base initialization, including a later genuine300-tile upload. Exact normal base calls at40C3CC/40C5C6 use aBlk4, distinct from descriptor aBlk4_0; new explicit argument mode passes optimized/sanitized six-case tests and eight source-hook tests. Independently reviewed source shows40CD70 draws preview/status art after both normal base calls, including pixels outside the343 sidebar. The replacement must preserve and independently replay this layer before any further activation run. Complete640 equality remains required. No knowingly incomplete full-game rebuild, cloud job, device or battery claim. Next contract: .build/hd-location-normal-base-hooks-v1/NEXT-PREVIEW-CONTRACT.md. Final checkpoint aba9f319cce41f43a73a502bc766bfe7584ef771586e6993e1762c897457c7e1. Goal remains active.


22 October - Remastered Location entry and normal track refresh verified locally:
Local source commit 3a5cb65 preserves independently replayed original preview/status writes as an immutable RGB565 layer with semantic coverage, joined to the remastered background/sidebar, caption, native300 uploads and600 triangles. Actual failures were retained and diagnosed: independent scratch descriptors released shared X-LUT references11to10 without changing the caption owner; an earlier unobserved transition upload also latched the prior redraw invalid before the later exact base capture. Descriptor-release tracking now keeps true owner/bitmap/final-reference failures conservative, and only a fresh fully verified Base capture can recover the scene. Optimized and ASan/UBSan packet/join/producer/lifecycle tests pass, including failed-upload-before-Base recovery and shared-reference negatives. All22 final registered tests pass with fixture assertions explicitly enabled in Release; game flags are unchanged.
The final coherent local build took63.25s/two workers, binary156ee7c4, with352 consumed input pins checked. The bounded actual2186x1640 Main -> NorthCountry Location -> PacificSpirit -> stable Main test passes. Its final log contains88 paired Location publications with actual300/600 consumption, including fresh frames after the visibly changed track label;87 was the last screenshot snapshot before one additional valid frame. Parent and independent review confirm the authored map/logo/sidebar/caption persist and the correct original photograph changes. Original photos remain low-resolution preserved artwork, not remastered assets. NormalWM_DELETE exit0, copied config/baseline artwork/display restored, exact children stopped and lease absent. Independent review and source privacy/secret/commit-hook gates pass; no push/cloud build. The only post-build changes were authenticated test-only assertion wiring and trailing blank lines. Checkpoint: .build/hd-location-preview-maintained-v3/CHECKPOINT.json. Physical iPad HD acceptance, remaining UI forms/HUD gaps, artwork refinement and battery/LPM60 remain open; full goal remains active.


22 October - Audio remaster maintained foundation and original control baseline:
Local UI source commit e465a3867f5d5efcef952aca01b8109df737820a adds immutable original Audio background/bar/text replay records, an authenticated process-lifetime Audio atlas provider, a native bar-only staging helper, optional version4 artwork-pack loading/startup and synthetic regression fixtures. All30 registered Audio plus existing artwork/font/Location tests pass optimized and ASan/UBSan with stable source pins. The CPU fixture passes615289 checks/30 negatives; isolated real GLES passes1089 checks per mode, including owner loss after upload/draw without foreign-context cleanup. Retained historical13 bar passes and5 reset/sidebar passes match their original full output planes; derived per-row compatibility data is explicitly not new production capture authority. Independent pack/startup/link review passes; source policy, independent secret scan and commit hook pass. Nothing pushed and no cloud build.
The private authentic version4 pack is89415304 bytes, SHA88e91ccd0167473edc53a5e52024d80f13e6adcd981d95d8fd0e9f4ffd6692a3, with all37 earlier section payloads unchanged, authored6MB Audio atlas and metadata-only original source identities. Its loader, six actual original sources and retained atlas/source owners pass optimized and ASan/UBSan. It has not been copied into the game or activated.
The bounded actual2186x1640 Main -> Options -> Audio -> three normal volume rows -> Options -> stable Main baseline passes with the prior verified game binary156ee7c4/private12-art pack. Original menu/racing/effects bars each visibly decrease and increase: red pixel counts1000/874/1000,1000/869/1000,1000/872/1000. Short keyboard edges did not change the bars; the original source has an Audio throttle, and paced held normal keys work. The exact dropped-input cause was not instrumented. Earlier mouse-navigation, short-key and150s deadline negatives are retained. Final300s bounded test closes normally viaWM_DELETE0/SDL0; copied configuration, baseline artwork and display restored, exact children stopped and lease absent. Independent actual-file/screenshot review confirms the result. Mouse-held13draw highlights and actual numeric values were not measured.
Checkpoint: .build/hd-audio-foundation-maintained-v1/CHECKPOINT.json. Native Audio HD rendering is not active: exact live owner/pass/source/bar/text capture and ordered native background/bar/text staging with original300/600/context/FIFO proof are next. Existing Main/Location behavior remains the current verified runtime. Physical HD acceptance, other frontends/HUD artwork, final art approval and battery/LPM60 remain open. Full goal remains active.


22 October - Maintained Graphics remaster integrated and locally validated:
Exact full640 reset/transparent343copy/text and retained343 caption/cache producers now join independently observed current selector, both sidebar slots and full owner; prior pure slot3-sidebar assumption corrected with direct negatives. All original game operations remain. Original-preserving additions reverse exactly across six actual generated units; those units compile as C++14 and production renderer as C99. Ten focused tests plus one actual adapter fixture pass optimized and ASan/UBSan; four existing Audio/software regressions per mode also pass. Adapter fixture uses real GLES with synthetic art/native observations and an explicit no-GL original flush stub: 3875 checks and547 optional-operation context-loss points per mode. It does not prove original backend fault recovery.
One coherent local two-worker build79.51s consumed413 pinned inputs, binary3c45382c; previous verified5e6bac57 binary preserved. Actual owned2186x1640 Main->Options->Graphics->Options->stable Main protocols pass twice:83 and107 final complete Graphics publications, each original300uploads/300selections/600triangles, zero native cancellations. First incomplete sidebar falls back to originals. Parent viewed actual authored maps/sidebar/logo/caption/fulltext and selection change; normal Left/Right changes brightness6->4->6, 2664-pixel numeric region restores exactly. Both normalWM_DELETE exits0, copied config/baselinepack restored, owned displays/processes stopped and lease absent. Repeated entry within one process/context recreation and physical iPad appearance/performance/energy remain open; other forms/HUD and final art polish remain. Post-build change only registers the adapter test; runtime sources remain pinned. Public sanitized evidence: evidence/hd-graphics-maintained-integration.json; private checkpoint .build/hd-graphics-integration-v1/CHECKPOINT.json. No cloud build or push. Full goal remains active.

22 October - Maintained Options typography locally validated:
Separate mode1/source-authenticated Options font path now uses nine authored word images plus four existing shared words, with original input glyph geometry, whole draw-string identities, vertex colors, shadow placement and hit areas retained. Eight Main words lose only proven all-zero trailing rows; 83 prior visible images reconstruct exactly. Deterministic repack retains 1024x2048/8MiB, adds188 rows to2163 total, uses23 bounded RLE parts and25 generated headers. Full generator check and authenticated no-op layout-only checks pass with hashes AND mtimes stable;29 Python tests and3 registered frontend tests in each optimized/ASan+UBSan mode pass, enumerating8652 triangle cases/302 ambiguous positions.
Independent actual GLES GL_LINEAR old/new sampling covers1975 prior rows/7900 samples/11098208 pixels at480/1640 heights and2 tints:3155 pixels differ, maximum2/255 per channel, none above2. Visible authored source pixels are exact; filtered output is explicitly not byte-exact. One local two-worker build87.25s consumes418 stable input pins, binary660db3d2; old3c45382c retained. Actual owned2186x1640 Options test passes mph->kmh->mph and on->off->on with observed selection before editing; Graphics then Options then stableMain pass,84 complete native Graphics frames. WM_DELETE exit0, copied config/baseline artwork restored, owned game/display terminal and lease absent. Four rejected harness runs remain: old-font mask, lost half-unit reference placement, blind key count/dim background, and unacknowledged Enter. Corrected only private recognizers/input acknowledgments; same game binary reused.
Public sanitized evidence: evidence/hd-options-typography-maintained-integration.json. Private checkpoint .build/hd-options-font-maintained-v1/CHECKPOINT.json. Options background remains original and is not a completed remaster. Next separate exact Options background ownership/cached-upload integration; remaining Setup/Player/other forms, full HUD and artwork polish, actual context recreation, physical iPad appearance/performance and battery/LPM60 acceptance remain open. No cloud build, push or paid service. Full goal active.

22 October - Maintained Options cached background integrated and locally validated:
Previous goal turn was progress: authenticated Options typography saved78f2f4e. This turn adds independent exact off4D3A88 Options ownership, compact343 options caption with actualselector6/resource7/right168/width48/left120/y364, and complete source map/side/words/logos/caption/copy/upload join. Source review corrected the initial pristineCopy.PRE assumption: lazy40BBE0 retains oldopaque sidebar pixels. Independent originalcopy replay plus complete307200-word POST reconstruction from authenticatedmap/current343 constrains all visible pixels, including transparent/outside stale negatives. No Location preview is invented; Main/Location guards stay exact. The existing single generic presentation arm is reused, avoiding nested invalidation. Original mode1 GPUtext follows the background COLORcommit unchanged.
Six source-hook tests and four registered tests in each optimized/ASan+UBSan mode pass. Packet55negatives and compactcaption full replay/owner/font/measurement/lease/descriptor tests pass. ActualGLES syntheticadapter11048checks and1852 helper context-loss points per mode pass; original drawTriangles is explicitly a no-GL stub in that fixture, not original backend recovery. Six actual generated production units reverse to originalbytes exactly and compileC++14; production renderer compilesC99. One local2worker build99.59s/429stableinputs yieldsb96fa214, prior660db3d2 preserved.
Actualowned2186x1640 menu run passes mph->kmh->mph and stats on->off->on plusGraphics/Options/Main route:324 complete Options frames. Second run passes2186x1640->640x480->2186x1640 resizing andsameprocessGraphics->Options reentry:283complete frames (40logical/243native), fresh38/22resizeframes and84reentryframes atcapture. BothnativeOptionsruns zero refusals/cancellations; actual highresolution and640screenshots viewed. BothWM_DELETE exit0, copiedconfig+baselineartwork restored, displays stopped, noownedgamePIDs andleasegone. No originalgameasset writes, cloudbuild, push orpaidservice.
Public sanitized evidence: evidence/hd-options-background-maintained-integration.json; privatecheckpoint .build/hd-options-background-v1/CHECKPOINT.json. Original backend context recreation/freshoriginalcacheupload recovery remains separate from resize; remaining Setup/Player/other forms, fullHUD/artwork polish, physicaliPad appearance/performance and measuredbattery/LPM60 remain open. Finalartuserapproval pending. Fullgoalactive.

22 October - Setup caption foundation locally verified:
Previous goal turn was progress (Options native background db3f781). Exact original Setup off4D2790 selector0/resource0 caption is game setup: signed advance75, right168, left93,y126. Shared fixed-policy compact caption implementation preserves exact Options6/7/48/120/364 and independent per-policy owner atomic/TLS/state/lifetimes. Both policies coexist in one translation unit; runtime isolation, original compact plane replay/font/store/lease/retirement negatives and six source-hook checks pass optimized and ASan/UBSan (three registered tests each). Copied actual production generated unit with both wrappers compiles C++14, original unit unchanged. No game rebuild/run or cloud/push. This is a tested caption foundation, NOT activated Setup background. Next exact Setup scene/live capture and independent tail4 renderer guards, actual settings/navigation acceptance, Options regression. Existing verified binary remains b96fa214. Full native port goal active; physical HD, battery/LPM60 and remaining frontend/HUD/art acceptance open. Private .build/hd-setup-background-v1/CAPTION_CHECKPOINT.json; public evidence/hd-setup-caption-foundation.json.

22 October - Game Setup background integrated and actual local menus verified:
Previous goal turn progress: exact caption policies saved3b10e91. Shared fixed-policy CPU/live cached scene core now retains exact Options semantics (independent expanded-token equivalence PASS) and separate Setup off4D2790/selector0/resource0/caption75/right168/93/y126. Independent typed Setup tail4/schema1 and exact Options tail3/schema1 share one GPUcaption/FBO lifecycle; cross-kind finish cannot commit/steal pending state, nested begin cancels/refuses. Existing one generic presentation arm reused; original300selects/600triangles and following native mode1 body preserved. Full307200 POST/copy oracle and source lifetimes remain mandatory.
Seven registered tests each optimized andASan/UBSan PASS;55packet negatives perpolicy,6sourcehook tests each, actualGLES11177checks+1852sharedhelperloss points each. Fixture has syntheticdata/no-GL originalflush stub; no originalbackendrecovery claim. Six actualproductionunits byte-reverse exactly and compileC++14; rendererC99PASS. Single local2worker build100.71s/439stableinputs yieldsd8ffc571; priorb96fa214 preserved. Post-build onlygenerator LF restored to originalstyle; normalizedbytes identical, no generatedruntimecode/binarychange, bothhook suites rerun.
Actual2186x1640 original-input test PASS:542complete Setupframes, oneplayer->split->oneplayer, mirroredoff->on->off, sameprocessMain/Setupreentry;325Optionsframes withmph/kmh/mph andstats on/off/on;83Graphicsframes andstableMainreturn. Allbegin/finish balanced,0native refusal/cancel, cleanWM_DELETE0, copiedprofile/artworkrestored, noownedgamePIDs/display/lease. Firstactualrun retainedFAIL: selected-bright valuerecognizer applied to unselectedmirroredrow onreentry; game responsive andrestored. Correctedtest observesrowselection before unchangedrecognizer; samebinaryreused. ViewedactualSetupinitial/reentry/splitchoice andreturnedOptions. Someanimated/manufacturerlogo frames remainoriginalresolution; finalartcoverage/approvalopen.
Public evidence evidence/hd-setup-background-maintained-integration.json; private .build/hd-setup-background-v1/INTEGRATION_CHECKPOINT.json. NextPlayer1/2 require actual full640preview and persistentcolor-swatch writes (410090) includingpost-nativebody writes; do not simply widenSetupcore. Remainingfrontend/HUD/art, actualcontextrecreation, physicalHD/performance/battery/LPM60acceptance remain. No cloudbuild/push/paidservice. FullnativeARM64goalactive.

22 October - Player1/2 original replay foundations verified, live integration pending:
Shared fixed-wrapper preview replay preserves Location policy (independent expanded-token equivalence, one equivalent negated-equality spelling) while separate Player policy supports15photos/five graphdraws and distinctselectedcar/cacheindex. New immutable swatchreplay follows originalSHR/ADCrounding/lighting, exactshadow367,142/color366,141/union85x22 and full/partial/empty selected640coverage. Eight focused tests each optimized/ASanUBSanPASS:324107previewchecks/34negatives,167swatchchecks/41negatives,287compositionchecks/34negatives plus8validclipped/empty packets refusedbyboundedcomposer. ActualproductionC++14unit compiles allheaders. Composer remainsfullclip-only and rejects unexplainedinheritedpixels; persistentoverlayhistory/livecaption/copy/upload/renderer admission NOTimplemented.
Private6unit original-preserving observer build70.97s/2workers/445stableinputs, candidatea62a9e0a separate from maintainedd8ffc571. Actualowned2186x1640 originalmenuinput runPASS bothplayers car/colour/graph change+restore,6004records/nocap/caption68/70. NormalWM_DELETE0, independentlyrestoredcopiedprofile/artwork/maintainedbinary,68originalobjectsunchanged, ownedprocessesterminal/leaseabsent. Earlierharnessfailures retained. Privateobserverlimits explicitlypreserved: incorrectshortcachedindexread, invalidbodyselectedtargetcapture, unusedrow-sourcepatternfields, incompleteinitial/postuploadsourcewitnesses. These mustbecorrected beforeliveadmission; fullSHAalone isnot sufficient authority. Twochangedpureheaders excludedfromall6compilerdependencyclosures, noobserverrebuildneeded.
Public sanitized evidence: evidence/hd-player-replay-foundations.json. Private FOUNDATION_CHECKPOINT.json under .build/hd-player-observation-v1. No cloudbuild/push/paidservice orphysicalHD/batteryclaim. Nextcorrectobserver witnesses andimplementpersistentoverlay/reset/lifetime proof, thenPlayernativeintegration; remainingfrontend/HUD/artapproval, actualcontextrecreation, physicaliPadperformance/battery/LPM60open. Fullgoalactive.

Player replay foundation source/evidence saved locally as 0541abc697d9e91607fc6f4b0e774bc6ebc09581 after policy/independent secret scan/commit hook PASS; all10 committed files match working bytes. No push/cloud. Pre-existing HD_UI_PLAN.md and src/hd/README.md remain unstaged.

22 October - User correction, Main Menu typography:
User rejects currentMain font likeness and identifies Compacta or very similar as
reference. Current8wordSVGs useOxanium500,26px,.65horizontalcompression; narrowwidth
alone does not fix its squaredtechno anatomy. TreatcurrentMainfont as unapproved,
not finalized. Match originaluprightnarrowgrotesque shapes, actuallowercase, stroke
weight/counters/spacing and originalanchors. CompareRegular/Bold weight against
privateactualspritepage, not genericcondensedness. ExactCompactaattribution isnot
established; officialfamily specimen isreference, not permission to spend orbundle
commercialfont. Need faithfulcandidate proof before atlasregeneration/activation.
KeepMainlowercaselabels, compactsidebar and largeRACE/pause roles independently
mapped; do not globallyreplaceallfontfamilies. Publicsource/data/privacyconstraints
remain. Preservepreviousart; no newpaidservice orcloudbuild forfontcomparison.

22 October - Player captions and corrected source target classification verified:
Previousengineeringgoalturnprogress0541abc. NewexactseparatePlayer1/2compactcaptionpolicies:selector/resource2/2,player one,advance68/right168/left100/y198;3/3,player two,70/168/98/234. Actualtextresources/sourcefontadvances andcurrentrunmeasurementsagree. Ninefocusedtests eachoptimized/ASanUBSanPASS; all4captionpolicies coexistwithindependentowner/TLS/state andcrossownerrejection; existingcompact343replay/font/measurement/lifetime/leasechecksretained. ActualproductionC++14combinedsyntaxPASS; originalunitunchanged. NotyetwiredtoPlayerliveproducer/renderer.
CorrectedprivateV3observer6exactreversibleunits/100actualcompileclosureinputs/2workers77.45s,candidate0cc3b0e8,maintainedd8ffc571unchanged. Actualowned2186x1640 originalinputrun04PASS bothPlayers car/colour/graphchange+restore,3784events/nocap. Correctconsumed32bitcacheindex/perrowglobalpattern/separatecapturevalidity; allrowpatternchanges0. Body150/153callswrite separate640/384x160 scratchstorage orzero-nulltarget; cachedfullSHAunchanged. Refresh7eachwritesactualcached640. Uploadpre/postcaptured134/137;cachedsource133/136,onelegitimatedifferent-source slot1eachnearexit. ThereforeSLOT1+outerPlayer isnotauthority. Supersedesearlieralways-inheritedBodyswatchhypothesis. GenericBodyhasnointrinsictargetguard: futureproducerrequiresexactstorage/allrowspanseparationfromcachedAND343 beforeexcluding,else replay/rejectoptional; preserveoriginalcalls. AggregateobserverrowenvelopesareNOTfullmaintainedrowproof.
Testedboundedhistoryprototype9tests/modePASS butkeptPRIVATE/notadopted basedactualsourcecorrection; its6sourcefiles/receipts/failures retained. Onlyowntrackededitsrestoredbyteexact0541abcbeforecaptionwork. Sourcecommentsexplicitlydocumentcorrectindexandstorageguards. WM_DELETE0, independentlycopiedprofile/artwork/maintainedbinary/68objectsrestoredorunchanged, ownedgame/displayterminal andleaseabsent. Nooriginalassetwrites/cloud/push/paidservice.
Mainfontcomparisonnowprivatecomparison-v2:original/currentactualroles/Oswald400/LeagueGothic100,8words atmatchedinkheight. Leaguecloser,notexactorapproved;7lowercasecurrentOxanium500x.65, RACEseparateBebasx.9. Sixofficialpinnedfont/license/descriptionhashesverified; nofontinstallation/atlasactivation/purchase. V1wrongcurrentRACErole retainedandsuperseded; v2 explicitlyPILspecimen/notexactPango/gameoutput. Originalfontpixelcomparison staysprivate.
Publicsanitizedevidence:evidence/hd-player-caption-and-target-foundation.json,evidence/hd-main-menu-font-comparison.json. PrivateV3_CAPTION_CHECKPOINT.json under .build/hd-player-observation-v1. NextfullPlayerlivecollector/nativeintegrationusingcorrectsourceclassifications; faithfulfontchoice/remainingfrontend/HUD/artapproval; physicaliPadUI/performance/contextrecreation/battery/LPM60open. Fullgoalactive.

Playercaption/target/source andfontcomparison evidence saved locally as fe39028afefb6933e1bce681357ddc999a820057 after independentreview, publicationpolicy/independentsecret scanner andcommithookPASS. All8committedfiles matchworkingbytes; originalcaptures/fontspecimens/observer/historyprototypeexcluded. No push/cloud. ExistingdirtyHD_UI_PLAN.md andsrc/hd/README.md remainunstaged. Fullgoalactive.


23 October - Player source and independent upload foundation verified locally:
14 focused tests pass in optimized and ASan/UBSan configurations; actual production C++14 syntax passes with all new Player scopes included. The 64 repository compiler dependency inputs match between configurations and final source pins are stable. Preview capture now observes the consumed index AFTER original4E5928 write, with selected==consumed; entry cache may be stale. Corrected earlier overbroad mismatch positives.
Added exact Player preview/copy/Refresh-swatch lexical collectors, immutable swatch PRE hash, truthful retained343 sidebar proof, full640 source reconstruction, and independent sealed300tile CPU upload witness. Guards cover current form/formGuest/sourceOwner, Game/thread/epoch/archive, direct0 versus real lazyiteration, exact copy/swatches, original call/row/palette/pixels, overwrite and escaped proof lifetime. Both-player/cars/graphs/clips/changedsource/foreign-disabled/no-allocation-in-lease fixtures pass; scene owner/preview stubs are explicit, not actual game acceptance. Compile/link/fixture failures are retained privately and corrected. Independent source reviews completed.
No activation: sealUpload still declines, generated methods/native callbacks/typed Player queue+renderer remain next. Preview must render below authored sidebar, not rely on old-pixel silhouette clipping for HD overlap. Retirement must cover rawbitmap+owner and current sealedproof. No new game execution, IPA/cloud/push or physical claims. Maintained binary/copied profile/artwork unchanged and shared lease absent. Faithful MainMenu Compacta-like typography and other artwork/physical/LPM60/battery requirements remain open. Sanitized evidence: evidence/hd-player-scene-source-foundation.json; private receipts in .build/hd-player-scene-foundation-v1. Full goal active.

Player source foundation saved locally as 35fc1c7e0f803d7f826f972dafca9addf90472a3 after staged policy/independent secret scan and commit hook PASS. All19 committed blobs match tested working bytes; all68 maintained link objects unchanged. Existing dirty HD_UI_PLAN.md and src/hd/README.md remain unstaged. No push/cloud. Full goal active.


23 October - Player native cached-menu integration saved locally (4e0b2065):
Actual source, separate 300-tile upload witness, owned FIFO and independent Player GPU renderer are connected. Final candidate ef8ea0b26b63d21f502764f5a93e44a07eabbf207d0263d958f9f77d85b5a1b7. Nineteen focused checks pass optimized and ASan/UBSan; real GLES fixture covers preview reuse through framebuffer resizing. Final privacy-only hook assertion cleanup separately passes all seven Python tests. Actual production source reversals exact; final incremental build compiles only changed renderer, verifies compiler dependency closure before reusing six unchanged units; maintained binary and 74 original objects unchanged.
Original-data final run records both Players car/colour/graph changes and restoration, and Player One resize 2186x1640 -> 640x480 -> 2186x1640. Its trace hits the bounded 2048-line limit before Player Two resize evidence; retained as a measurement failure, not game failure. Short complementary same-binary Player Two run passes car change/restoration, resize/recovery and Main return before the cap. Both resize sequences retain actual context generation 1 and complete original 300 selections/600 triangles. Captures inspected; preview preserved. Both exact owned game/display lifetimes terminal, clean WM_DELETE status0, copied profile/artwork restored and lease absent. Original assets unchanged by this work.
Fixes include independently owned pre-copy343 pixels, non-overwritten copy occurrence aliases, documented ambient retained-map comparison (strict within-copy checks remain), and avoiding stale preview texture deletion on framebuffer resize. Initial direct upload still safely uses original rendering until first complete lazy sidebar capture; first-frame remaster coverage remains open. Synthetic fixture proof does not replace original callbacks or iPad acceptance.
Public evidence: evidence/hd-player-native-menu-integration.json. Private receipts/captures/candidates: .build/hd-player-scene-integration-v1/v2/v3; final-validation.json in v3. All 20 committed code/test/evidence files match verified bytes. Explicit staging, publication policy, independent gitleaks and commit hook pass. Rejected generic directory literal removed before commit. Existing HD_UI_PLAN.md and src/hd/README.md changes preserved unstaged. No push, cloud build, billing or paid service.
Next: first-frame Player capture coverage, faithful main-menu font comparison/replacement (Compacta not installed/bundled/applied), remaining semantic artwork/UI/HUD work and eventual physical iPad acceptance. LPM60/battery and full native goal remain active; do not infer completion or physical performance from WSL menu results.


23 October - Sealed Player caption resource foundation locally validated:
Actual405580 resource ->41B710 result ->measure ->String PRE/POST is now tracked into a separately sealed immutable historical caption proof. Existing full343 replay and ordinary incomingEBP guards remain; absent supplemental observations keep legacy packets but cannot seal new evidence. Namespace-local owner token explicitly named policyThread to avoid confusion with publisher tokens. Initial caller/first-frame handoff remains unimplemented.
Nineteen focused checks pass each optimized and ASan/UBSan, all six actual transformed production units pass C++14 syntax and insertion-only byte-exact reversal, compiler dependencies stable. New fixture initially overlapped text with captured image plane; moved text to disjoint storage, preserving strict full-plane checks. Failed receipts retained privately. No new linked game run or physical acceptance for this patch.
Main actual-game Haettenschweiler whole-word preview completed enabled Main navigation and clean restore. User confirms typeface match, requests lighter strokes and approximately2 original UI pixels wider. Private refined eight-word candidate preserves baseline/right anchor; not integrated/runtime tested or visually accepted yet. No Compacta file acquired. Original assets/font files/captures remain private. Evidence hd-player-caption-resource-authority.json and hd-main-menu-typography-preview.json. No cloud/push/paid service. Full goal active; first-frame UI/remaining semantic remaster and LPM60/battery/iPad acceptance remain open.

5 November â€” Results and frontend return integration

- The original two-lap Proving Grounds race reaches authored Results and Lap Times, then returns to the authored Main Menu in WSL. The previous fallback was caused by race fog remaining enabled. Main now flushes queued draws before clearing fog at its authenticated frontend boundary. Evidence is in evidence/hd-postrace-main-fog-local.json.
- A disposable local fixture reaches the original name-entry prompt over the authored Results screen. Escape returns through Lap Times to Main. The copied profile and save inventory are restored. Name submission and physical iPad acceptance remain open. Evidence is in evidence/hd-results-name-prompt-local.json.
- The ordinary replay does not qualify for a record because its race time exceeds the tenth-place cutoff. Read-only diagnostics preserve this distinction from UI coverage. Evidence is in evidence/hd-results-record-qualification-local.json.
- The post-race View Records route reaches the original table. A scoped frontend fog reset has been applied at Records entry, with its local runtime replay pending.
- Complete the remaining UI coverage and artwork integration before the next Apple package. After delivering that package, resume performance and battery work without waiting for user testing.

5 November â€” The Records fog handoff now passes the original full race -> Summary -> View Records -> Summary -> Lap Times -> Main route in WSL. The screenshot shows the authored drafting backdrop, badge and HD table text. Renderer publication verifies all 300 original tile uploads/selections and 600 triangles. Copied profile, saves and baseline artwork restored. Physical iPad verification remains open. Evidence is in evidence/hd-postrace-records-local.json.

5 November â€” Extended the frontend fog handoff to menu transition admission, after validating all 300 original tile payloads. Queued draws flush first; the default target checks elsewhere remain strict. This addresses a source-level animation fallback path after races. The updated animation path still requires a combined renderer build and runtime capture.


## Slideshow integration checkpoint - 2026-11-06

All 98 original slideshow pictures have private remaster drafts. They remain
unaccepted pending the recorded fidelity corrections; no draft is claimed final.
The source trace corrected the slideshow owner to 412300/type12 (411BD0/type8
is car history). Native exact archive matching, bounded selected-image loading,
lossless RGBA decoding and immutable session lifetime passed their scoped local
checks. The current private runtime pack and generated native catalog bind all
98 current draft versions and source identities.

Lifecycle/draw event hooks and the game bridge are implemented but not yet
compiled together or activated in the maintained source pipeline. Remaining:
connect renderer with original target/overlay proof; configure exact form and
private catalog at startup; complete artwork fidelity corrections; perform one
grouped compile and actual slideshow navigation run; then prepare the next iPad
package after the whole UI is integrated. Resume performance/battery work after
that package without waiting for user feedback. No cloud build started here.


6 November — Slideshow renderer implementation checkpoint

The grouped prepared-native-engine compile now passes for Methods_01, Methods_03
and Methods_13. The complete current renderer also passes syntax compilation
with the dedicated slideshow pass and its context/upload/select/triangle hooks.
Legacy buf/packed macro collisions were fixed locally without changing original
engine operations. The insertion-only transformer handles both source layouts.

Selected images retain their exact original 640x480 words. Before HD control
text is collected, the complete displayed original background must match those
words; changed/short surfaces are refused. A native original-McLaren session
check passes selection, rejection, changed-archive retirement and surviving
immutable render borrows. Special format126 conversion remains untraced and is
explicitly refused at this surface boundary rather than guessed.

The new renderer stages the selected high-resolution picture and captured HD
text together after validating the complete original native presentation pass.
Image decoding and digest calculation occur on image changes; renderer uploads
cache the image identity and context. This is implementation/compilation proof,
not live slideshow or iPad acceptance. Startup catalog/pack configuration and
maintained engine-pipeline activation remain open. All98 art items are still
drafts pending their recorded fidelity corrections. No cloud build or push.
Evidence: evidence/hd-slideshow-renderer-implementation-local.json and
evidence/hd-slideshow-surface-renderer-local.json.


6 November — Slideshow startup connected and original route observed

The maintained classic transformation now adds slideshow owner/text/presentation
hooks, optionally stages the private identity catalog, and uses the existing
SDK/project compression dependency. Artwork startup supplies the slideshow
resource path; unaccepted drafts still require an explicit local preview opt-in.
Three prepared engine objects, the renderer and startup compiled and linked
into an isolated Linux candidate. The copied game reached Player One ->
Showcase -> Slide Show for Ford Indigo and shut down cleanly. This did NOT
publish the remastered slideshow: the reused compiler recipe selected the older
artwork configuration pin, so activation correctly refused the current pack.
A separate corrected-pin candidate is linking; inspect its terminal receipt
before claiming success. Originals/shared saves were not changed.

Format126 is now traced through original499C98/table index19/4A6A03. The real
sldmus8 surface and all65536 input words pass the native conversion check; a
changed pixel and unsupported format are refused. This separate source-derived
check does not prove live special-source presentation or iPad acceptance.

The eight existing UI resources plus current slideshow draft pack total
552262123bytes, exceeding the private512MiB package cap. Reduce losslessly before
packaging; no artwork should be discarded to meet the cap. No cloud build/push.
Evidence: hd-slideshow-startup-live-checkpoint.json,
hd-slideshow-format126-source.json and hd-slideshow-format126-native.json.

6 November â€” Corrected configuration result: the separate v2 native candidate
linked successfully, loaded the matching authored UI pack in a real local
startup, and completed SDL shutdown with status0. This resolves the observed
activation/configuration mismatch. The remastered slideshow drawing itself
still needs the next actual route capture; the earlier route used the refused
configuration and only proves original slideshow navigation. The active
candidate does not yet contain the separately validated format126 source change;
include that change in the next grouped engine build. No iPad/cloud acceptance.


6 November — Slideshow idle persistence observed; Indigo artwork reviewed

The corrected local candidate reached the actual Ford Indigo slideshow. Its
first version published a remaster once but later showed the original again.
Admission now follows the live slideshow owner rather than just its initial
405C70 presentation call. Each lease freshly checks the complete original
current target plane, exact form/archive and selected image, then pins that
current target for its paired request. The grouped v3 build passes and the real
game validates two complete native presentations (300uploads/300selections/
600triangles each). Its captured settled screen shows the remastered image and
HD controls, and shutdown completes with status0. Coverage is one image; slide
changes, car changes and return still need the next grouped runtime run.

A separate timed run requesting the existing WSL d3d12 path exited cleanly but
never reached the slideshow; zero slideshow publications. Retain it as failed
route coverage, not GPU or slideshow acceptance. Its screenshots again expose
old/new frontend differences during navigation; broad UI coverage remains open.
A source-only early return avoids guest-target reads outside slideshow in the
next grouped build; the v3 compiled headers are retained in its private snapshot.

All8 Ford Indigo slideshow pictures have now received source-based semantic
review, distinct from user or iPad acceptance. The rear-three-quarter picture
was refined non-destructively throughv4 to restore original rear-wheel framing
and readable white Goodyear/Eagle sidewalls while preserving the four flat
rectangular exhaust outlets. Built-in image generation was used; assets/prompts
are private on E. Christie original concept auction descriptions clarify the
different tire variants, forward-hinged doors and mirror-housed headlights;
the supplied original images remain the composition authority.90 slideshow
pictures remain under review. Existing preview pack/catalog were deliberately
not rebuilt after individual reviews; regenerate the final reviewed set once.
No new Apple build, public upload or original-data modification.
Evidence: hd-slideshow-owner-persistence-live.json and
hd-slideshow-indigo-artwork-review.json.


6 November — Parallel photo review and deterministic footer foundation

Source-based photographic review now covers39/98 slideshow pictures:8 Indigo,
8 Ferrari355F1,8 McLarenF1,8 JaguarXJ220 and7 FordGT90. Material corrections
include neutral Ferrari overcast lighting/clear lenses, invented McLaren engine
hardware/tire lettering and GT90 lamp/tire details. Eight clean Jaguar photo
plates remove generated leapers, including protrusions above the footer seam.
Their old footer must be completely replaced; one plate's generated title loses
a digit outside the photo. Original drafts and game assets remain unchanged.

Cross-review found AI brand approximations across the shared footers. Earlier
Indigo review completion is now explicitly photographic only: all complete-
artwork gates remain false until deterministic branding is integrated. Genuine
vector Ferrari badge geometry was extracted directly from66 paths in the
Stellantis-hosted original Ferrari corporate prospectus. Period source-compared
Jaguar, McLaren and Ford vector candidates/provenance are saved privately.

An immutable shared-footer catalog validates dimensions, bytes, PMA, digests,
source bindings and ownership; optimized and sanitizer component checks pass.
Startup/session/lease now carry that owner into schema2 renderer composition,
with cached photograph/footer textures and HD controls drawn last. One grouped
syntax compile of all3 affected engine units and complete renderer passes.
This is NOT another linked app/build or live footer acceptance: footer resource
loading/binding and all-car assets remain unfinished. Shipping selection now
requires the reviewed footer; unfinished whole-image previews stay explicit.

The first2048x1536 Ferrari native footer candidate uses exact source brand paths
and a new high-definition outlined Michroma UI caption. An isolated existing
browser SVG renderer produces its PMA layer and composite preview. MuPDF's
incomplete nested/gradient raster preview was rejected; no photo pixels were
changed by footer authoring. The native footer layer is source-reviewed locally
but not installed in the game.59 photo reviews, GT90 photo6 body oval, other
footers, remaining UI and true aspect-ratio coverage are still required before
the next Apple package. Parallel workers continue F50/Isdera/LotusGT1 reviews.
No public upload/cloud build/paid service. After the complete UI package, resume
performance and battery engineering without waiting for user feedback.
Evidence: hd-slideshow-parallel-artwork-review.json,
hd-slideshow-shared-footer-syntax-local.json and
hd-slideshow-native-footer-foundation.json.
