# Building

Read [CI_POLICY.md](CI_POLICY.md) before requesting a cloud run. The workflow
uses standard Ubuntu and macOS runners only, with manual dispatch, sequential
validation and bounded timeouts. No automatic push/PR builds or caches are enabled.
The reviewed workflow is `.github/workflows/ipados.yml`. It runs only on manual
dispatch for a specific validation question after local checks pass. Source
publication does not trigger builds. See CI_POLICY.md before dispatch.

## Source preparation

```sh
python3 scripts/prepare_full_mapped.py
python3 scripts/prepare_renderer.py
```

These scripts fetch and verify pinned source revisions and prepare disposable
generated source. They do not fetch the separately supplied main game assets.
Do not commit generated source, dependency checkouts or local tools.

## Host checks

On a supported host with the required compiler and platform libraries:

```sh
cmake -S . -B .build/host -DCMAKE_BUILD_TYPE=Debug -DBOUNDARY_SANITIZERS=ON
cmake --build .build/host --target core_tests mapped_engine_tests -j 3
ctest --test-dir .build/host --output-on-failure
```

Build the remaining test targets before requesting the full host suite. Linux
GLES/SDL checks additionally require the dependencies installed by the workflow
and NFS_RENDERER_PIXELS=ON. Original-data tests use separately supplied assets
in an ignored owned copy. Never upload the files or their private captures.

## Apple build

Dispatch the reviewed workflow for a specific public main-branch commit. Linux
privacy and native validation gates precede the optional macOS build. Select
build_ios deliberately; rebuild the separate Probe only when a platform change
requires it. Xcode 16.4 is explicitly selected on macos-15 and versions are logged.
The workflow compiles the original mapped engine, links native platform services,
and validates unsigned ARM64 iPad bundles. No signing credentials are used.

IPA uploads default to off. Before selecting publish_ipa, verify account-wide
storage headroom. The payload is bounded and expires after one day. Ordinary
workflow logs provide build diagnostics without redundant stored log artifacts.
Apple device validation has passed for the renderer optimization; physical performance remains unverified.

## Publication checks

Install the digest-pinned independent scanner with
`python scripts/setup_secret_scanner.py` on supported Windows/x64 Linux hosts.
Use `python scripts/publication_gate.py --mode staged` before a commit and
`python scripts/publication_gate.py --mode history` before a push. Configure
repository-local Git hooks with `git config --local core.hooksPath .githooks`;
on Unix, make both hook files executable. Use a GitHub no-reply commit identity.
Checks supplement manual review of the exact diff, history and package inputs.

Select `configuration=Release` for a deliberate optimized device-test build,
keeping `include_probe=false` unless the diagnostic changed. Enable `publish_ipa`
only after checking storage headroom; the aggregate upload limit is 64 MiB and
retention is one day. Debug remains available for targeted compiler diagnostics.
The package manifest records the selected configuration.

ThinLTO is an opt-in Release compiler experiment. Select `thin_lto=true` in the
manual workflow, or configure locally with `-DNFS_THIN_LTO=ON
-DNFS_THIN_LTO_JOBS=2` and a Release build. The selected game targets use ThinLTO;
SDL keeps its existing settings. Compiler/linker probes run before the full
build, and the workflow limits compile jobs and linker backend workers to two.
Debug and the default workflow configuration retain their existing compiler
settings. The optional Apple host gate also checks 40,000 mixed integer/FPU
states with ThinLTO enabled. Build 9 passed the Apple SDK build and independent downloaded-package checks.
Physical split-screen FPS and race-stability validation remain required.


## Executable-only development downloads

Select publish_release for a deliberate prerelease download instead of an
expiring Actions artifact. This forces Release and excludes the separate Probe
and Actions artifact uploads. The checked ZIP contains an unsigned IPA and its
manifest; the IPA contains only the executable, app metadata and license notices.
No original data, startup configuration or language files are distributed.
Ordinary SDK validation remains read-only; only the explicit release caller can
publish. No build runs automatically when source is pushed.

Build3 is an update for an existing imported Documents/game installation. Install
over the existing app without deleting it. Its startup files must remain in that
app-owned folder. The newer source importer additionally accepts separately
supplied install.win and text.eng in the selected root beside GAMEDATA/FEDATA,
with optional text.* language files. Those files must be compatible with the
translated engine; do not assume an original CD root contains them. This importer
path has host validation only and is not included in build3 or device-validated.


The production engine link check includes 45 span-routing and lifetime cases.
These exercise EngineMemory itself, including adjacent-allocation rejection,
unaligned access, free/reallocation invalidation and DATA/BSS boundaries. The
mapped-engine unit fixture uses a substitute resolver and cannot replace this
gate. A successful fixed-heap ownership check now returns the existing backing
pointer directly; invalid spans retain their original rejection paths. This
source optimization is included in the independently inspected build 10,
which passed the Apple SDK build and still requires physical split-screen validation.


