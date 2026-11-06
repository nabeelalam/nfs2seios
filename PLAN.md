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
