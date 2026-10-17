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

## Current shared implementation

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
