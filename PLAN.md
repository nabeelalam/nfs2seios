# Native iPadOS port plan

## Mission and operating requirements

Deliver the original Need for Speed II Special Edition as a native ARM64 iPad
app for iPad Air M3 with controller input, accelerated graphics and game audio.
Use separately supplied original assets; no x86 emulation or JIT. Keep compiler,
linker, package, signing and physical gameplay evidence separate.

Two requirements are part of the goal:

1. Use standard public GitHub Actions efficiently. Build only to answer a
   specific remaining Apple SDK or packaging question after relevant local
   checks pass. Use the bounded, manual, sequential workflow in CI_POLICY.md.
   Avoid duplicate builds and unchanged deterministic failure reruns.
2. Publish only reviewed source and public documentation. Independently audit
   files and Git metadata before every commit or push. Keep credentials, signing
   material, original assets, captures, private evidence and personal filesystem
   information outside the public repository.

## Current UI checkpoint and next work

### 23 November: current car-menu checkpoint

The latest local candidate authenticates both original graph formats and uses
the existing authored graph plate/bars throughout the original moving return.
Player One at800x450 and Player Two at800x450/708x492 passed all21graph frames
and four21frame bonus-photo transitions on v27. See
evidence/ui-player-graph-authored-animation-local.json.

Candidate v28 additionally fixes the reproduced32frame Graph-to-Car focus
fallback. Both800x450 and708x492 pass with zero native fallback events, fresh
layout epochs and newly verified original300selection/600triangle cached draws.
Clear invalidation is unchanged; no old displayed framebuffer is retained.
Both runs also preserve all tested21frame graph/photo transitions, exit0,
restore copied game files exactly and preserve22art resources. See
evidence/ui-player-focus-cached-redraw-local.json.

The first Main swipe separately passed59authored-wide frames at both ratios.
Next run a grouped current-candidate menu sweep, fix reproduced remaining UI
gaps, then prepare the coherent Apple/device candidate. Physical iPad and whole
UI acceptance remain unverified. Do not force ordinary graph refreshes through
a different original branch or weaken source/clear checks.

Current v31 now accepts all eight distinct Location track changes at800x450:
all21moving frames each, authored final redraws and zero native fallback
events across the cycle. It owns and validates the refreshed sidebar history.
See evidence/ui-location-full-refresh-history-local.json. Next current-ratio
and Showcase regressions; physical and whole-UI acceptance remain pending.

### Victory animated logo integration in progress

The maintained UI branch now observes the original winner-logo load/upload and
its exact two-triangle draw after the authored Victory pass. Queued commands
retain the copied source and parent frame. Widescreen presentation refuses an
unclosed tail; other renderer operations retain their strict rejection policy.
Insertion reversal checks pass for six engine units. Fresh coherent preparation
and local app compilation pass. A forced original-call reproduction passed all
36 actual widescreen presentations across twelve standard trophy-car choices,
with copied game state restored exactly and all artwork resources unchanged.
The same compiled fixture also passed all 36 presentations at 800x450 (16:9).
A normal two-controller, two-lap Proving Grounds race then reached Victory,
completed 35 source-bound HD logo frames and confirmed 32 actual widescreen
presentations before the trace cap, with matched native/logo request identities.
Results return and shutdown were clean; copied state and all 22 resources were
restored unchanged. See `evidence/ui-victory-natural-logo-local.json`.
A combined seven-case invalid-source/interruption check then rejected all 36
attempts, including five already-completed logo batches before END, with zero
stale widescreen presentations. Production source and copied state were restored.
See `evidence/ui-victory-logo-negative-local.json`. Bonus-winner remote-path
presentation and physical-device acceptance remain unverified. See
`evidence/ui-victory-logo-integration-local.json`. This is not an iPad delivery.

### Bonus player-photo coverage in progress

The source catalog and provider now cover all fifteen photographs. The twelve
standard photo records remain byte-identical; the three new bonus scenes use a
consistent opaque 1228x584 canvas. The compressed pack adds 4,499,068 bytes.
Actual loader/provider and opponent-provider sanitizer checks pass, as do two
delivery checks. Fresh coherent local compilation passed.

Original keyboard unlock codes reached all three bonus cars in Player One;
each completed 100 selections and 200 triangles. Captures show consistent
rectangular photo placement without the old curved black mask. A corrected
Player Two driver saved split-screen mode and reached all three bonus sources,
but initially none published its animation join. The exact outgoing/incoming
receipt still had twelve-car bounds. Extending those to the shared fifteen-source
catalog fixed all three Player Two joins in a freshly compiled app; original
source, stack, selection and owner checks remain mandatory. Shutdown and copied
fixture/resource restoration passed. Captures expose a separate remaining gap:
the HD Player Two composition still has 4:3 side bars at 708x492 and 800x450.
The attempted graph toggle changed color instead, so graph-ON remains untested.
Apple/iPad acceptance remains unproven.
See `evidence/ui-bonus-player-photo-integration-local.json`.

The Player Two wide-tail refusal is source-matched to FZR2000 fzr5. Its exact
converted digest is absent from the active thirty-binding pack; the retained
three-bonus candidate has it. A private additive merge now preserves every
existing binding/payload and appends the three bonus artworks as roles 30-32.
The additive 33-binding pack passed actual provider and owned-loader checks
under address/undefined-behavior sanitizers, preserving all thirty existing
bindings and artwork payloads. Its lossless template adds only 184,197 bytes.
Fresh owned-recipe/pin compilation and all 22 resource checks passed. Original
Player Two selection reached all three bonus photographs and animations; actual
menu-wide presentation now fills both 708x492 and 800x450 captures without side
bars. Original photo-change wide transitions also publish. One native frame
occurs at the resize boundary. Clean exit, exact copied fixture restoration and
unchanged resources passed. Graph-ON and Apple/iPad acceptance remain separate. A valid standard-car
graph-ON route subsequently showed the first transition to bonus12 lacks a
photo-source publication; bonus13/14 do publish their exact animations. The
original loc_40C8F8 cached-car branch needs a separate source-bound handoff
audit. Both graph probes shut down cleanly and restored copied state exactly.
See `evidence/ui-player-two-wide-badge-diagnosis-local.json` and
`evidence/ui-bonus-badge-additive-integration-local.json`.

### Current local iPad candidate

Workflow 44323861157 passed Linux and Apple validation for source
`9e98035cfc57a83e9aa4a60176b7b274663388fe`. The verified unsigned local ZIP
contains the original Apple ARM64 executable and all 13 pinned artwork
packs. Its SHA-256 is
`990ed988f5c2ef1371f7324af76ba6030667f6228e377869b8b88fe51678dc42`.
Original game data is supplied separately. GitHub verified the private
release asset against this checksum. The temporary draft base was removed.
Workflow 44301780017 is the previous private download. See
`evidence/track-info-ipad-delivery-local.json`.

