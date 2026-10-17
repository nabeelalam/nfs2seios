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

Build 8 physical feedback reports playable racing and better FPS, with lower
split-screen performance and an intermittent unmapped-engine-span race failure.
Build 10 has passed sequential Linux and Apple checks and independent downloaded
package inspection. Its memory-check optimizations and ThinLTO retain ownership
and bounds checks. Physical split-screen FPS and stability remain pending.

## Remaining sequence

1. Reach constant 60 FPS in full-detail native-resolution split-screen racing
   and fix the intermittent race failure with a verified cause. Use both players;
   remap both original control profiles after selecting split-screen. Preserve
   original pointer/span rejection while investigating. Local measurements select
   candidates; physical iPad tests establish performance and crash acceptance.
2. Then remove controller-to-keyboard aliases and prevent resume presses from
   also triggering car actions. Retain native analog remapping and text entry.
3. Then map touch movement to the cursor, one-finger tap to click, two-finger tap
   to Escape and three-finger tap to Enter.
4. Finally investigate sharper HUD text, speedometer, map and starting lights.
   Original assets remain private and unchanged.
5. Continue scoped controller reconnect, audio, lifecycle, signing refresh and
   complete-race acceptance. Repeat only affected Apple build/package gates when
   relevant source changes justify another controlled standard-runner build.

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