The heap also tracks conservative 4096-byte logical chunks, independently of
operating-system page size. A flag is true only when an entire chunk belongs to
one live allocation. Single-chunk requests can use that proof directly; partial
chunks and cross-chunk requests retain the existing ownership checks. Freeing
an allocation clears its flags before reuse. Initialization validates the mapping
before metadata allocation and rolls the new mapping back on allocation failure.
The metadata costs 32 KiB for a 128 MiB heap. Bounds, cancellation and guest-copy
serialization remain enabled. Added production cases cover complete chunks,
cross-chunk spans, frees and smaller allocations reusing the same addresses.
The actual C++14 ThinLTO engine and shared runtime pass the production link
suite. Separate address/undefined sanitizer checks cover initialization rejection
and allocation-failure rollback, threaded lifetimes and 12,320 independently
checked ownership cases. Build 10 passes actual Apple compilation and independent
downloaded-package checks. Physical acceptance remains pending.


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


Build 13 candidate: four-finger tap toggles diagnostics, hidden initially;
the keyboard button remains available. Diagnostics omit the unavailable GPU
label and show Low Power Mode. A separate switch enables an optional 30 FPS
worker-side cap, off by default. Original simulation timing is unchanged;
actual iPad pacing and battery effects still require device tests.

The game window explicitly requests iPad fullscreen and the bundle opts into
Game Mode. Gaming Focus controls notification filtering separately. A best-effort
monitor attempts to save one bounded NFSIISE-stall.txt in Documents after ten
seconds without a presented frame while active. It samples original function
IDs, event-loop progress and CPU-time change; it does not publish game data or
force recovery. Long loading can also trigger a report, so a report alone does
not establish the cause of a freeze. The Tournament/Proving Grounds collision
freeze and 60 FPS in Low Power Mode remain unresolved physical-device targets.

Local validation: 37 sanitizer checks, including four-finger gesture isolation,
actual SDL worker pacing/cancellation and bounded concurrent call sampling.
Build 12 has physical acceptance for split-screen FPS and touch gestures. That
does not establish build 13 acceptance. No battery saving is claimed.


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

## Build 17 checked-copy validation candidate

The manual workflow explicitly selects twelve reviewed word-copy loops and
two complete helper scopes, with eight and twenty-nine checked copies. Each
scope retains the existing typed span and cancellation checks and stays within
the thirty-two-copy limit. Original integer and floating-point behavior is
preserved; no unchecked pointer cache is enabled. Both complete helper bodies
are pinned by input and output hashes so upstream changes require review.

Generation remains opt-in outside the reviewed workflow. A new output directory
is required; clean CI jobs use `.build/fullmapped` so existing compile and test
include paths remain valid. To review separately after pinned preparation:

```sh
python3 scripts/prepare_full_mapped.py --checked-copy-bundle \
  --output-dir .build/fullmapped-copy-review
```

Private original-data Release comparisons in both run orders and the complete
ASan/UBSan candidate passed bounded two-player driving, reset, simulated
lifecycle and clean shutdown. Scene differences limit the timing comparison;
no completed-event, physical iPad speedup, Low Power Mode or energy improvement
is established. Five generated units exactly match the tested candidate and
the other ten method units remain unchanged. Build 17 requires Apple compilation,
package inspection and physical acceptance before those claims can advance.


### Reproduce the new small HUD font

Use the same local librsvg/Cairo vector rasterizer as the timing and speed
sources. No original game files or downloaded font are used by the generator.

```sh
python3 scripts/generate_small_builtin.py --check
```

The `small_font_tests` and `small_font_select_tests` host targets validate the
bounded tile decoder and complete glyph-corner selection. For private original
data validation, the renderer pixel fixture optionally reads the local case
file named by `NFS_HUD_FONT_CASES`; leave that variable unset for ordinary
public CI. The optional case file and original captures must stay outside Git.


Record-time artwork uses the same local vector rasterizer and bounded source
policy. Check its generated source with:

```sh
python3 scripts/generate_record_builtin.py --check
```

The `record_font_tests` host target checks the eleven new stored tiles. Zero
shares the O tile and needs no additional raster. Private renderer-matrix input
now contains 38 ordered visible cases: A-Z, 0-9, dot and colon. Keep those
original cases outside public Git and leave `NFS_HUD_FONT_CASES` unset for
ordinary public CI.

A bounded local original-game split-screen run also exercised both virtual
controllers, resolution changes, reset and simulated suspension/context recovery.
New record characters were observed in both views; shutdown was clean and the
copied profile was restored. This was a short driving check, not a complete race
or iPad visual, performance or energy acceptance.