[Download the current personal ZIP](https://github.com/nabeelalam/nfs-ii-se-ios/releases/download/classic-ui-widescreen-44323861157-1/NFSIISE-remastered-personal-unsigned.zip).

The later Location resize guard is local to the UI branch. Its desktop
trace covers one suppressed stale swap followed by a fresh full-width frame.
See `evidence/ui-location-resize-guard-local.json`.

The new build keeps the remastered artwork through Next Track, improves game
folder validation and path redaction, and preserves complete diagnostic rows
after a failed write. See evidence/ui-track-info-next-track-fade-local.json
and evidence/ui-track-info-next-track-ipad-ratio-local.json for desktop results.

The exact package still needs SideStore installation, launch, Next Track,
Opponents entry, the Race Summary return, audio and controller recovery, and
matched Native-resolution North Country split-screen recordings with Low Power
Mode off and on.

### Showcase return verified locally

The local candidate now keeps the remastered Showcase return wide through all
17 fade frames and all 21 car-swipe frames. State-checked original-data replays
at 708x492 and 800x450 had no native-aspect return frames or wide-renderer
declines. Both exited cleanly and restored the copied test profile. The source
joins the actual slot1 map upload to the separately authenticated Player
sidebar; it never substitutes a padded sidebar upload for the map pass.

The local binary SHA-256 is
`53c98df18e8dbf076174c2d62a1b82c60e2b81c9e1cd32d75f7c511b6f0132c3`.
Both Player fixtures also cover caption/source lifetime, complete copy replay,
300 native upload tiles and the explicit renderer source kind. See
`evidence/ui-showcase-return-wide-local.json`.

Follow-up source `03196f6` covers the extra exact-bitmap redraw outside the
normal presentation scope. A matched 20-second idle replay changed from one
fallback to zero, including a directly observed extra redraw. Six guarded car
changes and the return at 800x450 also completed without information-wide
fallback. See `evidence/ui-showcase-idle-redraw-local.json`; these results do
not establish every UI route or physical iPad acceptance.

The previous manufacturer mark appears early in the return and remains a
separate observation. Magnification identifies the settled Jaguar mark as
mirrored by its spin; the earlier Mazda identification was an inspection error.
Apple compilation and physical iPad validation of this source are pending.
No cloud build was started for these local changes.

### Coherent History artwork and local acceptance

All seven History scenes have matched central backgrounds and wide panoramas,
with the retained car photographs and manufacturer marks placed at fixed
source-derived coordinates. The grouped lossless resource set is 56.53 MiB;
all 42 localized title images remain unchanged.

The original-data candidate completed seven pages, slideshow and returns at
800x450 and 708x492. Every panorama source was observed, with no logged
information-wide declines. Four registered History checks passed, and fourteen
actual captures were reviewed for captions, photos, marks and scene joins.
The prior executable, original History resource symlink/target and inactive
local configuration were restored. See
`evidence/ui-history-coherent-runtime-local.json`.

The verified release is now selected by the source pins and metadata-only
`HistoryArtworkRelease.json` catalog. Fresh owned builds embed all seven
panorama identities, while deferred public builds copy no private artwork.
The private injector requires the History background and all seven panoramas
together and checks their pins in the main executable before creating output.
It preserves existing app bytes and compressed member storage. Twenty personal
artwork resources are recognized; original game data remains separate.

Twenty-five focused delivery/configuration/compatibility checks pass. The
release matcher accepts the actual tested local ELF candidate and rejects the
older local baseline; all eight real History resource identities match. This
does not prove the new Apple binary. The lightweight release checks now run in
Linux preflight before dependency installation or engine compilation. See
`evidence/ui-history-private-release-integration-local.json`.

Review the remaining UI scope before one useful Apple build, then verify the
new ARM64 iOS base, inject the complete final private resources and inspect the
actual finished package. No new IPA or physical iPad acceptance is claimed.

### Earlier iPad candidate 44172570289

Workflow 44172570289 passed Linux and Apple validation on source
`6c5e98e806716d1f1cae66203dc37eedb6e313d2`. The unsigned ARM64 candidate
reuses all 13 artwork packs from the previous delivery. The private ZIP is
468,341,532 bytes. The uploaded asset matched local SHA-256
`75b2c093b9683cd3cf33f41443387dcc54beb975c2392ee624ace8307a8c37f3`; the
temporary unpublished base was removed after private delivery.

The package includes the checked guest-copy batching optimization,
same-aspect Opponents resize fix, and resident-memory checkpoints. Test
SideStore upgrade, Opponents entry and class changes, and matched North
Country split-screen captures with Low Power Mode off and on. Physical
acceptance for this source remains pending.

The user recalled that switching from Native to 480p in the previous iPad
test produced no perceptible FPS change. The exact build and measurements
for that comparison are unavailable. Prioritize engine and frame-pacing
evidence in the matched device recordings. The observation does not
distinguish engine work from per-draw graphics work or presentation pacing.

### Previous private iPad candidate

Workflow 44146584443 passed Linux and Apple validation at source
7131fbc4db35c9f97d3c3513361ae410d3f3d910. The unsigned ARM64 IPA includes
13 separately verified private artwork resources. The
468,343,477-byte ZIP matched the private release asset SHA-256
6805bc1c57536ca3b00d5d94617789e08060ac05aff28a4ac01bb6da01ee64ab.
The temporary artwork-free base draft was removed after delivery.
This supersedes workflow 44084402478 and saves 21,386,748 ZIP bytes.

The update selects the compact lossless slideshow, logs first Opponents
frame commit and OS memory warnings, and retains the race-world and
display-link diagnostics. Desktop playback with the compact pack and
two Opponents visits passed. SideStore upgrade and physical gameplay
are pending for this exact package. Verify Opponents entry first, then
record matched North Country split-screen sections with Low Power Mode
off and on.

The desktop Player One check at 2360x1640 covered entry, two observed
car swipes and the return to Main. Three Right inputs were sent. The trace
shows 148 consecutive wide route selections between entry and the return
to Main and zero native fallbacks in that window. This is local renderer
evidence for native proportions. Actual iPad rendering remains unverified.

### Latest physical widescreen preview feedback

The reported iPad preview confirms the HD splash and settled fullscreen Main.
Startup, Location list and track changes, Player car changes, and Opponents
entry showed original 4:3 frames during animation. Opponents crashed just
after the four car cards slid into view. The report does not identify the
package digest, so keep this observation separate from the current candidate.

The current candidate includes the shared Location photo mask and separate
soft shadow, wider single-player racing and slideshow, and higher-resolution
spinning manufacturer marks. Those have local coverage and still need device
verification. Check native iPad proportions on device and 16:9 separately.

The existing 2360x1640 desktop replay covers two Opponents entries, four
class changes, and both returns to Main without a crash. Its memory reading
includes software GLES and cannot identify an iPad termination cause. If
the crash recurs on the current candidate, obtain the matching app or
JetsamEvent report at the crash time before changing the card handoff.
An app exception or native fault and an OS memory termination require
different fixes. Keep the report private and retain only the package
revision, route, and sanitized failure classification in project evidence.

The renderer now uploads validated remastered previews directly from retained
artwork in the settled menu and the entry/return transitions. A synthetic
GLES test checks authored pixel readback and texture reuse, and the full
desktop Glide renderer compiles with all three upload paths. The existing
frame ownership and upload validation still apply. The renderer change was replayed in a linked desktop game at 2360x1640:
two Opponents entries, four class changes, both returns to Main, and clean
shutdown. The renderer's uncapped route-change counters cover 221 and 214
consecutive wide presentations through the respective entry, class changes,
and return sequences, with zero native route selections in either window.
The next native selection occurred after the second return to wide Main.
These are presentation-route and settled-capture checks in WSL software
GLES. Physical iPad behavior and the cause of the crash remain open.

The next iPad app retains one previous startup log in Documents on relaunch.
For an interrupted session, the previous log remains reachable from Files
after restarting once. Crash reports and logs stay private. A startup log
records Opponents entry, first verified scene preparation, and exit. It cannot identify all
native faults or memory terminations.

The reported 40-60 FPS was single-player Low Power Mode. Sustained
60 FPS in split-screen Low Power Mode remains the performance target.


### Build 22 physical feedback and active correction batch

The device test reached the race, post-race screens and return to Main. It
exposed incomplete coverage: the opening splash and track-info pages remained
original; old art appeared during startup/menu animations; post-race name entry
and Track Records remained original; Main's background/logo and opponent tier
could revert after a race; radar names had uneven vertical alignment. Racing
HUD appearance was otherwise reported as expected. This is feedback, not full
UI, controller/audio, lifecycle, performance or battery acceptance.

Work this complete flow as one correction batch:

1. Cover both initial and reloaded Main backgrounds, first presentation and
   menu animations, retaining HD artwork throughout the original transitions.
2. Cover the actual post-race/record-name handlers and qualified name-entry
   child, plus ordinary Track Records; recover on a subsequent complete frame
   after a refused frame rather than poisoning the entire screen lifetime.
3. Add the separate opening-splash and track-info routes, including their
   static pictures, panels, localized headings and dynamic text.
4. Correct the radar glyph bearings without moving map markers.
5. Support true 16:9 and native iPad aspect ratios: fit complete pictures without
   stretching or cropping, retain separate UI anchors/focal regions, and correct
   world camera projection/culling plus split-screen HUD layout. Final-quad
   stretching alone is not widescreen support.
   Author pictures with extra scenery around a protected subject region, keeping
   the complete car, route and important landmarks visible at both screen ratios.
   Place logos and text independently so they remain sharp and correctly aligned.
   Fit existing narrow masters with a deliberate background until wider art is
   ready; never use cover/crop or non-uniform stretching as the default.
   Menus, touch coordinates and each split-screen viewport must follow the same
   displayed content rectangle. Inspect both players and both target ratios.
   Current bounded presentation/intro containment edits compile locally, but do
   not yet provide expanded world projection or complete adaptive UI layouts.
6. AI-remaster the car photos and track/location imagery from the actual
   original references, preserving subjects, composition and exact independent
   logos/route geometry. Preserve lossless masters; defer compression tuning.
7. Investigate temporally consistent high-resolution video remastering last.
   No 4K video result or native 4K photo output is claimed.

Verified correction snapshot: the information-screen/transition batch
passes the grouped Release build with two compiler workers and two link jobs.
One ordinary local UI flow observed North Country TrackInfo, Ford Indigo
Showcase general/performance/mechanical pages, return to Main and Race Loading.
Authored static art and HD text published; menu-transition passes also published.
The process shut down cleanly and copied settings, artwork and all 49 save files
were restored. See `evidence/hd-information-screen-local-runtime.json`.
The earlier failed observation stopped because its fixed button count skipped
Showcase when disabled Player rows were skipped; visible-row navigation fixed
that observation without changing the game. Failed evidence is retained privately.
This selected flow does not prove all tracks/cars, every animation frame,
post-race/name entry/Records, race stability or physical iPad performance.

The private TrackInfo bundle contains eight backgrounds, eight panels and 36
localized heading identities. CarInfo contains twelve whole-car backgrounds.
All packed pixels were independently compared with authored inputs before trust
pins were set. All 36 technical inset drafts now have a private integration
bundle. The production loader returns the correct asset for each car and tab
and preserves the original transparent divider. The Ford Indigo Showcase route
displayed all three tabs at 640Ã—480 on Linux. Fidelity review and wider runtime
coverage remain open. See `evidence/hd-carinfo-insets-local-runtime.json`.

History, slideshow, and videos still need separate coverage. The earlier
input-recording baseline captures driving and a clean shutdown. It supplies no
verified race-summary, name-entry, or post-race return to Main.

Wide photo masters and independent branding/route/panel layers are preserved,
but current runtime information pages still use 4:3 composites. Adaptive world,
HUD, menu reflow and matching touch coordinates remain incomplete. Containment
and wide masters alone do not fulfill widescreen support. The source research
must distinguish original CPU projection from deferred renderer view ownership;
46FCF0 extracts a draw list, rather than saving camera state.

New source snapshot: the Player carousel and separate animated TrackInfo/Records
badges pass combined engine generation and a Release compile/link. The initial
343-pixel sidebar proof correction is included. Private Player and badge bundles
were independently pixel-checked before their trust pins were enabled. See
`evidence/hd-player-badge-build-snapshot.json` for the exact binary snapshot.
The latest grouped build fixes copy-lock recursion and renderer dispatch under
checked-read locks in PlayerPhoto and TrackBadge. The subsequent bounded flow
actually renders TrackInfo and Track Records with animated native badges, and
observes Player photo animation. It exits cleanly with all copied settings,
artwork, 49 saves and 1800 input pins exact. The earlier full-route attempt stopped because
the Player-row observer rejected dim unselected car-name text. A finite
source-palette color correction now recognizes normal and selected values using
unchanged glyph masks and score thresholds. The next complete bounded flow
passes TrackInfo, Records, Player car change, Showcase, Main return and Loading,
with clean shutdown, balanced native resources and all 1803 inputs plus copied
settings/artwork/saves exact. Earlier failed runs remain retained. No new
physical iPad acceptance is claimed for this snapshot.

The earlier twelve Player-photo canvases were provisional Showcase-derived
engineering fixtures; the newly activated private bundle now uses the actual
Player-photo remasters. Actual Player photographs have different colors, viewpoints
and scenery and must be remastered from their own references before final art
acceptance. All twelve references and three bonus references are preserved;
bonus assets have different widths/placement and need explicit integration.
All twelve actual Player-photo AI masters remain private and are now integrated
in the local resource bundle; the
latest revisions preserve mid-spin Isdera wheels, a parked F355 with static
background and the user-corrected Jaguar plate ND 4 SPD.

At the user's request, eight independent reference-guided AI badge drafts and
original/current/AI comparison plates are prepared separately. The North Country
castle crown received a focused correction. The user approved this direction; the registered AI badge bundle is now applied
in the local game,
retaining the Last Resort crescent while removing facial features.

Local packaging preserves the existing app byte-for-byte and rejects altered
resources. The five-resource fixture measured 134,587,334 bytes, already above
the current private Release envelope of 128 MiB; a sixth badge resource is now
prepared too. Resolve the final measured envelope before any upload. No new
Apple build, resource upload, billing change or public artwork publication occurred.

Source hooks and standalone materials are insufficient acceptance. Review the
whole startup-to-menu-to-race-to-post-race-to-menu sequence, including animation
frames, in one combined check before another coherent device preview. Do not
publish original reference images or game data with the public source.


The classic UI remains on `codex/hd-ui`, separate from the performance branch.
The third coherent Apple ARM64 UI preview (app version 22) was built, inspected
and privately delivered, including the loading-screen typography plus the
earlier Player-statistics and Victory refinements. Its package
and uploaded digest are verified; physical UI and gameplay acceptance remain
pending. The same app identity and personal SideStore signing path are retained.

The car-stat panel and bars have been observed in both Player screens through
normal car changes and graph toggles. Race HUD, minimap and pause controls have
also been observed locally. The six localized Victory congratulations phrases
have confirmed original source consumers and validated GLES materials. Six
champion graphics are prepared, but their consuming game path is unconfirmed.
A bounded ordinary-input attempt did not reach Victory; clean shutdown and
copied-data restoration do not establish that screen's gameplay acceptance.

Next work, in order:

1. Review the delivered version 22 on the physical iPad: current UI appearance,
   ordinary menu/race behavior, controller/audio and a completed split-screen
   event. The physical feedback above supersedes the earlier narrow source audit.
2. Address concrete UI feedback in coherent batches, with one combined check.
   Keep the current package available until a relevant change justifies another
   Apple build. Preserve accepted artwork and original behavior.
3. Observe Victory through a practical normal-game route when available. Do not
   repeat the slow host attempts or force winner state to obtain a screenshot.
4. Use privately recorded matched normal/Low Power Mode sessions for frame-time
   and battery decisions. Local pixels and approximate CPU percentages do not
   establish physical energy savings or sustained Low Power Mode 60 FPS.
5. Keep unfinished UI separate from main until visual, gameplay and performance
   acceptance is established. The recorder and personal signing path are retained.

Current evidence: `evidence/hd-classic-ui-preview3-delivery.json`,
`evidence/hd-classic-ui-preview2-delivery.json`,
`evidence/hd-classic-ui-refinement-batch.json` and
`evidence/hd-victory-finite-words.json`. The first preview is retained separately
in `evidence/hd-classic-ui-apple-preview.json`. The loading-screen typography
batch is implemented and its combined local release build passed. It covers six
active languages across eight original photo backgrounds, preserving the photos
and original game operations. English Loading was observed through normal
Race selection over the preserved North Country photo at 640x480; both native
publications completed and copied files were restored. Other-language
runtime coverage and physical iPad acceptance remain pending. This batch is
in the third delivered preview (version 22), whose Apple build and package
inspection passed. See
`evidence/hd-loading-finite-words.json`. The separate music-cache caption and
champion assets still need verified source consumers before any new integration.
Earlier build entries below are retained
as history; they do not supersede this checkpoint.

## Historical implementation and performance log

- Explicit mapped 32-bit game values with full-width native pointers.
- Original translated functions linked to native platform services.
- File, event, thread, timer, audio, input, renderer and lifecycle boundaries.
- Asset import validates and copies separately supplied files into owned app storage.
- Local original menu/race rendering, keyboard driving and generated game PCM.
- Host regression and sanitizer checks, plus portable ARM64 compile checks.

Historical build10 physical split-screen testing reported60FPS without an
observed crash. Build12 feedback accepts split-screen FPS and relative touch
gestures, but reports an unexplained single-player Tournament freeze.
Build14 is the latest delivered and independently inspected ARM64 device package;
its current physical regression, Low Power Mode60FPS and battery cost remain open.
The source-only worker-error/cleanup update below has local validation and is not
yet included in a delivered IPA.

## Remaining sequence

1. Retain full-detail native-resolution split-screen60FPS and aim for the same
   with Low Power Mode enabled. Compare matched physical scenes/settings; measure
   before selecting further optimizations. Retain bounds/lifetime/cancellation
   checks. Host samples select experiments; device evidence establishes results.
2. Investigate the reported Tournament collision freeze and complete current
   race/lifecycle acceptance. Controlled host error-path repairs do not establish
   the physical cause or a physical fix.
3. Preserve the implemented native controller ownership, analog remapping,
   held-resume protection and keyboard button. Verify current name entry,
   reconnect and real OS/audio recovery on device.
4. Preserve relative touch movement, one-finger click, two-finger Escape and
   three-finger Enter; build12 gestures were accepted. Validate current fullscreen
   and overlay/pacing behaviour separately.
5. Keep sharper HUD text, speedometer, map and starting lights last. Original
   assets stay separate. Continue personal signing/refresh and complete-race gates.
   Repeat only affected Apple SDK/package checks when reviewed source changes
   justify one controlled standard-runner build.

Completion requires stable, playable original-game behavior on the physical
iPad with the controller and native audio/graphics. A host test or an unsigned
IPA alone is insufficient.


This source revision removes a duplicate region lookup after the production
heap resolver has already verified the complete span belongs to a live allocation.
Allocation ownership, bounds, copy serialization and cancellation checks remain.
The production link test now covers 29 static-region/heap boundary and allocation
lifetime cases; both original and optimized full ThinLTO engines pass. Separate
ASan/UBSan checks match 7,744 lifetime/boundary outcomes and 16,005 original matrix
states. An additional 432 exact boundary outcomes match optimized and sanitized
builds. The current original-game split-screen pair remapped both players and
kept both cars active at native render size. Both runs stopped cleanly with
nonzero captured audio. The optimized game worker used about 12.65% less CPU
per presented frame in this pair; host FPS did not improve. Natural car positions
vary, and software-rendered Linux results do not establish iPad performance or
repair the intermittent race failure. Build 9 remains the latest packaged IPA;
this source change has no Apple build or physical-device acceptance yet.


The next performance candidate adds conservative logical-chunk ownership
metadata to avoid repeated allocation-span cache searches for fully owned
single-chunk requests. It retains all original bounds, ownership, cancellation
and copy-serialization safeguards. Initialization and allocation-failure rollback
are separately validated. The production link regression now has 45 span and
lifetime cases, including large chunks, cross-chunk requests and smaller reuse.

Two original North Country split-screen comparisons ran in opposite orders,
with both native player profiles remapped, native rendering and captured audio.
Observed game-thread CPU time per presented frame fell about 7.2-7.4%, while
Linux software-rendered FPS rose about 2.3-3.2%. Natural traffic varies between
runs; these observations are not physical iPad measurements or a formal causal
speedup estimate. A longer corrected-header session exercised independent
steering/throttle and exited cleanly, but the test driver hit barriers and did
not complete a lap. No intermittent split-screen crash repair is established.
The separately inspected original Split Screen graphics menu showed car detail
on, far view distance and horizon on.

Build 10 is now the latest independently inspected package. Its single deliberate
Apple SDK build passed. Constant 60 FPS and full-race stability remain required,
unverified physical targets.


Build 10 delivery used source 22f8bcb026669c891e87dc7010bdc9c7a0d0a9bb and
[manual run 38941504663](https://github.com/nabeelalam/nfs2seios/actions/runs/38941504663).
Linux completed in 145 seconds, then Apple completed in 449 seconds. The
downloaded thin ARM64 Release package has the existing app identity and version
10, with only executable, metadata and licenses. Source, release digest, contents
and private-path checks pass; there are no original assets, signing files,
sanitizer runtimes or Actions artifact uploads. Apple host regressions and
baseline/ThinLTO mixed-state checks pass; the changed memory runtime compiles
with C++14, O3 and ThinLTO for the iOS device target. This is package/compiler
validation, not physical split-screen 60 FPS or intermittent crash acceptance.


The next input candidate retains build 10's accepted split-screen rendering
configuration. Controller buttons use native joystick state and frontend input,
without also sending keyboard aliases. Short menu presses and control-recording
presses have separate bounded ownership; closing a menu consumes held buttons
until release so a resume press cannot also shift. Both controller profiles and
analog remapping remain supported. Player one's Back/View button resets the car.

A small keyboard icon below the performance overlay shows or hides the system
keyboard. Use it for names or Return when opening a control-recording prompt.
It stays available for explicit dismissal; automatic field detection is deferred.
Touch works as a relative trackpad: move one finger to move the original cursor,
tap one finger to click, two fingers for Escape, and three fingers for Enter.
Chords commit only after the last finger lifts. Long presses, pans and cancelled
gestures do not also click. This adds menu input, not touch racing buttons.

Local Clang C++14 address/undefined sanitizer checks pass 34 tests. Actual SDL
finger-event injection moves the original cursor and exercises all three tap
actions in the original menus. A two-player original race regression exercises
manual shifts, held/fresh resume, reset and injected background/foreground
recovery, with clean shutdown. These are Linux functional checks, not physical
iPad touch/keyboard acceptance or new iPad FPS or energy measurements. The user
accepted build 10 at split-screen 60 FPS; this input revision needs its own
physical regression. Reported battery drain still needs controlled measurement.


An additional original split-screen test found that immediate synthetic Escape
down/up works in menus but can be missed by racing's held-key sampler. Actual
held keyboard Escape pauses the same race. Build 11 is inspected and retains a
public release-note warning for that limitation. The next input candidate uses
a nonblocking 250 ms gesture key hold with a 40 ms neutral gap between queued
pulses; no key repeats or app-thread sleep. Pending actions are bounded to eight.
Cancellation, focus loss and background release the active key and clear queued
gestures. Controller input remains native and unchanged.

The exact previously failed original-race gesture case now pauses on two-finger
Escape and resumes on three-finger Enter with the same gear, clean 88.819 s.
The original menu pan/click/Escape/Enter sequence still passes, clean 28.733 s.
All 34 local ASan/UBSan checks pass, including queued keys, cancellation, bounded
overflow, neutral timing and clock wrap. This correction needs its own Apple
build and physical iPad acceptance; it makes no energy-saving claim.


Build 12 source fd63a194441b135434de2adea977b5fdde9ca782 passed the single
sequential Linux/Apple run 38980392788. Independent downloaded-package inspection
confirms version 12, thin ARM64 iOS-device Release, matching release/source
digests and exactly executable, metadata and licenses. No game assets, signing
material or sanitizer runtimes are included. All three changed input runtime
files and all 15 translated units use verified O3/C++14/ThinLTO; both final links
retain the two-worker backend cap. The 27 Apple host tests and 40,000 baseline
and ThinLTO cases each pass; Linux production spans and new input suites pass.
No probe was rebuilt and the Actions artifact count is zero.

Build 11's immediate touch Escape race limitation is corrected locally in this
source: the original two-player race pauses with two fingers and resumes with
three, retaining the gear. Original menu gestures still pass. Physical build 12
controller/remapping, keyboard entry, gestures, FPS/audio and OS recovery remain
pending. Build 10 remains the user-accepted split-screen 60 FPS baseline.
Battery savings and HD artwork are not established.

Verified unsigned download: [build 12 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-38980392788-1/NFSIISEGame-development-unsigned.zip). Personal signing and
separately supplied original assets are required.


Build 13 delivery, 17 October 2026: runtime source 6b9d5466d05e7678b39f708efc6239ca6d4a0ca6 passed
run 38987697730. Sequential standard Linux/Apple jobs took 151/419 seconds;
no Probe and zero Actions artifacts. The independent download is version 13,
thin ARM64 iOS-device Release, exactly five executable/metadata/license files,
matching source/API size and SHA-256. Game Mode opt-in and the existing full-screen
requirement are present. All 15 translated units and five changed C++ runtime
units use verified O3/C++14/ThinLTO; both links retain two backends. Apple host
28 tests and 40,000 baseline/ThinLTO cases each pass. The 37-test local sanitizer
suite passes. Linux cloud compiled the three new pacing/sampler targets;
standalone pacing execution was local, and sampler execution also passed on
the Apple host. Future workflow dispatches explicitly execute all three on Linux.

The same-process original-data test verified Tournament, Proving Grounds,
white McLaren F1, automatic transmission and eight cars. A 338.88-second bounded
run continued through tunnel/wall contacts into lap 2 of 4 with clean sanitizer
shutdown and no presentation stall. Exact traffic collision followed by wall
contact was not independently established; no completed event or physical freeze
fix is claimed. Earlier private test-harness reference/argument errors were
corrected and were not game failures.

Build 12 physical feedback accepts split-screen FPS and touch gestures. Build 13
still needs physical UI/fullscreen/Game Mode, optional pacing and regression
acceptance. The Tournament freeze and stable 60 FPS in Low Power Mode remain
open. No controlled energy saving is established. A static exit confirmation's
FPS and the black screen after Exit to System are lower-priority observations;
do not force iOS process termination to imitate a desktop exit.

Verified unsigned download: [build 13 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-38987697730-1/NFSIISEGame-development-unsigned.zip).


## Build 14 delivered; next physical acceptance

Source3ade3b3 passed one manual sequential standard Linux169s/Apple520s run.
Version14 thinARM64 iOS-device Release has matching source/digests and inspected
executable/metadata/license-only contents. No Probe,Actions artifacts,caches,
billing change or duplicate dispatch. The reviewed source and public notes stay
separate from originals,profiles,captures and raw local evidence.

Test the same split-screen settings with Low Power Mode off/on and the optional
30 FPS lock off. Normal/Low Power Mode60FPS,battery cost,UIKit/keyboard and real
OS/audio recovery remain physical gates. Tournament freeze diagnosis continues;
the separate captured long-run shutdown wait is not a demonstrated physical
freeze cause. Sharper HUD artwork remains last priority.

## Unreleased worker failure and exit cleanup changes

The source now reports managed-worker failures during the running session, rather
than waiting until final joins. Interrupted or failed original work uses native
owner cleanup. Healthy exits retain the original save callbacks; a separate
failure control lets an exit callback stop waiting for a failed dependency while
respecting the original critical sections. The legacy exit-registry API retains
its previous deferred cleanup behaviour.

All37 local host checks pass with address/undefined sanitizers. Controlled native
session tests cover failure during gameplay, exit cleanup and a nested critical
section. Local original-data split-screen checks cover both native players,
pause/resume/reset and simulated lifecycle transitions, followed by a deliberate
separate worker error and clean diagnostic shutdown. Ordinary original Exit/Yes
also completes and writes settings. Original assets and raw evidence remain
private. These checks do not reproduce or explain the physical Tournament freeze.

This is a source-only update. Build14 remains the latest delivered IPA; no Apple
build was dispatched for this change. Apple compilation and current-device
regression, Low Power Mode60FPS and battery measurements remain open.

## Build 15 candidate: measure the device bottleneck

The next candidate combines the validated worker-error cleanup with an opt-in
game-thread CPU measurement. The measurement is disabled while diagnostics are
hidden. Visible detailed diagnostics average owner-thread CPU time over60
original submissions; this excludes waiting, other workers and GPU work. It is
a bottleneck clue, not a whole-frame or energy measurement.

All39 local sanitizer host checks and eight controlled native-session cases
pass. An optimized sanitizer original-data replay exercises two active players,
pause/resume/reset and simulated lifecycle recovery. It verifies capture enable,
reset and disable, including60 further submissions without new CPU queries after
hiding. An unoptimized replay was too slow to meet its race-readiness bound; it
was not counted as a gameplay pass. Portable ARM64 object checks do not validate
Apple SDK branches. One bounded manual sequential standard-runner Apple build
is justified to check those branches and prepare device measurement.

Build14 remains the delivered package until the candidate is independently
inspected. Stable60FPS in Low Power Mode, battery savings and the physical
Tournament freeze remain open. No new rendering optimization is claimed.


## Build15 delivered; device measurement is next

One manual sequential standard public runner build passed. Apple host tests
include a real current-thread CPU provider check; native ARM64 iOS-device
Release/ThinLTO compilation and independent downloaded-package inspection pass.
No Probe,Actions artifacts,caches,billing changes or duplicate build occurred.
Build15 supersedes14 for the next device test; both use the same app identity.
The Game CPU row measures the owner only,averaged over60 submissions. It cannot
establish whole-frame,GPU or energy cost. Stable60FPS in Low Power Mode,battery
improvement and the physical Tournament freeze remain unverified. Continue with
matched physical measurements before choosing the next performance change.


## Build16 sound-operation candidate

The original mixer shares work buffers between sound voices. The candidate
serializes complete original sound setup,voice initialization,pitch/volume
updates,mixing and stop work. This retains original math,audio callbacks,checked
memory access and nested stop/fade behavior. A diagnostic original-data replay
observed initialization overlapping active output;the guarded local replay did
not observe that overlap. This does not prove the physical Tournament freeze
had the same cause or is fixed.

Production generator/header checks preserve all15translated bodies apart from
nine ownership scopes. Actual original entry,active-stop,callback-error and
estimator checks pass under sanitizers. A matched optimized original split-screen
pilot found no material owner-CPU or submission-throughput regression;near-zero
Linux differences do not establish a performance or battery improvement. A separate
optimized Tournament/collision replay reached lap2/4 and stopped cleanly after
its bounded window. No completed four-lap event or physical fix is claimed.

Build15 remains the delivered IPA until16passes one necessary sequential
standard-public Apple build and independent package checks. Keep the same app
identity and existing imported data. Device stability,Low Power Mode60FPS and
battery cost remain unverified. No new graphics optimization is claimed.


## Build 16 compiled, inspected and delivered

One manual sequential standard-public Linux/Apple run passed. The downloaded
version 16 thin ARM64 iOS-device Release/ThinLTO package passes exact source,
five-entry allowlist, hashes and privacy checks. No Probe, Actions artifact, cache,
billing change or duplicate build occurred. The original repository stays
private; the public source update passed independent staged/history scans.

The sound-operation source preserves original bodies apart from ownership
scopes. Production sanitizer contracts and optimized original split/Tournament
checks pass. One bounded optimized Tournament run reaches lap 2/4 through wall
contacts and stops cleanly; no full event or physical crash fix is proved.
Local CPU/throughput pilot differences are near zero, not an energy improvement.
Next:physical current-build Tournament completion and matched split normal/
Low Power Mode FPS/Game CPU, then evidence-led optimization. Full OS recovery,
signing refresh and sharper HUD artwork remain separate acceptance gates.

## Build 17 compiled and independently inspected

The reviewed checked-copy bundle passed one sequential standard-public
Linux/Apple ReleaseThinLTO run. Independent download inspection verifies source,
version17,thinARM64,iOSdevice,exact five-entry allowlist,privacy and digests.
No Probe,Actions artifacts,caches,paid runner or extra dispatch. Original-data
exact-state,cancellation and complete sanitizer checks pass locally. These
checks do not prove completed events,physical LowPower60FPS or battery savings.
Next:physical Tournament completion and matched split normal/LowPower FPS
and GameCPU. OS recovery,signing refresh and sharper HUD remain open.

## Local Player photo / track badge lock fix and partial runtime

The grouped incremental build passes with two compiler workers and two ThinLTO
jobs, binary SHA256 2b4ca365b25bd78ff8fe1a60ce1fd5952ba74dca598093fdb415e62245a2ecc1.
TrackBadge and PlayerPhoto checked-read scopes now end before nested source
validation or synchronous renderer dispatch; source and descriptor checks remain.
The next bounded original-data run actually publishes TrackInfo, Track Records,
qualified native animated badge triangles and Player photo animation. The full
route remains unaccepted: a later Player row-readiness observation exhausts its
finite capture budget. Shutdown is clean; copied profile/artwork, 49 saves and
all 1800 sealed inputs restore/verify, with balanced native resource lifetime.
The earlier deadlocked run and independent exact-state recovery remain retained.

All twelve actual Player road-car remaster candidates now exist privately,
separate from the provisional Showcase-derived runtime fixtures. Latest user
corrections are Jaguar plate ND 4 SPD, mid-spin Isdera wheels and a parked F355
with static road/background. Versioned masters and comparison pages remain
outside Git; these candidates have not yet replaced the runtime bundle.
Bonus-car layout, remaining information pictures and true adaptive aspect remain
open. This is local evidence, not a new Apple build or physical iPad acceptance.

## Approved AI badge direction and expanded artwork queue

The user approves the reference-guided AI badge redrafts as the final visual
direction. Finalize that direction at high quality, with Last Resort's sun an
abstract solar disk without a face. Preserve exact original identity, relief,
composition and native registration; keep original and draft masters privately.
Apply the same semantic remaster method to the main world-map art and other menu
backgrounds. Preserve the three actual cut-sheet layers, continuing material
through their bar patterns, inter-layer drop shadows and soft out-of-focus map
depth. Keep the established red, green, yellow, blue ordering. Independent logos,
text and controls remain separate; do not bake them into a background picture.

Additional queued idea: research and remaster in-race horizon/sky image textures
after the active menu/artwork integration. First establish exact original source
textures, track/weather variants, panorama wrap, fog/blending and horizon joins.
Retain original geography, lighting, palette and landmark silhouettes, and avoid
seams or invented scenery. This is queued research, not implemented gameplay art.

The first local horizon asset audit separated geometry and color settings from
image data. The copied install contains 18 text-format HRZ files across the
software and Special Edition tracks directories. The Special Edition loader
selects a per-track HRZ file by track index. Those settings describe the dome
offset, distant horizon radius and rotation, pixmap vertical bounds, and the
Gouraud-shaded earth and sky colors. The separate sky FSH archive contains 17
entries. Nine are indexed cloud textures, eight at 64x64 and one at 32x32.
The original loader selects a cloud entry using the active track index.
The other eight archive entries use the SPL and DRP names.

The distant textured horizon uses the first eight directory entries of the
track QFS in the inspected native source. The race loader selects the normal
or mirrored archive, materializes the eight leading texture records, and
builds a 16-sector horizon using those records. The horizon draw path reads
the sector texture table, including reversal when required by the HRZ.
Directory inspection verifies seven tracks have eight 128x128 tiles with
identical entry bytes in their normal and mirrored archives. Monolithic
Studios instead has 2x2 entries and separate archive identities that must
be handled independently. Treat the ordinary set as a panorama with
shared seams, preserving the sector mapping, alpha and independent
cloud and color layers. Runtime attribution and image review are still
needed before replacement. This audit exported no image data.


A private directory-order contact sheet now covers the seven standard
horizon strips and a separate Monolithic Studios sample. The native format
0x78 conversion masks the green component to five bits and uses 0x07c0
as transparent. The seven 1024x128 reference strips preserve that key:
one includes a full opaque twilight sky, while six leave part of the sky
transparent. Visual review shows continuous terrain features across the
internal tile boundaries, including waterlines, cliffs and ridges. Preview
order is archive order. The original sector builder and draw code confirm
an indexed sequence of 0 through 7 followed by 7 through 0 with horizontal
UV reversal when the HRZ mirror flag is set. All eight supplied Special
Edition profiles set that flag. A private source-ordered preview matched
every row at both mirrored joins. Monolithic Studios also sets the black
horizon flag and remains a separate case. A retained natural Proving Grounds
race-start capture agrees with the source panorama at two opposing headings.
The main camera shows the blue twilight band and dark paired landforms. The
rear-view mirror shows the narrow warm sunset band at the opposite join.
This confirms the visible artwork identity and opposite color regions in an
actual rendered race. The screenshot does not resolve individual sector
indices or their flip direction, which are established by the source.
Original pixels and previews stay in ignored local build storage.


A private Proving Grounds desktop build replaced all eight authenticated
128x128 uploads with the fitted 256x256 artwork. The renderer's original
texture coordinates, projection, and sector duplication were used. Its
race-start capture shows the blue night in the main camera and the orange
sunset in the rear-view mirror. The session exited cleanly and the copied
profile and replay were restored. This test covers the starting camera
and mirror. Roadside rotation, a complete race, and Apple behavior remain
unverified. The experimental renderer and artwork are ignored locally.


A second start using the same private build drove the McLaren F1 through
the first banked turn. Six samples span 4.60 to 27.56 seconds of race time
and include straight, turning and off-line headings. No vertical tile
seam was apparent in the sampled main and mirror views. A horizontal
sky boundary is visible at some headings. Compare it against a matched
original drive before deciding whether artwork or composition needs a
fix. This test did not cover the full wrap continuously. The eight uploads
again matched, shutdown was clean and the copied inputs were restored.


The same desktop executable repeated the driving inputs with the replacement
disabled. Nearby captures on the straight and the banked turn show a
horizontal dark-to-blue transition in both versions at the top of the
horizon layer. The draft did not introduce it. The input timings matched
the fixture schedule; the original and replacement runs reached slightly
different camera positions. This supports preserving the current horizon
composition for the pilot and does not establish pixel equivalence.
The control run exited cleanly and restored its copied profile and replay.


Last Resort clarification: retain the original subtle crescent inside the sun;
remove only facial features. A smooth featureless disk alone loses that motif.


## Complete bounded UI route after finite text-color observation fix

The next local run passes the full normal information/menu route and clean
shutdown with the grouped lock fix binary. All 1803 input pins and 49 copied
saves verify, copied profile/artwork restore, resource creation/retirement balance
and no owned child remains. The oracle now respects original normal/selected
car-name palettes without lowering glyph thresholds or changing game behavior.
The faithful twelve-Player-picture bundle is independently assembled and
pixel-validated, but not activated yet. Current runtime acceptance still covers
the provisional engineering photo fixtures, not the new photographic masters.
Approved AI badge direction, corrected crescent/no-face Last Resort and new menu
background work remain separate private art candidates pending integration.

## Approved Player pictures and AI badges integrated locally

One incremental two-worker/two-link build selects both newly validated artwork
bundles. Binary SHA256 0be473af49ae68da189d160f77accd1e5430a18dfaeaa5c060fa501dffe90efa.
The complete bounded normal UI flow passes TrackInfo, Track Records, Player car
change, Showcase, observed DONE acceptance, changed-photo exit animation, Main
return and race Loading. All 1811 inputs and 49 copied saves verify, settings and
artwork restore, native resources balance and no owned process remains. Actual
captures show the new North Country badge and actual Player-photo remasters.
This does not prove every car/track, post-race name entry, completed race or
physical-device performance. Old packs and the previous verified binary remain.

The new Results/Records background masters are wide drafts (1672 by 941), with
interface lettering/logos separate. Main map/gold material studies and these wide
drafts remain unactivated while exact registration and responsive layout are
implemented. Wider art alone is not widescreen runtime support. No new cloud
build, public artwork upload, source commit or battery improvement is claimed.

## Responsive Main implementation and technical-picture inventory

The first responsive Main source path now has a separate full-drawable UI target,
height-uniform semantic layer anchors, owned foreground/cursor replay and a shared
presented-layout snapshot for touch/mouse routing. Exact Main source identity and
frame/context/target completion gate this path; unknown owners retain 4:3. Root
added the shared C ABI implementation to Apple platform and renderer test targets.
Generation/build/wide rendering/input acceptance are still pending. Other menus
and gameplay world/HUD aspect remain in scope and are not implemented by this
Main-only stage. The prior accepted artwork binary remains preserved.

All 36 original technical picture references are now exported with exact source
identity, placement and per-picture semantic guides. Thirty-three prior exports
were pixel-verified; three missing Mustang references were decoded. Eight source
pictures have private draft remasters. None replaces the original technical
pictures yet: ambiguous wheel marks, selector details and engine topology need
source comparison, not invented symbols or generic mechanical components.

## First responsive Main batch compiles; wide runtime still pending

Fresh source generation and the grouped local build pass with two compiler
workers/two ThinLTO jobs. Binary SHA256 55e7402ef7db66a215476c7205ed4fdc6ee974d776793be21a027ca246f4c047.
The original guest operations and renderer body are preserved; the actual shared
input module is compiled, all 3497 source/tool pins remain stable, and the prior
accepted artwork binary is untouched. The first startup attempt stopped before
generation because its tool check assumed an unused linker; existing GNU/LLVM
tools are now correctly pinned. No installation or system change was needed.

The actual compiler dependency inventory has 96085 admissions but2094 unique
files. A separate deduplicated validator produced exactly the same classification
as the original completed check; use that optimization for later recipe versions
without dropping source-root/hash coverage or recompiling unchanged app code.
The current finished build used its original frozen validator unchanged.

No actual wide frame/input acceptance is claimed yet. The held runtime recipe
will resize the owned game window to16:9/iPad/4:3, verify full-window output and
matching pointer/anchor/fallback behavior. Other menus and world/HUD aspect remain
in scope. Private historical generated snapshots remain exact evidence alongside
new current-source/output pins; they must not be relabeled as current compilation.

## First actual wide-window observation rejects fallback correctly

The sealed local run opened a genuine1280x720 game window. All16 bounded
readiness captures remained the original960x720 presentation with160black
columns at each side. Original Main published30complete native backgrounds,
but no responsive presentation was witnessed. This is a failed first-stage
wide test, not a widescreen milestone or an oracle-threshold issue.
Shutdown, copied configuration/artwork,49save hashes,5718input pins and native
resource retirement all verify; no owned child remains. Preserve that evidence.
Add bounded source/preparation/tail/cancellation/presentation refusal diagnostics
to identify the exact rejecting guard. Do not loosen ownership, matrix, texture,
geometry or cursor checks merely to make the wide path publish. The working
4:3 fallback and earlier verified artwork build remain intact.

## Responsive Main refusal localized in an actual wide-window run

The diagnostic incremental build passed with exactly two changed objects and 368
unchanged objects; build manifest and rules stayed identical. The short local
run captured six genuine 1280x720 frames. Source Main recognition, wide background
preparation, the original native tile pass and recognized font replay all pass.
A later foreground draw is rejected at tail stage 3 / reason 6: unrecognized
texture (runtime-local ID 630). That cancellation explains the continuing 4:3
fallback; this is diagnostic progress, not accepted widescreen presentation.
Trace that draw to its actual source role before adding narrow handling. Never
whitelist arbitrary texture IDs. Shutdown, all 49 saves, configuration, artwork,
6167 sealed inputs and balanced native resources verify; no owned child remains.
The prepared owner implementation map groups shared cached menus first, followed
by Records and information/results screens. Child controls cross the Main sidebar
boundary and require source-bound widget mapping rather than reuse of its gate.

## Original opponent-class fallback: source descriptor correction pending

The A5-only incremental build passed with three changed objects and 367 reused
objects; original engine/renderer operations and build rules remain preserved.
Its corrected launcher captured six genuine wide-window frames, still with zero
responsive presentations. Actual original A5 selection is native address 1992704,
texture ID 630 in this run. The new source snapshot reports zero because the Main
slot is a guest descriptor pointer, not the native address: the original producer
and consumer put/read that address at descriptor offset 0x14. Resolve that field
inside the same checked transaction and represent observation validity explicitly;
zero native addresses are valid. Keep all content/profile/context/sampler checks.
The first attempt stopped before wide resize because a nested capture helper was
omitted from the launcher copy; preserve that failed receipt. The complete retry
verifies clean shutdown, all 49 saves, configuration/artwork restoration, 6600
sealed inputs, balanced native resources and zero remaining owned children.
No physical widescreen claim or new Apple package follows from these diagnostics.

The exact artwork inventory also confirms three missing core A/B/C remasters.
Historical tier candidates are not active and their former role numbers now belong
to car logos. Reserve new roles 24 through 26; keep paired alternate states,
class-photo collages and knockout artwork as separate source identities. The first
AI A-class draft and its one correction remain unactivated for proportion/identity
review. Native-size composition takes priority over repeating speculative redraws.

## Native badge canvas extent is the next Main-wide rejection

The descriptor correction builds successfully and the full-route observation now
accepts the original A5 material with matching source address and explicit valid
observation. It then rejects source corner x638/y534 (z1/q1) at the generic UI
vertex range. The original badge uses a full 128-square canvas, including padding
below the visible 480-high viewport. Validate that recognized source canvas and
its clipping explicitly, including future authenticated HD tier bindings; do not
crop it or enlarge the generic unknown/font/cursor allowance. The route stopped
at its first wide-frame gate, so later pointer/iPad/fallback stages remain untested.
Clean shutdown, 49 saves, config/artwork, all 6196 inputs and owned-process cleanup
verify. Preserve the failed route; no widescreen acceptance is claimed yet.

## First genuine full-width Main output and remaining input/art gaps

The padded-canvas correction produced actual 1280x720 responsive presentations:
two fresh physical-swap witnesses, matching full-window layout/map/brand oracles,
initial cursor recognition and a blank-wide click that leaves Main open. Source
binary 7fc4f898... is the evidence for that limited milestone. The full interaction
route still fails in its private mouse driver: a raw relative delta was treated as
an absolute OS destination, then a retry incorrectly assumed periodic recentering.
All failed runs exit cleanly and preserve the copied 49 saves/config/artwork.
Do not call the full Main route or iPad ratio accepted from those first frames.

The actual wide image exposes an artwork gap: original and earlier authored map
layers retain the foreground sheet's curved occlusion/comb shadows. Separate
unoccluded material drafts now exist; none is activated. Genuine wide presentation
does not establish visually complete widescreen backgrounds.

Source review also proves the old cursor warp heuristic can reject a valid queued
input prefix. The grouped four-object build now passes with exact consumed FIFO
receipts, original-reader completion/menu-write observation and explicit original
warp/flush handling, plus app-local desktop relative capture. Current binary
15f68701... preserves the renderer and all original guest operations; 366 objects
are reused. It is not runtime accepted yet. Keep all native shape/source guards,
full pixel/cursor/click thresholds and save/config cleanup for the next grouped run.

## Main responsive layout and real input now pass the grouped local route

Binary 15f68701... passes the full Main-only route in the default original A5
fixture: actual1280x720 and1180x820 presentations, unchanged landmark/glyph pixel
thresholds, uniform height scaling, right-anchored map/branding, actual cursor
recognition with multi-event relative delivery, blank-wide click rejection,
source-checked Options click, centered Options fallback, new-owner Main reentry
and final1024x768 fallback. Six actual physical-swap witnesses cover the three
owner/size tuples. Desktop relative mode is actually enabled while focused and
restored on exit. All49 saves, config/artwork,6204inputs and exact owned-process
cleanup pass. No new compiler or renderer change was needed during this route.

This accepts the EXERCISED Main layout/input scope, not finished widescreen
backgrounds or all configurations. The central artwork mask gap remains until
an unoccluded map is registered/activated. Other Main branding profiles, other
menus, world/HUD aspect, same-owner4:3-to-wide return and physical iPad acceptance
remain open. No performance or new-IPA claim follows from the WSL route.

## Unoccluded Main map registered; activation pending composition review

The private hd-unoccluded-map-registered-pack-v1 candidate registers clean-v2
(master SHA73055235...) uniformly to the existing2048x1536 map roles0/1.
Raw pack SHA2e8f9f97... remains103492520bytes; the new owner template is
16354749bytes, SHA8b217f6e.... All51 other section payloads, original raw table,
catalog records and24owner-hole offsets remain byte-exact; owner reconstruction
roundtrips to the candidate raw digest. Role0 uses an explicitly approximate
source-fitted exposure grade. This is private prepared artwork, not activation,
runtime visual acceptance, native iPad proof or a new delivered build. Review
composition with the existing foreground at4:3,16:9 and iPad ratio before the
next grouped pin/source integration. The increased compressed size also remains
part of the unresolved final personal-package storage budget.

Setup/Options source now exists in private hd-responsive-cached-menu-staged-v1:
typed owners, measured original widget lifecycle, immutable packet snapshots,
shared responsive renderer and inverse pointer mapping. Parent consistency and
independent pointer review are in progress; canonical files are unchanged by
that staging. No child-menu runtime acceptance is claimed.

Monolithic gold badge v2 corrects the upright transparent opening and retains
the filled base. Its exact prompt/reference hashes and output SHA9176774c... are
saved in MON-V2.json. Visible colored edge fringe still holds it from activation.

## Reviewed background and cached-menu source batch integrated

The clean map candidate passes independent byte/reconstruction checks and offline
composition review with the exact active foreground alpha at4:3,16:9 and1180:820.
All three have zero geometry gaps or partial black-clear contribution; the baked
arc/comb ghosts are removed. These are illustrative composites, not game frames.
Root applied the14reviewed Setup/Options files plus3exact artwork pin files as one
reversible17file canonical batch. Activation receipt829dfd3d... retains every
prior source and old resources; protected docs and public layout ABI remain exact.
One grouped build is being prepared with the new private template path and raw
pin. Generated engine/renderer output, child-menu runtime and physical iPad proof
are still pending. No cloud build, upload, commit or push follows from this batch.


## Grouped responsive build passed; actual Main background works, child proof gap located

Root build08f0e8ac... passes the dependency-derived ten-object compile with360
objects reused; four required recompiles reproduce identical object bytes. All
original engine/renderer operations preserve exact reversal proofs. Dependencies
cover96140admissions/2102unique files, uncovered0/missing0; old15f remains saved.
Actual1280x720 Main frames now show the clean map without the old exposed mask.
The first route rejected the original Setup-hover badge brightness; source proof
confirms selected white tint versus unselected129/84/18 and normal mode captions.
A fresh scoped route preserves all unaffected thresholds and reaches Setup.

Actual Setup's cached original native pass works, but the wide stage correctly
refuses original A5 branding: selected native texture630/address1992704 is known,
while the child source has slot5Valid0. Main-only40DC40 observation does not supply
that child metadata. Capture the exact checked slot5 descriptor with the original
widget transaction; do not weaken the renderer guard. Both terminal routes restore
all49 saves/config/artwork, retain all sealed pins, balance resources and leave
zero owned children. Child wide acceptance, other-menu/gameplay aspect and a new
physical iPad package remain unproven. No cloud build or publication occurred.


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


6 November â€” Autonomous continuation / shared footer loading

User is unavailable and explicitly authorizes routine decisions without waiting
for approval. Complete all UI/artwork/integration before one grouped iPad build;
then resume performance/battery work automatically. No cloud build this pass.
Photo review is now63/98 after hash-verified F50, Isdera and LotusGT1 selections.
Cala, Esprit, Nazca and remaining Mustang/bonus review continue in parallel;
whole-artwork approval remains zero until deterministic branding is complete.

SlideshowFooterResource.h now loads only a bounded pinned resource, with exact
header/file size, contiguous chunk ranges, compressed/pixel hashes, PMA checking,
immutable identity routing and decoded-memory budgets. Optional generated private
SlideshowFooterData.h hooks it to startup; no private catalog is installed yet.
One small ASan/UBSan resource check passes valid load and corruption/trailing
bytes/budget/offset/duplicate-binding rejection. This is component evidence,
not game, linked app or iPad validation. Evidence: hd-slideshow-footer-resource-local.json.
Original assets unchanged; no runtime pack regenerated, public upload or push.


6 November â€” All98 slideshow photographs source-reviewed

Hash-verified selections from Cala/Esprit/Nazca/Mustang and three bonus cars are
merged into the private draft/correction manifests. All98 photographs now have
source-based semantic review; complete-artwork gates remain false. Material
Esprit corrections restore source-dark driver silhouettes rather than invented
recognizable faces. Tombstone invented sponsor wording, GT90photo6 Ford oval,
other small body/scene marks and all shared footers still require faithful
identity work. This milestone does not accept those details or certify iPad UI.

The reusable F50 footer is authored/rendered privately with unchanged genuine
Ferrari vector badge and outlined model caption. Both Ferrari footer candidates
have local source/layout review. A real Ferrari355 resource packs301187bytes and
passes native ASan/UBSan loading with eight exact original source routes and one
2048x1536 immutable PMA canvas. Generic explicit-input private pack authoring and
optional generated header staging are now implemented. No full app rebuilt,
Apple/cloud build, runtime pack regeneration, push or public artwork upload.
Jaguar/McLaren/Ford footer authors are active; next integrate finalized all-car
footers, resolve brand details, complete remaining UI and responsive coverage.


6 November â€” Shared footer artwork assembly

Jaguar and McLaren native footer previews inspected against original layouts;
period vector brand paths preserved. Four shared footers (two Ferrari, Jaguar,
McLaren) packed privately with32 paired original identities:1,272,371bytes,
50,331,648 decoded bytes. Not installed in a game/app yet. McLaren all8 native
coverage receipts prove generated footer marks lie below the opaque new panel;
no extra photo generation was needed. Jaguar uses its existing clean plates.

Three bonus model footers authored as editable SVG and rendered2048x1536PMA,
with source-inspired color, shallow beveled hexagonal plaque and scalable UI
caption outlines. Captions are new typography, not authenticated original brand
letter geometry. Source/layout previews inspected; Tombstone's invented on-car
sponsor lettering remains unresolved and not accepted. Seven footer candidates
now exist; remaining car footer authors continue. No Apple build, public upload,
full-game rebuild, final runtime-pack regeneration or original-data modification.
Evidence: hd-slideshow-four-footer-resource.json and
hd-slideshow-footer-artwork-checkpoint.json.


6 November â€” All15 footer candidates assembled

Native source-reviewed footer candidates now cover all15 car/bonus families.
A private resource binds all98 exact original identities, contains5,321,403bytes
compressed and188,743,680 decoded PMA bytes, within the native256MiB hard budget.
This is resource preparation only: no game/app build or device acceptance.
Ford/Lotus source previews inspected; Ferrari16 and Nazca/bonus11 selected-photo
coverage composites inspected with no apparent old footer spill. Their exact
selected photo/footer hashes are recorded privately, not whole-artwork approval.

Nazca uses the genuine official1987 Italdesign1024x462 embedded logo image,
with unchanged geometry mapped to white coverage. Mustang's visibly low-res
background patch was resolved by finding the already saved official Ford
Heritage Vault brochure; its original4137x3323 page contains matching cursive
Mustang and narrow machIII ink. Nativev2 maps those authentic white/red shapes
to blue without an invented font, AI redraw or source-background patch. Preview
inspected; earlier fallback remains saved for rollback/provenance.

Lotus16 upper-rim ghosts need clean photo plates (in progress). GT90photo6 body
badge correction and Tombstone sponsor fidelity are in progress. A per-image
body/footer variant may require a count bound above15 while keeping the256MiB
memory limit; don't drop the correction to fit an arbitrary family-only count.
Remaining broad main-background/UI review is delegated, preserving current
assets. No cloud build, full app rebuild, public upload, push or original-data
modification. Final reviewed photograph pack remains deliberately unregenerated.
Evidence: hd-slideshow-all-car-footer-resource.json.


6 November â€” Clean photo inputs and actual-selection correction

Selected8 Jaguar +16 Lotus source-reviewed clean plates as authoritative photo
inputs, with previous draft names/hashes preserved. Private assembler accepts
confined relative PNG subdirectories (no absolute paths/traversal/outside-root
resolution). No final photo pack regenerated. GT90photo6 combined authentic
Ford sill/footer variant hash is now required by its private selection metadata.
Catalog/packer support up to98 per-slide variants while retaining256MiB decoded
budget and98 binding limit; no correction is dropped to fit15 family slots.

One grouped real-resource ASan/UBSan component load passes all15 shared footer
images,98 exact source lookups and15 retained immutable owners; decoded180MiB.
This does not establish game or device rendering. Final variants are not yet in
that resource. See hd-slideshow-all-car-footer-native-load.json.

Background audit caught actual selection drift: current OwnedArtworkRecipe and
personal-IPA template agree with old v5, despite earlier notes describing clean2
as applied. Current files take precedence. Worker is finishing one corrected
main background with matched private activation inputs, preserving all versions.
GT1 showroom and Tombstone sponsor corrections continue. Updated the top-level
HD_UI_PLAN current checkpoint so historical notes do not conceal remaining work.
No full-game/Apple build, public upload, push or original-data modification.


6 November â€” Background matched selection and lossless photo transport

Inspected corrected main-background16:9/iPad offline composites and applied all
four prepared pin changes only after exact current/candidate hash preflight.
Previous source copies are preserved privately for rollback. New private raw,
classic template and wide resource hashes match their selected settings; other
51 raw sections and both wide panel records remain unchanged. Updated personal
IPA input metadata; its old fixtures explicitly apply to prior pins, not new
resources. This is next-build source selection, not a game/device result.

Tombstone's two invented wordings now require the source-backed local livery/
footer variant in private selection metadata. GT1 showroom correction continues.
All98 base photo inputs are otherwise finalized and the one fresh lossless pack
assembly is running under a confirmed local process handle; no app/cloud build.

The existing raw-zlib photo resource plus other artwork exceeds the512MiB IPA
budget. Added version3 lossless RGBA8 PNG method0 row-filter payloads with trusted
dimensions/codec/raw digest; raw version2 and footer resources retain compatibility.
One independent grouped native Clang ASan/UBSan check passes24 meaningful cases:
real Pillow PNG IDAT, all5 filters, boundaries/modulo/Paeth ties, old aggregates,
malformed/trailing/truncated payload and version/predictor rejection. Actual
source hashes match the checked headers. Package-size improvement is not claimed
until the complete98-photo assembly finishes. No resolution/pixel quality loss.
Evidence: hd-main-background-matched-selection.json and
hd-slideshow-lossless-row-filter-local.json.


6 November â€” Complete slideshow resource preparation / remaining technical art

The full98-photo version3 resource measures240,423,232bytes versus the previous
337,028,055 raw-zlib resource:96,604,823bytes saved without changing decoded RGBA.
The showroom wall correction supplied a new cleaned base; selected it with its
required combined wall/footer layer. A hash-verified reuse path copied97 existing
compressed image chunks and encoded only that changed photo. Resources remain
explicit previews with zero whole-artwork accepted flags, not shipped assets.

Final layer resource now includes17 unique native canvases (15 car families plus
source-specific variants),98 exact source routes,6,229,762 compressed bytes and
213,909,504 decoded bytes. GT90 sill, Tombstone two invented wordings and GT1
wall marks are all selected with exact corrected-photo/layer pairing. Small
original-source lettering and reconstructed ordinary showroom strip typography
retain documented limits; no unsupported exact-brand claim. Current artwork
subtotal469,197,005bytes leaves67,673,907bytes below the512MiB IPA limit for its
actual future base; remeasure after remaining technical image updates. No ARM64
base or installable IPA is produced by this work.

Broader selected-input audit found all36 CarInfo technical remasters exist but
current pack54549f... still contains semantic holds: F355 selector symbols,
McLaren invented controls and other inset details. Stale inventory8/24 is not
current evidence. All36 are now partitioned across3 workers for actual source
comparison and bounded correction; no shared pins/pack changes until coherent
reviewed set. This is remaining requested UI artwork, not extra test work.
No cloud build, app build, push or public artwork upload. Next after completed
assets: grouped native integration/aspect-route check, then matching Apple IPA;
resume performance/battery without waiting for user feedback.
Evidence: hd-slideshow-lossless-resource-assembly.json,
hd-slideshow-final-layer-resource.json, hd-main-background-matched-selection.json.


6 November â€” Personal IPA slideshow wiring and aspect boundary

Added the two completed slideshow resources to existing authenticated personal
IPA injection, retaining exact resource digests, archive name/CRC/MachO checks,
combined size reserve, executable/member-storage preservation and new-output
requirement. New --slideshow and --slideshow-footer inputs must appear together.
All10 selected resource sizes are recorded; no matching ARM64 base or IPA was
created. Existing8-resource fixture evidence does not validate this10-resource
path; run one grouped packaging check after the final CarInfo update.

Source trace confirms default keepAspectRatio=1 and generated presentation fits
640x480 into the actual drawable without cover/crop. Main's validated responsive
output is separately allowed full drawable. Native-aspect world projection/HUD/
culling stage remains inactive and is not claimed by that containment behavior.
Current picture fit therefore has a source boundary, not new runtime evidence.

Prepared background source export introduced doubledCR line endings in3files;
normalized those only and recorded post-normalization source hashes. No artwork
settings changed by normalization. Three workers continue all36 selected CarInfo
technical comparisons/corrections, preserving prior versions and source references.
No full app/Apple build, public upload, push or original-data modification.
Evidence: hd-slideshow-personal-ipa-wiring.json and
hd-ui-current-aspect-boundary.json.


6 November â€” Technical-image cleanup progress

First9-image group complete and exact selected-file hashes checked: McLaren,
FerrariF50 and Ferrari355F1 technical panels. Four reviewed corrections remove
invented McLaren service controls/silver control plate, restore the F355 AUTO
selector without guessed lower-symbol lettering, and remove unsupported F50
plenum emblem. Previous candidates remain saved. Other two groups cover the
remaining27 technical images. No shared CarInfo pack/pin update until coherent
all36 selection is ready. Further bounded work reuses genuine period Jaguar,
Mustang and Isdera branding for any concrete current CarInfo identity holds.
Broader asset inventory confirms Intro, TrackInfo, CarInfo and Results content
sets exist; missing-draft counts from earlier notes were stale. Actual runtime,
transitions and screen-ratio coverage are still for one grouped integration pass.
No Apple/full-app build or public upload. Evidence:
hd-carinfo-technical-group-c-selection.json.


6 November â€” Source-reviewed artwork batch assembled for grouped integration

All36 CarInfo technical selections merged by original sourceIndex, with six
corrected native insets and one genuine Mustang background title. Preserve41
unchanged image records byte-for-byte and all48 original layout/focal records.
Three subpixel transparent fit edges were caught before pin selection and given
opaque panel mattes without cropping the photos or changing the original gutter.
Provisional export/pack remains saved; final resource113,527,409bytes/payload
251,652,104, SHA4f21c6d024c0d370ea30299daacb39ce5a11c50756d8d7dc41b30134e2436f1c.
Current loader/provider/publicpin native check passes36car/tab pairs, sourceIDs,
1444x572 dimensions, transparent divider, adjacent opacity, borrow release/clear.
Source pins and personal injection metadata now select that coherent resource.

Final slideshow composition review passes98/98 with zero observed material
issues. A suspected photo7 badge was a thumbnail misreading; full-resolution
selected picture already has correct Ford script, so retain17layers, no new edit.
Source-review gates cleared only after all frozen photo/footer hashes matched;
user/device/live-runtime fields remain false. Final version3 catalog accepts all98
reviewed artworks; its240,423,232-byte payload reuses98existing chunks unchanged.

All10 resources staged as a matched verified input set,468,289,521bytes. Added
CMake optional companion copies with exact digest checks and mandatory photo/
overlay pairing; deferred public bases still omit these artwork files. Nine
other required private headers need the concrete generation/build recipe audit
before grouped local source preparation. No cloud build, app build, public upload,
push or original game-data change. Next: one matched local renderer/game build,
actual route/aspect checks, then one clean matching Apple package. Performance/
battery work resumes after the UI package without waiting for a user reply.
Evidence: hd-carinfo-coherent-reviewed-resource.json,
hd-final-ui-matched-inputs.json and hd-slideshow-lossless-resource-assembly.json.

### Grouped local UI runtime checkpoint
The isolated Release app built successfully (339 seconds) with the matched ten-resource set. Actual virtual-display captures confirm the remastered Main at 1280x720 and 1180x820 and the Player screen; clean application exit. The first scripted mouse route did not enter Showcase, so slideshow and CarInfo runtime acceptance remain open. A keyboard-driven route is being checked. This is software rendering with dummy audio, not iPad performance or physical-device acceptance. No cloud build or publication was started.

### Showcase runtime and publication checkpoint
Actual local ordinary-input Showcase, technical-page change, Slideshow and next-slide/next-car routes run with the selected resources and clean exits. The Ford information-page mark rotates through front, edge and reversed backface; it is not an incorrectly mirrored static replacement. Source privacy and independent staged secret scans pass, and the real tracked metadata CI approval passes after the new notice uses Markdown. No push/cloud job. History has HD text but its photographic collage remains original; investigate this coverage gap before claiming the complete artwork batch.

### Seven manufacturer histories and grouped local completion checkpoint
Seven previously original history collages now have source-referenced remasters, with42finite localized transparent headings. Source-authenticated type8 history module preserves original locale behavior and live text, proves complete original-plane replay and300tile/600triangle native publication. One grouped local build reused unchanged objects and completed in127.77seconds. Ordinary keyboard cycling visibly reaches all seven manufacturers and returns to menus with clean shutdown. NativeASan/UBSan accepts all42family/title pairs and rejects invalid pairs; leases survive clearing. The11resource packaging fixture passes,494,757,492artwork bytes under the unchanged512MiBfinished-package bound; no real newIPA yet. Italdesign Calà accent glyph is under review.
The prior remote Apple attempt was cancelled before any step due to ARMrunner capacity, not a code error. A documented standardfree IntelMac route with pinned Xcode16.4 is prepared, still compilingARM64devicecode. A separate optional unpublished base-draft transfer avoids unverified accountwide Actions/Packages storage and strips supportfiles/artwork; current public-release refusal stays default. No new cloud dispatch/push/finishedpackage or physicalacceptance is claimed.

Final history accent checkpoint: exactLatin1byte224 nowuses genuineLeagueGothicgravecoverage with the approvedvowelbody; allotherglyphdescriptors, advances, sourcebytes andmetrics stayunchanged. FocusedASan/UBSan andactualItaldesignhistorypage confirm Calà withouttofu; finalincrementalappbuild andcleanshutdown pass. All16existingLinuxfastsource/policychecks and6newunpublishedtransfertests plus5publicreleaseregressiontests pass. An independentread-only review found no must-fix issue in the drafttransfer. No newApplejob orpublicpush yet.

### Observed debug-link failure and exact local correction
Run42163506478 failed in the LinuxGLESdebug engine_startup_check link before anyApplejob ran. Its exactmissing symbol was WideMenuArtwork::dimensions: aC++14odr-used staticconstexprarray lackedseparate storage, maskedbytheoptimized localReleasebuild. An isolatedtwo-unit C++14-O0ASan/UBSan link reproduces theoldfailure. Thefixed inlineaccessor defines immutable functionlocalstorage; the samefixture linksandruns bothactual1180x820/1280x720privateart layouts, withunchanged fileSHA, and rejects out-of-range layers. Noartwork geometry orpayload changed. A newsourcevalidationrun isjustified by thisspecificfix; do not rerun thefailedrevision. BundleVersion advances22to23 in the same necessarybuild. Physicalacceptance remainsopen.

### Build23 delivered; performance phase resumes
Corrected source35ce32851f8273d3842420f66d15996372caf4ec passed run42165885240: LinuxGLES/regression gates and standardmacos-15-intel nativeARM64Apple build. The real unsignedbase was downloaded withserverdigest verification; all11personalresources were injectedlocally with originalnativecode/memberbytes/compressedstorage unchanged. PrivateZIP507,938,966bytes(SHA256c0ff35c13694c1e8ac0a1e721ea7cde634cea1fab81e7dcd96e9277be3ead4d1)was uploadedandserververified ONLYin theoriginalprivate deliveryrepository. Existingtag/olderassets preserved, avoidingits legacyunfilteredpush trigger; no newprivateGitref orbuilddispatch. TemporarypublicbaseDraft deletedafterdelivery. OriginalgameDataexcluded, unsignedpersonalSideStoresigningrequired, physicalacceptanceopen.
No userresponse isneededtoday. Next implementopt-indesktopdiagnostic recording foractualownerCPU/sampleIDs andboundedframeintervals, provecurrentNorthCountry/two-playerroute, thenrunoneunsampledbaseline beforetheboundedcopyspanpilot. Preparation/sourcepins/controls frozenprivatelyunder .build/hd-lpm-next-performance-v1. Do not use stale standardReleasecache or nonexistentprivate-mainenvironmentflags. UIbranchstaysseparate; commonperformancepatches mustremaincherry-pickable to main.

### Local performance recording checkpoint
The opt-in desktop recorder and bounded presentation stream build locally and focused checks pass. One actual copied-game/native-two-pad menu session exits cleanly with397parsed samples,4312sequential frame records, real engineCPU sampleIDs, zero ring drops and no writer failure. Unsupported desktop battery/power/thermal data is null. This is software rendering with dummy audio, not a race benchmark or iPad energy result. Current HD menu input routing still needs diagnosis before selecting split-screen settings and driving both players. Independent review found an edge-case truncation undercount; final bounded drain now accounts all1024rows under frame-cap/backlog tests. Corrected incremental Release app link and focused CTest pass. No cloud dispatch or new public/private delivery.

### Startup allocation optimization verified separately from race performance
CarInfo now transfers its decoded allocation into the immutable provider rather than copying251,649,792pixelbytes. The public C installer still copies arbitrary caller data; owned offsets are bounded before access and publication, all48image hashes/metadata remain exact, all36actual provider pairs pass, retained leases survive clear/reinstall, and focused address/undefined sanitizers pass. One matched fresh-process Linux component comparison records605604KiB old versus359772KiB new peakRSS, a245832KiB reduction. This is component startup evidence, not app/iPad RAM, battery or raceFPS. Both CarInfo and History release verified compressed buffers after their inflater has completed; actual49history resources/42pairs and retained-lifetime sanitizer checks pass. No new app/cloud build for this batch. Synthetic ownership regression is retained in the existing Linux gates for the next necessary grouped validation.

### Current two-native-player race timing reference recorded
The copied-data run now source/visually verifies NorthCountry, split-screen, fullgrid, FordIndigo and NazcaC2; both move after original native shift-up engages first gear. A nominal60s recording produced an actual65.607556s SDL marker span (end acknowledgement delay); analysis uses actual markers. There were2229swap submissions,2228complete intervals,36fresh unique ownerCPU windows after boundary exclusion, zero lost records and clean shutdown. OwnerCPU mean13.5557ms/window-per-submission and p9519.9194ms are Linux reference values, not iPad/LPM/battery results. Both cars use manualfirstgear, neutralsteering/fullthrottle and wallcontact; this is not matched rendered-work or a completedrace. Actualsession443.725s includes finishing the in-flight measurement beyond the nominal420s command-loop deadline, disclosed privately; no further driving run was started.
The startup ownership regression also passes its retained CMake/CTest target. Next priority is a private exactstaticDATA/BSS destination-only bounded8word copy pilot for4326E0. The older heap revision-cache shortcut is not being promoted: its equivalence to live per-word resolution under concurrent retirement lacks proof. Keep heap accesses checked, preserve old resolver as independent control, and require semantic/cancellation/alias/fault fidelity plus a measured component gate before any Game candidate build. No cloud dispatch or physical FPS/energy improvement is claimed.

### Bounded CPU pilots: integration proof and a failed performance gate
The private static-destination-only eight-word pilot passes4320optimized+4320ASan/UBSan semantic cases,3960matching deterministic cancellation checkpoints, and a12.8579%median kernelCPU reduction across6alternating pairs. Independent review found no must-fix within its scoped proof. One isolated Game candidate changes exactly3sites in4326E0, retains all baselineobjects/libs and realbackend, and runs the verified two-native-player scenario with clean exit and7554lossless frame records. Its timing window did not fit the nominal420sloop limit; actualrecorder428.923s includes current-command/capture/cleanup. It is unpromoted and establishes no wholeGamegain.
A separate checked DATA/BSS resolver fastpath preserves all optimized+sanitizer oracle outputs but fails the predeclared performance gate: DATA4.50%, mixedheap-to-DATA8.82% versus required10% (BSS15.71%; unchangedheapnegative2.49%noise). Keep this candidate private; do not rerun unchanged timing or build it into a Game. Existing top functions already have extensive transaction batching and HeapArena already has owned-chunk/TLSspan paths; inventing another cache without current attribution is not justified.
Next collect a bounded current CPU profile using the already-installed paused gprofng collector and unchanged baseline, with no new app objects or OS changes. A planned600stotal allowance accounts for observed~420sUIsetup plus a30srace-only sample and clean shutdown. Identify Gameowner versus Mesa software-renderer work and stack-coverage limitations. Separately inspect the delivered ARM64 code for actual TLS/stop-check behavior; do not assume Linux cost predicts Apple behavior. No new cloud build, shipping CPU change, physical FPS gain or battery saving is claimed.


### Apple thread-local code-generation check; engine headers unchanged
One isolated manual compiler job on standard macos-15-intel/Xcode16.4 passes at source c8370f5 (run42281326046). It compiles only three small ARM64 iOS functions for old and private address-cache headers: no app/link/cache/artifact/release, and all app jobs are skipped. The tested accessor removes3indirect TLS address lookups while retaining its live atomic stop load. Old transform/REP already hoist address calls, so these assembly counts do not imply3calls per runtime word. Source and independent semantic/actual-thread cancellation review remain necessary alongside assembly; static cleanup markers alone are insufficient. Canonical engine headers stay unchanged and no FPS/energy improvement is claimed.
A smaller alternative changes only getter storage to one constant-initialized POD TLS aggregate while preserving the existing16byte transaction layout. Private12288checkpoint comparisons and3actual-thread modes match OLD in optimized and ASan/UBSan builds. Independent review finds no semantic must-fix; consistent whole-consumer rebuild, multi-TU identity and independent-worker checks remain integration gates. Compare actual Apple code generation before choosing either design; do not promote both speculative variants.
The first bounded current gprofng attempt never resumed, produced zero CPU samples and supports no hotspot conclusion. Continuous semantic routing and owned-process cleanup are being reviewed privately before one changed retry; retain this negative evidence. Evidence: apple-stop-slot-assembly-local.json.


### Build23 physical UI regression report changes immediate priority
The user reports original splash and original backgrounds during Main/child swipes, delayed HD activation, persistent Cars/Opponents original backgrounds/logo/class pictures, Location pillarboxing, Setup game-type-dependent width/jerky cursor and a whitish Main left-side line. This contradicts full first-frame/whole-menu UI acceptance; existing stable local captures did not cover those conditions. Immediate priority is complete authenticated replacement from the first visible frame through transitions/returns, not a shorter delay or hiding it. Performance profiling is deferred. Work isolated on codex/ui-activation; no new Apple app build until coherent fixes and grouped local acceptance. Use actual ClassicScreens/RendererHooks.inc selected by prepare_renderer, not the legacy renderer. Location responsive enrollment is absent, while Player already has a separate compositor; inspect its real failure chain rather than adding a duplicate. Preserve all source/lifetime/context guards. Evidence: build23-device-ui-regressions.json.


### First-frame UI activation: local acceptance checkpoint
The physical Build23 report drove source fixes for the distinct original render
paths, not a shorter loading delay. Main now binds its original archive at the
correct post-load boundary. Fixed child initial routes admit the original cold
zero previous-form value while retaining exact source, owner, target and pixel
checks. Both initial and refresh full-cache copies now produce their own typed
proofs; the refresh had replaced the first cache before any draw consumed it.

The grouped changed app compiles as C++14/C11 and retains all9471transactions,
12reviewed word loops and2complete copy helpers. Its local run records674first
full-menu replacements with300original texture selections and600triangles each.
All14saved Setup-entry images retain the HD panel/map through first widget
appearance; the prior old-background interval is absent from these samples.
Main, Player One, Location, Setup, Opponents and Options were observed with authored
responsive menus. The additional Modem setup mode now fills the drawable using
an independently fingerprinted original badge; unknown textures still reject.

Native static-title publication is observed at2186x1640 attachment sampling in a
2360x1640 drawable, with the authored photo/logo/copyright visible. All three large
class collages are observed after positively verified source selectors and exact
rendered artwork digests. Original visible305x144 coverage is retained separately
from the independently authenticated306x146logical canvas, so its transparent
borders no longer reject the1224x584remaster. The panel's fractional left join has
also been corrected and visually/quantitatively checked.

The session exited cleanly; its ELF,12resources, copied immutable game assets and
restored profile verify unchanged. A worktree publication-policy audit passes.
Original game data and private artwork remain outside Git. No new Apple app build,
remote push or release was started during this local UI investigation.

These are sampled desktop results, not physical iPad acceptance or proof of every
unsaved frame. Native intro videos and Modem/link/network badge remastering remain
separate asset work. Initial transitions retain native4:3staging before the steady
responsive surface; completing their wide presentation requires its own source
underlay capability. Next delivery needs source/privacy gates and a matching Apple
package. Battery/LPM performance work remains active after the UI delivery boundary.
Evidence: ui-activation-local-acceptance.json and ui-activation-source-checkpoint.json.

Shared swipe geometry now compiles as strict C11 and drives the native transition at its unchanged 640x480 aspect. An independent comparison covered 50,886 motion plans across native, 16:9 and iPad-like dimensions. Full-width GPU composition and source-command underlay remain pending; this source change has not been included in an app binary or device test.

### Wide transition integration checkpoint

The combined local v10 engine/renderer build passed, with source pins unchanged and both focused geometry and Main-origin lifetime regressions passing. Fresh wide map/full-Main/first-sidebar source paths now compile, using semantic map replay and exact host lifetime/paint joins; no prior framebuffer image is used as their source. Their actual wide GPU publication has not yet been observed.

The targeted two-aspect Setup observation exited cleanly and preserved the executable, all 12 resources, copied original assets and profile. Reviewed entry-hold samples show HD artwork but still have side bars: 160 columns each side at 1280x720, and 86/87 at 2360x1640. This makes the next implementation target the exact initial-full cached frame (kinds 8/9/10), with its own captions/previews, before completing remaining sidebar and PlayerIntro command models. No new iPad package or device acceptance is claimed. No cloud app build or publication occurred.

### First complete Setup frame: actual wide publication

The v13 renderer links against the unchanged qualified v12 engine ABI. A fixed 2360x1640 observer positively recognized the source Main menu before keyboard navigation and retained that size through Setup entry. The first accepted native kind-8 pass (serial 169) also published the first wide kind-8 surface; 11 wide publications were observed. Reviewed complete Setup samples have HD background/heading plus actual buttons, text and cursor, with zero side-bar columns. Earlier opening-sidebar samples remain in native 4:3 geometry while showing HD artwork.

This run exposed and corrected a missing nonzero framebuffer in the context-only save/restore guard. Source and draw ownership checks remain intact. All app/resource/copied-asset hashes and copied profile restoration passed, and the app exited cleanly. Location/Opponents full-frame runtime coverage, source-backed opening swipes and independent car-intro expansion remain pending. No new Apple package, publication, physical-device or battery-performance acceptance is claimed. See evidence/ui-first-full-wide-runtime.json.

### Opening map/sidebar handoff: local correction

The sidebar carried a placeholder map role, rejecting the otherwise exact destination/domain match. It now borrows the unique current map receipt and preserves the producer role. Stale, foreign and duplicate matches still reject; C11 and sanitizer cases pass.

The v17 run exits cleanly with app/resource/copied-game/profile checks unchanged. Saved menu-entry samples 2 through 5 have zero black left-side columns, including the previously narrow handoff. The title remains independently aspect-fitted. Child swipes and car-intro expansion remain pending. No new Apple package or publication. See evidence/ui-wide-map-sidebar-handoff.json.

### Child swipe and tier follow-up

The grouped v18 local build prepares authored child-sidebar images over exact source-command full-Main backdrops. Setup and Location first complete wide frames are observed; all three bounded menu observations preserve the executable, twelve resources, copied game and restored profile. Actual post-sidebar draws are six branding slots, requiring their own noninteractive source witness rather than a widget snapshot. That extension and independent Player entrance composition remain in progress.

A longer Opponents observation reaches native first-full serial 221 but rejects the old tier fallback at its Main-specific anchor. The three accepted authored tier badges were absent from the selected pack. A new coherent pack preserves all twenty-four old bindings and all fifty-two other section bodies, adds the three badges, and passes the actual component loader. Recipe and injector pins are integrated in source; the runtime resource and configuration remain unchanged until the grouped rebuild. No new Apple package or device acceptance is claimed.


### UI activation checkpoint: first Main frame and typed child entrances

Local integration 20 removes the one-frame original/narrow Main fallback after its opening swipe: the presentation trace changes directly from transition-wide to menu-wide. This is local desktop evidence only. Setup still falls back during its typed swipe; bounded diagnostics isolate a converted-texture verification mismatch in branding slot 0 while all source-domain and other binding checks pass. Player/Location photo entrance investigation remains active. No new IPA, cloud build, publication, or physical-device acceptance is claimed.


### UI activation checkpoint: complete Player entrance and remaining coverage

Local integration 21 fixed signed off-screen picture coordinates in both photo channels and removed retired CPU scratch reads from initial branding. Setup and Location entrances now stay full-width through their actual source-backed swipes and picture animation. Local integration 22 adds the previously uncovered first Player sidebar fade: all 17 actual fade frames remain wide, followed directly by the wide picture animation. These are desktop results, not a new iPad build or device acceptance. The static remastered title is also visible in captured startup pixels; the preceding original animated movie remains a separate low-priority video task.

The finished Opponents menu still falls back because its authenticated slot-5 badge pulses signed horizontal width while retaining legitimate vertical transparent padding. The next correction must recognize that exact source animation rather than relax general geometry bounds. Earlier intermittent Main navigation gaps did not reproduce in either integration-22 run; bounded upstream diagnostics remain in place, and the issue stays open. A static native-background cache for Setup/Options is being implemented without caching cursor-bearing images or bypassing current source admission.

All completed local runs preserve original data and restore the isolated copied profile. No cloud build or publication was performed. Finish coverage and artwork integration before the next iPad package, then resume performance and Low Power Mode work. Eight Location scenery images need faithful standalone remasters; existing related drafts change landmarks/composition and are not substitutes.

### UI activation: completed scenery and remaining presentation ownership

Local integration26 compiles the exact Location native converter wrapper and its independent completed-preview artwork path. The actual Location run publishes the authored photo through its animation, then two source-qualified complete kind9 layouts. Reviewed final pixels retain the remastered North Country scene; presentation changes directly from Main wide to sidebar wide, picture wide and completed wide. The run exits0 with all13 selected resources and isolated immutable game data unchanged and its copied profile restored. This is desktop evidence, not physical iPad acceptance.

A separately controlled Setup observation reaches One Player, Split Screen, Modem, Serial Link and Network, then returns to Main. Each of four mode changes follows two fresh source proofs and an individually reviewed visible label. Full-width modern backgrounds are visible in saved Split, Modem and Network samples. However, the full presentation trace still contains source-quiet native intervals; these modes are NOT accepted as gap-free. One-shot wide presentation state is discarded after a swap while the original engine can swap its two native buffers without repainting. Any retained authored output must be associated with the exact physical native buffer and current source/paint/resource authority, rather than reusing an arbitrary previous displayed frame. Child exit animation coverage is also being traced separately from Main entrance.

Local integration27 fixes a real asynchronous handoff error: renderer commands are queued, so a completed native upload seal must be published by the renderer callback and acquired by the worker afterward. Strong queued ownership and immutable terminal publication replace premature worker-side completion. Focused delayed-queue/threaded sanitizer cases and independent source review pass; live reuse remains a separate runtime gate. This integration also selects three faithful authored Modem, Serial and Network badges. The coherent pack has30 bindings and59 sections; all27 prior bindings and55 other section bodies are byte-identical. No new cloud app build, push or release has occurred. Complete the remaining presentation paths before the next IPA, then resume performance and Low Power Mode work.
### Integration28 runtime contradicts complete-return acceptance

The combined app builds with unchanged reviewed source pins; actual generated hooks and compiler dependencies select the new helpers. Its controlled five-mode run exits0, restores the isolated profile and preserves all13 resources and immutable copied assets. All three remastered mode badges are visible. Actual Main seal publication is observed, but pending reuse remains unobserved.

The continuous trace still contains97 native Network frames,17 native frames after its resize and12 native return frames. Two-buffer output sealing alone is not evidence of idle reuse: no idle-select/present event occurred. Missing producer logs had been capped, so the earlier inference that the97frames were source-quiet is not sufficient. This remains an active source/admission problem.

Actual source control flow corrects a second assumption:40D1B0 is a21-frame car/track preview-change animation, not Escape. Its new channel was not executed during this test. Network Escape calls427320 with140decimal(0x8C),230,slot1 and produces12 gradient-fade frames; modes1..4 use the separate19-frame reverse-photo loop40C9B3 after a fresh bare-map upload. Independent return contracts are required; the old child full-upload event must not authorize the fresh bare map. No old UI, first-frame completeness, physical iPad acceptance or UI completion is claimed from integration28. Do not publish another IPA until the actual remaining routes are corrected and observed.
### Final artwork delivery: lossless size optimization
User requires a final lossless compression pass once artwork is complete. Preserve accepted editable masters; omit drafts, comparisons and unused variants from app delivery. Current 13 selected runtime bundles total 511,140,830 bytes (487.46 MiB), already using lossless resource compression. Audit final per-image/category sizes, eliminate redundant packaged assets, and evaluate compatible lossless encoding improvements without resizing, palette reduction or lossy conversion. Verify decoded pixel equality (including alpha) before accepting optimized assets, and confirm the runtime still loads them. Disk compression does not reduce decoded GPU/RAM footprint by itself. No new cloud build solely for this audit.

### Child animation foreground profile correction
Fixed the pending shared ChildExit/ChildReturn quad admission: authenticated native marks now require their exact registered profile (128px/LOD1 RGBA1555 badge or32px/LOD3 RGBA4444 cursor), texture size/format and full prepared-byte memory bounds. Font and photo paths remain32px/LOD3. The previous unconditional LOD3 check incorrectly rejected valid128px badges. Regenerated the complete candidate renderer in an isolated project build folder; its actual C11 translation unit passes syntax compilation with the retained app compiler flags. Evidence: private child-return-profile-fix-v1/RESULT.json and COMMAND.json. This is source/compile evidence only; runtime transition acceptance and the next iPad build remain pending. Retained app/resources were not replaced.


### Integration29 and interrupted return observation
Grouped local engine/renderer generation and app build passed with unchanged source pins. Actual generated hooks match35 ChildReturn blocks across Methods01/03/12 plus25 preview-change blocks. Selected GPU helpers match current sources. Runtime reached Location and armed reverse route2, but repeated source reason4 with zero sidebar tiles; no complete return acceptance. Pause interrupted the observer before a final result. On resume its process handle was missing and scoped WSL inventory found no app/observer. Restored only the isolated copied profile from its pre-reset backup, with SHA equality; no full immutable post-run verification claimed. Added bounded zero-pointer/empty-source/raw-mismatch/zero-tile diagnostics without changing admission. No new cloud build, publication or physical acceptance.

### Exact cached-sidebar return route (source candidate)
Original428250 can early-return without tile uploads; MenuTransition UploadScope already authenticates this path by restoring the identical previous Packet after zero tiles. The outer ChildReturn observer now recognizes only that exact same retained slot0 Packet, source pointer, raw header/pixels, normalized side pixels, game/thread/catalog/archive and current lifetime. It retains the Packet and rechecks its source on every owner-current test. Existing GPU native texture/tile validation remains mandatory. Independently restricted SidebarTileScope to the lifetime of the validated slot0 upload so the later bare-map slot1 upload cannot contaminate sidebar receipts. Runtime sideTiles0 did not prove which rejection occurred; bounded diagnostics remain. This is not yet a runtime fix claim; integration29 remains the last built candidate and the new source needs the next grouped build/observation.


### Integration30 runtime narrows actual failure
Local candidate07560b81 built with unchanged source pins; actual generated hooks and selected GPU helpers verified. Clean isolated observer reached Location and armed reverse route2, then source reason8 identifies raw sidebar mismatch before tile observation. Cache handling did not resolve this path; no successful return claimed. Observer183.60s, preflight41.22s, app exited0/appGone, all13resources and copied immutable assets unchanged, copied profile restored. Added trace-capped shape/header/pixel/first-difference classification for the next candidate; no broad source-identity waiver. Optional offline observer test could not import Pillow in system WSL Python; no dependencies installed, actual runtime observer completed independently. UI still incomplete; no cloud build or IPA publication.

### Confirmed Location reverse sidebar-header truncation
Integration31 diagnostic independently reports initial=1, family=3, actual343x480, expected329296bytes, headerEqual0, pixelsEqual1, firstOffset1. Source audit confirms Target.header is array<uint32_t,4>, but initialSemantic copied its four elements directly into vector<uint8_t>, truncating each word and leaving12headerbytes unwritten. Replaced this with explicit little-endian serialization of all16bytes. The actual Methods03 C++ translation unit passes syntax compilation after the correction; pixel/source/lifetime checks are unchanged. Running integration31 still contains the previous diagnostic version and cannot validate the new correction. Wait for its verified cleanup before replacing the retained app. Source route audit also confirms modes1..4 ordinary children reverse19; Opponents byte512DF6 chooses reverse19/nonzero or fade12/zero; modes outside1..4 fade12. No animation-skip exception is needed in the observer.


### Integration32 source correction verified; renderer return remains incomplete
Local corrected app9872d288 built with unchanged source pins and verified compiled hooks. Private pipeline initially referenced verification31; stopped before app launch, corrected and reused qualified32 without rebuilding. First runtime missed250ms Escape (no source arm), so no game failure inferred. Fresh32b observation used one550ms Escape: route2 armed, all300sidebar tiles captured, fresh baremap accepted (reason5); header correction is now runtime verified. No HD return frames accepted, so full transition remains incomplete. Both runs exit0/appGone, preserve all13resources and copied immutable assets, restore copied profile. Added bounded Frame owner/phase/photo/submission and6GPU-entry rejection reasons to identify the next strict refusal; no guard waiver. No cloud build or IPA publication.


### Integration33 narrows rejection to queued owner admission
Local app5b8f7917 built with unchanged source pins and verified generated hooks. Observer33 mistakenly changed fixedX11format32 to33 during numeric version substitution; strict ownership check stopped safely. Corrected source-only observer33b restored32 while retaining selected-input version33, reused same ELF. Actual trace has arm, fresh side300, and19 enqueued frames, but no GPU begin-refusal or stage/WIDE log. Thus queued lambda owner-current rejection is the next supported boundary to inspect, not a shader/geometry conclusion. Decomposed exactly the same Owner::current predicates into reason codes and logged queued-consumer failures; checks unchanged, actual Methods03 syntax passes. Runtime184.28s, app exit0/appGone, all13resources/copied immutableassets unchanged, copied profile restored. No cloud build or publication.

Correction to integration33 interpretation: absence of begin-refused does not prove owner-current failure; successful begin followed by silent clear cancellation is also possible. Added bounded GPU begin-success/clear-refusal markers alongside Owner reasons. Independent source audit found no support for a thread-local catalog or later-upload race; grBufferSwap is a barrier. Preserve all lifetime/event checks.


### Integration34 confirms clear depth/colour mixup
Actual route2 GPU begin succeeds atframes0/1, then ChildExit clear refuses reason33 before any tile drawing. Source audit confirms caller passes depth,r,g,b (OpenGL2.c), while ChildExit named the first16bit argument colour and required zero. Original49797A stores depth65535; existing PlayerIntro clear requires65535. Corrected to depth!=65535 rejection plus native clearValid witness and retained blackRGB check. This changes a wrong expectation to the actual original clear contract, not a lifetime/source waiver. Integration34 exited0/appGone; all13resources/copiedassets unchanged and copied profile restored. Next grouped candidate35 must verify rendering after this fix.


### Integration35 clear fix verified; branding descriptor field correction
Actual GPU return now passes clear and base native300selections/600triangles, reachingphase3 before first branding quad stage8 reason24. This verifies depth65535 correction in runtime; no fullreturn success. Reason24 covers mark profile checks as well as missingmark/font, so unknownbranding was not established. Independent source audit identifies descriptor[2] as aspect, whileLOD fields are0/1: reviewed128branding has[0]=1,[1]=1,[2]=3. Corrected exactprofile checks to bothLOD fields, retaining aspect3, mask6=3, nativeedge/format/memorybounds/currentauthenticatedmark.32photo/font paths now check bothLOD3 values. No additionaltextureidentity allowance. New correction pending grouped integration36. Integration35 exits0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. No cloud or publication.


### Integration36 branding fix verified; initial HD photo identity correction
Actual return passes background300/600 and branding, reaches firstphoto phase4 with one selection/two triangles, then batch refuses. Independent source audit confirms initialSemantic leavesphotoFamily/Serial/Epoch zero while actual4279E0 independently activates/binds published HD ChildPhoto. Shared nfs_ce_photo_texture consequently expects native texture instead of the HD binding. Refactored existing captureSemantic optional current-photo attachment into sharedattachPhoto and invokes it frominitialSemantic; retains same game/archive/current proof predicates and shared lifetime. Missingoptionalphoto still cannot grant entry, native100receipts remain mandatory, existing independent ChildPhoto domain/count checks remain. Actual Methods03 C++syntax passes after fix. Integration36 exits0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. Newsource awaits integration37; no fullreturn or physicaliPad acceptance claimed.


### Integration37 photo path verified; wide caption adapter candidate
Actual photo100selections/200triangles and sidebar300/600 complete, then SidebarEnd stage5 refuses. Independent source audit identifies guaranteedcaption failure: generic captiondraw calls legacy nfs_frame_sampling_valid, which accepts4:3dimensions only, while CE target is2360x1640 with wideprojection. Added separately CE-owned captionquad adapter forvalidatedLocation/compactSetupOptions/Player resources, matching workingentry pattern but using only CEcurrent/attachment/matrix and aliaschecks. Existing4:3validator unchanged; no entrancepermission borrowed. Complete regeneratedrenderer C11syntax passes. New source awaits integration38 runtime; complete19frame return still unverified. Corrected Setupdiagnostic uploaded-count bit7 to boolean300 so flags remainindependent; no renderingchange. Integration37 exits0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. No cloud/publication.


### Integration38 complete Location/PlayerOne return milestone
Actual source/GPU/presentation traces verify19continuousHDreturnframes forLocation andPlayerOne each, followedby2freshMain proofs perroute. Rootviewed actualLocation/P1 animation samples andLocationfinalMain: remastered panel/logo/text/photo visible. Opponents remainsunverified: combinedcapturelimit320MiB reached at63PNGs/330775573bytes beforecompletion. Observer stoppedownedapp(launcher-9), appGone; all13resources/copiedimmutableassets unchanged; copiedprofile restored. No physicaliPad acceptance.
Read-onlysourceaudit finds nextguaranteedpreview/fadebug: CE stage leftmagFilter0 andpreviewdraw passed0toGL_TEXTURE_MAG_FILTER. Set stage.magFilter to existingnative linearFiltering?GL_LINEAR:GL_NEAREST policy; nolegacy4:3factory or guardchange. Sourceawaitsnextintegration39. BenchmarkedlosslessPNGUp filtering onexisting6.14MBcapture:3.93MB,0.99sencode, identicaldecodedpixelSHA. Use forfutureobservercaps; this doesnotcompressorchangeacceptedartworkmasters. Network97framehandoff stillneedsfreshdiagnosticrun; noIPA/cloud/publication.


### Integration39 preserves two returns; Opponent hot-check optimization candidate
Location/PlayerOne again pass19continuousHDreturnframes and2freshMain each. LosslessUpPNG captured64images/205325787bytes below320MiBcap, allowing Opponent sourcegatedEscape. Opponent source/sidebar accepted but renderer callbacks take~20s each beforecompletion; runfailsreturnacceptance. Sourceaudit confirms Owner::current kind10 calls previewView, which runsImage.apply full2.8MBhash+PMApixel scan on everyGLguard. Providerscopyinputs intoimmutableretainedstorage andacquire alreadyrunsfullvalidation. Added Image.current requiringretainednonzeroidentity/nonzero matchingatomicprovider generation; returnhotcheck usesretainedpreview/child/artcurrent instead ofrebuildingpreviewview. Fullacquire/armvalidation, game/catalog/session/event checks unchanged. ActualMethods03syntaxpasses. Performance/returnimprovement stillneedsintegration40runtime. Run39 exits0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. Prepared sourceonlyNetworkobserverv9 targetsqualified39,27offlinecheckspass; rootreadpreparation, noapp/nonce/commandcreated. No cloud/publication.


### Integration40 allthree return loops verified; Main pulse remaining
Grouped40startup observation stayedinintro movie and correctlystoppedbeforeunknownMain input. Rootviewedactualmoviecapture; fresh40b reusedsamequalifiedELF0856ee92 withsame2startupskips delayed2s initially/1s aftereach andearlierMainpublishedstopguard. Noapprebuild.40b verifiesLocation,PlayerOne,Opponent19continuousHDreturnframes each plus2freshMainproofs. Opponent current-generation optimization eliminatesprevious20s callbackstall sufficientlytocomplete; no physicalFPSclaim. RootviewedactualOpponentanimatedClassAHDcollage/panel/caption. WholeOpponentpath stillfails: after4Mainwideframes itfallsnative102frames, thenrecoverswide. Logreason11rect511,406,637,534 isexactshared4276F0slot5pulse onMain. Existingpulseexception permittedonlyOpponentowner; Mainfellstatic128squaretest. Sourcecandidate41 extendsexactauthored128pulsebranch toMain0x401910 underALLsame schema/form/cursor/epoch/originalOpponentValid/address/currentnativeMark/profile/revision/bounds/texture checks. Genericbounds/staticotherowners unchanged; runtimepending.40bexits0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. Networkobserverv10sourceonlytargets40,29offlinechecks; hold/adaptafterMainpulsequalification. No cloud/publication.


### Integration41 Mainpulse fix verified through172HDMain presentations
FocusedOpponent returnpasses19continuousHDGPUframes and2freshMainproofs. Mainremainswide172consecutivepresentations, eliminatingprior4HD+102nativepulsefallback. Extendedautomaticwatchthenrejectsnativeoutput. Rootsourceaudit findsoriginal40D3D0 idle3200tickbranch calls4242F0 withdemoav%d.dct; originalmovilite/SHOWDCTstrings proveitsmoviepurpose. Fresh41b usesread-onlyinotifywatch on3copieddemoassets andcapturesactualnativeboundary. NoIN_OPENwitnessarrives, so rawobserverfailure isretained. Rootviewedactualcapture: demo/cinematic snowtrackcars withcornerNFSlogo, notoldMainUI. PresentationRLE355->527 closes172Mainwideframes; specificdemofilename/callsite remainsunproven. Scopedmanualacceptance: return+Mainpulse/UIstability; no broadautomaticpass orphysicalclaim. Bothruns exit0/appGone, all13resources/copiedassets unchanged, copiedprofile restored. Source-onlyNetworkobserverv11targetsqualified41 with35offlinechecks; rootreadymailboxprotocol needednext, noappcurrentlylive. No cloud/publication.

### Setup interrupted redraw investigation
- Local integration41 mode observer ended cleanly; copied profile restored and all13 selected resources unchanged. Modem and Serial Link exposed113/114 native fallback frames; Network restore exposed1. Return to Main was not verified before the observer deadline.
- Original40BBE0 may abort after group or completed CPU copy without uploading slot1. Existing capture clears its publication too early.
- Added Setup-only exact original-abort receipt, with owner/iteration/event and group/copy evidence. Actual generated-code transform is reversible and preserves original poll calls/branches. No new app build or device claim.
- Next: retain only a previously fully completed native upload across source-proven aborts, with fresh GPU proof and current widget state. Do not promote incomplete CPU collector data.

### Setup completed-upload retention implementation
- Added an independent Setup-only GPU upload lease: seed only from the ordinary verified full upload, seal only after native drawing completes, recheck all300 native texture receipts and exact descriptors before reuse, revoke on owner/upload/context changes. Fresh responsive widget snapshots remain required.
- Reuse is limited to an exact original40BCFF abort receipt, distinguishing pre-copy and post-copy phases. Retained caption objects preserve their immutable ordinary source proof. No incomplete collector is published.
- Local integration42 compiled successfully with unchanged frozen source; source review identified and corrected independent Setup/collector counter alignment and asynchronous completion adoption before runtime testing. Integration43 contains those corrections. Physical iPad behavior remains unverified.

### Integration43 Setup runtime milestone
- All four root-reviewed Setup game-type transitions stayed menu-wide; previous Modem113frame and Serial114frame native fallback gaps did not recur. Both post-copy aborts and repeated unchanged-content native300/600 finishes were observed.
- Five1frame resize fallbacks remain. Setup-to-Main fades native for12frames (1179�1191), then two fresh Main proofs and root-viewed HD Main capture. Do not claim all transitions fixed.
- Observer exited0; binary/all13resources and copied immutable assets unchanged, copied profile restored. No cloud build or physical iPad claim.
- Next source issue: return semantic capture requires State.published, but Escape can trigger the same original redraw abort. Join return authority to the completed Setup upload lease under exact owner/archive/event/descriptor/native checks; never promote unfinished collector state.

### Setup Escape completed-upload handoff implementation
- Integration43 showed Escape itself can abort the current redraw immediately before the12frame return animation. Ordinary return capture then requires a missing State.published.
- Added a Setup-only return-boundary acquisition of the same exact completed-upload lease used by the verified mode fix. Normal427B30 draw arguments remain separately mandatory. Return Session retains immutable uploaded content/domain and still requires its existing actual300/600 native pass proof. Partial current collector data is not promoted.
- Source implementation pending grouped local compile/runtime validation; no new IPA or device acceptance claim.

### Integration44 timing-case regression and next fix
- Local44 compile passed with frozen source unchanged. Observer13 returned Main and exited0, binary/all13resources/copied immutable assets unchanged, profile restored.
- A114frame pre-group fallback occurred despite43 mode timing passing. Distinguish source-proven passing timing from a universal fix; universal removal remains unproven. Return still12nativeframes.
- Original40BBE0 can abort at40BCFF before beginGroup after drawing side/words. Added distinct phase0 receipt with collector revision separate from Setup iteration. Pending reuse continues to require only the older fully completed GPU upload with exact owner/catalog/archive/event/descriptor/native tile proofs, never partial CPU state. New source not yet compiled.
- Return source handoff compiled but did not eliminate the12frame fade. Added bounded arm/semantic/fade-side rejection diagnostics to identify the formerly silent refusal; no acceptance guard weakened. Pre-copy sidebar still requires independent equality/provenance.
- Next: compile grouped phase0 and return diagnostics, observe exact failure, then requalify the prepared source-gated21frame car/track preview observer. No Apple build until coherent UI.

### Integration45 return failure identified; grouped46 implementation
- Observer14 exited0 with two fresh Main proofs; binary/all13resources/copied immutable assets unchanged, profile restored. Return diagnostics30,31,2 prove arm and semantic capture succeeded, then sidebar copy guard rejected. Return12frame fallback remains. No retained event2 observed; repeated-content native passes alone do not prove reuse.
- Enumerated all7 original404D60 abort sites:2pre-group,2partialgroup(1/2draws),1completegroup,1postcopy,1postupload. Old lease forbidden after upload.
- Implemented partialgroup phase3 and exactpostreturn identity, without preparing partial CPU content; receipt excludes published/newupload state.
- Same fully verified candidate copy carries immutable exactsidebar raw/history owner. Exactpending Setup return may use history only when no current verified copy exists; malformed current copies reject. Fade requires exactslot0 bytes/game/thread/archive/catalog/current; reverse actualuploadbytes/native300tile proofs remain.
- Bounded source/GPU refusal/completion diagnostics distinguish actual sealing/reuse and asyncledger issues. Source review passed; compile/runtime validation pending.
- Source-gated21frame preview observer45 is prepared/pinned but not run. Requalify after Setup corrections. No Apple build before coherent UI.

### Integration46 evidence and absent-cache fade binding
- Both46observers exited0 with all13resources/copied immutable assets unchanged and profile restored. Fullmode observer15 stopped at rootcommand deadline before return; do not claim it passed return. Targeted native-only16 reachedNetwork and returnedMain with two fresh proofs.
- GPUreason15 proves actual completed native sealing. Retained event2 still not observed, so successful ordinary frames are not evidence that interrupted retention was exercised.
- Targetedreturn reasons30,31,21 identify absentoptionalMTslot0cache, after authenticatedsemantic/sidebar acquisition. Added exactSetup/fade-only alternative when cache absent: checked300native slot0descriptors plus authenticatedsidebar provenance and exactgame/lifetime/catalog/archive/upload before/after capture. ExistingGPUcached_tiles checks everyconvertedpixel/uploadwitness/filter/liveGLtexture and original300selections/600triangles before composition. Presentmismatchingcache and reverse remain unchanged. Source review passed;47compile/runtime pending.
- Completed bounded13pack artworksize audit:487.46MiB storedtotal; largestdecodedtexture2012x1924=14.77MiB. Elevenwhole-stream packs have no honest per-image compressed attribution. Per-image averages/maxima and counts/dimensions retained in evidence/artwork-sizes-lossless.json. Final lossless pixel/alpha roundtrip remains required after artwork is finalized.

### Integration47 local Setup return milestone
- Focused observer17 verified exact12fresh wide fade frames0..11, phases140..250, presented1507..1518 then directly wideMain1519. No native presentation after Setupwideentry308.
- Explicit source notes26(history),27(actualslot0descriptors), native full/sidebar300selections/600triangles and phase2 event2 reuse observed. Root viewed final HDMain capture; twofreshMainproofs.
- App/SDL exited0, binary/all13resources/copied immutable assets unchanged, copiedprofile restored. Desktop only; do not claim physical iPad or allUIcomplete. Phase0/3 runtime coverage, preview21, resize1frame gaps and broader acceptance remain.
- Next: run already prepared/pinned preview observer47 against same qualifiedbinary; no new build is required for that check.

### Preview47 failure and static-scene handoff work
- Actual source-gated track change reached preview21 but rejected semantic source at stage1, then presented native UI. Observer stopped without further input; app exited0, all13 resources and copied immutable assets unchanged, profile restored.
- The preview routine runs inside the parent menu. Its resource preparation can invalidate the current producer publication before delayed capture. Implementing scope-bound authenticated immutable Location/Player scene history, retained before preparation; transfer still requires exact game/thread/form/lifetime/catalog/upload domain and actual native texture/pixel/pass proofs.
- History header and narrow producer hooks passed C++17/Python syntax checks only. Transition integration and runtime verification remain pending; no iPad build or UI completion claim.

### Integration48 static preview handoff compiled
- Retained fully authenticated immutable Location/Player scenes inside a nested parent-menu lifetime. Exact-entry preview transfer, late incoming-photo refresh and actual sidebar descriptor binding preserve independent native pixel/upload/pass proofs. CPU scope checks remain separate from renderer lifetime.
- Local generation and compilation passed; two generated files changed; source pins unchanged. ELF SHA256 286c22dfa5650e3adff48c6b07031075142c01a0d9050a8152c4b3c3a5853e3f. No cloud or physical-device run.
- Preview48 observer cloned and held for exact build receipt qualification. Runtime effectiveness remains unverified. Compiler accepted two C++17-extension warnings in the new type guard; portability cleanup should accompany the next grouped change if needed.

### Integration48 actual preview evidence and next source correction
- First attempt could not claim default-display startup window; app exit0 and cleanup verified. Preserved separately. Retried identical ELF in isolated Xvfb/X11 display; reached source-proven Location and one track change.
- Preview21 still falls back. Source reason6(history unavailable), then direct stage1 refusal; no late event-mismatch proof. Immediately preceding completed Location frames use persistent kind9 initial-full source, not ordinary iterative Prepared used by the new registration hook.
- Correct next change is a dedicated exact-kind9 snapshot factory at successful full upload seal, retaining its independent full/sidebar/caption/preview source proofs. Do not manufacture a normal LocationPrepared from a partial collector or relax upload/lifetime checks.
- Runtime observer terminal0; ELF/all13resources/copied immutable assets unchanged; profile restored. No physical-device claim. New dedicated source-path integration in progress; next grouped compile only after review.

### Integration49 exact initial-full source compiled
- Dedicated LocationInitialFullSnapshot retains complete initial-full/sidebar/caption/preview proof chain. Registration occurs only after verified kind9 slot1 upload seal or authenticated exact previous-packet reuse; checks raw full pixels, actual300 keys, sidebar before/after, catalog/game/archive/menu token and unchanged upload event.
- Added bounded default-off registration/acquire refusal diagnostics; C++14 exact-type overloads replace the previous extension warning. No native pixel/upload/pass proof weakened.
- Local generation/compile passed; source pins unchanged; ELF SHA256 3bf583d6880bbac0820f21234bd106de774109692e6714b06e96057ee642bb73. No cloud/device build.
- Observer49 launched in isolated Xvfb with exact compiled receipt/resources; process session77206 live at checkpoint. Poll that handle before any further app action. Runtime result pending.

### Integration49 runtime and catalogue identity correction
- Actual kind9 factory reached but refused stage2. Source inspection proved a deterministic pointer mismatch: original Main proof deliberately copies its catalogue in Main343OriginProof.h. Factory now compares authenticated full catalogue contents using existing sameCatalog; live canonical catalogue pointer check remains.
- Observer terminal0; ELF/all13resources/copied immutable assets unchanged, profile restored. Corrected source not yet compiled or runtime verified. No device/cloud claim.

### Integration50 catalogue correction compiled
- Full local generation/build passed with unchanged source pins. ELF SHA256 db34e40563d0947cf5dfb6b174d671934749fcedabab91c5ea35a2c427ab4d47. Actual catalogue copy compares using original sameCatalog API; canonical live catalogue identity retained.
- Observer50 admitted against exact build receipt and launched isolated Xvfb; session17377 live at checkpoint. Poll same handle, no parallel app. Runtime result pending; no physical-device or full-UI success claim.
- Read-only audit also identified unsupported equality between initial-sidebar and Location-owner independent epoch counters. No runtime mismatch yet; resolve with original proof identities rather than assuming shared counter domain in later grouped correction.

### Integration50 retained-source milestone and renderer failure
- Kind9 history sealed/acquired and transferred successfully: source reason7,6 and ChildExit owner family3 stage6 at upload81. Initial full pass300/600 completed. First clear fails widgets stage14 quadreason22; later moving-frame refusal is downstream of missing completed backing surfaces.
- Observer interrupted before final receipt. Handle missing and /proc confirmed no app/observer. Recovered profile from its saved pre-reset copy; ELF/all13resources/copied immutable assets unchanged. RECOVERED-RESULT explicitly labels interruption; no clean-exit or passing-animation claim.
- Source audit found renderer unconditionally rejects hdDigits while its frontend-font alternative requires authenticated nonzero font tags41/42. It also assumes32-square/LOD3 although frontend font pages are256-square. Strict owner/page/profile correction in progress; native source/geometry/GL/pass checks remain.
- Removed unsupported equality of two independent owner epochs: require each nonzero and keep original full-proof currentChild/game/thread/archive/form ownership chain. No source authority broadened to unrelated forms.

### Integration51 frontend-font profile correction compiled
- Strict frontend font branch now admits only SHA-matched registry pages41/42, exact TextureInfo owner/current selected tag,256-square ARGB4444 LOD0 and full131072-byte range. Photo/mark paths still reject font tags; native geometry/GL/texture/pass checks preserved. Added distinct rejection reasons.
- Local generation/build passed; source pins unchanged; ELF SHA256 7c1599ad54f123e132f5c50e103411dda819905bdff0646928f77c70b26c91ec. No cloud/device build.
- Observer51 launched against exact build receipt/resources in isolated Xvfb; session59352 live at checkpoint. Poll same handle before other app actions. Runtime outcome pending.

### Integration51 Location preview21 local milestone
- Source owner accepted, full native300/600 completed, first clear widgets no longer reject. Presentation trace enters child-exit-wide frame375 and returns directly transition-wide396 with previousFrames21: all21 preview frames continuouswide, no native inside this run. Root viewed passive animation capture with HD background/text/badges/photo.
- Observer subsequently stopped before Escape because fresh correct-child proof was missing. Exact kind9 presentation log count was64, matching its per-kind diagnostic cap; later render source continued but required fresh presentation log was suppressed. Raised trace-enabled-only bounded cap256; no rendering/admission logic changed. Recompile needed before retrying full route; do not weaken observer source gates.
- App/SDL exit0; ELF/all13resources/copied immutable assets unchanged; profile restored. This proves local Location preview only, not Player preview, all UI, or iPad acceptance.

### Integration52 Location preview and return verified locally
- Extended bounded presentation diagnostics allowed actual fresh-child proofs. PREVIEW-location-RESULT and EXIT-location-RESULT both complete true, source-gated one Right then actual return; Location preview21 remains continuouswide.
- Test reached P1 and stopped on preview21 source failure: static history reason2 no-record; direct current source unavailable. Separate source audit confirmed Main Up navigation has genuine12frame native fallback during completed-copy/preupload original40BBE0 abort, not intro/demo.
- Root investigating both remaining source paths in parallel for one grouped correction; no next build until source review. Location milestone does not establish P1, Main navigation, broad UI or iPad success.
- App/SDL exit0; exact ELF/all13resources/copied immutable assets unchanged; profile restored. Evidence sanitized and retained in ui-preview-static-source-progress.json.

### Grouped53 Main navigation and Player preview corrections
- Player preview uses dedicated immutable Prepared/native-proof/VerifiedSource snapshot: original Player verifier authenticates post-copy colour swatch, so generic Copy.POST==final-upload equality was inapplicable. Keeps exact source/form/P1-P2 binding, copy sidebar evidence, uploaded pixels/keys, catalogue/life/menu token/event. Actual generated-header syntax check passed; full runtime pending.
- Main navigation correction adds exact original postcopy/preupload abort receipt. It retains only previously GPU-sealed source with unchanged descriptors/event/lifetime; never prepares or uploads the newer incomplete redraw. Receipt binds exact owner/invocation/iteration/group/copy/error-free source and expected NativePresentationScope StopOwner state.
- Both corrections await one grouped local compile and runtime. No broader UI/device success claim; no cloud build/publication.

### Integration53 grouped correction compiled
- Local generation/build passed; source pins unchanged; ELF SHA256 9f640149b4b7bc905a26aa98929ff9896ceccced8f5bfbed3ba0a422ecee86b7. Two generated files changed, original artwork resources retained. No cloud/device build.
- Observer53 launched against exact compiled receipt/resources in isolated Xvfb, session67055 live at checkpoint. Poll same handle before further app actions. Main postcopy retention and Player preview effectiveness remain runtime-unverified.

### Integration53 runtime: Player history passes, incoming-photo timing identified
- Location preview+return remained complete. Initial Main navigation showed continuouswide without native fallback, but exact Main abort receipt/event5 was not observed; do not claim interrupted-redraw branch validated.
- Player dedicated history registered (CHILD STATIC8stage6), acquired exactevent171 and transferred owner family5stage6. Latearm reason14 rejects incomingphoto after source publishedserial2car5.
- Original40D1B0 deliberately restores outgoing selectedcar from stack[ESP] after427ED0 and before arm; incoming saved at[ESP+4]. Generic PlayerPhoto.current correctly compares selectedcar, so is inapplicable after this original restoration. Implement exact post427ED0 verified incomingphoto receipt before restore, scoped to this preview; keep ordinary current() strict, actual100descriptor/upload/pixel/200triangle proofs mandatory.
- App/SDL exited0; ELF/all13resources/copied immutable assets unchanged, profile restored. No physical-device/full-UI completion claim.

### Incoming Player photo restoration handoff in progress
- Exact post427ED0 receipt captures incoming photo while ordinary current() is valid, before original40D1B0 restores outgoing selection. Scoped checks bind saved old/new stack selection, exact parent form/game/thread/epoch/catalogue/archive/life/source bytes and published photo identity; same arm covers both Player1/Player2.
- Also integrating scoped HD photo begin/end for the intentional old!=new selection interval, because ordinary DrawScope correctly refuses the incoming photo after restoration. It uses existing HD photo API and native100tile/200triangle proofs; any failure cancels the CE frame. Generic photo current()/DrawScope remain unchanged.
- One grouped compile follows source review. No runtime or device success claim yet.

### Integration54 compiled; incoming-photo branch not exercised by first input attempt
- Generation/build passed, source pins unchanged; ELF b983d9dbb85ddb018bff85da354201ecb4bae129db7d040b9078c7e85428f852. Observer retained clean Location preview/return but P1 Right produced no40D1B0 source entry or incoming photo publication at all; do not claim new photo receipt failed or passed.
- Driver Right uses250ms keydown while software renderer can take longer between input polls. Preparing same-binary P1-only observer with bounded longer single pulse and original source gates, no extra unknown-state retry. No new build for this harness issue.
- Main fallback13frames occurred twice after child return. Source facts show State iteration15 but collector/group/copy90: two independent counters were wrongly compared in new retention admission. Correct global collector chain separately from State event iteration; keep exact abort/domain/upload/native proof requirements.
- App/SDL exited0; ELF/all13resources/copied immutable assets unchanged; profile restored. No physical-device/full-UI claim.

### Integration54 same-binary P1 preview and return verified
- Focused54b guarded one500ms Right after read-only server repeat-delay check and two fresh source-selected row0 proofs. Actual P1 preview21 continuouswide, no native during run, two fresh completedPlayer proofs; reverse19 directMain with all19 sourceGPU completion indices.
- Incoming photo serial2 published19 HD draws, each exact100tiles/200triangles; scoped begin/end notes25/26 accepted. Root viewed passive HD animation capture. No current-photo guard weakened; prior250ms attempt was unexercised.
- Whole focused observer complete true; app/SDL exit0; ELF/all13resources/copied immutable assets unchanged, profile restored. Sanitized evidence ui-player-preview-hd-local.json.
- Main independent-counter mapping source/hook check passed; next55 compile then targeted Main navigation. P2, broad UI/current-pack pages/world-aspect/device acceptance and final lossless packaging remain incomplete.

### Integration55 independent counter mapping compiled
- Main lazy-start now binds State iteration and global collector revision while both exact lexical scopes are active; consume verifies each counter in its own domain, supporting pregroup/error-free partial/completed-copy retention without adopting CPU pixels as GPU content. Added bounded scalar refusal reasons and separate event5 allowance8 beyond ordinary64trace lines.
- Local generation/build passed; source pins unchanged; ELF SHA256 6e6eed38b83ff6dc33de051e718906708d9def6e1830773c1ee98403d19f7e65. No cloud/device build.
- FocusedMain55b launched session34294: one source-gated Location entry/Reverse19 return before target, then up to20Up each with two fresh Main proofs. Pass requires exact abort/event5 match whose group differs from State iteration, plus no native target presentation. Cold aligned counters alone cannot pass. Runtime pending; poll same handle.

### Integration55 Main post-return navigation observation
- Controlled source-gated Location visit and Reverse19 return completed, then20Up actions each with two fresh Main source/pixel proofs. No native presentation in target interval; copied profile/ELF/all13resources/immutable copied assets unchanged; app/SDL exit0.
- Exact completed-copy abort receipt/event5 was not exercised. Outcome explicitly not-exercised, not a counter-branch pass. Behavioural20-action noflash observation retained separately in ui-main-navigation-counter-local.json; rare branch remains unverified and watched in broader flows.
- Continue current55 wider UI acceptance (Setup5, Options, OpponentsA/B/C and ratios), not indefinite repetition of an unexercised rare abort. Full title firstvisible proof, current-pack information/results/nameentry/world aspect/physical device/final lossless delivery remain open.

### Full-menu startup harness timing correction; no game rebuild
- First RunA startup keys could land before original movie keyboard flush/decode loop. Passive55b retained video throughout its120s bound; Root viewed actual tachometer video, no authored-title/Main source appeared. App/SDL exit0; ELF/all13resources/copied immutable assets unchanged/profile restored. Setup matrix was not exercised, not a Setup renderer failure.
- Exact copied intro metadata:1761video frames and matching audio chunks; original stream initialized at15fps, nominal117.4s. Observed local14.5presentationfps implies~121.5s plus initialization;120s passive wait is insufficient. Next same-binary55c allows180s within600s app budget, captures>=5s apart/max36, zero movie keys, exact-title Return gate unchanged.
- Rare Modem/SerialLink/Network artwork is already authored/integrated: roles27/28/29, identities5566821096482734107/4108/4109. Current recipe/template pins match earlier actual draw-accepted evidence. Old protocol wording saying intentionally-original was stale; no new artwork/code needed, current55 RunA visual reconfirmation pending.


Setup five-mode review, local integration55d: visually reviewed One Player, Split Screen, Modem, Serial Link and Network at 1280x720 and 2360x1640. Mode-change RLE recorded no native fallback; resize evidence remains separate. Final Network Escape failed: full base300/600 passed, sidebar-begin stage4 refused, so this is not complete transition acceptance. Added bounded subreason diagnostics without relaxing guards; not rebuilt yet. Original host cleanup used expired short deadlines; preserved its failed receipt and independently recovered the copied profile after confirming the app was gone. Recovery verifies ELF, all13 packs and immutable copied game assets unchanged. Evidence: evidence/ui-setup-five-mode-local.json. No cloud build.


Integration56: five Setup modes/two ratios reviewed, no mode-change native fallback; final Escape failed cached sidebar validation (stage4 reason14), after full300/600. Clean exit0, all13 resources/ELF/immutable copied assets unchanged, profile restored. Applied absent-MT Setup-fade-only retained initial-sidebar repair with independent child lifetime/epoch; actual descriptors, GPU pixel equality and native300/600 remain mandatory. Header-context syntax passes; repaired runtime unbuilt/unverified. Evidence: evidence/ui-setup-return-sidebar-repair-local.json. RunB56 held. No cloud build.


Integration57: retained initial-sidebar repair compiled, all5 modes/two ratios visually reviewed; actual fade still refuses cached_tiles at stage4 reason14 after full300/600. This repair alone does NOT fix old-UI return. Run exit0 and all13 packs/ELF/immutable copied assets unchanged/profile restored. Added bounded precise cache-rejection diagnostics (pixel versus profile/native-upload witness/context/GL state); current generated renderer syntax passes, not linked/run yet. Evidence: evidence/ui-setup-return-cache-runtime-local.json. RunB57 prepared/held; P2 strict host draft prepared/held pending actualP2reference capture. No cloud build.


Integration58 diagnostic attempt stopped before Escape at final Network resize-restoration proof: Setup publication logging reached exact1024 finishes (shared2048 begin/finish cap). No CHILD RETURN CACHE result yet; not evidence of rendering failure or idle reuse. All13/ELF/immutable copied assets unchanged/profile restored, clean exit0. Reuse same58 binary in focused constant2360 observer without repeating prior reviewed ratio matrix, retaining fresh source/root mode gates. Evidence: evidence/ui-setup-cache-diagnostic-log-cap-local.json.


Focused58b (same58ELF, constant2360): all5 modes root-reviewed; actual Escape diagnostic identifies exact native sidebar pixel mismatch at tile0, while profile/upload witness passed. Immediate Main verification now succeeds twice and Root viewed returned HDMain. Clean exit0/all13packs/ELF/immutable copied assets unchanged/profile restored. Applied minimal current-full-frame left343 candidate for missing-MT Setup fade; old unused initial-source retention removed. Existing full GPU pixel equality and native300/600 remain mandatory. Header-context syntax passes; new candidate not linked/runtime-verified. Evidence: evidence/ui-setup-return-exact-cache-failure-local.json.


Local59 current-full-derived source compiled (one generated input change), SHA e10e3f2f575af3960d99871d3ea53136e8101f9f22517097689ad1e11935578c. Manual-checkpoint run stopped before Escape as review waits consumed trace budget; clean0/all13/ELF/copied assets/profile verified. Samebinary59b now uses exact immutable prior58b root-approved mode-preview hashes plus unchanged current source/focus/300600/native-presentation/input gates, no manual mailbox wait and no fabricated current human review. Any preview mismatch stops. Final12-frame/no-native/2freshMain criteria unchanged. Reference provenance pinned independently; no ratio retest claim. Source change runtime remains unverified until result.


Local59b exact immutable prior-image matches all5 modes; no manual pause or tracecap. Actual fade full-derived candidate passes first97tiles then fails transparent pixel (tile97), confirming composite is not raw343 source. Two freshHDMain proofs; clean0/all13/ELF/copied assets/profile verified. Applied explicit typed SetupFrame retention from genuine published snapshot or uploaded.frame, no erased-owner cast. Use same frame caption.packet.original exact pre-composite343 pixels (already independently verified against Copy source); preserve source/surface/lifetime/serial/shared-owner and GPU pixel/native300600 checks. Header-context syntax passes; runtime not yet verified. Evidence: evidence/ui-setup-return-precomposite-source-local.json.


Local60 VERIFIED Setup return: exact prior-approved images matched all5 modes; actual Network Escape emits fresh owned frames0..11 continuously, zero native fallback, directly next menu-wide and2freshMain proofs; Root viewed HDMain. Exact current typed caption.original343 before colorkey/compositing fixes native sidebar identity. Pixel/native300600/owner/geometry guards unchanged. Clean0/all13packs/ELF/copied assets unchanged/profile restored. Evidence: evidence/ui-setup-precomposite-return-verified-local.json. Scope2360desktop only; first-title/otherUI/physicaliPad/new-ratio acceptance still separate. Continue Options/class cycle using same60binary, no rebuild.


B60 samebinary stopped before Options entry: Two fresh positive Main states absent afterresize640. Root viewedfailure screenshot: genuineHDMain, so game regression notestablished. Need inspect exactclassifier/presentation/fresh-source predicate before host correction; no threshold lowering/reference mutation. Clean0/all13/ELF/copiedassets/profile. Options/classcycle stillunexercised. Setup60 return milestone remains verified. Evidence: evidence/ui-options-class-entry-gate-local.json.


B60 gate diagnosis: actual raw640 classifier passed Main/setup in all10 postresizecaptures (scores.987..1). wide_now640 was wrong: renderer intentionally retains directHD4:3 path at logical<=640; fresh BGcorecommit640/full300600 continues. ChildReturnWide also intentionally excludes logical<=640, so Bhost mustperform actualwide-return at2360 with freshnative source, then restore640 forsourceinputs. Keepwide criteria strict atactualwide extents; no thresholds/reference changes or game rebuild.


B60b sampled Options verified locally: root viewed two actual2360 images plus1280 and640; exact RGB/span hashes and fresh native300600/form proofs checked. Wide samples have no bars or obvious white seam. Manual review timer expired before any Escape; run clean0, all13/ELF/copied assets unchanged/profile restored. Return/class cycle remains unverified; use same60 binary for fresh bounded continuation, no cloud build.


B60d isolatedX11 fixed host window discovery; currentOptions two2360 images rootreviewed, exact ACK. ActualEscape source capture fails (reason30 onlyarm entry, EXITreason4), native13frames thenHDMain. Host waited48s for absentarm and missedMain before idle authoredtitle; root reviewed actualtitle final image. Clean0/all13/ELF/assets/profile. Retain genuine transition failure; next host should stopwait on freshMain even iffailedreturn and collectindependent classes. Options-specific completedGPU fallback absent unlikeSetup; targetguarded sourcefix.


Local61 grouped diagnostic build passed (engine22s/renderer6s/build159s,7generatedfiles, sourcepinsunchanged). ELF57e6796f8e8034b6b2d491548af9dc142461931af5d58e43704393439f5aeb9c. Defaultoff exactOptions fullSource first-refusal/original40BCFFabort and source/art-retained title-to-actualswap diagnostic, attachment/context/mutation guards, noGLerrorconsumption/readback/suppression. Actualheader/renderer syntax passed beforebuild. Runtime pending19252 B61isolatedX11; explicitlatestactualB60dcleanup, nofabricatedA61acceptance. No cloud/devicebuild.


B61 clean0/all13/ELF/assets/profile; firstsourcequalifiedTitle request1/art1 native300600 -> authored matchingdesktop swap1753 success1 first1 (no readback/iPadclaim). OptionsfullSource succeeds thisrun; arm31family2 thenmissingSide21, native12 then2fresh2360Main. Applied strict distincttypedOptions caption.original343 binder onlymissingMTslot0, Setup unchanged/GPUbyte300600 mandatory. EarlierB60d semanticcapturefailure remains separate. Postresize640 hostpredicate failedbecauseMain diagnostics reached256cap: oldextent640Optionsrow survived whilelaterMain2360epoch10+fresh640BG300600+Mainlabels proven. Preparebounded exactowner resizehandoff; no rendererweakening.


Local62 typedOptions343 source compiled (onegeneratedfile,125s build, sourcepinsunchanged), ELF1976d1e7c3771ee0f50fe2461b9c90e7b3a24af7dbc158ea158ece0bc6dbb2e3. Sameactualnative/GPU300600/cachebyteguards; Setup unchanged. Runtime pending6401 B62isolatedX11, boundedMainresizehandoff from justproven2360 owner+fresh640BG/labels, revokedbeforechildinput. FirstlaunchshellCRLF failedbeforeapp; normalizedlauncherLF only/refrozen; no runtime state overwritten. No cloud/devicebuild.


B62 VERIFIED Options currenttyped343 binder28; actualEscape Fade12 GPUindices0..11, continuouschildreturnwide12frames thenmenuwide, zeroNativefallback,2freshMain. RootreviewedcurrentOptionsimages. BoundedMain640resizehandoffpassed andOpponentClassA form reached; frozenlabels/focusgate refusedbeforeRight, so classes notaccepted. RootviewedactualHDClassA failureimage. Clean0/all13/ELF/copiedassets unchanged/profile restored. Earlier60d fullSourcefailure remainsunexercised. Evidence ui-options-typed-sidebar-return-verified-local.json. Continue same62 hostfocusaudit, no rebuild.


B62b focusedclass test: removedwrongabsolutehover, sourcecarfocuspasses; actualClassA source12serial1/2authenticated+IMAGEselectedclassGeneration1. RequiredOPPONENT ART RENDER identity absent, as isOPPONENT NATIVE fullscene; stoppedbeforeRight. RootviewedHDClassA widget at640, notfullwideproof. Clean0/all13/ELF/assets/profile. Auditactualwidget-vsfullScene route beforehost/sourcechange. Same62P2bootstrap preparedheld withDonecommit andownactualreferencecapture; notexecuted. Options12HD andtitlefirstswapmilestones preservedindependently.


62d actual3classes12/13/14 source/native coverage; rootviewedall9steadyimages640/2360/1280, remasteredpictures/badges/rosters coherent/no widebars. InitialA exactqualifiedsource+retainedImage+nativeMTconstruction+visualreview (noinventedruntimeSHA); B/C actualdigestlogs matchfrozenprofiles. Steady OpponentBG compositor activatesafterRight. Returnobserver incorrectlyexpectedonlyinitialMT; noEscape sent, no gamefaultestablished. ActualBGbegin opponent-staging-only+sameSerialcorecommit300600/menuwide supportspropersteadyproof. Clean0/all13/ELF/assets/profile. Actualnative2360 photoanimations remainunverified becauseRights were640; focused62enative actor preparednext. Evidence ui-opponent-three-class-layouts-local.json.


P2-62 firstattempt failedimportbeforeapp (missingdiagnostichelper); fixedfromfrozen62d, actualWSLimportpassed. Actualrun14868 thenstoppedbeforecheckpoint/input at15s initialSetupgate; initialMTkind8 valid. Final log DOES have lateSetupNative371..374 andactual2360Setupformepoch3, so no-nativeclaimwouldbewrong. Clean0/all13/ELF/assets/profile. FreshP2-62b supportsactualinitialMT orsteadywithin30s, ownimages/nullablecontext/visualcontinuity; held. NativeClass62e41023 runningactual2360Rightanimations withtwo rootvisualcheckpoints andbothinitial/steady returnproof; no newbuild.


AUTO native62f actual2Rights2360 changesA->B->C, priorrootreviewedart/focus refs matched; actualnativefallback1frame each, originalFrameScope reason2 repeated/HDarmfailed. NOTevidenceoriginal21skip. Capturefamily4stage2 lacksMTslot0 becauseOpponent omittedfromexistingScopedHistory. ReturnReverse19/allframes/noNative+2Main verified. Clean0/all13/ELF/assets/profile. AUTO P2-62f1Right+6Down/finalRootDoneSplitreview; DoneFade12/noNative+2Main verified, thenhostdebugMainowner gatefailedbeforeP2. Clean0. Appliedgroup63 historyOpponent2files+boundedMTidentity+postflushmarker source5files, exacthashbackups; C++headercontext+movieheader syntaxpassed. Compile/runtimepending. SourceauditInfo/Records/Results routesimplemented butdedicatedwideoutputmissing; optionalclean565+HDtextfallbacknotauthored-imageacceptance. No cloudbuild.

Local integration63 built successfully with unchanged source pins; ELF ac92deba12e63fb34cb73260684fcde2e4de48c72130a17e4d96969d6399c08c. Actual2360x1640 two class changes: scoped outgoing history now captures/acquires and HD source arms, but21 frame-scope rejections per change; original UI fallback persists. Return19 also fails in this run, unlike62f; investigate earliest rejection/regression before next build. Actual app clean exit0,13resources/ELF/copiedassets unchanged/profile restored. No cloud or iPad acceptance. Evidence ui-opponent-scoped-history-local.json. Records responsive full-output implementation drafted privately, not integrated/compiled. Final lossless packing remains required.


Native63 earliest rejection localized: outgoing capture reaches family4 stage6; arm reason14 occurs because incoming ChildPhoto BuildScope31 still requires absent MTslot0 kind6/opponentInitial, so no incoming photo publication. Return fullSource can reject before consulting scoped history. Next patch must enroll actual incoming photo factory against exact current Opponent history domain and narrowly use authenticated343 history on return fullSource failure, preserving catalogue/planeReplay/100tiles/current upload/descriptor/GPU proof. Private draft in progress; no claimed fix and no next build yet.


Applied integration64: exact current scoped Opponent incoming-photo ownership and abort-safe return sidebar, plus owned fullwindow procedural Records/LapTimes presentation with retained live source and original content proportions. Eight Records files and two Opponent hooks backed up privately. Agent source hashes were normalized LF; all text matched exactly and raw-byte rollback/output pins recorded. Actual combined engine-header syntax passed; private generated renderer+Methods02 syntax passed including new ABI header. Full compile and runtime acceptance pending. No cloud/device build. Evidence ui-incoming-photo-records-integration-local.json.


Integration64 build PASS165s/sourcepinsunchanged ELFccbcc89e99040bc43ee3dbcbc2995fab84c9bc4647ee239c1c05dd85c9d77bcb. Actual native2360 twoRight classchanges: incomingcar13/14 authenticated100tiles publish; all21HDframes each and0FrameScope rejection. Overall changes stillFAIL because nativefirststatic handoff after21 persists. Actual return19PASS/noNative+2freshMain. Clean0/all13/ELF/copiedassets unchanged/profile restored. Records compiled, runtime untested. Correct route scope: normal TrackRecords contains record-lap-time table; PersonalStatistics/type21 and postrace22 are separate, no invented second LapTimes route. Continue firststatic handoff repair and held Records observer. No iPad/cloud acceptance.


CORRECTION64: handoff is NOT one native frame. Actual RLE choice native694→menuwide797 and native864→menuwide967 proves103 original/native presentations EACH after21HDpreviewframes. previousFrames21 describes precedingHDsequence, notnativegap. Evidence corrected. InitialFull copy-ctor child-source and copy-dtor rejected precede fallback; next acceptedsteadycore followsgap. Do not reduce acceptance to successful21alone.


Applied bounded default-off exact Initial343 acquire rejection diagnostics plus preview/sidebar association; original acceptance predicates unchanged. Actual engine-header syntax passes, not compiled yet. Next diagnostic run must identify target/header/pixel difference and compare already-read raw343 against exactcurrentacceptedhistory; do not invent a new sourcefactory from unknown mismatch. Records64 actualattempt stoppedbeforemenu because inherited startup gate timedout with no moviekeys; clean0/all13/ELF/copiedassets unchanged/profile restored. No Records runtimeacceptance. Fresh64b observer using proven guardedmovie path prepared next; keep compiled64 while running it before diagnostic65 build.


Results iPad authored draft root-reviewed for faithful fold/material/grid composition, privateonly1504x1046 nativeoutput (notrequested2360x1640, noresampling/crop). Private3variantpack8286561B exact2oldRGBAentries retained+newRGBunchanged/opaquealpha; strictlosslessroundtrip passed. Notactivated. ExistingResults ratioselection reads4:3nativeviewport, so fullwindowrenderer+NameEntry parent integration STILLrequired; addingpackalone doesnotenablewideart. Private implementationinprogress. Evidence ui-results-ipad-artwork-lossless-draft.json.


Records64b/c/d allclean terminal; b/c stoppedbyhost fresh-log/hover-sensitive classifier gates, noRecordsentered. Root actual64cimage showscorrectHDLocation/trackinfoselected. 64d eliminatedclassifier butactualafterONE Down hadnativeRLE671→wide684=13presentations; recordasrealLocationappearancefailure, NOT reason toabortharmlessGUI/navigationtowardindependentRecords. Sourceentry+ownedfocus/PID/window+ROOTvisiblepage/row isGUIauthority; renderer acceptance measuredseparately. Fresh64e actor mustcollecttwoRecordsratios andrecordappearancefailures withoutmaskingthem. All13/ELF/copiedassets/profilecleanverified. Evidence ui-location-arrow-native-fallback-local.json.


ActualRecords64e reachednormalTrackRecords via3Down+rootReturn; rootCP8 verifiesHDgreenRecords/table. Nativeauthored300600finish observed; NOrecords-wide/WIDEpublication at2360or1280, sofullwindow FAIL. Earlynative-stable0 wasoutgoingLocationtransition (mustNOTcallstableRecords);1280sampleoriginalpixelatedRecords+bars;laterCP8HDnative4:3. Escapeonce returnedactualLocationpage butoldBG/logo+HDtext, notHDreturnacceptance. Rootreview savedprivate; rawRESULT remainsfalse, nopassrewrite. Clean0/all13/ELF/copiedassets unchanged/profile restored. Investigate newRecords frame-life/current/preparerevocation againstactualqueue semantics, notspeculativeguardweakening. FuturestablecapturewaitmustuseactualRecordspage/receipt. Evidence ui-records-fullwindow-runtime-failure-local.json.


Pending65 nowincludes3exactdiagnosticfiles: OpponentInitialSidebarGameHooks, OpponentInitialFullGameHooks, LocationSceneQueueAdapter; exactrawpins/backups andcombinedMethods01headersyntaxPASS. Notfrozen/compiled. LastactualappRecords64e terminalclean; latestrealcleanupreceipt pinnedfornextrun. Recordswide failureunderprivatequeue/lifetimeaudit, Resultswide+NameEntryfullwindow implementationdraft private; noUResults/packactivation. Nextone65localbuild shouldgroupneededdiagnostics/provenfixes, preserveacceptance; noCloud/iPadtestuntilcoherentUI.


Appliedgroup65ninefiles: exactOpponent343/Locationfirstsource diagnostics3files +Records/sharedownedresponsive textfix6files. Definitelegacysoftwaretextvalidator4:3restriction fixedbyEXPLICITnewownedresponsiveAPI; oldhelperbody/validatorunchanged. Sourceowner/lifetime/content guards retained, notrelaxedfromunprovenCPUqueue race. NewboundedRecords prepare/revoke/source reasons. ActualcombinedMethods01headersyntaxPASS, independentreviewPASS, privateshadowC/Methods02syntaxPASS. Fullcompile/runtimepending. Noartpackchange. Resultswide+NameEntryprivateimplementationrebaseneededonnewRecords/SharedAPI beforelateractivation. LatestactualcleanRecords64e. Freeze65beforeonebuild; noUeditsduringbuild.


Full65compileFAILED beforelink: Methods09includesOpponentInitialSidebarwithoutContinuousNativeUploadHooks, so newdiagnosticuploadEventsundeclared. Failedlog/progressretained; appELF64 remainsbaseline. CorrectedEXPLICITheaderdependency inOpponentInitialSidebar (rawf99899d8e39f8a1cafc7cbe3c9356c03e87848d195b08ea8f0a0b8b335076b0f); actualMethods01ANDMethods09header-contextsyntaxPASS. Group66retry retains9sourcechanges/Recordsresponsivefix+sameboundeddiagnostics; noacceptedpredicateweakening. Frozen66/fullbuildnext; Results/Nameprivateonly. Observers66heldunknownactualSHA/latestrealcleanupRecords64e, notinvented65runtime.


Local build 66 linked successfully. Records prepared a complete widescreen image with its source still current, then the native triangle hook revoked it before presentation. The original Records handler draws the cursor between the Records commit and the swap. The pending fix captures that specific draw and replays it on the widescreen image. Records walkthrough exited cleanly after its manual review timed out, before Escape. Location arrow movement stayed widescreen in this run. The earlier 13-frame fallback remains intermittent.

Both Opponent class changes presented 21 HD animation frames followed by 103 original presentations before the steady HD menu. The first refresh selected a different registered sidebar bitmap. The second failed to reacquire the original entry snapshot. The 19-frame return passed. Both local runs restored the copied profile and preserved all 13 artwork resources.

Local build 67 compiled the Records cursor replay. Its walkthrough reached the first Records native pass and then stopped submitting renderer work. The cursor constructor held GuestCopyTransaction's non-recursive mutex while calling sourceCurrent(), which reads through another GuestCopyTransaction. The source correction releases the transaction before validation and releases the quad transaction before renderer submission. The running executable predates this correction. Rebuild after the isolated observer finishes cleanup, then repeat both aspect ratios and the return check.

Build 68 compiled the Records lock-scope correction and started the isolated walkthrough. Separately, the Opponent refresh now captures the currently selected original 343x480 sidebar buffer and verifies the exact snapshot through row expansion and the final copy. Initial entry retains its original proof. The refresh header passed the generated engine context syntax check. The change is pending full build and runtime verification against both class-change paths.

Local build 68 passed compilation and entered Records without the build 67 deadlock. Its cursor replay completed and the widescreen route presented at 2360x1640 and once at 1280x720. The subsequent 960x720 native target repeatedly rejected the authored backdrop: the legacy projection's float inverse is 639.99994 and the backdrop required at least 640. The next build accepts the existing small projection tolerance at the horizontal limits and clamps the resulting canvas width to its bounds. The observer sent Escape after restoring 2360x1640. The complete aspect-ratio acceptance remains false.

Local build 69 passed compile and source integrity checks with the Records projection correction and Opponent refresh changes. The Records68 return screenshots confirm visible Location and the retained Track Records selection. They also show side bars and the original Location backdrop during the sampled return. The copy trace shows the reopened Location owner had a verified base but lacked its initial child sidebar proof. Track Records return navigation is confirmed locally, while its HD handoff remains open.

Build 69 passed the desktop Track Records renderer walkthrough. The presentation stayed on records-wide from frame 2514 at 2360x1640 through resize to 1280x720 at frame 2530 and restoration to 2360x1640 at frame 2552. There were 22 consecutive 16:9 presentations before restoration, no original presentation while Records was active, and visible cursor and full-width table in the 1280x720 screenshot. The observer sent one Escape. The subsequent Location handoff entered the original presentation path at frame 2577 and remains an open UI issue. The run exited 0, restored the copied profile, and verified all 13 resources. The separate Opponent test has launched on this executable.

Build 69 Opponent walkthrough completed both inputs and returned to Main in 19 wide frames. First change presented all 21 HD animation frames (690 to 711) and went directly to the HD full-width cached image at 711. Second change could not arm its HD source. Its source selection rejected the outgoing scene at stage 1 with upload event zero while the prior typed full-image route was presenting. The native route appeared at 807 for 19 frames before wide presentation resumed at 826. The correction must carry a current authenticated outgoing source through another class change. The first change's success does not establish that behavior for the second. Test exited 0 with copied profile and all resources verified.

The Records69 return trace further localizes the Location fallback. Main origin acquisition succeeded with two live entries on initial entry and found zero entries on the return. Location then reopened with a valid base but no initial child proof. The return repair requires authenticated Location provenance and a fresh current pane, rather than relabeling that pane as Main artwork.

Build 72 desktop Opponents: both real class changes armed via the bounded static history, each rendered 21 consecutive child-exit-wide presentations and handed straight to the new HD image. No original presentation occurred between the two inputs and their verified steady states. The refreshed sidebar is read twice and checked against the exact pane identity, RGB565 copy, descriptor keys and upload event. Build 71's attempted reuse of the entry sidebar failed at the resource comparison. Both build 72 class receipts are positive. The separate Escape step was skipped after all 256 kind-10 per-frame trace records had been emitted, leaving no fresh source log in the final observer window. The run exited 0 and restored the copied profile. All 13 resources, the copied game assets and the binary passed integrity checks. Device validation remains open.

The Results and record-name source changes are staged privately as one application batch, with the rebased Records cursor hooks and matching CMake/package version of the three-variant backdrop. The tested Records renderer is retained as-is. The batch is pending generated build and in-game validation.

Build 73 compiled the responsive Race Summary and record-name integration locally. The staged 18-file source batch was applied after verifying base and proposed hashes and saving rollback copies. Engine and renderer generation, CMake configuration and app linking passed. Binary and all 13 resource hashes were verified against build 72 plus the expected lossless three-variant Results pack. No in-game Results or Name Entry acceptance yet.

The build 73 desktop controller fixture completed a real two-lap Proving Grounds race in the McLaren F1. Race Summary published the authored backdrop and text through the qualified 300-upload/300-select/600-triangle native compositor. Actual 1280x720 and 1180x820 presents remained native. The 16:9 screenshot confirms black side bars and sharp remastered elements inside the central image. No full-width Results present was logged. The original record check returned 0 because this time was above the cutoff, so the run did not enter the name dialog. The generated 419044 tail performs 4274F0 and then the optional 4267C0 cursor call before swap. Generic Results mutation hooks currently revoke the wide image on such late draws. This is a source-supported investigation target and the exact failing condition has not been instrumented.

The controller fixture only replaced the desktop entry object. Engine and renderer objects were taken from build 73. The app exited 0 and the copied profile, binary and all 13 packs passed checks. The race legitimately wrote replay.rpy in the copied test directory. Byte-level inventory comparison found it was the sole change. The new replay was retained in the private run evidence and the shared copied replay was restored to its exact prior hash. The next test should snapshot the replay before driving.

Build 74 completed a second real Proving Grounds race and captured the specific Results wide failure. The 1280x720 and 1180x820 backdrop variants both prepared successfully. Each completed wide frame was subsequently revoked by a triangle before swap. The adjacent actual ARGB4444 art ID 31 draw matches the cursor mark used in Main. The next correction should carry the exact Results cursor draw through the wide render, keeping unknown draw invalidation. No widescreen or Name Entry pass is claimed. The repeat exited 0 with verified binary, all 13 packs, copied assets, profile and automatically restored replay intact.

Build 75's desktop race completed and reproduced the fallback at both tested wide ratios. The full-width draw still finished, but the original triangle did not pass the new cursor path. The generated engine has the lexical scope at all six original Results cursor call sites and its new requests appear between successive Results draw-request numbers. That alone does not prove the scope was accepted. The follow-up trace distinguishes source refusal, begin refusal, unarmed scope, mark mismatch and vertex mismatch. The normal run exited 0 and verified all 13 packs, copied profile, copied replay and original copied assets. A separate test fixture uses the existing private in-memory qualifying-record setup and its own disposable game copy to reach the real record-name prompt. Production engine and app outputs exclude that fixture.

Build 75's disposable qualification fixture completed Proving Grounds and entered the original record-name handler after seeding its in-memory cutoff. Build 75 engine and renderer produced 13 consecutive authenticated 800x450 name-entry presents. Viewed captures show the initial Player1 value, a typed lowercase a and caret, and the original handler returning on Escape. The original game then displayed its default Player1 record on the narrower Results screen. The Omega logo is visible in that later narrower screenshot and absent from the wide prompt's retained parent. Attribution to a particular source operation is still open. Scope is one real 16:9 child interaction on desktop with diagnostic qualification, not natural record qualification, successful full-width return or iPad acceptance. The fixture used a disposable full game copy, exited 0 and restored its profile and replay.

Build 76 compiled with separate cursor admission traces. The runner starts recording at a wide ratio before the first post-race frame to avoid exhausting its CPU trace budget at 4:3. This test is underway.

Build 76 completed a race with the cursor trace enabled before the first wide frame. Original engine cursor requests were queued for each Results content revision, but the corresponding GPU frame had no bound cursor when its first late triangle arrived. CPU queue acceptance is not evidence that renderer cursor binding succeeded. All 13 packs and the copied profile, replay and assets verified after terminal exit 0.

Build 77 separates the lexical cursor receipt from frame preparation. It binds the receipt to the exact game, epoch, content revision and renderer context at the first actual triangle, while preserving the existing native texture, sampling and geometry checks. Unknown geometry still revokes the wide frame. Local compilation passed and the repeated race check was started. Separately, 26 hash-identical pack copies from two terminal evidence fixtures were removed, recovering 979.8 MiB. Their logs, manifests and retained byte-equivalent packs remain available.

Build 77 finished the desktop race, but the first post-composite triangle still arrived before a cursor receipt at all three tested dimensions. The test exited 0 and restored the copied profile and replay with all asset and pack hashes verified. Generated source review found a separate original branding call, 4274F0, in all six Results forms between the primary image draw and the cursor call. The function emits a textured quad. Build 78 adds an exact lexical scope for that call, validates its original 256-pixel ARGB1555 selection and current Results frame, then replays the two submitted triangles before the existing cursor scope. This is a source-supported repair for the extra draw. Runtime acceptance is pending. The same build caches the unchanged wide backdrop texture within its renderer context, avoiding a repeated upload while keeping per-frame overlay uploads and ownership checks.

Build 78's desktop race reproduced side bars at 800x450, 1280x720 and 1180x820. The original post-commit branding and cursor selected distinct sources in the queue. Compared with build 77, the first late draw no longer reported a missing receipt, but the branding draw did not pass its live renderer validation. The next candidate checks the exact original selected texture on texture unit 0 and the actual framebuffer, viewport, matrix and shader. It preserves original blend and fog state and traces the individual branding checks. Build 78 cleanup verified all 13 packs, original copied data, copied profile and replay after exit 0.

Build 79 compiled with a stable source snapshot and the matching three-variant artwork. The normal desktop race check is underway. Independently, the generated original name dialog restores and draws the saved parent in 401F40 before 4824BC deselects its surface. The prior proof attempted its pixel comparison only when the handler exited, after deselection. The pending source change compares exact copied parent pixels while the restored surface is still selected and requires the same live Results parent before allowing the existing return compositor. It is staged in the working source for the next grouped build. Actual name-dialog return remains unverified.

Build 79 completed the race and reached the exact cursor check after the earlier branding draw, with no branding-draw refusal in the captured sequence. The cursor mark guard refused the actual subsequent triangle at all three outputs, so no wide Results presentation is claimed. The adjacent native trace shows the authored 32-pixel cursor mark really drew. The next build shares the branding draw's actual framebuffer, viewport, projection, program and texture-unit checks with the cursor. The cursor still requires its exact mark address, descriptor-derived native texture, art generation, context and stable binding revision. The name-child return proof is included in this same grouped build. Build 79 exited 0 with all 13 packs, copied original assets, profile and replay verified.

Build 80 desktop qualification fixture passed the in-game Results widescreen regression. The first wide Summary at frame 14630 went directly into eleven consecutive 800x450 wide name-entry frames, then returned directly to wide Summary at frame 14642 after Escape. The exact original selected parent was authenticated before deselection. Subsequent presents remained wide through real resizes to 1280x720 and 1180x820, with no native fallback in the observed post-race route. Screenshots show the typed lowercase a, original animated Omega logo, and cursor in the full-width layout. The tested entry used a private in-memory qualifying threshold on disposable game data. The name prompt was exercised at 16:9 only. Physical iPad acceptance still needs verification. The unseeded build 80 check is recorded below. Fixture exit 0, replay and copied profile restored, all 13 packs and copied original assets verified.

Build 80 also finished an unseeded two-lap Proving Grounds race using an isolated virtual controller. The original qualification returned no new record. The game presented the remastered Summary at 800x450 and stayed on the wide path after resize to 1280x720 and 1180x820. Captures show the Omega logo, Summary text, original result values and cursor. The observer exited cleanly and verified its executable, all 13 resource hashes and the restored copied profile, replay and assets. Natural record entry and iPad presentation remain unverified.

Removed 26 verified byte-identical artwork copies from completed build 76 and 77 test fixtures, freeing 979.8 MiB. Their manifests, hashes, logs and retained equivalent packs remain available.

Build 80 also finished a normal, unseeded two-lap Proving Grounds race in the desktop harness. Its first post-race presentation used the full-width Results route at frame 13947. The same route survived resizing to 1280x720 and 1180x820. The original record qualifier returned false. The copied profile and replay were restored, and all thirteen art packs, the binary and copied original assets verified after shutdown. This establishes the ordinary desktop race path. The iPad build remains unverified.

Build 81 reproduced the Location return fallback on desktop. Track Records reached the wide route at 2360x1640 and 1280x720. After restoring 2360x1640 and pressing Escape once, frame 963 used the original narrow route. The original Location handler selected a fresh 343x480 pane on return from descriptor 004049e1. Its RGB565 digest differed from the first Main-to-Location pane both before and after painting the Location caption. The Main history for that entry held zero eligible frames. A return fix needs a fresh proof of the current Location pane and its Records provenance. The first-entry Main proof cannot establish this return source.

Build 81 observer and launcher exited successfully. All thirteen selected packs and the executable verified, the copied profile was restored and the app exited. The second owned return screenshot shows the selected Track Records row back in Location, with black side bars and the old backdrop. The first native return presentation was frame 963. Its transition packet used the generic kind 1 slot 1 while the Location base was verified and the initial-child proof was absent. In the original 40C160 handler, the signed high-word selector at 4D2316 chooses one of the two 343-pixel working panes before the exact 40C110 caption call. The same selected pane's bitmap is then passed to 428250 after deselection. This is the source boundary for a scoped Records-return proof. Check the selected handle and freshly painted caption there, then carry the measured source through the existing slot 0 upload and slot 1 merge checks. Current screenshots and traces cover the sampled return transition. They do not establish behavior after the transition has settled.

Build 82 desktop test confirmed the normal Records descriptor selects a live Location working pane and produces a current, target-matching caption proof. The return used the other double-buffer handle compared with build 81, and both return pixel digests differed. A patch cannot rely on either hardcoded handle or a digest from a single run. The first return screenshot has the old panel and side bars during the animation. The final screenshot after a passive wait shows the wide authored Location panel with the Track Records row retained and no additional input. The trace records native presentation from frame 2488 until menu-wide resumed at frame 2629, a 141-frame interval on this run. Fix scope is the return animation and the following cached full-frame presentation before the regular Location producer becomes current.

Build 84 desktop runtime verified the first visible Records-to-Location frame uses the wide HD transition with the animated logo and authored map. The rendered route starts at frame 787 and stays wide for 17 fade frames. At frame 804 it switches to the narrow fallback for 124 frames, until the normal wide Location scene resumes at frame 928. The first two owned screenshots show the corrected wide fade and the later narrow frame with the preview, respectively. The exact first full-frame copy reads bitmap 101b3d68 while the fade caption proof owns bitmap 101636d8. The existing copy validator rejects the mismatched buffer. The full-cache stage needs a fresh source for the buffer used in the copy. Track Records itself presented wide at both 2360x1640 and 1280x720 during this run. No full return or iPad acceptance yet.

Build 85 selected-source full-frame fix passed the desktop copy and presentation boundary. On the Records-to-Location route, the authored fade starts at frame 769 for 17 frames, the intermediate native photo transition starts at frame 786 for 21 frames, and the full authored cache is presented from frame 807 using kind 9 and all 300 tile selections and 600 triangles. The final capture shows the full-width Location menu with the original Track Records selection and the authored North Country preview. The first intermediate fallback coincides with the newly published two-layer child photo source; the remaining renderer work is the scoped Records-return photo/countdown path. This run reduced the native interval from 124 to 21 frames on desktop. Physical iPad acceptance remains open.

Build 86 desktop Records-to-Location route passed end to end at 2360x1640. Wide fade starts at frame 769, the wide photo animation at frame 786, and the full wide Location cache at frame 807. Zero native fallback route changes were logged from the Escape through the last settled capture. Owned screenshots confirm the authored photo moving into place, full-width backdrop and preserved Track Records selection. Track Records itself was also rendered wide at 1280x720. The actual return was exercised at the iPad-shaped desktop extent; the 16:9 return and physical iPad remain unverified.

Build 86 also passed the Records-to-Location route at 1280x720. On the corrected second observer run, frame 760 starts the wide fade, frame 777 starts the wide photo animation, and frame 798 starts the wide full cache. There were zero native fallback route changes through the settled capture. The first attempt exited cleanly at a test-only focus check which still required 2360x1640, before sending Escape. The second run used the same binary and a 1280x720 check for the single Escape and four output captures. Visual review found a remaining spatial change at 16:9 between the photo animation and the settled cache: the Location caption and right-hand branding change horizontal positions. The whole return stays HD; alignment across that handoff still needs work. Physical iPad remains unverified.

Build 87 desktop 1280x720 rerun verified consistent Locations alignment on the Records return. The Location label, SE mark, crest, car name and tier badge hold their horizontal anchors between the photo animation and settled captures. Frame 783 starts the wide fade, frame 800 starts the wide photo slide, and frame 821 starts the settled wide menu. The return trace contains no native fallback. The authored photo continues the original slide and the Track Records row stays selected. Build 87 physical iPad verification remains open.

Grouped follow-up after build 87. The same desktop run reproduced 19 original-layout presentations during ordinary Location navigation before entering Records, at frames 644 through 662. Location had invalidated its prepared scene for the next iteration, and the trace recorded two logo draws and no completed copy. A new complete Location packet restored wide output on frame 663. This is a separate live issue from the now-wide Records return. Group its cache/redraw correction with current Track Info and Personal Statistics entry and return review. Use the existing executable and prior pixel evidence for diagnosis, then compile and exercise the related changes as one batch. Keep the Records return and post-race Results routes in that regression. No Apple build yet.

Build 88 grouped desktop run used ordinary input to open Records, Track Info and Personal Statistics. The initial Location row navigation stayed wide continuously until Records entry. The 1280x720 Records return stayed wide from frame 747, through photo motion at 764 and the full cache at 785. The separate Personal Statistics return also passed all three wide phases at 2360x1640, frames 1613, 1630 and 1651, with a newly sealed Stats sidebar and no original-layout frame in the return trace. Both information pages still present through the 4:3 native route. North Country Track Info published authored static art plus HD text, and the observed Stats page used the authored crest and table. The separate Track Info handoff reached steady wide Location at frame 1332. Moving down after that return still reproduced 16 original-layout frames between 1388 and 1404. Restoring iPad proportions after the 16:9 Records return showed one native frame at 836. Group the remaining first-frame and aspect fixes around those measured paths. No Apple or physical device verification.

Build 88's second Location input fallback followed a different original redraw interruption. At frame 1388 the collector retained a complete linked group and copy, but the Location upload event was zero and the existing upload event remained unchanged at 465. Original 40BBE0 polls input once after the complete copy and before 428250 uploads it. The source now records that exact abort phase. It retains the completed prior backdrop only if the Location owner, base, archive, preview, full target identity, upload event and all slot 1 descriptors remain valid. The renderer must still match the uploaded pixels and witness all 300 tiles; the original game tail draws the current menu selection. This second-phase fix is staged for the grouped local build with information page layout work.

Build 89 grouped desktop run verified Track Info and Personal Statistics at both 2360x1640 and 1280x720, including returning to the first ratio while each page remained open. The 16:9 captures preserve the complete North Country route, authored track image, headings, statistics graph, and live cursor. The side treatment is provisional; the Track Info edge strip is visible and final wider art remains open. Records and Personal Statistics still return to Location through wide fade, photo, and full-cache routes. The Location input sequence after Track Info stayed wide once the menu settled, so the 16-frame post-copy interruption from build 88 did not recur in this run. Separate defects remain: Track Info entry stayed on the native route for 107 frames before its first wide page, and its return spent 141 frames on the native route before the settled wide Location scene. The one-frame aspect-change fallback also reproduced. This records desktop behavior only.

### Track Info entry and responsive information pages

Local build 91 exercised Track Records, Track Info and Personal Statistics with the copied game data. The Track Info entry committed its HD sidebar on 59 observed transition draws, and the return authenticated its Location sidebar. The settled Track Info page was reviewed at 2360×1640 and 1280×720. A dark vertical boundary remains visible. Sampled green-channel changes at the left join peaked at 13.35 and 19.06 respectively. The run exited cleanly and restored its copied profile with all 13 runtime assets and the executable unchanged.

The next grouped candidate adds the independently authenticated Track Info background fade, wide presentation of completed HD animation frames, and a five-source-pixel inset at the visible viewport join. The outgoing Location transition and the next grouped runtime verification remain pending. Physical iPadOS verification remains open.

Build 92 completed the grouped desktop run and restored the copied profile with all 13 resources intact. The Track Info incoming slide and two-layer fade selected wide output. The left edge comparison at 1280×720 and 2360×1640 measured 19.06 to 0.85 and 13.35 to 0.91 respectively.

The 48 outgoing frames have a specific missing texture. The fade has a graded map in original slot 1, but the Location sidebar has no ordinary transition packet in slot 0. The outgoing slide likewise has no slot 0 packet. The preceding Child Return observer had a separately verified 300-tile sidebar. Investigate a narrow handoff of that proof to the Track Info owner, keeping the original shade, geometry, and texture comparisons. A new unrelated texture upload invalidates the prior complete-frame owner, so reusing that entire owner would bypass an important lifetime check. Verify the exact sidebar payload and renderer tile identities when testing a handoff. The current result verifies desktop behavior only.
Build 93 exercised the outgoing handoff, which committed zero draws in this run. Before Track Info took ownership, the preceding Location reverse transition lacked a current semantic snapshot (capture stage 1); the original renderer still drew the intended frames. The next source change reuses Location’s matching archived copy and current texture-upload record for this boundary, then runs the same new upload and texture checks. Incoming authored fade and settled Track Info remained functional.

Build 96 completes the North Country Track Info opening in the observed desktop route. All 48 outgoing draws committed the authenticated HD Location sidebar, then the incoming slide and fade continued without a native presentation fallback in the route trace. The remastered return passed, and settled 16:9 and iPad-ratio samples were captured. The desktop app exited cleanly and restored the copied profile. See evidence/ui-track-info-complete-swipe-local.json. Other tracks, wider menu coverage, and physical iPad verification remain open.

### Resolver candidate decision

The bounded split-screen profile attributed 2.722 sampled CPU seconds to
address resolution. Its three largest direct callers accounted for 1.17 of
those seconds. Mutex samples were concentrated in guest copies and transaction
setup. Unresolved stacks were 62.5% of sampled CPU, so this result cannot rank
all engine and renderer costs.

Moving resolver exception formatting into a cold helper reduced median time
17.37% in the isolated DATA kernel and 18.95% in the mixed kernel. It slowed
both full North Country desktop comparisons. Median swap spacing changed from
29.85 to 30.55 ms in A/B order and from 29.33 to 30.56 ms in B/A order. The
engine thread CPU median also increased in both orders. Keep the original
resolver in the build and use the paired app metric as the promotion gate.
These comparisons used software GLES on WSL and cannot establish iPad results.


Showcase idle redraw follow-up: the original game also calls the full-page bitmap draw outside its normal Presentation scope. An exact published-bitmap qualification now covers that lexical draw, retaining target, pixel, epoch, archive and lifetime checks at entry and completion. The paired 20-second local idle replay changed from one information-wide decline to zero; the extra draw itself completed remastered, and the return route exited cleanly. Evidence: evidence/ui-showcase-idle-redraw-local.json. Repeated car changes, 16:9 and physical iPad acceptance remain pending.

Showcase branding visual correction: magnification of the unchanged settled-return capture identifies the Jaguar leaper and mirrored JAGUAR lettering in the original spinning-logo animation. The prior Mazda identification was an inspection error, not evidence of a settled identity mismatch. No manufacturer remapping is justified by that capture. The previous McLaren mark seen early in the return remains a separate observation; determine whether it follows the original branding transition before changing behavior.

History/slideshow grouped local baseline: both pages and returns completed cleanly. History published 17 remastered frames but remained pillarboxed because its successful authored commit omitted the existing style-3 information-wide mark. The scoped mark compiled and the identical candidate replay completed both routes and returns cleanly with zero information-wide declines. Screenshot review confirms History side bars removed at 708x492; edge reflection is visible and remains a separate art-quality limitation. The 800x450 follow-up also completed both routes and returns with zero information-wide declines; its History screenshot was reviewed. Fresh renderer generation passed with all 46 typed exports and a byte-identical History helper. Physical validation and creative edge-extension quality remain pending. Slideshow published remastered photograph/footer and filled output, with visibly extended side strips; this is not final artwork approval. Evidence: evidence/ui-history-wide-local.json.

Panoramic History source audit: existing provider/loader schema requires exact 4:3 backgrounds and complete pinned payload consumption. Wider art needs a separate bounded, source-keyed immutable side resource joined to the completed History frame and context; do not widen the existing canvas or append data to its pack. Current center-preserving draft has visible seams and is not activated. Implementation requirements and source pins: evidence/ui-history-panorama-integration-audit.json.

History panorama implementation foundation: immutable source/base-generation-bound ownership and bounded whole-file-authenticated stream loading now pass optimized and ASan/UBSan checks, including malformed/tampered/trailing data and retained lifetimes. The standalone payload is currently raw lossless RGBA; final compressed packaging, application activation, FIFO/context integration, finished seams and physical acceptance remain incomplete. No active artwork changes. Evidence: evidence/ui-history-panorama-owner-local.json.

History panorama CPU integration: StaticArt retains only the optional panorama matching its exact source and base generation. Existing frame validation rechecks registry owner identity; base-art clear removes registry entries. Replacement/clear/retained-memory cases pass optimized, sanitizer and registered CTest checks. All four affected engine units compile; provider syntax passes. No game relink, GPU presentation, draft activation or physical acceptance is claimed. Evidence: evidence/ui-history-panorama-owner-local.json.

History panorama GPU handoff compiled locally: ABI schema 2 carries immutable source/base-generation identity, bytes/digest and paired ownership/current callbacks. Presentation retains the lease, caches a context-owned texture by pixel hash, samples panorama only outside the central page, restores texture unit 1/unpack state and releases after presentation/retirement. The local app linked and owner CTest passed. These are implementation/compile claims, not GPU pixel/state or in-game panorama acceptance; no draft is activated. Synthetic GPU verification, authoring/activation, seams and iPad checks remain. Evidence: evidence/ui-history-panorama-owner-local.json.

History artwork batch: one bounded original-data local route captured all seven manufacturer archives, then slideshow and Player return, with clean shutdown and zero information-wide declines. Six new side-extension drafts now complement the earlier McLaren candidate. Actual native central rectangles remain pixel-identical in all seven 800x450 layout previews; other-ratio previews are composition checks only. The first-pass extensions still show brightness and architectural join mismatches; the Isdera lighting revision remains unfinished as well. Do not activate these drafts or call the art complete. Preserve the generated candidates and exact prompts privately, correct the joins, then review/activate finished resources together before another Apple build. No game relink or cloud build was performed for these drafts. Evidence: evidence/ui-history-all-archives-artwork-local.json.

History background-pair authoring prototype: locked older 4:3 backgrounds retain their edge vignettes, so independently generated side scenery leaves visible joins. A coherent Ferrari widescreen candidate now supplies both a central 1440x1080 background and its matching 1920x1080 panorama. The new lossless pair packer preserves logical bounds and all 48 unrelated sources, including every native-language title. Six bounded packer checks pass; private actual-data resources round-trip exactly. This intentionally revises the static photographic artwork, while native live text remains separate. The prototype still needs subject/logo/layout and live-text-clearance review, then the remaining six backgrounds must follow the same authored approach before grouped activation. No application or Apple build was made. Evidence: evidence/ui-history-background-pair-local.json.


History coherent artwork batch: all seven continuous scenery plates are authored. Existing remastered car photographs and manufacturer marks are placed with shared source-derived coordinates and soft shadows; source-color mattes remove the Jaguar/Italdesign footer rectangles. Footer boundaries are normalized, and Jaguar/Lotus portrait positions keep the heads inside the 4:3 crop. Wide and central layout sheets were reviewed. One grouped resource set contains the revised 49-image History pack plus seven panoramas, totaling 56.53 MiB with lossless compression. The 42 title sources are byte-identical; every panorama passes the existing production C++ decoder and its expected pixel digest. These candidates are not activated, and live text/transition behavior is not yet verified. No application or Apple build was made for this batch. Evidence: evidence/ui-history-coherent-artwork-batch-local.json.


Coherent History original-game acceptance: the grouped candidate built locally and all four registered History checks passed. Both 800x450 and 708x492 routes presented sources 0,7,14,21,28,35,42, with two bounded panorama records per source, zero logged information-wide declines, slideshow/Showcase/Player returns and clean game shutdown. Fourteen actual screenshots and live captions were reviewed. Prior photographs remain source-derived; measured interior differences reflect the new resampling pipeline and are not claimed pixel-identical. The experiment restored the exact baseline executable, original resource symlink and target digest, source pin, cached helper and inactive panorama configuration. The only lasting renderer change is the bounded per-manufacturer diagnostic. The shipping pack/injector still uses the earlier History pin, so promote these together before a future Apple candidate; do not inject this resource into an older Apple base. No cloud build or physical iPad test was performed. Evidence: evidence/ui-history-coherent-runtime-local.json.


Matched History release integration: the native base pin, shared metadata catalog, fresh-build panorama defaults and private injector now agree on the locally accepted set. History injection requires all eight files and their release identifiers in the main executable, with raw/hexadecimal and chunk-boundary matching; an older baseline is rejected before output. Twenty personal resources are recognized. Eight delivery, three metadata-only CMake, two existing class delivery, four workflow input, six unsigned-base and two Linux bundle checks pass. All eight real resources match the catalog, and the guard accepts the tested local ELF candidate while rejecting the prior baseline. Linux preflight runs the lightweight release checks before dependency setup or compilation. No cloud job, actual Apple-package injection, billing change or iPad test occurred. Evidence: evidence/ui-history-private-release-integration-local.json.


Parent semantic fades: the exact current typed Setup/Options full frame now owns its authored sidebar during the original fade. Both 800x450 and 708x492 grouped routes completed cleanly, restored copied profiles and preserved records, with 60 HD fade frames per route. Legacy original-cache pixel matching remains for schema 1; schema 2 instead validates the complete parent source plus current native sidebar bindings and the unchanged 300-selection/600-triangle pass. Existing real GLES regression passed. Evidence: evidence/ui-parent-semantic-fade-local.json.

The next first-frame hypothesis did not fix the gap: allowing a sealed sidebar during an open iteration and allowing cursor-hidden presentation still left two native frames at each nested dialog entry and 115 at each return. Both changes were reverted. The first Audio full frame is accepted before the first sidebar cache packet exists; admission rejects it as current-sidebar. Follow the initial source-cache production path rather than widening admission further. One separate focused Options return lacked its published source owner; the two complete grouped routes passed, so retain that intermittent gap as open. No cloud build or physical-device acceptance.


Nested dialog first-frame handoff: Audio, Controls and Graphics now publish their first complete original frame as HD at both 800x450 and 708x492 in the direct Options replay. All six entries used pass 1 and the unchanged 300-selection/600-triangle native witness, with no original-layout presentation between the observed parent fade and dialog. Both routes exited cleanly; durable backups restored all protected copied files and saved records were unchanged. The four affected scene/queue regressions pass. The source handoff retains immutable, exact Options sidebar content rather than the retired Options owner. Its panel, words, logos and caption are independently verified at the earlier caption-completion point, before a full upload may be interrupted. Graphics keeps its independent text and publisher thread identities. Evidence: evidence/ui-nested-first-frame-handoff-local.json.

This does not complete menu continuity: interrupted Options navigation and nested dialog return animations still select original-layout output. Next, preserve a genuinely completed Options full-source packet across freshScene only under its same live owner/domain and original upload/texture proof, following the existing Setup completed-source pattern. Then extend explicit completed dialog content into the original outgoing transition scopes. A prior replay was interrupted and a separate Setup keypress was missed; retain those failures, not acceptance claims. Other UI routes and physical device/performance/battery validation remain open. No Apple build or publication occurred.


Options completed-source reuse: a distinct Options upload-domain owner and GPU seal now retain only a successfully completed original source. Reuse checks the same menu/game/catalog/archive/epoch/upload and exact original early-return receipt, then the current descriptor and renderer texture witnesses. Overlapping uploads and context retirement revoke it. The actual native pass still runs on every reuse; no partial new redraw is promoted. The normal 800x450 and 708x492 routes and a held-navigation 708x492 route all exited cleanly, restored protected copied files and preserved records. Retained-source begins completed the native 300/600 witness in the original game, including a post-copy abort with no current published source. Parent fades used the matching completed source. All observed non-startup native intervals in these three routes were the three nested dialog returns, each 115 frames; these remain unfinished. Phases 0, 1 and 3 are guarded but not directly exercised for Options yet.

Fresh renderer preparation passed its reverse audit and produced 46 typed exports. Five affected CPU/queue regressions pass. The legacy real-GLES Options adapter passed 11159 checks and 1852 loss points after restoring its missing external responsive-presenter stubs; it uses a native draw counter stub and does not cover the new uploaded-source wrapper. Evidence: evidence/ui-options-completed-source-local.json. No Apple build, publication or physical-device acceptance. Next, trace and qualify the outgoing child and incoming Options sources across the 115-frame nested return animation, rather than assuming all of it is one fade.


Nested Options return source trace: one read-only desktop replay at 708x492 completed all Audio/Controls/Graphics entries and returns, clean shutdown, exact protected-file restoration and unchanged records. Each new Options owner bypasses InitialMain caption creation and runs 12 iterations of loc_40C792. Its 405C70 -> 428770 -> 426B70 presentation uses the full original bitmap, distinct from existing 427xxx cached-tile fades. Later initial-full mode 0x2800 still lacks a child proof. All three returns retain 115 native presentations; do not equate the loop count with complete temporal attribution. The main-loop diagnostic exhausted its cap in the first owner. Next qualify the explicit nested-return bitmap source and subsequent full-cache source under the new owner, preserving original animation and exact source validation. Shipping source unchanged; no Apple build or iPad claim. Evidence: evidence/ui-options-return-inline-path-local.json.


Nested Options return cached-source handoff: the complete Audio/Controls/Graphics sidebar now supplies immutable content for the new Options owner. The words animate while a dialog is open, so the earlier Options sidebar is not byte-identical. Completed nested source packets have copied catalogues; compare their complete pinned identities rather than object pointers, then retain the receiving trusted catalogue. Up to four completed per-bitmap sources are retained; exact raw bytes and new owner/base/copy/upload/texture witnesses remain mandatory. Six original-data desktop returns at 708x492 and 800x450 now show 12 native bitmap presentations followed by 103 remastered cached presentations, instead of 115 native presentations. All entries, returns and final Main route completed cleanly; protected files restored exactly and records unchanged. The seven affected regressions and four ASan/UBSan cases passed, including copied-catalogue identity, changed-source rejection and Game lifetime retirement. Reviewed return captures remain authored and full-width. Removed 47 superseded compiler outputs (349.87 MiB); current objects, baseline, private captured-source oracle and scoped evidence retained.

This remains a partial continuity fix: the first 12 bitmap frames use loc_40C792 -> 405C70 -> 428770 -> 426950 and still need their own independently qualified native bitmap compositor. Preserve the original animation, current Options owner/base and exact mode-0x1000 source copy; use the existing interleaved native-pass witness pattern rather than inventing cached texture authority. Do not request an iPad test or start an Apple build yet. Evidence: evidence/ui-options-return-full-source-local.json.


Options bitmap entrance completed in the observed desktop routes. The original loc_40C792 frame now has an explicit Options-only owner/previous-form/phase source. It observes one actual mode-0x1000 copy and verifies the complete current base/sidebar/output replay. A distinct RGB565 adapter witnesses the original 300 downloads, 300 selections and 600 triangles, then prepares an independent full-width HD image; it never substitutes cached1555 authority or samples/overwrites the native color/depth target. Only this explicitly qualified hidden-cursor entrance may present without a cursor. The original BufferSwap barrier drains its finish before lexical frame retirement.

The final guarded candidate completed all Audio/Controls/Graphics entries and returns at 708x492 and 800x450 with zero native-layout route presentations in those action intervals. Seventy-two bitmap frames passed the complete native witness. Both apps exited cleanly; protected files restored exactly and records unchanged. Bitmap/cache captures were reviewed at both ratios: authored art, logo and Options caption align through the handoff. Three source-copy/lifetime groups and four ASan/UBSan groups passed. The focused real-GLES adapter passed 57 checks, including changed source, incomplete pass, cancellation and context loss; its original draw and wide output are explicit stubs, so do not use it as pixel acceptance. Fresh preparation generated all 15 units and 46 typed Glide exports; stripping the insertion-only observer blocks restores the three exact paired input units. Removed the superseded first bitmap executable and renderer object (75.28 MiB); the final binary/renderer and its required three engine objects remain. Evidence: evidence/ui-options-bitmap-prelude-local.json.

This resolves the formerly remaining 12 bitmap frames for these three return paths, following the earlier 103-frame cached handoff. It does not accept startup: this driver resizes only after Main is available. Next group the first-Main/startup animation and broader menu/post-race route coverage at the intended ratios, then address remaining art/layout gaps. No Apple build or iPad test request yet; device performance/battery/Low Power Mode validation remains open.


First-startup observation now sets 708x492 and 800x450 before the first presentation, using the existing tested bitmap candidate rather than a new build. Both bounded original-game runs reached authored full-width Main and exited cleanly with protected files restored and records unchanged. Ninety periodic captures per shape retained. Reviewed swipe samples at both shapes contain authored panel/logo before settled Main; wide frames 23/24 also show the authored map reveal. Periodic captures do not establish every-frame continuity. Separately, the authored HD title compositor remains centered 4:3 (656x492 or 600x450): its full-width presentation is not finished. Intro movies are still original low-resolution content. Broader menu/post-race, artwork and physical-device/performance acceptance remain open. Evidence: evidence/ui-first-startup-aspect-local.json.

Title widescreen artwork: losslessly extracted the retained 4:3 photograph, then authored a separate 1672x941 widescreen extension using built-in image editing. Same upper red-car montage and lower racing pair; logo/copyright remain separate. Candidate saved privately on E:, existing pack untouched. Runtime integration is still pending: authenticated optional wide photograph, independently owned full-drawable output qualified by the original title pass and actual presentation ownership, proportional overlay layout, resize/mutation retirement, and visual checks at 4:3/iPad/16:9. No new Apple build or physical-device claim. Evidence: evidence/ui-title-wide-artwork-candidate-local.json.

Title wide resource support implemented: schema1 remains accepted; schema2 retains an independent opaque authenticated 16:9 photograph. Lossless pack extension preserves the original photograph and all six copyright planes exactly. The actual candidate pack is 6,187,645 bytes (13,772,320 payload); the current active pin/pack is unchanged. Provider ownership/compatibility and both synthetic pinned-loader versions passed ASan/UBSan; the actual wide pack also passed the pinned-loader fixture with a synthetic logo provider. Pack boundaries passed. These are asset-loader checks, not application pixels or device acceptance. Next implement full-drawable title presentation and its source/FBO/context/resize retirement, then activate/test the new pack. Recompile renderer and provider together because the appended internal C view fields are not binary compatible with old objects. Evidence: evidence/ui-title-wide-provider-local.json.

Full-width title compositor implemented and locally validated with the new title pack. It independently draws the authored wide photograph, proportional authentic logo and locale copyright after the existing exact 300-upload/300-selection/600-triangle title pass. New output never samples/writes native attachments; actual front FBO/color/depth, context/thread/generation, artwork generation and dimensions qualify selection. Native mutation/resize/context retirement revoke retained authority; new operations are guarded individually. A context-loss fixture exposed the shared dimension helper making two unguarded queries, so this path now uses CPU bounds plus guarded queries. Inactive revocation avoids large structure copies on gameplay triangles.

The final local binary 101382fcf07040d55953d1e3b8d22d722b4e758cb724e8050fbeaa82fafa64eb completed title/startup at 708x492, 800x450 and 640x480 with clean shutdown, unchanged records and exact fixture restoration. Sixty periodic captures per shape retained. Wider shapes select title-wide; 4:3 retains the original title photograph path. The real-GLES compositor test passes 61 checks under ASan/UBSan (Mesa LSan off), including full output pixels, native color/state, leases, resize and selected context-loss points. Its preceding native pass is an explicit stub; actual game runs supply that separate proof. Source title pin and private injector now require the new 6,187,645-byte pack; the private supplement manifest records it, and the compiled binary contains its release pin. All unrelated injector resource pins stay unchanged. Four superseded executables/renderer objects removed (150.66 MiB); required matching provider/loader objects, baseline, artwork and evidence remain.

This is not complete UI or physical-device acceptance. Exact final title captures reviewed at all three shapes. Next classify the single native-route Main assembly boundary before claiming frame-by-frame startup continuity, then continue broader menu/post-race routes and remaining artwork/layout. Original movies remain low resolution. No Apple build, IPA publication or iPad test request yet. Evidence: evidence/ui-title-wide-runtime-local.json.

Current UI priorities (2026-11-20; use later evidence rather than resurrecting older pending entries):
- Title/first Main handoff: retained final-binary screenshots 021/022 at both 708x492 and 800x450 are entirely RGB black during the single native-route boundary. The next presentation is the authored Main swipe. This is a cleared transition, not observed old UI artwork. No extra app build/replay or behavior suppression was needed. Evidence ui-main-cleared-startup-boundary-local.json.
- History: all seven coherent background pairs already exist and are promoted by HistoryArtworkRelease.json (history digest 144f4adc797de220b00ad96f5807a07c8fac6179c1dc3023687a1f929234e963). Later ui-history-coherent-runtime-local and ui-history-private-release-integration-local supersede the older one-Ferrari prototype note. Do not regenerate the six other backgrounds.
- Credits: confirmed open. The current renderer snapshots the old background and is still 4:3. New 1672x941 clean petrol-teal backdrop authored from original Bac1, with stationary car/reflections and corrected ND 4 SPD plate; no baked menu text or NFS marks. Authentic NFS logo/watermark and all six locale headings must be separate. Original partner sprites are Kenwood and Creative House Y2, and must retain their actual original scrolling placement. Added identity-only CreditsSourceCatalog for all nine original resources; it is not connected to runtime yet. Next group source-qualified backdrop-only initial frames, current archive lifetime, wide composition, headings/branding/partner draws and parent return before local runtime checks. Evidence ui-credits-wide-artwork-local.json.
- Load/Save: settled wide output exists, but original entry and parent-return intervals still need source-qualified continuity. Preserve saves/configuration; copied-fixture checks only.
- Preserve completed Audio/Controls/Graphics wide composition and their verified bitmap/cache returns. After Credits/LoadSave, perform one coherent coverage pass for remaining top-level/post-race UI, HUD alignment, spinning manufacturer-logo page and ratio behavior rather than treating old historical pending notes as new work. Physical device and Low Power Mode/battery acceptance remain open. Original videos and horizon remastering remain lower priority. No Apple build or user test request until the UI/artwork batch is complete.


Credits source foundation integrated: complete owned 842,384-byte credits.qfs snapshot matches its exact archive identity once per owner, with hashing/registration outside the guest-copy lock. Prepared text now retains a source lease tied to the actual game lifetime; owner exit and both original archive-free entries retire it, and queued invalid views cancel their request. The source generator adds only observer blocks to four units; removing those blocks restores the inputs exactly. ASan/UBSan with leak detection passed against privately decoded original data, altered headers/all nine resources, bounds and cross-thread retirement/address reuse. All four affected engine units passed syntax checks. Default CTest has rejection checks; the positive original-data checks require the ignored private fixture and were run separately.

This is a foundation for the new Credits renderer, not visual completion. Original output/timing/input remain in place; no new image or wide frame has been activated. The initial identity does not prove arbitrary later in-place archive writes or the native draw target. Next connect actual source/partner draw witnesses and locale branding, allow qualified backdrop-only frames, then compose the authored wide output and preserve return continuity. No app replay, Apple build, IPA or device acceptance in this step. Evidence ui-credits-source-lifetime-local.json.


Credits eleven-layer artwork batch prepared (not activated): opaque authored wide backdrop; accepted exact NFS II SE vector; a vertical ghost using those same twelve brand letter outlines rather than a new logo font; all six separately outlined italic headings (credits, Mitwirkende, remerciements, créditos, riconoscimenti, eftertext); fine geometric Kenwood mark/red triangle; and a transparent Creative House Y2 remaster from its original sprite. The gray mark is the studio letter Y, not a bird. All masters and generated images remain private. Static base compositions reviewed at 800x450 and 708x492; these contain no live scrolling body text and are not application screenshots.

credits-ui.ownedz is 3,189,214 bytes with a 21,655,040-byte decoded payload; lossless zlib round-trip verified. Pinned portable loader checks exact file digest/size, bounded complete decompression, every role digest and premultiplied pixels before installing independent immutable copies. Acquired views retain bytes across clear, while generations revoke active authority. Provider/loader sources are registered with owned-artwork targets but startup does not load the unused pack yet. ASan/UBSan with leak detection passed the actual eleven-layer pack, changed metadata/digests/PMA, copy ownership, retained leases and cross-thread clear. Default CTest checks absent-input handling; the positive artwork fixture is private and was run separately. System libz was unavailable, so the local check used the already retained project zlib archive, without installing system packages.

Next implement the actual Credits draw/scroll witnesses, qualified initial empty frames, native source/target and swap authority, guarded independent full-width composition and parent-return continuity; only then connect optional startup activation/bundling. No new app/Apple build, IPA, publication or physical iPad test in this artwork/provider step. Evidence ui-credits-eleven-layer-pack-local.json.


Credits static drawing provenance is now connected to the genuine original 4A10BC bitmap primitive. It observes a complete pinned resource span before and after the original call, exact current target owner/geometry/maps and source lifetime, and records actual final edx/ebx coordinates. The sealed four-call source recipe is Kenwood, Creative House Y2, Bac1 and current locale heading. It never derives draw position from the negative byte8 hotspot. Prepared Credits frames retain this optional immutable scene source; existing output remains unchanged.

A bounded original-data 708x492 Credits visit confirmed all four calls: Kenwood (100,50), Creative House Y2 (300,30), background (0,0), English credits (456,17). This materially corrects the tempting but wrong Kenwood header coordinate (64,311). The original text scrolled, Credits returned through Options to Main, and shutdown was clean. The outer post-exit harness hit a PermissionError while inspecting unrelated protected system-process metadata before its restoration section; recovery independently verified no game was live and restored durable backups without replaying the game. Records and all 20 retained artwork resources were unchanged. The private wrapper now inspects only named game PIDs, rather than enumerating unrelated system processes.

All five observer-bearing units compiled in the bounded O1 desktop diagnostic; graphics still use the verified title/Options renderer and this executable is not a performance measurement. ASan/UBSan/LSan source/scene tests passed. The source generator now explicitly writes UTF-8 after a Windows default-encoding failure on a Unicode arrow in Methods_13 comments. A fresh five-unit reverse audit restores all original input operations exactly; ignore the incomplete earlier audit/empty output. This is CPU static-draw proof only, not cached texture/upload/native-pass or GL presentation authority. Next carry actual partner cache translation/clip through full-frame Credits collection, witness the actual cache pass, implement the guarded full-width compositor including empty initial frames, and connect optional artwork activation. No iPad/Apple build or acceptance claim. Evidence ui-credits-static-draw-source-local.json.


Credits whole-frame collection and HD preparation implemented. The independent collector starts after the genuine base draw, preserves text before and after the translated partner-cache draw in one ordered event list, records the original partner offset/clip, and seals at the original swap boundary. Legitimate empty first frames are accepted. Incomplete/repeated partner calls, unsupported commands, stale source/locale and interrupted strings are rejected. Old frame authority is atomically revoked at the next base, abort or collector destruction; the retained original game/archive lifetime is also required. Existing native text-fragment output remains unchanged.

PreparedWhole adapts the complete source to immutable HD glyph ranges plus ordered partner events, original command colors/geometry, six-locale heading placement and the eleven-layer artwork lease. Empty frames need no font upload. C views borrow from their retained frame owner; callbacks inspect only immutable fields and atomic frame/source/art generations, never worker TLS or game memory. ASan/UBSan with leak detection passed original-source and whole-preparation groups using private source/font/artwork fixtures. Cases cover empty and partner-only frames, text/partner/text order, ranges/tints, translated rectangles/clip coordinates, incomplete/duplicate draws, frame/source retirement, retained C leases and artwork-generation replacement. Geometry in these tests is manually constructed from the previous live source trace, not a new game observation. One representative generated engine unit parses the complete inline hook integration successfully. The private-font-dependent test target is registered conditionally; no CMake reconfiguration, app replay or new Apple build occurred.

Next implement the independent guarded Credits compositor with its original cached-native-draw receipt and actual target/swap ownership, then activate the pinned pack. Rebuild all five affected game units together before a grouped application check. The collector is intentionally independent of whether legacy text-fragment staging accepts a frame, so the completed whole-frame path can avoid redundant legacy native-target snapshot/copy/text passes once it is active; preserve original game rendering/input/timing. Full-drawable static layers use authored background/brand/heading placement; body text and partner events retain their exact 640x480 coordinates with uniform centered scaling. Do not substitute source-content callbacks for cached GPU/GL presentation proof or claim the new screen is live yet. Evidence ui-credits-whole-frame-preparation-local.json.


Credits independent full-width GPU compositor implemented, still unpublished staging only. It draws the opaque photograph across the full drawable, separate authentic logo/watermark and selected right-anchored heading, then centered original-coordinate text/partner events in their original order. Partner images are clipped in logical coordinates. Adjacent glyph ranges are combined without crossing a partner event. Artwork, font, program and output textures are cached; stable-size frames perform no texture uploads. New GL calls check context/source ownership individually. Source-only retirement uses the independently trusted context guard to restore original GL state; actual context loss stops all further foreign-context work. Failed shader/image/font/output preparation does not seal half-created cache generations or dimensions.

Real GLES fixture passes 181 checks under ASan/UBSan (Mesa LSan off): full opaque pixels at 640x480, 708x492 and 800x450; locale alignment; text/partner/text order and both partner clip edges; original native color/depth and GL-state preservation; no repeated uploads; combined text draw; malformed input and shader/upload retry; source/art retirement; selected context-loss phases and context-safe deletion. Synthetic frame/art/source authority in this fixture is explicit; it does not execute the original cached pass or app swap. Separately, the actual pinned private artwork pack passed the complete-empty-frame preparer and GLES compositor at all three ratios. Private pixel captures were reviewed; lossless PNG conversion checked pixel equality before deleting raw readbacks. Those static captures preceded only the later adjacent-text batching change, which does not affect empty frames.

The helper also passes syntax compilation as C inside a matched retained production renderer. An initial check accidentally mixed current player-photo helper declarations with that retained checkpoint; corrected include precedence used the coherent retained helpers plus the new Credits module, leaving unrelated source untouched. Only the pre-existing framebuffer pointer warning remains. The focused GLES target is registered, with no full CMake/app/Apple rebuild or publication.

Immediate next step: connect original cache/upload/draw receipts and actual presentation ownership, rather than adding another standalone content abstraction. Credits cache0 is the two partner sources and cache1 is background plus locale heading; dword_4EB74C holds 300 descriptors per slot, with a 1200-byte slot stride. Preserve actual source/owner/locale/texture identities and the original 300 cached selections/600 triangles at origin0/0 or the recorded partner translation. Whole-frame source callbacks and staging_color alone are not native-pass/swap authority. Then submit PreparedWhole at the original swap boundary, qualify the real front FBO/color/depth/drawable and add credits-wide presentation selection. Skip redundant legacy fragment snapshots/text commits only after that complete path is active; preserve all original engine input/render timing operations. Finally run one grouped original-data empty/scroll/partner/return check at the intended ratios; use the original Down fast-scroll input if needed to reach the partner section within a bounded run. No iPad test request yet. Evidence ui-credits-whole-gpu-local.json.


Credits native integration (partial): the authentic original cache sources now qualify all 300 native selections/600 triangles, including source pixels, upload receipts, clip projection and real color/depth ownership. The complete eleven-layer artwork/text frame is submitted at the original swap boundary and selected only for the actual front target at the full drawable. Trusted startup loading, optional CMake bundling and private-injector pins are connected. No original input/draw/timing operation was removed.

The first integrated original-data run exposed 12 native fallback frames while its first text command was wholly clipped. Fixed the whole-frame adapter to preserve logical dimensions for zero visible vertices; the focused original-data regression passed ASan/UBSan. Final 708x492 and 800x450 runs select credits-wide from the first original Credits frame, scroll and exit cleanly; all 21 resources and copied config/records/replay are restored/unchanged. Reviewed actual full-width captures. The five observer insertions reverse exactly to their source units. This is a local O1 diagnostic build, not a performance or iPad claim.

Credits is not closed: returning to Options still has 115 native-route presentations in each shape, and these bounded runs did not reach the late partner-logo section. Next extend the completed parent source/bitmap/cache return mechanism to the actual Credits return, then exercise late partners using original fast-scroll input. Keep those original-game acceptance checks separate from the earlier standalone GPU fixture. After Credits, continue Load/Save and grouped UI/HUD/manufacturer/aspect coverage; no new Apple build or user test request yet. Evidence: evidence/ui-credits-native-integration-local.json.


Credits late-partner and parent-return group completed for the observed English desktop routes at 708x492 and 800x450. The original Options return whitelist omitted the actual Credits form. Added it as a distinct fourth family, retaining all current raw sidebar/base/copy and native bitmap/cache validation. The original Down-key fast-scroll reached both partner sources: exact 300-selection/600-triangle partner draws passed at real translated offsets and partial clipping, followed by full-width composed partner/text output. Reviewed both actual partner captures and the early return.

Both original-data runs exited cleanly, with all 21 resources unchanged and copied config/records/replay restored. Credits stays wide from its first frame through the exercised late partner interval, then all 115 previously native return frames now use 12 nested-wide bitmap frames plus 103 transition-wide cached frames. This supersedes the Credits return/late-partner gaps in ui-credits-native-integration-local.json for these two observed routes. This is not physical-iPad, other-locale, performance or battery acceptance. Next Load/Save entrance/return and grouped remaining UI/HUD/manufacturer/aspect coverage; no Apple build or iPad test request yet. Evidence: evidence/ui-credits-return-partners-local.json.


Load/Save continuity partial milestone: the complete verified Load/Save cache now publishes immutable sidebar/caption/word/logo content to the existing Options return mechanism. Added the actual Load/Save form as a fifth return family. Its dedicated owner TLS token differs from the common logo publisher token: preserve the original cache owner token for source validation and record the common publisher token on that same verified thread. An intermediate diagnostic flag was misread as catalogue mismatch; catalogue adaptation was removed. No catalogue pointer/source-byte/native-pass requirement was relaxed.

The final original-data 708x492 English run exits cleanly, restores copied config/records/replay and leaves all 21 resources unchanged. All 115 core parent-return animation frames are now remastered: 12 nested-wide bitmap frames with full 300-upload/300-selection/600-triangle proof, then 103 transition-wide cache frames. Reviewed the actual early return. An additional single final-exit native frame remains immediately after LOADSAVE SOURCE declined phase=7 and before that animation.

The first Load/Save presentation also remains one native frame. An ignored diagnostic in the retained local renderer captures the exact original 614,400-byte bitmap only after its successful original native pass; visual inspection proves old Load/Save artwork, not a cleared transition. The normal full-scene collector has no completed selected sidebar cache yet and deliberately declines that first source. Do not hide/delay this frame, borrow a later cache, or claim entry continuity. Trace the actual initial-sidebar producer (including the pre-40938C initialization path) and separately qualify the final exit frame. Group the iPad/16:9 acceptance check once both edges are implemented, rather than repeating 16:9 for each partial fix. No Apple build or iPad test request yet. Evidence: evidence/ui-loadsave-return-source-local.json.


Load/Save terminal-frame follow-up: a fresh bounded original-data repeat at 708x492 completed and returned to Main with the Options highlight confirmed. It retained 12 nested-wide + 103 transition-wide frames and did not reproduce the additional terminal native frame. All 21 resource hashes and copied config/records/replay passed restore checks. Earlier terminal rejection is localized to Presentation construction only; its failed predicate remains unknown. No acceptance comparison was relaxed. Numeric compiler-specific diagnostics were saved privately and removed from production source. First-frame native fallback remains confirmed and is the next implementation target: bind the genuine inherited Options sidebar, exact copied bytes and actual initial-caption/reset/copy observations, rather than manufacturing a completed Load/Save cache. No Apple build, device performance or continuity-completion claim. Evidence: evidence/ui-loadsave-terminal-observation-local.json.


Load/Save first-frame source implemented and verified locally. A read-only original-game observation established exact equality to the retained completed Options sidebar (game, common publisher thread, archive, raw bytes and four words). The first body frame now retains that distinct inherited origin; it never fabricates a Load/Save Cache or borrows a later revision. Current map reset and sidebar copy callbacks/stores replay to the actual clean whole plane, then the existing text/native pass and owner-epoch lease apply. A source-local frame serial permits genuine revision-zero assembly.

Original-data runs at 708x492 and 800x450 both exit cleanly and restore protected copied files, with all 21 resource hashes unchanged. Entry is 12 child-return-wide frames directly into loadsave-wide, removing the previously verified single native first frame. Return remains 12 nested-wide + 103 transition-wide then menu-wide. Actual first native draw proves 300 uploads/300 selections/600 triangles; the old wrapper aggregate counters use stale prefixes and are not relied on. Reviewed actual full-width menu capture. Earlier intermittent final rejection was not reproduced in these runs and stays open. No iPad/performance/battery claim, Apple build or publication. Evidence: evidence/ui-loadsave-inherited-first-frame-local.json. Continue remaining grouped UI/transition/aspect coverage.


Load/Save empty tournament chooser follow-up: current candidate completed original 401090 opening, input-ready observation, Escape cancellation, parent restoration, Options and Main return at 708x492. Clean shutdown, protected copied config/records/replay restored and all 21 resources unchanged. The accepted chooser interval retains the current wide route throughout. Actual popup image is an empty gold list over the remastered parent; the copied fixture has no tournament filenames, so this is not evidence of missing filename text or nonempty-list acceptance.

Preserve two unsuccessful diagnostics: initial driver sent Escape before opening finished and did not reach expected Options; that run also recorded one native frame just before chooser admission. An older numeric-diagnostic executable separately refused the seeded parent, so it cannot verify current-source child behavior. The extra native prelude remains intermittent; phase7 alone is not a failed-selection proof (the explicit Enter-action invalidation can retain the last phase label). Do not move action invalidation or relax source checks based on that label. Next exercise nonempty list content in a disposable fixture without loading/deleting originals, then group remaining name/confirm/alert coverage. Evidence: evidence/ui-loadsave-empty-chooser-local.json. No rebuild/cloud/iPad acceptance claim.


Populated Load/Save popup group completed at 708x492 using the same current candidate, without rebuilding. Three unique test-only tournament filenames were placed in the copied save directory. Original chooser renders all filenames and vertical CANCEL in HD; Down selects the second row with its original highlight. A separate original Delete Tournament route selected a test filename, displayed the authored SE/question/Yes-No confirmation, and cancelled using Escape. Reviewed both actual captures.

Both runs return through Load/Save, Options and Main and shut down cleanly, with no native route selections in those observed intervals. All 21 resources are unchanged; copied configuration/records/replay and existing save entries are restored. All test entries remained unchanged before cleanup and were then removed. This supersedes the suggestion that the empty-list panel alone indicated missing text. It does not close the earlier intermittent native prelude, prove successful save/load/delete operations, or establish physical-device/performance acceptance. Next prioritize the remaining extra native redraw source, then group outstanding dialog/HUD/result-mode acceptance before one coherent Apple build. Evidence: evidence/ui-loadsave-populated-dialogs-local.json.


Load/Save input-interrupted redraw source fixed locally as one related batch. Bitmap redisplay now reads the actual persistent surface/pixels and validates the immutable sealed paint receipt, rather than rereading glyph-painting state that the original bitmap blit does not use. Strict String/Tail/seal font/palette/callback/store validation and actual native300/300/600 proof remain. A reproduced original snapshot failure led to a second, independent collector gap: original40BCFF can cancel an unselected-sidebar update before or during the logo group.

The exact original abort receipt now preserves only the previous completed scene when owner/epoch/archive/catalogue, full target/pixels and selected sidebar bytes remain unchanged. Partial pixels are not published. Attempt revision advances independently of retained completed-cache revision. The maintained insertion helper matches the actually compiled staged unit; removing the single observer restores original bytes exactly. Focused bitmap ownership/pixel/ticket/surface negatives pass ASan/UBSan; eight source/insertion contracts pass.

Final candidate e29f75b34a677fbc323e6509b8d956df56390cdda59094c6a86ea80243e2dad5 directly exercises genuine original abort receipts at 708x492 and800x450. All three targeted runs shut down cleanly with no native route selections in observed transitions, exact copied-file restoration and all21 resource hashes unchanged. Witnesses include inherited Options source before any completed LS cache, and retention of completed cache3 after attempt4 stops after one logo. This is stronger than a run that merely avoids the interruption. Earlier failed/intermediate traces are retained in evidence/ui-loadsave-original-abort-retention-local.json.

Remaining: final-candidate populated chooser/confirmation replay, other grouped UI/HUD/result-mode coverage and fresh complete source preparation before one coherent Apple build. The earlier wide chooser failure belongs to the bitmap-only intermediate candidate and is not declared fixed. Menu snapshot cost and device behavior remain unmeasured. No cloud build, upload, gameplay FPS or full-port completion claim.


Final-candidate widescreen Load/Save popups completed. The800x450 route verifies actual original Enter acknowledgement, selected filename, delete-question display, Escape cancellation, and return through Options/Main. Reviewed captures show HD words/materials over the authored background. No native route selections occurred in the observed sequence. Disposable entries remained unchanged before removal; copied saves/config/records/replay and21 resources passed restoration checks. No application input code changed: the slow software-GLES harness waits for original key acknowledgement instead of relying on a fixed-duration chooser keypress. This closes the final-candidate wide-popup gap, without physical-device input/performance claims. Evidence: evidence/ui-loadsave-final-wide-popups-local.json.

Fresh maintained preparation completed from pinned local engine/SDL: all15 engine units,46 typed Glide exports, classic observers/assets and the adaptive pair. The single original-abort insertion appears in the fresh engine, adaptive patches reverse exactly, and compilation dependencies resolve current canonical project headers (the pipeline intentionally makes no helper-header copies). Fresh Methods01 and the complete Glide renderer compile; only the existing framebuffer const-array warning remains. Removed155MiB of newly generated redundant source intermediates after retaining hashes. This is not a full fresh link or Apple build. Evidence: evidence/ui-current-fresh-preparation-local.json.

Next grouped scope: actual Single Race/Tournament/Knockout Setup and tournament Location preview, then remaining current UI/HUD/result coverage. The old source-only inventory is stale: SetupEmblemProvider andLocationCompletedFlag implement the authored rule badges/preview, while Results/name-entry/radar/manufacturer implementations have later coverage. Do not invent missing integrations from that list. Prepared champion words still lack a verified consumer; preserve the uncertainty rather than attaching an invented owner.


Grouped Setup mode walkthrough completed locally at 708x492: actual captures identify Single Race, Tournament and Knockout labels. Knockout exposes an unresolved responsive-tail refusal: owner004d2790 serial643, reason6, lower-right128-square at510,387..638,515, original1555 source address1992704. Presentation switches to native until race type returns to Single Race. Existing Setup top-mode pulse admission does not establish this separate lower badge source. Trace exact original resource/draw and add narrowly qualified support; do not loosen generic unknown-texture admission. Both bounded runs exit0, restore copied protected files and preserve21resources. Attempted Tournament commit/800x450 Location route reaches ordinary Proving Grounds photo, so Tournament preview remains unverified despite driver success. Evidence: evidence/ui-setup-modes-grouped-local.json. No Apple build/device/performance claim.


Setup Knockout fallback fixed and replayed at 708x492 and800x450. The rejected lower emblem is exact original TKO5 (archive73,85-square artwork converted into128-square canvas), not a tier or top rules badge. A separate exact digest enters the existing upload/selection ledger; only Setup may use it, and only its actual510,387..638,515 canvas is admitted. All original context/epoch/slot5/native texture checks remain. Both complete Single Race/Tournament/Knockout cycles stay wide with no nonstartup native route, exit0, restore copied protected files and preserve21resources. Renderer candidate compiles/links; original TKO art itself remains unchanged. Broader saved-Knockout owners and actual Tournament commit/track preview remain next, not proven by this fix. Evidence: evidence/ui-knockout-slot5-continuity-local.json. No cloud/device/performance claim.


Saved-Knockout continuity completed as one related batch. Authentic original TKO5 also appears on Main and through return/entrance animations; the earlier Setup-only fix was insufficient. The shared slot5 ledger now supports its exact387..515 canvas and original signed pulse. Independently owned return/preview paths validate their own typed descriptor plus exact current TKO bytes; initial and Player branding use their own source owners with exact upload/selection identity and ordinary native shader state. No unknown texture or mismatched owner is granted permission. Both final708x492 and800x450 complete Setup Done/Main/Player One/Main with no nonstartup native presentation, clean shutdown, copied-file restoration and21 unchanged resources. Current production renderer also compiles from fresh prepared source, independently of the retained runnable diagnostic. Evidence: evidence/ui-knockout-complete-continuity-local.json. TKO artwork remains original and needs separate remaster; no device/performance/cloud claim.

Tournament Done navigation corrected: four Down actions reach Done because the original skips a disabled row; five reached Cancel. Actual wide capture confirms Tournament saved on Main, then Location and Main render wide. Original Knockout disables Location, so its grouped driver uses enabled Player One. The trnm image is a completed-track overlay guarded by40D0C0/current tournament record completion bits, not a preview that must appear on every Tournament visit. Completed-track variant remains unexercised; ordinary Tournament Location coverage is verified locally. Evidence: evidence/ui-tournament-location-committed-local.json. Continue remaining UI/art completion before one coherent Apple build.


Knockout artwork gap filled: a reference-guided authored gold globe/curved chequered flag replaces activeTKO5, with a source-pinned subduedTKOD variant prepared too. Original128-square canvas/offset22,0/85-square extent is retained. Lossless deflate stores both512-square PMA textures in348998bytes; decoded storage2MiB. Generated source chunks each stay below1MiB; no original game pixels or personal paths are published. Full-resolution master and exact prompt remain private in the build folder. Existing provider precedence and4rules emblems remain. Native upload/draw receipts and two complete aspect-ratio replays verify active remaster with no poststartup native fallback, exit0, copied-file restoration and21unchangedresources. InactiveTKOD not yet game-observed. Current provider/renderer compile and local CMake configuration with downloads disabled pass. Evidence: evidence/ui-knockout-emblem-remaster-local.json. No cloud/device/fullUIcompletion claim. Continue remaining artwork/UI completion.


Victory artwork is a confirmed remaining coherent-screen gap: source4154C0/table414C48 selects15distinct RGB565 winner photographs, separate from menu photos, over the original640x480 trophy backdrop. Current Victory text/word renderer still retains these original nontext pixels. Privately decoded all15with bounded sizes and preserved intrinsic positions; no original imagery added toGit. Authored a reference-guided trophy base and McLarenF1 full-scene variant, with one shared composition and a corrected unintended right-handle detail. Masters are private and not activated. Fourteen car variants, adaptive framing, source-bound photo/backdrop installation and grouped original-game acceptance remain. Do not call the existing text-only Victory pass complete HD coverage or substitute different menu photos. Evidence: evidence/ui-victory-artwork-source-local.json. No app/cloud build or device claim.


Completed the first authored4:3 Victory car pass: all15original source names now have retained private full-scene masters sharing the trophy composition, rather than unrelated menu-photo substitutions. Exact source pose/color references guided each distinct car. Corrected Isdera roof-mirror scale and removed invented FZR2000 road stripes as v2siblings; earlier drafts remain. All15selected masters are exact4:3; master hashes, source indices and sizes are recorded. No byte-identical background claim or runtime activation. Next final grouped artwork review, adaptive compositions, exact background/photo draw-chain binding and original victory gameplay acceptance. Evidence: evidence/ui-victory-fifteen-artwork-local.json. No compilation/cloud build or private images committed in this batch.


Victory lossless packaging/reader implemented. All16current4:3 authored plates form an explicitly marked24447735-byte preview; production generation/open requires32reviewed4:3+16:9 images. The bounded metadata reader retains only metadata/path and source-matches one requested image before reading its chunk; compressed and decoded hashes are validated before changing output. SharedPNG row-filter decoder is reused through a new bounded adaptive-canvas entry; existing slideshow inflatePixels remains4:3only. ASan/UBSan decoded16real+32synthetic images exactly, checked9malformed metadata/chunk mutations plus identity/size/pin failures, and preserved outputs on failures. Sanitizer stderr empty. Optional fixture target compiles through existing CMake with downloadsdisabled; no private packs required by defaultCTest. Biggestrealdecodedimage6290112bytes, not a peakprocessmemory measurement. No game activation or physical/performance claim. Evidence: evidence/ui-victory-lossless-archive-local.json. Adaptive masters and exact original background/photo drawing order/lifetime are next; a file reader is not live-source authority.


Victory original-art binding preparation: added reproducible finite metadata for the16backdrop/car resources, pinned to the whole decoded original archive before parsing. All digests match retained inventory; regeneration is deterministic, invalid archive is rejected before output, and the header compiles asC++14. No original pixels are committed. Source inspection resolves drawing order as back0400, optional winner photo0400, localized congratulations2800, then dynamic415CC0text. Earlier speculation that the photo might follow the finite word is not supported by the actual original call order. No collector checks were loosened. Authored two private widescreen composition drafts (base/McLaren);1672x941is not exact16:9, so they are deliberately not production-packaged. Fourteen wide variants, final consistent geometry, live photo/backdrop ownership and actual victory-route acceptance remain. No app/cloud build or iPad acceptance. Evidence: evidence/ui-victory-original-binding-prep-local.json.


Victory source-bound composition collector implemented behind diagnostic-only NFS_PRIVATE_VICTORY_ART_PROOF=1. Existing named image hooks capture finite exact original backdrop/photo sources under the actual4154C0owner; source bytes and target are checked across the original calls. Immutable640x480composition must match the entire observed plane, then the final text/word reversal on the same target before a receipt is attached. Optional proof failure clears itself independently, preserving strict current text handling. No replacement renderer activation or default gameplay overhead from image snapshots. Source-model ASan/UBSan checks all16original image identities and15winner compositions with corruption/inside-outside pixel/order negatives; source fixtures are not actual game draw captures. Actual prepared engine collector syntax passes after correcting an initial local harness that mixed header directories. Live Victory publication, true copy-mode witness and adaptive artwork integration remain next. Evidence: evidence/ui-victory-original-composition-proof-local.json. No app/cloud build or device/performance claim.


Victory original bitmap execution verified locally for all15winner resources. A private diagnostic branch at Main40D3D0loads the genuine Victory archive via410E80and manually enters the existing collector scope; it does not manufacture a race-win claim or call the natural4154C0owner. Each original428A20frame-start selects the active target, then original405B30/405B60/483470/4A10BCdraw back and one winner photo. Exact source digest, same-target and whole-plane checks pass for all15; diagnostic owner is cleared before exit0. Two earlier trials rejected the unselected target because the fixture omitted428A20. Production collector/model unchanged; no checks weakened. Copied fixture fully restored and all21owned-art resources unchanged in all3runs. Connection loss interrupted a link, not a live accepted run; process state was inspected and only unfinished link/run resumed. Dynamic text/finite-word reversal/publication and actual race-win route remain unverified. No new art activation, device/performance/battery/cloud claim. Evidence: evidence/ui-victory-original-bitmap-runtime-local.json.


Victory final source publication verified for all15cars in the same forced local engine fixture. After original frame-start/background/photo draws, original locale congratulations is drawn in mode2800and415CC0draws original Victory track text. Each of15immutable frames publishes one finite-word receipt, two original text commands and the correctly indexed authenticated artwork scene after strict word/text reversal. No source/target/pixel validation changed. Exit0, diagnostic owner cleared, whole copied fixture restored and21art resources unchanged. English-only; the fixture writes the track-selector global and manually enters collector scope, so natural4154C0race-win and final GPU presentation remain unproven. Artwork renderer/provider installation and remaining widescreen masters are next. Evidence: evidence/ui-victory-original-text-publication-local.json. No cloud/device/performance claim.


Victory selected-image provider implemented. Pinned metadata registration validates every finite original-source association without decoding pictures; production requires32images, while16real4:3drafts require explicit diagnostic preview admission. Acquisition source-matches the independently authenticated game image, decodes only that image, caches one immutable pixel owner and reuses it on repeated requests. Retained views survive cache eviction/clear/reload with generation changes; they do not remain eligible for publication after invalidation. Failed requests leave output/cache unchanged. ASan/UBSan verifies16real+32synthetic images, wrong-source/range/config rejection, retained-view concurrency and metadata-only registration of a damaged synthetic chunk followed by deferred decode refusal while another chunk remains valid. Optional CMake test target compiles; no app target rebuilt or connected. Real widescreen masters, provider startup registration/ABI/renderer consumer and natural victory presentation remain. Evidence: evidence/ui-victory-lazy-provider-local.json. No device/battery/FPS/cloud claim.


Authored Victory4:3preview now renders through the real native GPU path in a forced local fixture for all15cars. Internal frame schema2carries only the immutable composition-proven image/source digest; the C renderer uses a C identity boundary and the lazy provider, validates the complete original300/300/600pass, then stages authored background/photo plus existing HD words/text. GPU cache keys include context, provider generation, image, dimensions and pixel digest. Generation/target are checked before stage commit. Final fixture flushes accepted renderer work; all15finish, exit0, whole copied fixture restored and21resources unchanged. Inspected640x480private-stage readbacks for McLaren/FZR. This also corrects earlier evidence:415CC0renders a track name, not a car caption; the private index loop had written a track selector, with>8clamped to0by41A3A0. No claims for15car-name labels.

Wide-canvas layout preserves central4:3text and crops only authored side extensions. Source/provider/geometry ASan/UBSan checks pass for4:3, two iPad sizes and16:9, with central edges within1physical pixel. Real wide-art GPU acceptance remains open; the current real resource is diagnostic4:3only. Provider static library builds; all5affected engine units and the C renderer compile. Private renderer uses a consistent retained header base with current Victory leaves; this is not a full fresh-preparation/all-UI build. Ordinary startup does not register this preview. Remaining: finish/finalize adaptive masters, production resource pins/startup/packaging, natural victory and grouped UI/aspect acceptance. Evidence: evidence/ui-victory-authored-renderer-local.json. No cloud/device/performance/battery claim.


Victory widescreen first authoring pass complete: retained shared trophy base plus all15car variants,14newcarplates this batch. Reference-guided car-specific details preserve the concept/road/race/bonus distinctions, Isdera small periscope/no side mirrors and corrected FZR2000 no road stripes. Corrected Nazca side lettering to NAZCA C2 in a retained v2sibling. All selected masters and exact prompts remain private; original drafts remain. A32-image4:3+wide draft manifest binds each wide scene to the same finite original source as its4:3counterpart. All16wide master hashes are distinct. Generation-native1672x941canvases are not exact16:9, so production packing/activation remain deliberately unchanged. Next grouped model/proportion/alignment review and resolve lossless canvas handling before a complete adaptive resource; no app/cloud build or device claim. Evidence: evidence/ui-victory-wide-first-pass-local.json.


Victory adaptive canvas preparation completed locally: private32-image4:3+wide preview is47854261bytes. All32real decoded RGBA hashes match; all16wide canvases retain native1672x941pixels. Exact16:9 remains accepted; whole-pixel-rounded wide exports allow at most0.5vertical pixel only at real-art sizes, with4096dimension bounds. Legacy4:3 remains strict. Renderer cover/crop uses actual source dimensions for bothUVaxes rather than stretching. Five iPad/16:9 layout cases preserve pixel aspect and centered4:3text within1pixel. The1671x941Nazca outlier was correctly refused, then replaced by a retained1672x941correction with readableNAZCA C2; old variants remain private.

Grouped ASan/UBSan checks pass:32real adaptive images, prior48archive decodes/9malformed cases, provider source/cache/retained-view/deferred-corruption cases; sanitizer stderr empty. Production still refuses the unreviewed32-image preview. Optional fixture target excluded from normal builds/tests. Remaining: grouped artwork review, rounded-wide GPU execution, production pins/startup/packaging and natural race-win/full UI transition acceptance. No cloud build/device/performance/battery claim. Evidence: evidence/ui-victory-adaptive-canvas-local.json.


Victory reviewed production resource and normal-startup wiring completed locally. Visually compared all32selected4:3/wide images in8paired review sheets for car identity, cup/logo/handle consistency, composition and text space; this is agent review, not user/device acceptance. Private production-format pack retains47854261bytes and unchanged decoded pixels. Public metadata records only filenames/sizes/digests; artwork/prompts stay private. Startup registers authenticated production metadata before worker creation and enables original-image proof only after success; lazy selected-image decoding remains. Companion-resource CMake and no-Mac IPA injection support victory-ui.ownedz; injection additionally requires the matching metadata pin in the base main executable.

ASan/UBSan startup-loader check acquires all32production images, refuses preview/missing files and preserves an active generation after failures. Grouped12synthetic packaging checks pass including native/compressed-storage preservation, mutated resource rejection, base-release mismatch and existing size limits. Provider library and changed normal-app startup/loader objects compile offline. Added optional fixture test, excluded from normal builds/tests. Earlier stale delivery-count assertion was updated for existing Credits plus new Victory resource; no production check weakened. Remaining: full normal-startup runtime, rounded-wide GPU and natural race-win/UI transition acceptance, then coherent Apple delivery. No cloud/device/performance/battery claim. Evidence: evidence/ui-victory-production-startup-local.json.


Pre-delivery size audit found current22-resource injector catalog totals541576669bytes, above unchanged536870912byteIPA guard before app/ZIP overhead. Do not raise the limit or silently omit requested artwork. Reconcile retained lossless opaque-RGB slideshow work and final release pins before delivery: injector currently uses219036319byte slideshow pin while CMake points to a retained240423232byte schema3receipt. A verified final v4 pack has not yet been located in this worktree. This catalog audit is not a measurement of a newly producedIPA. Evidence: evidence/ui-artwork-delivery-size-pending-local.json.


Compact slideshow release recovered exactly from the pinned retained240423232byteversion3pack:219036319byteversion4output SHA32ec9ebc8d683ccfb6567d33de3a17e113842d56a59ba9665ae3b602d452d837. Whole-file identity matches the earlier verified opaque-RGB artifact. Current native ASan/UBSan decoder compares all98images byte for byte; sanitizer stderr empty. Tracked image catalog matches the earlier verified candidate exactly and is unchanged. Added one metadata-only release catalog consumed by CMake and private IPA injection, replacing the stale version3CMakepin. Grouped12synthetic delivery checks pass. All source/old packs remain unchanged.

This restores prior20.4MiBcompression; it is not new savings beyond the219MBpack already included in the size audit. The22-resource541576669bytecatalog still exceeds the536870912byte packaging-tool guard before base/ZIP overhead. That guard is local policy, not an established iPad/SideStore limit. Do not confuse recovery with completed final-package sizing. Continue normal-startup/wide-GPU/natural-victory verification and resolve full-package accounting before delivery. Evidence: evidence/ui-slideshow-compact-release-recovery-local.json and evidence/ui-artwork-delivery-size-pending-local.json. No new cloud/device/performance/battery claim.


Normal production Victory startup now executes in the real local native runtime. Isolated Methods01fixture no longer calls the provider loader directly and runner removes manualVictoryHD/art-proof overrides; it requires startup generation before original image calls. All15car compositions/text publish and GPUfinish after300uploads/300selections/600triangles in each of2runs; exit0, copied fixture restored exactly and22resources unchanged. Inspected actual stage readbacks. This remains forced manualOwnerScope, not natural4154C0entry or a race-win claim.

The708x492window probe reveals a real remaining gap: native framebuffer is656x492(4:3), so current Victory layout/provider path selects4:3art even in a wider window. Algebraic adaptive layout tests did not establish actual full-width presentation. Add a separate completed-pass-bound full-window compositor and select it on the exact front attachments, using existing Title/Credits swap/mutation guards; do not loosen native pass/source checks or stretch the4:3frame. Wide-GPU/natural-owner acceptance and final package-size accounting remain. Evidence: evidence/ui-victory-production-runtime-local.json. No cloud/device/performance/battery claim.


Victory full-window compositor implemented and verified locally at 708x492and800x450. It stages the authenticated wide plate plus centered original-layout HD words/text only after complete original300/300/600native validation. Separate output attachments never sample the native color. Actual display arbitration checks the completed front color/depth attachments, extent and context/resource generations; native uploads/selections/unknown triangles/state and size changes revoke the cached stage, with only the exact swap observer preserving it.

Final candidate completes15original/art/text frames and15wide preparations at each ratio, with2actual successful wide swaps,1wrong-size refusal and1unowned-triangle invalidation perrun. Both exit0, restore the entire copied fixture and preserve22resources. Four final McLaren/FZR captures match reviewed images exactly. Maintained hook insertion reverses exactly; C renderer compiles with only the existing LFB const-array warning. The first wide candidate preserved15native frames but refused wide text: generic4:3sampling correctly rejected the centered subviewport. Final uses the existing strictly owned responsive-text path; no generic checks loosened.

This closes the observed4:3-only Victory output in the forced local fixture, not natural4154C0entry/race-win, fresh entire-app integration or physical iPad acceptance. Next: natural owner/return flow, fresh coherent preparation and grouped UI-transition acceptance before Apple delivery. Repeated-frame residency/allocation costs and full package-size accounting remain unprofiled/unresolved. Evidence: evidence/ui-victory-fullwindow-runtime-local.json. No cloud/device/FPS/battery claim.


Fresh coherent full desktop app built from current maintained preparation: pinned local engine/SDL verified, all15engine units generated, renderer and adaptive pair prepared, and full nfs_game_app target compiled/linked with2jobs using existing dependencies and no downloads/cloud. Restored reviewed default History panorama pins instead of the prior diagnostic empty cache settings. All22private artwork inputs match current injector release pins; compact slideshow staged without changing originals. This removes the retained-renderer-header baseline limitation from the new normal-app smoke evidence.

Unmodified app/main and engine entry launched, opened Opponents (actual four-car transition accepted/committed), returned to Main and shut down cleanly at 708x492. Inspected Opponents/Main captures. After resize, observed menu/transition/intro/child-return routes stay wide with no native route in that interval. Whole copied fixture restored exactly and22resources unchanged. Earlier smoke01had a driver-budget error:150sdeadline equaled the helper cleanup reserve, so Main readiness was never given a loop interval; corrected240sdeadline ran the same unchanged binary. This is not an app-crash fix.

Victory source inspection confirms winner/player-state gating, so an ordinary results summary or the prior forced draw loop does not prove original4154C0entry. Next use this coherent baseline for normal race/results/Victory/return flow; a local-only two-controller input shim is prepared but not compiled/run. No real race win, iPad, Apple build, gameplay FPS or battery claim. Final package size remains pending. Evidence: evidence/ui-victory-coherent-app-local.json.


Fresh coherent app completed a normal two-player2-lap Proving Grounds race: McLarenF1andNazcaC2, with8cars actually present. Original engine/renderer unchanged; private host entry adds only2ordinarySDLvirtual controllers. Actual RESULTS native frames appear before controller-release/pause commands in the log. Reviewed authored Lap Times (F1laps2:05.46/2:07.49, C2laps3:01.81/3:00.50) and full-window Main return. App stopped by explicit driver request and shut down cleanly. No forced Victory calls or race-state edits.

Important limits: this does not prove a human win/Victory owner entry or independent player assignment. A no-opponents selection was not committed: the driver counted disabled menu rows, so the full AI field remained. Future test must verify the committed field and use independent acceleration phases. race02was interrupted before race start; with no process live, its recorded backups restored the copied fixture exactly and22resources remained unchanged.

Post-race03audit catches one generated track-cache change; all saved settings/stats/saves/replay and every other copied file restored, and22artwork resources unchanged. The old generated cache was not backed up and could not be recovered exactly from retained copies; record whole-fixture restoration asfalse, not success. Cache has one hardlink and is isolated from retained sources. Future driver backups now include the track-cache directory and reject symlinked/shared mutable inputs. This changes the test harness only, not game/port code. No user action is needed for this disposable-cache exception. Evidence: evidence/ui-coherent-natural-race-results-local.json. No cloud/device/performance/battery claim.

Normal coherent split-screen race now verifies exactly2cars, saved/reopened no-opponents selection and PlayerTwo automatic transmission committed with DONE. Independent host SDL controller phases show P1speed143/P2zero, then P1coasting20/P2speed93; both subsequently accelerate. Unmodified original engine completes2laps at ProvingGrounds: McLarenF1wins4:14.00, NazcaC2second4:22.46. Normal Continue opens authored LapTimes; Main return reviewed. No guest race-state edits or forced Victory calls. This configuration did not display separate Victory artwork, so original eligibility/source-owner acceptance remains open despite the genuine win.

Explicit stop exits0/cleanly. Future backup correction is now exercised: entire copied fixture restores exactly, including generated track cache, settings/saves/stats/replay;22artwork resources unchanged. This supersedes the previous cache-restoration exception only for this new run, not its historical receipt. Grouped UI/controller integration evidence only; no Apple/iPad/performance/battery acceptance. Next inspect original Victory eligibility through read-only diagnostics rather than repeat an assumed winning configuration. Evidence: evidence/ui-natural-split-player-win-local.json.
Complete lossless artwork package budget reconciled. Verified the retained base IPA directly inside its retained ZIP against its receipt SHA/13600399bytes, and matched all22resource catalog sizes/digests to the runtime audit. Resource bytes541576669 plus3684bytesnewZIPheaders yield555180752bytes with that reference base (602922077unpacked). This is size accounting against an older verified base, not a new compatible Apple artifact. No art omitted, pixels changed or duplicate large IPA produced.

Supersedes the earlier instruction to leave the arbitrary512MiBfinal guard untouched: it was local policy, not an Apple/SideStore limit, and cannot contain the requested authenticated full artwork set. Final output policy is now768MiB, with existing512MiBbase/member and1GiBunpacked limits retained; combined unpacked budget is checked before creating a temporary output. All12focused synthetic delivery checks pass, including input identity/storage preservation, old combined-output refusal, retained base limit and early expanded-size refusal. Full resource hashes/release pin validation unchanged. Actual fresh Apple package size and physical install still pending. Evidence: evidence/ui-complete-artwork-package-budget-local.json.
Read-only original eligibility observation narrows the natural Victory gap. Private Methods02adds only logging; two virtual controllers remain ordinary SDL input. Normal2-playerProvingGrounds2-lap race at 800x450finishes, with original4154C0entry: enabled1/mode1/local0/count2; row0rank1/flags5/state2, row1rank2/flags5/state2; candidateeligible1/index0. Thus original winner eligibility is proven and race-configuration guesses are no longer the next step. No HDVictory native pass or wide presentation follows, while authored RESULTS WIDE does present. Investigate actual collector/archive/bitmap/text/presentation checkpoints next; do not force a victory call or count this as HD acceptance.

Normal results/LapTimes/Main return and clean exit0; whole copied fixture including generated cache restores exactly,22artwork resources unchanged. Private logging build sources/hashes retained, baseline engine source/binary unmodified. This advances the prior natural-owner question but exposes a real integration gap not covered by the forced15-car fixture. Evidence: evidence/ui-natural-victory-entry-gap-local.json. No Apple/device/performance/battery claim.
Immediate natural-winner capture now shows the original4:3trophy image/spinning F1logo at 708x492. Read-only collector checkpoints prove feature/art enabled, ownerentered, archive ready, actual artwork composition collected, finite gold word collected, live caption captured, immutable frame published and Presentation entered withfailed0. No HDVictory native pass begins. This eliminates archive readiness and text collection as the current failure; next inspect exact native queue ownership/current-source/renderer-target admission. Normal race/results/Main return, exit0, exact whole copied-fixture restoration and22unchanged artwork resources again pass. Evidence: evidence/ui-natural-victory-collector-gap-local.json. Native rendering fix remains pending; no device/performance claim.

Scoped duplicate cleanup during the bounded run:307files/162481289bytes removed only from the unused unpaired intermediate classic-source-stages tree, each SHA matched against the retained paired tree. Retained paired source hash receipt and compiler dependencies remain intact. Private cleanup receipt retained. No artwork/game/save/delivery/evidence deleted.
Natural Victory target failure confirmed exactly128with source/target equality, queue unblocked, valid schema2/context/art identity,1word and168text vertices. Reviewed renderer-only fix now matches established Results constant-fog handling: exact128only fallback revalidates native target; each of600original triangles independently proves textured exact geometry/oow1and unchanged effective fog amount/color; generic AudioNativePass stays unchanged. Private shader applies the original premultiplied constant effect once after authored background/gold words/caption. Native/final/wide image copies explicitly reset the fog uniform; wide fog explicitly sets blending and checks private attachments/context. Original guest engine and resource pins unchanged.

Uninstrumented coherent-engine normal2-playerPG2-lap race now publishes33authored HDVictory native frames, each300uploads/300selections/600triangles/fog3. Actual trophy capture reviewed: remastered image/gold words/caption are visible with original spinningF1logo. Normal results/LapTimes/Main return, clean exit0, exact copied fixture restoration and22unchanged artwork resources. Wide prepares33but wide presents0: later original logo/presentation operations still revoke/bypass the wide stage, so do not claim natural full-window Victory complete; investigate that exact later handoff next, without arbitrary triangle admission or another race until reproduced in the short fixture.

One scoped actual-GPU six-case original-art fixture passes. Constant fog0/3/255each validates300/300/600, publishes native+wide, and restores exact saved GLstate. Both native/wide pixel comparisons for3are within1channel unit of the expected blend;255matches exactly. Changed amount/color and untextured second-tile cases all reject with no authored publication or readywide output. Forced fixture is labelled separately from the natural race. Whole copied fixture including generated cache restored exactly and22resources unchanged. Private initial helper compile lacked a fog-table declaration; corrected only the helper and reused its unchanged compiled engine object. Evidence: evidence/ui-victory-constant-fog-local.json and evidence/ui-natural-victory-target-refusal-local.json. No Apple/device/performance/battery claim.
Remaining natural Victory wide gap source-audited: after native426B70finishes, loc4156FEcalls4274F0. This is a textured animated logo quad, not a3Dmesh. Its first texture selection (497738selector1→4975E8→grTexSource) reaches Victory's no-active-view select branch and revokes the wide stage before the logo triangles; later state/triangle observers also revoke. Exempting the selection alone is not a fix.

The car6brand is already finite/catalogued by the final12role2TrackBadgeSourceCatalog rows. Original4159E0loads the winner's carQFS into transient4D4208, resolves car6with its original offset,4280B0stages256x256/converts alpha200/uploads131072bytes into descriptor4D4E44, then the QFS isfreed. Seal source/conversion/upload while live; later use owned immutable receipt/current descriptor, never dereference freed archive or weaken existing live TrackBadge proof. Next implement a lexical Victory tail bound to exact parent native request/content, following existing Results branding-tail pattern plus exact four converted vertices (two corresponding triangles/onebatch). Replay into the full-window private target, preserve all unrelated invalidations. Reproduce first in the short original-art fixture with real carQFS/4280B0/4274F0/original4976B8swap, no extra diagnostic swap, before another normal race. This is a source-backed plan, not completed replay/physical acceptance. Evidence: evidence/ui-victory-wide-logo-tail-source-audit-local.json.

Graph-enabled first standard-to-bonus Player photo remains an active UI transition defect. The narrowly gated original branch correction reaches all three bonus sources. Current local Player Two traversal publishes all three independent photo animations (100 selections/200 triangles), but the full moving composite still refuses PHOTO_END: exact sidebar cached-pixel comparison fails at tile0/address2048/local(0,16), expected8387/current10503. Two initial cleared full-width frames succeed; this is not complete transition acceptance. A separate earlier run also caught full-source upload-event79 versus current80 at entry. Keep strict upload stamps, native pixel comparison and300/600 passes intact; identify the actual producer replacement next. Source hooks pass12 tests. Latest completed diagnostic exits0/cleanly, restores the whole copied fixture and preserves22resources. Evidence: evidence/ui-first-graph-bonus-transition-local.json. No Apple build, device verification or performance claim.

The direct native upload trace proves sidebar tile0 was uploaded correctly then replaced in later normal Player frames. Exact shared-key join was tested and disproved (0 matches), removed. Current authenticated full-plane343 crop was also tested, remains different at tile0(7,23), expected16841/current18889, removed. Each bonus transition still presents2cleared wide frames then19native moving frames. No source-pixel alternative is retained. Next trace actual original428250slot0 source bytes and its completed300tile upload; do not infer the343producer from entrance or a cropped full plane. New bounded opt-in producer trace prepared locally. All completed probes shut down cleanly, restore copied fixture exactly and preserve22resources.

Missing ordinary Player sidebar producer identified:40BBE0 loc40C0E5 uploads the currently selected343 bitmap to slot0 after the lazy full upload at40C0B6. Direct source trace confirms latest raw565(0,16)=12611 and(7,23)=18884, explaining both the stale entrance receipt and inexact full-plane crop. Candidate now authenticates this actual343 upload against the current accepted Lazy full proof and its exact unchanged copy/source bitmap, observes300native tile payloads/descriptors, seals a new sidebar receipt and refreshes transition history afterward. Kind13is internal source evidence and explicitly bypasses entrance rendering. Conservative exact-bitmap gate retains alternating-buffer rejection unless independently proven. Read-only review found no source/authority blocker; compile/runtime pending.

Ordinary343receipt candidate compiles and captures some real refreshes (new sealed contents84/87 after initial80), but full transition is NOT accepted: first bonus entry still sees saved event88/current89; later bonus moving frames reject exact sidebar tile0(1,16), expected10567/current10503. Thus exact-copy gate misses a later original alternating-buffer refresh. Preserve the strict pixel check and diagnose that source/refresh boundary; do not claim fixed. Candidate run exits0/cleanly and restores copied fixture/all22resources exactly, while driver correctly reports target12animation failure. No new Apple build or device request.

### 2026-11-23: final car-photo redraw capture

- Added a distinct PreviewRebuild source recipe for original 40D2DE (base, preview, sidebar copy and cached upload). It has no swatch and does not weaken Refresh requirements.
- All 13 exact insertion/reversal tests pass, including changed-label/argument rejection. Source-verifier validation passed under AddressSanitizer/UBSan with 47 negative cases; both player rebuilds were accepted and swatch-bearing/wrong-iteration cases rejected.
- Reapplying hooks directly to the already-layered generated engine correctly declined an altered initial-caption anchor. The existing runnable generated engine was preserved; fresh maintained preparation is required before gameplay validation.
- This candidate is not yet runtime or iPad accepted. Offscreen display remains required for desktop game tests.

- Coherent PreviewRebuild candidate built locally (v21); all22 pinned artwork resources matched. Actual source-join fixture passed both-player PreviewRebuild and mismatched-site refusal after restoring3 missing synthetic Main addresses.
- Widescreen repeated run:11/12 transitions have complete21-frame wide traces; the final transition was stopped at20photo passes by the old fixed-wait driver. Not accepted as12/12. Driver now waits for an actual21-frame closing presentation before advancing/stopping.
- iPad-shaped desktop run reproduced the separate deferred graph selection mismatch (captured9/cache9 vsselected10; captured11/cache11 vsselected12), causing original-UI fallback and failing first-bonus acquisition. This is the next source fix; no runtime/iPad acceptance claimed. Both runs exited cleanly, restored the copied game fixture exactly and preserved22art resources.

### 2026-11-23: outgoing graph selection overlap

- Added Lazy-only outgoing graph observation: captured standard car equals consumed cached index, graph exactly1, live cached index and all4statistics unchanged, incoming selected car valid. The immutable preview and all source replay/native pixel checks remain unchanged.
- Swatch checks retain capturedcar color and4palette words. Incoming selection observation is read before the swatch guest-memory transaction, then its byte is rechecked inside; no nested transaction. Independent review confirmed this correction.
- Both-player source-join tests passed4new positive cases and28negative cases plus changed-stat current-callback rejection. The test reader now rejects nested-lease invocation. Actual preview-reader tests passed selected12/cache11 and changed outgoing statistics for bothplayers.
- Incremental local v22 build is running against the verified v21 source pair; gameplay/iPad acceptance remains pending. No cloud build or publish.

- v22 built successfully via incremental local rebuild; all22art resources matched. The previously failing708x492 desktop graph-to-bonus sequence now passed all3bonus transitions with21wide frames each, zero source-refusal diagnostics and17successful history seals. Clean exit0, copied fixture restored exactly,22resources unchanged. This is desktop evidence only; physical iPad and whole-UI acceptance remain pending. Repeated800x450 timing run is active.

- v22 repeated800x450 run passed12/12 complete21-frame graph-to-bonus wide transitions (252frames) across4timing rounds. Same candidate as708x492 pass. Clean exit0; copied fixture restored exactly and22resources unchanged. Remaining guard fault fixtures and broader UI transitions are not closed by these results; no new Apple/device build.

- v23 private guard fixture completed5/5 cases: unchanged pixels with changed counter retained21wide frames; corrupted full pixels stayed rejected after repair; corrupted sidebar and mid-animation sidebar/counter invalidations stopped wide composition after2valid background frames. Every run exited0, restored the copied fixture exactly and preserved22resources. Instrumented generated source was restored byte-for-byte; the normal v22 playable candidate hash is unchanged.
- Current next UI work is authenticating both32-bit and16-bit graph canvases in the original427ED0 source path, then its existing100tile/21frame transition. No new Apple/cloud build, public push or physical iPad acceptance.

### 23 November: original graph source recorder

Added an opt-in observation-only recorder for both original 384x160 graph canvases (32-bit then 16-bit), five named graph assets, fill/draw tables, colour argument, palettes, callback rows, cached car and four statistics. It retains bounded raw observations in memory; it cannot upload artwork, authorize presentation, modify game state or write game files. Known indexed8 and RGBA32 row input strides are distinguished; unknown callbacks remain explicitly unknown.

Reversible wiring passes all 14 hook tests. Fresh engine preparation and adaptive pairing passed; local v24 app compilation reached final linking. Actual runtime capture and graph replay verification remain pending. No Apple/cloud build or public push.

- v24 runtime capture succeeded for standard cars9 and11: both32-bit and16-bit canvases, each with128fill rows and5assets (147/15/15/15/15draw rows), with zero incomplete captures. Four existing photo transitions also passed all21wide frames; clean exit0, copied files restored exactly and22art resources unchanged.
- The actual32-bit callback is4A1090 (f000211c), converting via4A6A7E (f0002218), not the directly called RGBA row guessed from the draw table. The recorder correctly marked its input span unknown. Static inspection shows indexed input expanded through the palette pointer in off_4DD044 and forwarded through dword_571260. Capture those exact inputs/metadata next, then implement graph source replay and authored display. No HD graph-animation acceptance or physical iPad claim.

- Current v24 first-Main audit passed at708x492 and800x450 without rebuilding: all59 first-swipe presentations used the authenticated authored wide compositor, motion steps0..32 plus26held end frames, with no route fallback between first transition-wide and menu-wide. Periodic captures show HD title, cleared black boundary and authored first Main panel; capture/log timing may straddle swaps, so not all boundary samples are labelled black. Both copies restored exactly and22resources unchanged; physical iPad remains unverified. See evidence/ui-current-first-main-swipe-local.json.

### 23 November: authored graph animation integration

Original graph replay now matches both384x160 formats, including the indexed-to-RGBA wrapper, canonical assets/palettes and translated clipping. Qualified immutable sources feed the existing authored graph plate/bars; original100 native tiles and200 triangles remain checked. No new artwork resource or renderer ABI; C++14 preserved.

v27 desktop runs passed the bonus-photo-to-standard-graph moving return for Player Two at800x450 and708x492, and Player One at800x450: all21 frames wide each, plus four requested bonus-photo transitions per run. All exited0, restored the whole copied fixture and preserved22resources. Standard-car Graph toggle is an original static preload; the early animation expectation was corrected without changing game behavior.

A separate32-frame fallback remains while moving Graph-to-Car focus. Clear invalidation stays intact; correction must authenticate retained source against fresh native rendering/current layout, not reuse stale frames or responsive epochs. Later unknown font41 is separate. No Apple/cloud build, publication, whole-UI or physical iPad acceptance. Evidence: evidence/ui-player-graph-authored-animation-local.json.

- Tail-source reason5 was initially described as unknown font41; source review corrects that interpretation. It rejects missing/current responsive source before checking font pages (reason6). The observed final transmission glyph coincides with stop/relative-mode restore and resource retirement, so this is teardown evidence, not an established missing-font UI defect. Keep source invalidation strict.

### 23 November: grouped sweep reproduced Location redraw gap

Candidate v28 reached HD Setup, sent three ordinary race-type Right inputs, returned to Main and entered Location. After waiting for typed settled menu-wide, one Right changed Proving Grounds to Outback (final capture inspected). A real native presentation interval spans frames386 through509 (124frames), then authored wide resumes at510. The expected new photo-source log was absent, so the breadth driver stops honestly; this is not whole-UI acceptance or an app crash. Exit0, whole copied-fixture restoration and22unchanged resources all pass.

Next isolate the Location clear/cached-draw/source-collection boundary before another sweep. Do not weaken native pixel/source checks or assume every track refresh uses the Player photo protocol. The existing Player queue regression additionally passed ASan/UBSan with genuine sealed300tile proof and actual worker/owner FIFO; typed GPU/source stubs remain an explicit limit. Missing standalone provider links were supplied privately, and unused Main link adapters abort if invoked; production code was not changed for this check. Evidence: evidence/ui-current-location-track-redraw-gap-local.json. No Apple build, push or physical iPad acceptance.

### 23 November: Location photo handoff, moving phase accepted

The v28 gap diagnosis found a current outgoing Location history record but no incoming photo receipt: only Opponents could borrow independently authenticated history after the entrance343owner expired. Candidate v29 adds an exact closed family7/Location mapping alongside family6/Opponents. It retains token, game life, domain/catalog, strict upload equality, live track/locale and all three original assets; crop/conversion/native100tile and animation proofs are unchanged. Independent review found no source-authority bypass.

v29 built locally and matched22resources. The actual Proving Grounds-to-Outback change now publishes its scoped-history source and all21moving frames wide (406..426). The next original full-background rebuild still falls back to native at427: INITIAL FULL copy-ctor rejects child-source and no copy rows begin. The test stops there, so the later duration is unknown. This is a measured partial fix, not a complete transition or eight-track acceptance. Exit0, copied fixture restoration, unchanged binary and22resources all pass. Next authenticate that exact post-photo original background copy; do not relax native pixel/source guards. Evidence: evidence/ui-location-photo-history-handoff-local.json. No Apple build or device request.

The final redraw source boundary is now identified: original40C160 loc40C5C6 refresh405B60 follows a fresh blk4base405B30 and authenticated Location Site::Refresh preview40CD70. It selects343pane through dword4D2314 highword and4D231C/2320, then reads bitmap at pane+0x20. The existing full-refresh-copy hook passes an explicit refresh flag to Opponents but not Location. Next implement a distinct Location refresh source using exact captionIdentity/selector/descriptor, current owner/archive and fresh preview, with unchanged bytes afterward and existing480row replay/fullcopy/native proofs. Preserve entrance and Info-return contracts. Implementation has been delegated for review; no refreshed generated pair or runtime acceptance yet.
### 23 November: all eight Location track changes stay HD

Fresh v30 preparation succeeded; generated15engine units and both renderer source files were compared with the verified previous pair. Only Methods01 changes: the exact40C5C6 Location refresh call now passes true. All other generated methods and renderer source bytes are unchanged. The refresh source binds the actual selected registered343pane, fresh preview/base/epoch/archive, immutable source bytes and unchanged descriptor/selector, retaining480row replay/fullcopy/native proofs.

v30 passed its first full transition but its next transition refused stale history: rememberInitialFull stage4 still compared against entrance sidebar pixels. v31 therefore retains the exact copied sidebar and selector/handles in the completed proof, then rechecks current metadata/header/every pixel on both sides of history descriptor observation. Strict upload equality remains unchanged.

v31 now passes all eight distinct track indices0..7 at800x450 using only original menu inputs and the original verified unlock parser. Every track change has21moving wide frames and an authored wide final redraw:168moving frames total, zero native fallback events across the whole cycle. Location return, P1car change/return and Opponents class change/return also pass. The subsequent Options test guard stops on its bounded diagnostic assumption; independent typed Options native300/600 finishes and an inspected HD screenshot show the page rendered. This does not claim Options return or complete grouped acceptance. Exit0, binary unchanged, whole copied-fixture restoration and22unchanged resources pass. See evidence/ui-location-full-refresh-history-local.json.

Next: current708x492 regression, Showcase/History/slideshow and remaining first-frame/aspect issues, with a short independent Options return check rather than repeating the eight-track sweep. No Apple/cloud build, push or iPad request.

### 23 November: current Showcase and Options return checks

The same v31 binary passed an offscreen 708x492 Showcase run: statistics pages2/3/1, History and return, slideshow and return, six Next Car changes and two Showcase exits (19actions). Presentation traces contain no native fallback after the first wide menu; the initial640x480 startup interval is explicitly excluded. The older intermittent Next Car gap did not reproduce, which is not proof of its correction. A separate800x450 Options-only run passed typed native begin/finish300/600, live upload seal and ordinary Escape to wide Main. Both runs exited cleanly, restored the copied fixture and preserved all22resources and binary bytes. Evidence: evidence/ui-current-showcase-options-local.json.

Slideshow inspection exposed the current blurred side-band fallback. A first authored scenery extension preserves the accepted center exactly and extends the original footer material, but faint joins remain: no batch approval or runtime integration yet. A bounded lazy panorama provider foundation is under integration; it must bind the exact archive, entry, accepted center and footer and cannot substitute for native render authority. Current aspect documentation also corrects stale claims that gameplay Hor+ is inactive: the current prepared pair contains it, while broad camera/culling and physical acceptance remain separate. No Apple/cloud build or device test request.


### 23 November: optional slideshow panorama source path connected

A side-only lossless resource format now connects reviewed CPU panorama entries through pinned startup loading, exact selected archive/entry/center/footer routing, immutable slideshow leases and schema3 renderer transport. Schema2 prefix compatibility remains. The wide backing is marked only after the unchanged complete native source pass and successful HD publication; History behavior, gamma and edge-clamp sampling are unchanged. No panorama is enabled by default and no unfinished pilot is bundled. Apple executables explicitly link dependencies because TARGET_OBJECTS does not propagate private object-library dependencies.

Checks passed: provider/session ASan/UBSan, six Python packer cases, bounded C++ pack-parser/pin cases, actual two-route Python-to-C++ byte-exact interoperability, engine bridge/presentation and pinned activation syntax, cached renderer syntax, and seven isolated CMake resource cases. These establish source/resource integration only, not a linked new game or runtime visual acceptance. Current slideshow center clamps repeat five edge columns; native framebuffer edge validity must be inspected before a slideshow-only correction. The authored pilot still has a smaller paving texture mismatch, so the98photo batch remains unapproved. Evidence: evidence/ui-slideshow-panorama-foundation-local.json. No cloud/Apple build, push or physical test request.


### 23 November: slideshow center edge detail restored

A one-shot authenticated source-FBO readback proved all sampled slideshow edge pixels opaque, nonblack and detailed. The old wide shader repeated source column5 on the left and651 on the right. Slideshow page5 now uses zero interior edge inset; other information pages retain their guard. A renderer-only local rebuild and hidden708x492 run passed ordinary slideshow entry and return. The native source buffer is byte-identical before/after; interior photo RGB difference is zero. Identical adjacent edge rows fall from380/380 to1left/2right. All22resources, binary and copied fixture remain unchanged; clean exit0.

Initial800x450 entry also publishes the full HD slideshow at native600x450, and its screenshot was inspected. Its requested Escape did not return before timeout (scene stayed slideshow); input/focus versus game handling remains unresolved, so widescreen return is not accepted. A resize-only screenshot additionally showed a stale viewport before a fresh original redraw: no live-resize continuity claim. The authored-side pilot now looks cleaner after removing the repeated band, but high-density side filtering can tap hidden legacy-footer center texels; qualify a side-only texel boundary clamp before enabling the panorama or batching all photos. Evidence: evidence/ui-slideshow-edge-detail-local.json. No Apple/cloud build, push or device request.


### 23 November: high-density slideshow side filtering isolated

A production-shader GLES2 fixture reproduced hidden legacy-center bleed at the first new side pixel:100/255red at2360x1640 and105/255 at2732x2048. Slideshow-only side sampling now stops at the nearest actual side texel center; protected native-center sampling remains unclamped and History behavior is unchanged. Both actual GPU checks pass with zero hidden red, seam-adjacent native center intact and whole History output byte-identical. This closes the synthetic high-density filter defect, not actual panorama gameplay/UI acceptance. Evidence: evidence/ui-slideshow-side-sampler-local.json.

One opaque lossless photo-extension pilot is prepared privately (about1.2MB); its protected center matches both accepted master and current decoded photo release exactly. It is not yet a shipped resource or98photo batch approval. Widescreen input isolation also showed delayed parent publications followed by fallback navigation into the original movie, so the earlier timeout is not enough to label a port input defect. Next use timestamped single-Escape semantics and exact pinned pilot loading in one local candidate; do not send blind fallback keys or weaken current-source guards. No cloud/Apple build or iPad request.


### 23 November: first authored slideshow panorama runs end to end

The exact1,214,684byte lossless side-only pilot loaded through its compiled whole-file SHA/length and four source/entry/center/footer keys. Actual original native pass and qualified GPU panorama presentation were observed at800x450 with1932x1086backing. The protected600x450display center and the native source-FBO bytes are exactly unchanged from the prior blur fallback. The inspected frame now has natural warehouse/cobble continuation and matching footer material; no gamma or feather is baked. Individual joins remain subject to visual review before bulk acceptance.

One timestamped Escape was posted successfully, reached original405960 as0x1b with an open debounce gate, made410414 return-2 and exited412300. HD parent and Main returns pass; no native fallback after first wide menu. Clean exit0, all22release resources and the extra pilot unchanged, entire copied fixture restored. Earlier uninstrumented timeouts did not reproduce; no general input-code fix or live-resize acceptance is claimed. Initial diagnostic compilation selected a compile-only object by broad filename and Ninja overwrote another probe; exact mapped/app objects were subsequently compiled, directly linked and all three probe strings verified BEFORE this sole runtime. Evidence: evidence/ui-slideshow-panorama-pilot-local.json. This is one image, not98image/user/device approval, performance or battery acceptance.


### 23 November: Records-entry19frame source gap repaired

Current coherent navigation found a real Records-entry fallback after TrackInfo return: native800x450 frames609..627, then records-wide628. The reverse-source guard correctly rejected historical343pixels against the actual upload (header equal, pixel5row0 different). Retained history used the InfoReturn fade pane although the already-sealed full copy could use the opposite pane. No renderer/source guard was relaxed.

The complete480row/source-after/table/full-copy seal now retains the actual copied InfoReturn sidebar. History repeats the producer's established direct-child acquisition or opposite selected-pane validation, then exact header/pixel checks on both sides of descriptor observation. Refresh and ordinary entrance behavior remain separate. Independent review corrected an overstrict ambient selector-word comparison on the direct branch; a minimal extension of the genuine child-source fixture passes ASan/UBSan (custom allocator uses-fno-sized-deallocation, no suppression).

Normalv36 now passes the actual800x450 TrackInfo/Location/Records/Location chain: the former19native frames are19child-return-wide frames606..624, followed by records-wide625. Full route has zero native fallback after the firstwide menu. Records and returnedLocation captures reviewed; selectedRecordsrow restored. Exit0/clean, whole copied fixture and22resources/binary unchanged. Two earlier800attempts stopped before the route on unacknowledged synthetic keys; exactserver-acknowledged original input succeeded. The708attempt likewise stopped beforeTrackInfo when its singleReturn was unacknowledged, so this is NOT708/Stats/Opponents/wholeUI acceptance. No repeatedfallback keys or extra appbuilds were used. Evidence: evidence/ui-current-records-entry-gap-local.json and evidence/ui-location-info-return-records-local.json.

Artwork work also identified why naive whole-photo outpaint alignment is insufficient: one Ferrari draft lacked13edgecolumns (rejected), overscan repaired coverage, but a head-on draft moved background barrier/road levels independently of the car. All accepted masters remain untouched, rejected drafts are retained privately, and no such asset is packed or approved. A code-native exact-center transparent canvas is being evaluated for tighter side-fill geometry. No cloud/Apple build, push or iPad request.

### 23 November: coherent wide Ferrari family and matched resource staged

Eight photo-only Ferrari scenes now use one coherent camera scene across the widescreen canvas, with the exact native title/branding footer composited separately. New1920x1080 backings and1440x1080 centers replace the earlier attempt to glue generated backgrounds onto incompatible rectangular old centers. Both800x450 and708x492 previews retain complete cars/mirrors/wheels; parked versus burnout motion, plate spelling, high/low camera angles and the side-profile scene were reviewed. Original masters remain unchanged and these are new pixel identities, never bound to old center keys.

The eight-entry side resource is7,282,057bytes, losslessly reconstructed against all eight new centers and exact original archive/entry/footer bindings. A complete98-image candidate resource encodes only the8changed centers and reuses90unchanged compressed chunks/identities. Its214,837,977bytes plus the side resource total222,120,034bytes, about3.08MB above the previous center-only resource. New center decoding and cross-resource keys match exactly; neutral hidden padding is covered by the unchanged opaque native footer with a bilinear guard. No gamma/feather or repeated visible edge extension is baked. All artwork, private catalogs and resource bytes stay private; current releases/headers and original game data remain unchanged.

This is offline artwork/resource progress, not runtime activation, all98panorama completion or physical acceptance. No app/cloud build was made for these assets. A separate source review identified that record-name live resize needs a completed base/tail composition receipt; the existing exact-size rejection remains rather than stretching the saved mixed parent. Evidence: evidence/ui-ferrari-wide-photo-candidates-local.json.


### 23 November: InfoReturn sidebar history closes the measured Records-entry flash

The completed Location InfoReturn copy could use the opposite original343 buffer from its fade. Its480-row/full-copy proof authenticated that actual source, but history retained the fade sidebar. The next Records entry then rejected identical headers with changed pixels and presented19 native-route frames. The source correction now retains the actual copied sidebar for InfoReturn, preserves the producer's direct-child acquisition branch, and rechecks the opposite selected pane/target/header/every pixel before and after descriptor observation. Ordinary entrance, exact refresh semantics, current upload equality and native300/600 proof remain unchanged.

The same normal v36 binary now passes Track Info -> Location -> Records -> Location at800x450 and708x492. Records entry is19 consecutive child-return-wide frames at both ratios (606..624 and590..608), followed directly by records-wide. No native presentation occurs after the initial wide menu in either observed route. The708 run additionally reaches authored Personal Statistics and returns through wide fade811, photo828, full cache849 and settled Location952; actual typed request1163 completes300 selections/600 triangles and the inspected return capture has Personal Statistics selected.

The708 observer reports a false failure because its return predicate required a fresh kind9 wide diagnostic after the256-line per-kind cap. Positive native/route/capture evidence is retained; that route was not rerun. Ordinary pointer input used measured widgets, actual inverse-bilinear cursor recognition, one relative move and one click per Location action, on an owned hidden708x492 Xvfb server with TCP disabled. Earlier keyboard route-delivery precondition failures remain separate evidence, not source regressions. Both final games exit0/cleanly, restore the whole copied fixture exactly, and preserve22 resources and binary bytes. Opponents was not attempted in this group; other tracks, full UI, live resize and physical iPad remain open. Evidence: evidence/ui-location-info-return-history-current-local.json. No app rebuild during these runs, cloud build, push or device request.


### 23 November: current initial-window Main swipe refreshed

A private diagnostic candidate changes only the desktop initial SDL window dimensions to708x492 before the engine starts. One app object was compiled and directly linked against the unchanged normal paired engine/renderer and22resources; canonical source/cache stay unchanged. The observer captures the first actual708x492 window without resizing. One ordinary server-acknowledged Escape dismisses the positively observed original EA movie, then original title publication acknowledges the input.

Actual chronology is original movie/native frames1..33, authored title34, black clearing frame35, first Main transition-wide36..94 and settled menu-wide95. All59 swipe presentations are wide, motion steps0..32 plus26 held-end frames; there is no native presentation after Main opens. Authored title, black clear, partial HD swipe and settled Main captures were inspected. Periodic screenshots are not every-frame pixel evidence. Prior v24 two-ratio acceptance remains separate; this refresh covers708x492 only, not physical iPad or whole UI.

The whole copied fixture,22resources,binary and protected source/cache inputs remain unchanged; clean exit0 and owned processes terminal. Earlier staging and movie-only budget failures remain pre-Main evidence. Scoped cleanup removes314 redundant movie screenshots (328,149,468bytes), retaining six representatives, failure capture, full hashes/index/logs/receipts after resolved containment and regular-file checks. Evidence: evidence/ui-current-startup-window-main-local.json. No further compilation/runtime, cloud build, push or device request.

### 23 November: all 98 coherent slideshow scenes assembled once

All 98 selected scenes now have coherent wide photo masters, matching 1440x1080 centers, exact independent native footers and both 800x450/708x492 previews. Source-aware review retained wheel motion versus parked states, camera angles, open panels, road/display plates and deliberate detail crops. Oversized whole-car drafts and the clipped Cala headlamp were corrected; rejected versions and provenance remain private. The final local batch includes independent 31-case and 24-case derivative reviews plus root reviews of other groups and focused corrections. Tiny generated physical glyphs are not certified exact; the original footer badge contact in several Ferrari views and incidental Cala detail-mirror crop remain explicit scope notes.

Three old combined body/wall-plus-footer sprites could not follow a regenerated photograph. Their authentic source-ink/vector groups were registered to new surfaces and baked into the new private photo, with explicit plain-footer keys. Old screen-position overlays are not reused; measured manual-registration uncertainty is recorded. Original masters, original metadata inputs and current rollback resources remain intact. Only separately approved code-compatibility metadata is updated publicly; no pixel, source-ink or private path payload is added to source.

One actual assembly produced 159,889,342 bytes of photos, 5,321,403 bytes of 15 shared footers and 73,331,466 bytes of 98 side panoramas: 238,542,211 bytes total. Lossless photo packing saves 59,146,977 bytes and footer deduplication saves 908,359 bytes; all wide scenery therefore adds only 13,276,130 bytes over the previous photo/footer pair. No duplicate dry encoding or repeated full resource assembly was performed.

One consolidated production C++14 ASan/UBSan run decodes all 98 exact centers, acquires all 98 panorama routes with composed-pixel digest checks, loads all 15 native footers and checks 98 routes. All 98 obsolete center keys and all three obsolete combined-footer keys reject. This is artwork/CPU-resource acceptance, not a new app, GPU, device, battery or complete-port claim. Delivery plumbing is being updated for the 23rd private resource; one matched application and private IPA acceptance remain. Evidence: evidence/ui-slideshow-full-wide-resources-local.json.

The completed preview batch cleanup removed 219 owned browser profile/cache directories and one never-runtime-activated intermediate eight-photo resource: 1,059,548,231 logical bytes. All 2,807 retained slideshow files matched their before/after hashes; all 98 final selected centers/resources, source recipes/previews/receipts and all 22 prior-runtime rollback resources remain intact. No live owned Chrome reference, reparse target, symlink deletion, process kill or unrelated cleanup was involved. Evidence: evidence/ui-slideshow-preview-cleanup-local.json.


### 23 November: matched slideshow desktop link and private delivery plumbing

The final 98-photo/15-footer metadata and exact 98-panorama pin are connected. Twenty-six focused delivery/configuration checks pass, including deferred uppercase CMake configuration. One owned desktop candidate compiled only two objects and reused 97 existing objects for a 99-object direct link. The complete SHA/byte-count delivery marker survives optimized linking; all 112 protected inputs remain unchanged, and all 23 private resources are verified beside the actual executable. No cache rebuild, source-header overwrite, cloud build or publication occurred.

This proves matched desktop compilation and resource placement, not live GPU transitions, an Apple base, finished package size, installation or physical gameplay. The grouped hidden runtime check is separate. Evidence: evidence/ui-slideshow-matched-desktop-build-local.json.


### 23 November: final slideshow resources pass live hidden route

The matched v39 desktop candidate completed Main -> Player One -> Showcase -> slideshow -> two original Right photo changes -> Showcase -> Player One -> Main at708x492 in45.42s. Three actually different McLaren scenes each publish300uploads/300selections/600triangles, then present detailed coherent wide scenery and the independent native footer. Root inspected all three captures. No native fallback occurs after Main settles; startup movie34frames and one title-clear frame remain separately classified. All112 protected source/cache/reference inputs,23resources and executable hashes remain exact; the whole copied fixture is restored, gameexit0/SDLclean. The owned display has TCP disabled and never opens a desktop window.

Three preserved earlier observer failures were harness preconditions, not game defects: a variable order error, two old screen-size predicates and movie capture budget consumption after an unacknowledged dismissal. The corrected run uses the same game binary. This is scoped desktop route acceptance, not all98 runtime scenes, complete UI, Apple/signing/device or performance acceptance. Total23resource bytes are559,536,064; final IPA/ZIP size remains unmeasured and the separate private upload bound is unchanged. Evidence: evidence/ui-slideshow-full-wide-runtime-local.json. No cloud build or publication.


### 23 November: matched Apple base preflight fixture repaired

Source-only public snapshot d0a927f passed full local history/independent secret checks and preserves the existing public history with exactly the audited source tree. Manual run45123443379 stopped in the Linux preflight before any Apple job: its isolated History CMake fixture omitted the new panorama provider/pack sources. The fixture now copies those two real public source files; all19 preflight test programs from the actual workflow pass locally on WSL, including the three formerly failing configuration cases. Production game code/resources are unchanged, so no desktop rebuild is justified. A corrected public source revision, not an unchanged retry, is required for the matched ARM64 base. No private artwork upload or finished package exists. Evidence: evidence/ui-apple-base-preflight-repair-local.json.


### 23 November: full Linux host target coverage repaired and rehearsed

Corrected source run45124108837 passed the privacy/preparation stages, then CTest reported five missing executables in the explicit host build list. No Apple job started. The Linux host gate now builds the actual History GLES fixture plus four classic resource targets before unchanged CTest; a coverage regression guards those registered executables. Production game and artwork code are unchanged.

A new project-only Debug/clang host directory, using existing dependency sources and the same sanitizer settings, compiled every target from the corrected workflow. All72 registered tests complete with71passes, zero failures and the expected Apple-only system CPU provider skip. The History GLES test passes on a surfaceless software context; no game window is opened. All six workflow-coverage tests also pass. A corrected public revision is required, not an unchanged rerun. Evidence: evidence/ui-ci-host-target-coverage-local.json.


### 23 November: renderer test provider linkage repaired and rehearsed

Run45138928693 passed the host gate but the renderer pixel test link lacked the real Victory/Credits provider APIs now used by its renderer. No Apple job started. The test now compiles the real Credits provider and links the existing Victory provider library; no stub, renderer guard relaxation or production game code change is introduced.

A fresh local Debug/boundary-sanitized pixel configuration, using the retained v30 pair and current build definitions, compiled the complete explicit pixel target list. The actual GLES test passes hidden on a TCP-disabled owned display: native-size targets, RGB565 writes, palette/texture identities, guest mutation, context restoration and cleanup. All44 post-link program invocations pass, including input/timing and four missing-data startup modes plus the shared app entry; the expected missing-resource exit is kept separate from successful real-game startup. Original copied support-file sources remain unchanged. This is component/link/startup rehearsal, not exact fresh CI preparation, final98-art runtime, Apple or iPad acceptance. Evidence: evidence/ui-ci-renderer-provider-link-local.json. A corrected source revision is required before another cloud attempt.


### 23 November: explicit artwork thread dependency for iOS configuration

Run45142560236 passed the complete Linux gate and reached Apple configuration. CMake then rejected Victory/panorama library links because Threads::Threads had only been created in the host branch. The classic artwork module now explicitly finds Threads when its imported target is absent. The History metadata fixture no longer predeclares a substitute thread target, so it exposes this dependency requirement instead of hiding it.

All nine local metadata/delivery checks pass on WSL (three History configuration and six panorama delivery checks). Production C++ and artwork are unchanged; no repeated full host/pixel compile is justified by this dependency-only correction. Actual iOS SDK configuration and ARM64 output remain unverified until a corrected revision passes the Apple job. Evidence: evidence/ui-apple-thread-dependency-local.json. No unchanged-revision rerun or finished package.


### 23 November: verified Apple base and private build23 assembled

Run45146351757 passes Linux and Apple: SDK configuration, native ARM64 compilation/link/package and a base-only unpublished transfer. The downloaded ZIP matches its server digest; the base matches its build manifest/source and full panorama marker. It contains no personal artwork before local finishing.

Local assembly exposed an incorrect packaging requirement: photo/footer whole-file SHA labels are not native loader fields. Their complete ordered source/photo/footer/routing catalogs are. All four full tables (including226chunk/pixel digests) are present in the exact trusted Apple executable and independently reviewed. The local guard now checks those complete authenticated tables, preserving every resource-file SHA/size check, History/Victory/owner checks, the panorama hash/size marker and existing Mach-O/ZIP storage identity. Shared metadata parsing is bounded, standalone and reused by preparation; long-pattern streaming boundaries are fixed. All19 workflow preflight programs pass, as do10delivery,3HistoryCmake and8History delivery checks. Fifteen real-header ABI assertions are integrated into the existing CI-built History decoder test; its CTest passes. No new native build is needed for this local-tool correction.

Build23 now contains23 exact private resources and preserves all12 Apple-base members byte-for-byte, including compressed storage and executable. IPA573,687,323bytes; lossless ZIP571,357,169bytes, with full round-trip digest verification. The measured ZIP exceeds the old512MiB project upload bound. CI_POLICY explicitly reviews a768MiB private-only cap, within the existing local envelope and GitHub under2GiB per-file rule; public base transfers remain64MiB and no personal Actions artifact is allowed. Original game files/signing material are excluded. Upload and physical iPad acceptance remain pending. Evidence: evidence/ui-slideshow-compiled-catalog-delivery-local.json and evidence/ui-private-build23-assembled-local.json.


### 23 November: private build23 delivery verified

The complete lossless ZIP571,357,169bytes is transferred only through the original private repository. Private visibility is verified before and after, and the server SHA256 matches e44dce9d471e669d8e421b82f04e6ade4e8f4db06c4e748f5dbb7705bea688a4. No finished private artwork is published to the public repository or Actions artifacts. All23 resources and12 unchanged Apple-base members retain their checked identities; native code and compressed base storage are preserved. The exact temporary public base-only draft is deleted after verified private delivery; local base/package receipts and bytes are retained. No unrelated release or visibility setting changes.

Build23 remains unsigned for personal SideStore signing. Its physical launch, complete menu continuity, split-screen race and Low Power Mode/battery results remain unverified. Resume measured performance/battery work while device feedback is pending; no need to rebuild merely for artwork iteration or this local packaging-tool correction. Evidence: evidence/ui-private-build23-delivery-local.json.

### 23 November: measured battery-investigation groundwork

Source-only packaging corrections were published as d417883a387a32d3cf3f02a044e43e418355559d after both privacy scanners passed; no new cloud build was started. Build23 private delivery remains unchanged.

The iPad owner loop uses a1ms unlocked/4ms capped run-loop timeout. Apple energy guidance identifies unnecessary polling as a possible energy cost, but neither configured timeout nor app poll count proves actual kernel wakeups or battery attribution. Reuse the existing watchdog foreground-poll counter in one-second diagnostic samples; add a duration-weighted report for steady/race-only intervals. First samples and power changes remain excluded, old logs report unavailable. No gameplay timing, extra per-poll operations, GPU measurement or battery/FPS gain is claimed. The26Python summary tests and sanitizer-enabled C++ serializer pass locally; Apple compilation/device readings remain pending and build23 predates this field. Evidence: evidence/performance-event-pump-diagnostics-local.json.

### 23 November: bounded renderer-command allocation candidate

A private trial combines each owned callable with its shared completion record. The actual192byte triangle capture drops from two allocations to one; existing borrowed and legacy operations retain their std::function move-out path. Eight alternating-order complete-threaded-bridge rounds reduce triangle CPU time14.03% and wall time13.18%; small scalar CPU is1.32%slower, borrowed0.54%slower and syntheticmixed CPU effectivelyunchanged. The predeclared component gate passes, without implying whole-game/device FPS or energy savings.

Independent review caught a cancelled-target pressure-drain race and nullable-callable differences before integration. The corrected hybrid passes the existing main-thread suite, all12legacy ownership scenarios,11concrete large-closure/null/move cases and the deterministic cancellation-pressure oracle under ASan/UBSan. The oracle also passes the unchanged baseline. No guest checks, draw order, graphics quality or game timing changed. A coherent private desktop trial rebuilds all18direct bridge consumers and reuses81unchanged objects; matched full-game comparisons remain required before promoting source. No cloud build or public/private app delivery changes. Evidence: evidence/performance-typed-renderer-component-local.json.

### 23 November: whole-game renderer trial rejected

The coherent optimized candidate and baseline each ran twice, AB then BA, in North Country split screen with Ford Indigo/Nazca C2, full opponents and two native virtual controllers. Both players were visibly moving in first gear before each exact30second recorded interval. All4runs shut down cleanly, recording had no drops/writer errors, all23resources/binaries stayed exact and the1020file copied fixture was restored after every case; the source fixture was unchanged. The owned display remained hidden/TCPdisabled.

The predeclared2%ownerCPU improvement gate fails: AB changes13.12025ms to13.0879ms (0.25%better); BA changes12.93815ms to13.539ms (4.64%worse). Reverse-order median swap spacing is2.82%worse. Keep the existing production renderer; do not turn the14%component improvement into a device/FPS/battery claim or dispatch an Apple build for this candidate. Raw private artifacts and negative evidence are retained; no unchanged timing rerun is planned. An initial pre-timing bootstrap miss and an asynchronous end-marker duration guard were observer issues, retained separately; exact same30SDLsecond windows use existing recordings and unchanged performance thresholds. HUD insets are not world pillarboxing: inspected race pixels reach both drawable edges, although exact projection telemetry remains separate. Evidence: evidence/performance-typed-renderer-game-decision-local.json.

A separate read-only497B24 alias-hint proposal was rejected before implementation: Game/CPU/FPU itself is a valid malloc32 guest allocation, so checked guest operands are not globally disjoint from native register state. No restrict-this hint, raw pointer shortcut or reduced span/stop checking is authorized by this audit. Next inspect normal-path cost from existing compiler/profile evidence and pursue only ownership-aware arithmetic candidates. Evidence: evidence/performance-game-context-alias-rejection-local.json. Build23 private delivery and production source remain unchanged; iPad acceptance/LPM60/battery remain open.

### 23 November: exact current Apple cancellation path inspected

Extracted only the native executable from the authenticated build23 base, preserving both IPA archives. The current ARM64 check_worker_stop at0x10022e5e0 performs3threadlocal lookups for a boundworker before its false-stop branch; two address lookups follow the atomicflag load but precede its branch. This confirms a normal-path Apple cost, beyond old source-only symbol counts. It does not quantify FPS/energy cost or prove a replacement faster.

The existing unifiedPODTLS fixture passes new separate-compilation-unit identity checks, live transactioncontrol rebinding, stop-before-resolution, four simultaneouslylive isolatedworkers and nesteddeferrals in optimized and ASan/UBSan Linux builds. Canonical headers stay unchanged. Next compare updated Applecode generation/nativeARM component timing, preserving every cancellationcheckpoint and requiring a consistent whole-consumer rebuild if later promoted. Official GitHub standardrunner documentation currently lists macos-15 M1/3core7GB as free for publicrepositories; no job, paidrunner, settings or billing change was started. Evidence: evidence/performance-build23-stop-path-local.json.

### 23 November: bounded Apple-native helper measurement prepared

The exactbuild23 normal-path TLS finding motivates a source-only comparison,
not an app rebuild. A registered manual benchmarkroute now offers the reviewed
thread-local experiment: publiccodex/hd-ui only, readonlypermissions, local and
Linux privacy/secret/safety gates before a bounded standardmacos-15ARM job.
The same pinnedXcode16.4 will compile iOSassembly and nativeMaccomponent
variants; no app, game data, cache, artifact or release is produced. A5minute
unacquiredqueue budget prevents repeating the earlier ARMcapacity wait.

New separate-TU runtimechecks compare old/address-cache/unified fixtures on
unaligned, boundary, alias, partialfault, stop-precedence and realthread cases.
The native timing plan fixes12balanced rounds,10% accessorCPU improvement and
at most5% transform/REP regression before inspection. Canonicalheaders remain
unchanged. NativeMac results remain separate from iPadacceptance/LPM60/energy.

### 23 November: native ARM helper comparison passed

Public source2c54c8436ee557fbaae33f22560f64cecd3e1775 passed the Linux privacy/sanitizer gate and one standard macos-15 ARM job in run45172726180. Queue acquisition took53seconds; the native job took26seconds and produced zero artifacts. Original repository privacy was verified afterward. Xcode16.4/Appleclang17 compiled separate non-LTO units and iOS assembly, without an app rebuild.

All three variants pass126checks/583resolver calls with the exact matching memory checksum5252414852042320239. Twelve balanced rounds find cached-address accessor/transform/REP CPU improvements55.59%/22.60%/23.18%; unified TLS improves19.60%/15.40%/7.76%. Both pass the predeclared component gate. The iOS accessor has3/0/1 indirect TLS calls for old/cached/unified while retaining its atomic stop load; constructor TLS work remains. These component results do not establish game FPS, iPad performance or energy savings.

The cached-address fixture is the next integration candidate. Keep canonical headers unchanged until handwritten and generated transaction lifetimes are audited and every affected consumer can be rebuilt consistently; transaction layout changes forbid mixed old/new objects. No user action is needed for this local gate. Evidence: evidence/performance-native-stop-probe-result-local.json.

### 23 November: cached-address lifetime and lifecycle gate passed

Read-only audits cover705handwritten local transactions across97files and9426prepared engine scopes/42319checkedaccess sites. No concrete cross-thread transaction, escaped reference or callback inside generated lifetimes was found;13manual reference helpers are synchronous. Some manual scopes allocate/hash/log, so they are not described as arithmetic-only. Separate-TU optimized and ASan/UBSan old/candidate tests all pass: shared TLS/mutex identity, live control rebinding, stop-before-resolution, four live isolated workers and nested deferrals. Canonical code remains unchanged. Coherent complete-consumer rebuild and paired hidden-display whole-game comparison are the next gates; no app/device gain is claimed. Evidence: evidence/performance-cached-tls-lifetime-local.json.

### 23 November: full-game cached-address candidate rejected

A coherent15Methods rebuild retained84other objects, the original renderer and23exact resources. Four fresh NorthCountry split-screen races used two moving nativevirtualplayers, full opponents and identical controller main, hidden TCP-disabled display, exact30SDLsecond windows, alternating ABBA order. All4exit0/SDLshutdown0 with zero recorder drops/errors; the1020file copiedfixture was restored after each case and sourcefixture/binaries/resources stayed exact.

The Apple-TLV-specific pre-run host gate allowed at most5%CPU/swap regression in BOTH pairings, rather than requiring Linux speedup; Linux TLS differs from Apple TLV and the native ARM component gate remains separate. This was fixed before any gameplay timings. AB CPU worsens7.23% and swap spacing8.52%; BA CPU worsens5.70% while spacing improves4.11%. Both pairings fail. Reject the global cached-address candidate, keep canonical production code unchanged and do not spend an app build on it. No unchanged timing rerun is planned. The initial startup-only single-skip miss is retained separately; a bounded exact-movie-frame retry corrected the observer without changing timing gates or discarding successful measurements.

The next separate candidate is unified TLS, which preserves the16byte transaction layout and already passed native ARM component/correctness checks. Its broader TLS identity change requires a coherent rebuild of all stop-check consumers, not just transaction users. Whole-game and later physical iPad acceptance remain required. Evidence: evidence/performance-cached-tls-game-decision-local.json.

### 23 November: separate unified TLS desktop candidate built coherently

The exact unifiedPODTLS fixture passed a separate local build of all51WorkerControl consumers, reusing48unaffected objects, canonical oldGuestCopyTransaction and the original renderer. All125protected inputs (including117baseline pins), the baseline executable and23resources remain exact. The238second build produced a63,736,368byte candidate. Linked-symbol inspection finds the baseline has each of3old TLS slots and no unified slot; the candidate has exactly1unified slot and none of the3old slots. This checks against mixed inline storage definitions, not performance.

The existing nativeARM unified helper and Linux optimized/sanitizer two-TU lifecycle evidence remain component/correctness gates only. A new virtual-controller main must also use unified TLS before full-game testing; the original baseline main cannot be reused inside this candidate. Fresh paired hidden-display split-screen runs and physical Apple/device acceptance remain required. No source promotion or cloud app build. Evidence: evidence/performance-unified-tls-coherent-local.json.

### 23 November: unified TLS trial fails reverse-order host gate

All four fresh hidden-display NorthCountry split-screen cases completed with both controllers driving, zero recorder drops/errors, clean SDLshutdown0,1020file copiedfixture restoration and exact23resources/binaries/sourcefixture. The unified virtual-controller main was recompiled coherently; linked-symbol inspection confirms no old TLS storage remains in this isolated candidate.

Keep the predeclared5%CPU/swap nonregression limit in BOTH pairings. AB appears44.86%better CPU, but its first baseline23.92ms is substantially slower than the final baseline13.0394ms, so do not attribute that large change to unified TLS. BA candidate13.8699ms is6.37%worse than baseline, exceeding the gate; swap spacing is1.01%worse. Do not promote the unified candidate, dispatch an app build, change the threshold or rerun unchanged timings. Preserve all evidence and production headers.

Next investigate a smaller normal-path stop-check split: retain old TLS identities and16byte transaction layout; put critical/deferral checks behind a noinline cold helper reached only after the live atomic stopflag is true. This is a hypothesis only: actual Apple codegen must show the extra TLV work stays off the false path, with unchanged cancellation/alias/fault semantics, before another full-app trial. Evidence: evidence/performance-unified-tls-game-decision-local.json.

### 23 November: smaller cold-stop-path correctness preflight

A distinct candidate retains the3old TLS objects/getters and canonical16byte transaction. Only after the current control and default atomic stopflag are true does worker_stop_ready call a noinline cold helper that checks critical then deferral depth. No snapshots, weaker atomics, callbacks or removed checkpoints. Independent source review finds the original short-circuit semantics retained.

The existing126check/583resolution oracle matches exact checksum5252414852042320239 in optimized and ASan/UBSan Linux builds; the separate-TU live-rebinding/four-worker/deferral suite also passes both builds. Inspected Linux accessor assembly branches around the helper on stop-false and has no critical/deferral TLS access in that function. These are not Apple codegen or performance claims. Preserve the candidate as a test fixture only; canonical code remains unchanged. Before another app trial, require actual Apple CFG/ThinLTO evidence, native correctness and balanced component timing including the rare deferred-stop cost. Evidence: evidence/performance-cold-stop-preflight-local.json.

### 23 November: owner wake/input constraints audited before battery changes

The actual game entrypoint is app/main.cpp with AppleMainThread, not the platform Probe. Rendering barriers use dispatch_sync_f on the main queue and execute their actual drain there. That real wake does not cover all work: SDL UIKit has no native queued-event wait/wake backend; MFI values are polled; gesture pulses, completion and worker-failure consumption still depend on returning to outer polls. The app4ms snapshot cache does not throttle SDL joystick backend work performed earlier in PumpEvents.

Keep current1ms/unlocked and4ms/30lock wait policy. Configured timeout is not measured wakeup count or battery attribution, and no physical new foreground-pump readings exist. The next battery experiment should measure CFRunLoop return reasons/elapsed waits, owner/backend CPU and input/render dispatch age under the unchanged policy, separating renderer work executed during nested run-loop servicing. No speculative longer-sleep optimization or device energy claim. Evidence: evidence/performance-owner-wake-audit-local.json.

### 23 November: bounded cold-stop Apple probe ready for source review

The separate manual cold-stop route retains existing experiments and app builds. Its complete local gate passes42unit checks plus actual OLD/cold ASan/UBSan126check/583resolution oracles, exact checksum5252414852042320239 and normal/critical/deferral warmups with cancellation after clearing deferral. A private metadata-only vector from authenticated build23 verifies all3real TLS descriptor mappings without changing or executing that binary. Independent review found and corrected false-green CFG cases, including outlined tail branches; unfamiliar actual compiler forms remain UNPROVEN, not accepted by call counts.

The proposed single standardARM helper job remains behind Linux privacy/safety and exactsource gates, with5minute unacquired-queue,8minute job,360second compile and90second execution bounds. Twelve balanced OLD/cold rounds use the predeclared normal10%accessor/5%transform-REP gate and separately report rare stop cost. It also inspects source-only iOS preLTO and tiny ThinLTO-linked Mach-O code, never executes iOS output and produces no app/artifact/release. Canonical worker/transaction headers remain unchanged; no dispatch yet. Evidence: evidence/performance-cold-stop-native-probe-preflight-local.json.

### 23 November: cold-path Apple inspection stays unproven after link

Reviewed source2c76806f364967de632b82f03fd55d64127a3bc8 was published only after staged and full-history privacy/independent-secret gates passed. Run45192557790 passes Linux privacy/safety; standardmacos-15 ARM was acquired in2seconds and ran27seconds. Xcode16.4/Appleclang17 compiled native OLD/cold helpers and both iOS ThinLTO inspection executables. No artifacts were created and original repository privacy is independently reverified.

Actual preLTO cold-accessor CFG passes:1current-control TLS lookup,1live atomic load, correctnonnull/stop guard and falsepathbypassing the2depth-slot helper. Final linked inspection remains UNPROVEN, so the driver correctly stops before native correctness/timing execution;0measurement rows, no performance decision or source promotion. The initial diagnostic preview lacks exact failure reason and label/PC maps; numeric ADRP operands are visible, but do not assert a specific root cause from incomplete evidence. Improve bounded diagnostics/observed decoding locally before considering any changed-source follow-up; never rerun unchanged or label this a game crash. Canonical headers/build23 remain unchanged. Evidence: evidence/performance-cold-stop-native-inspection-local.json.

### 23 November: linked-inspection diagnostics corrected without changing candidate

The prior run remains unproven with0timings. The changed checker now supports the numeric signed ADRP displacement shown in the linked preview, deriving its page from the actual instruction PC and still requiring exact binary TLS identity. It retains bounded local labels/PCs, resolved direct-call targets and TLS maps for offline replay, and separates preLTO/link proof outcomes with whitelisted failure reasons. This improves evidence; it does not retroactively prove the first linked result or assert a single root cause.

All24focused tests pass. Candidate WorkerControl and both C++ oracle/probe source hashes remain exact to the sanitizer-checked inputs; no production/performance gate changes. A follow-up requires new source publication/privacy checks and the same single-attempt runner/resource bounds; no unchanged dispatch or app build. Evidence: evidence/performance-cold-stop-probe-diagnostics-local.json.

### 23 November: actual linked cold-helper code validated offline

Changed-source run45195432206 again passes Linux and uses standardARM for24seconds after3seconds acquisition, with0artifacts. Its richer report reproduces the exact linked-check failure locally: the standard C++ exception continuation __Unwind_Resume was not modeled. The checker now recognizes only this exact no-return exception exit (plus existing exact throw exits); returning/rethrow variants, aliases, unknown calls and unmatched metadata remain rejected. LLVM libunwind source confirms this continuation leaves the frame and cannot return.

All25focused tests pass, and root independently replays the actual recorded Apple machine code successfully:1current-control TLS lookup,1live atomic load, propernonnull/falsepath guard and2live helper depth lookups. Preserve the original job failure; this is later offline codegen validation, not a retroactive CI success, native runtime verification or performance gain. Candidate/header/C++ source bytes are unchanged, no timing rows exist, and production remains old. Only a changed-source, privacy-gated follow-up can now collect the unexecuted native correctness/timing data. Evidence: evidence/performance-cold-stop-linked-replay-local.json.
