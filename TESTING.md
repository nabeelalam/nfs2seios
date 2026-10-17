# Validation and acceptance

Keep test evidence scoped to what was executed. Source publication is not build
or gameplay evidence. Local source tests and a previous platform diagnostic do
not establish working gameplay in the current iPad app.

## Host checks

The test sources cover guest address bounds, unaligned and endian access, native
callback identity, original translated routines, heap lifetime, files/mappings,
events, threads, timers, renderer boundaries, audio callback lifecycle, virtual
controller input/reconnect and asset import. Use sanitizers where supported.
Boundary checks must not be weakened merely to let game startup continue.

Public-repository checks also test forbidden-file rejection, secret/path patterns,
safe commit metadata and aggregate artifact-size limits. An independent secret
scanner checks the exact published history in addition to the project policy.

## Original-data local checks

Use a separately supplied, ignored owned copy of the original assets. Record
menu/race behavior, acceleration and steering, callback failures and clean
shutdown. Keep captured graphics, audio and raw diagnostics private. Non-silent
PCM proves generation, not audible correctness. Software rendering results do
not establish Apple GPU performance. Incomplete or forcibly killed runs fail.

## Apple/device acceptance

- Current source compiles and links against the actual Apple device SDK.
- The bundle is ARM64, declares the iOS device platform and targets iPad.
- The package excludes separately supplied main game assets and signing secrets.
- Personal signing and physical launch succeed.
- Imported original assets load, menus work, and complete races are stable.
- Controller axes/buttons, disconnect/reconnect and release states work in-game.
- Original audio sounds correct and remains stable through transitions.
- Background/foreground, graphics context, audio and clean shutdown work.
- Performance and memory use are measured on the physical target.

Build5 physical feedback confirms completed playable races and no further
crashes in the reported sessions. Broader controller/lifecycle coverage and the
latest build's physical acceptance remain open. Do not declare the port complete
based on a successful cloud compilation or retry.

## Earlier Apple SDK validation

Revision `2b8a4a6fd3b751f93781a5178c4088fa490c7e03` passed the
[manual validation run](https://github.com/nabeelalam/nfs2seios/actions/runs/38450215719).
Linux checked native linkage, mapped contexts above 4 GB, original missing-asset
startup/shutdown, GLES pixels, SDL audio callbacks and gamepad reconnect.
macOS passed all 27 host checks using Xcode 16.4/iPhoneOS 18.5, compiled and linked
the complete mapped ARM64 device engine, renderer, platform services and game
app, then validated and packaged its unsigned Debug IPA.

No IPA artifact was uploaded in this validation run. Game assets were absent
from cloud jobs. Signing, installation, original-data gameplay, sustained racing,
physical controller behavior and audible game audio remain separate device gates.
The intermittent original audio callback failure remains unresolved.

## Configure original racing controls

The original game may start with a keyboard profile even when a controller
navigates menus. Open **Options -> Controls -> Setup**. Select each row, confirm
with the controller's south button, then supply the desired new input. Bind
steer left and steer right by moving the left stick in the respective direction.
Bind accelerate by pressing the right trigger. Choose **DONE** to accept the
bindings and leave the controls menus before starting a race.

A local original-data session verified this complete flow with an SDL virtual
gamepad: both steering directions were captured, right-trigger acceleration
reached displayed speed 105 in third gear, and calibrated steering returned
to neutral after release. The session exited cleanly and generated non-silent
game PCM. This does not establish physical iPad input, audible output or stable
complete laps. A separate sustained-button local run also accepts left-trigger braking, X
handbrake, shoulders for shifts and Y camera. Calibrated brake transitions
0 -> 255 -> 0, and a rendered race slows from speed 32 to 0 with the brake
held. Detailed handbrake/camera action effects, shift behavior, full laps and
physical-device braking remain separate checks.

## Physical game feedback and main-thread wait correction

The first optimized iPad game package launches and accepts controller input.
The tester reports original audio works, with occasional stutter, but both
menus and races run at an estimated 1-2 FPS. This establishes game execution
on the device, not playable performance or sustained stability.

All Glide calls currently wait synchronously for main-thread execution. On
iOS, those calls use GCD, whereas SDL event waiting falls back to a sleep
that GCD submissions cannot wake. The app now services the main CFRunLoop
while idle on iOS, retaining SDL event waiting elsewhere. SDL event polling
and lifecycle processing remain in the app loop. Five-second aggregate logs
report renderer call counts, time awaiting the owner and time executing on
it; counters are approximate concurrent aggregates, not GPU timer queries
or an FPS measurement. This removes a source-confirmed per-call sleep, but
physical speedup and any remaining bottleneck require another device test.
The Apple host transport test uses the same new idle function.

The first validation attempt stopped at the Linux missing-assets fixture: its
three-second internal deadline expired before the expected file-load error,
so the Apple job was skipped. All four current startup fixtures pass locally
with sanitizers and graphics shader caching disabled. The fixture now uses
the production idle function, logs elapsed/renderer timing, and allows ten
seconds inside the existing fifteen-second external limit. It still fails
on a deadline, wrong/missing expected error, runtime exception or wrong exit
code; the cause of the shared-runner delay is not established.

The corrected Release source in run 38463370000 passes Linux gates and all
27 Apple host checks, compiles/links the actual ARM64 iPad bundle and packages
a roughly 1.9 MiB artifact. Independent downloaded-package checks confirm
source/configuration identity, iOS-device ARM64 architecture, pinned support
hashes and the expected twelve bundle files. Owned assets and signing material
are absent. Physical performance of the new idle wait remains unverified.

Physical follow-up reports improved menus, successful controller remapping and
perceptually 10-19 race FPS, still below enjoyable driving. These estimates are
not instrumented measurements. Build 2 adds a small presentation-FPS overlay
and includes its sampled value in the existing aggregate timing log. It counts
successful swap submissions, including front-buffer updates; it is not a GPU
timer or a physical display scanout measurement. Counters reset across graphics
suspension and restoration.

The candidate queues up to 128 owned drawing commands, copying vertices and
scalar arguments. Results, readbacks, guest palettes/buffers, presentation and
context changes remain ordered barriers. Tests cover four concurrent producers,
4000 ordered commands, exception transport, borrowed-buffer cancellation and
actual GLES surfaces through suspension. D-pad navigation retains POV and menu
arrow messages while excluding duplicate native action-button bits; the original
frontend local reproduction stays on the selected menu instead of opening it.

Repeated heap allocation-span checks use four thread-local copied spans validated
against an allocation-table revision and arena lifetime identity. Allocation/free
invalidates the spans; cache misses retain locked bounds checks, and shared game
value-copy serialization remains. Sanitizers check cross-thread frees, smaller
same-address allocations, exact arena-object address reuse, concurrent allocation
churn and span boundaries. A bounded full-data Debug sanitizer run visibly drives
the original race, advances the clock, reaches 70 in second gear, steers left,
and exits with completed SDL shutdown and no sanitizer diagnostic. This is not
a complete lap or physical iPad acceptance.

Local software-GLES race samples improve from an average 11.388 to 17.044 over
the final 25 reports after the span cache. Scene timing, software rendering and
brief build overlap limit comparison: no precise speedup ratio or iPad FPS
prediction is claimed. The next Apple build validates the UIKit overlay and
native candidate; physical steady menu/race FPS and responsiveness remain open.

Build 5 serializes six original codec initializers and eight complete decoding
jobs within the matching sixteen-voice state pool. Engine-owned recursive locks
allow nested decoding phases and remain alive until worker cleanup completes.
Focused sanitizer checks cover same-voice initializer blocking, independent
voices, recursion and exception release. Original method bodies remain unchanged
apart from entry guards. The full reviewed production source, with only a private
virtual-controller entry fixture, passes a bounded three-minute original Outback
McLaren F1/eight-car run, observed steering/braking, PCM generation and clean
shutdown without sanitizer errors. The earlier guarded diagnostic prototype
completed a two-lap Proving Grounds race and returned to the menu. These host
results do not establish physical iPad race stability or a performance increase.

The same rebuilt production source subsequently completed the original two-lap
Proving Grounds race with McLaren F1 and seven opponents, showed results and
returned to the selected Race main-menu row. Clean shutdown occurred at 382.139
seconds without address/undefined/leak sanitizer errors. Original PCM was
captured locally; intermediate Lap Times and audible continuity were not
independently verified. Build 5 source `78c9cb04e9f0ed1dd886f38db691eeeedd2342ec`
passed sequential Linux and Apple Release checks in run `38637807635`.
The downloaded public ZIP was independently checked for that source revision,
version 5, iOS-device ARM64 and exactly five executable/license/metadata entries.
It contains no supplied startup or game files. Physical Outback completion,
performance and lifecycle acceptance remain open.

The same production candidate also passes original controller B pause/A resume
and synthetic SDL background/foreground recovery under sanitizers. Background
unbinds graphics, freezes both captured presentation and PCM output; foreground
rebinds graphics, produces fresh PCM and resumes observed driving. Shutdown is
clean. These injected SDL events do not prove real iPadOS suspension,
AVAudioSession interruption handling or audible quality.

Physical build 5 feedback reports automatic launch without the picker, completed
races and no further crashes. Reported race FPS is 35-57 (about 45 average),
improving to 57-59 with reduced horizon/reflection/detail settings. This shows
scene complexity matters, but does not isolate CPU work from GPU work.

Builds 6 and 7 add a tappable diagnostics overlay and 640x480, 960x720,
1280x960 or native-height 4:3 rendering choices. CPU/app uses process user/system
time divided by elapsed time and logical-core capacity; RAM is process resident
memory. Frame milliseconds are the averaged presentation interval, not GPU time.
GPU utilization is explicitly unavailable. Choices persist and framebuffer
changes occur at the existing swap boundary. Real GLES sanitizer tests verify
all presets, native target2186x1640 in drawable2360x1640, original RGB565 readback
and return to640x480. Initial asynchronous-resize observation was corrected with
SDL synchronization; the original failure log is retained locally.

The original decoder callback is also selected under its matching voice lock at
two original dispatch sites. The private prototype passes original Outback
controls/audio and a complete two-lap race/menu-return/clean sanitizer shutdown.
Actual original codec initializers validate callback publication blocking and
stack balance in the focused linkage check. Broader packet retirement lifetimes
and physical error causality are not proven by this ordering change.

Build 6 source `5fb787fc0efccfbc0ff8825894057190ac6550f3` passes run38646762033
after correcting the precise Apple compiler failure in the initial run38645086168:
the new overlay's weak ownership requires an explicit source-level ARC flag in
the mixed-language Xcode target. The released ZIP is independently downloaded and
checked for source, version6, iOS-device ARM64 and exact five-entry contents with
no supplied startup/assets or signing material. New overlay/selector and native
resolution performance still need physical acceptance.

## Build 7 cleanup and delivery

Build 7 source `b8202db35451d68944cbe2870df19c1eba5d70fa` passes
[manual run 38653406201](https://github.com/nabeelalam/nfs2seios/actions/runs/38653406201).
The downloaded Release ZIP independently passes source identity, version7,
thin ARM64 iOS-device architecture, same bundle identity and exact five-file
executable/metadata/license inspection. No supplied game/startup files or signing
material are included; zero Actions artifacts were uploaded.

An original-data local stop-while-suspended failure was captured in renderer
shutdown waiting for foreground work. The fix permanently retires queued
rendering, releases captures outside locks, resets CPU framebuffer ownership and
releases the already-unbound context without issuing explicit GL deletion calls.
It retains the original CPU exit callbacks and worker cleanup. Focused sanitizer
checks cover a full queue, reentrant capture destruction and cancellation while
a borrowed caller waits for retirement.

The candidate exits cleanly from the captured suspended-stop sequence, retains
ordinary pause/resume and foreground shutdown, and completes an original two-lap
Proving Grounds race with results and menu return under address/undefined
sanitizers. A separate local foreground cleanup timeout remains unexplained;
the suspended-stop fix is not claimed as its common cause. Local software
rendering and injected lifecycle events do not prove iPad GPU performance,
actual OS suspension or audible continuity.

On the iPad, compare 640x480 and Native in the same scene and detail settings,
record FPS and CPU/app, check the chosen resolution after relaunch, and verify
Home/resume. Real audio interruption, orientation and personal-signing refresh
acceptance also remain open. See INSTALLING.md for the current download.

## Build 8 arithmetic synchronization and delivery

Source `ffb3475cacc70289637dd26957c406deaea11662` passes
[manual run 38799395535](https://github.com/nabeelalam/nfs2seios/actions/runs/38799395535).
Linux validation precedes the standard ARM Mac Release build. Both hosts pass
40,000 O3 mixed integer/FPU/branch/alias equivalence cases; all 27 Apple host
tests pass. Bounded transactions retain individual span/ownership checks and
the existing mutex, ending before control flow, callbacks and unsafe helpers.

Private local sanitizer checks complete a two-lap original race, results/menu
return and clean shutdown. Outback input and render-density checks also pass.
Local software-rendered comparisons show a promising improvement, with traffic
and scene timing differences preventing a precise controlled percentage claim.
These results do not prove iPad performance, audio quality or full acceptance.

The downloaded version8 package independently matches the built source, thin
ARM64 iOS-device Release architecture and the existing bundle identity. It has
exactly five executable/metadata/license entries, no original assets or signing
material. No Actions artifacts were uploaded. Physical build8 testing remains
pending: Native resolution, unchanged full detail, Outback/McLaren F1/all
opponents, menu and busy/clear race FPS, full-race completion and working audio.
Constant60FPS is not yet established. See INSTALLING.md for the current ZIP.


## Build 10 acceptance boundary

The [build 10 run](https://github.com/nabeelalam/nfs2seios/actions/runs/38941504663)
passed Linux production linkage with 45 span/lifetime cases, real GLES pixel
checks, SDL audio and controller checks. Apple passed 27 host tests and 40,000
mixed states with each of the baseline and ThinLTO configurations. Actual device
compile logs verify C++14/O3/ThinLTO for the changed memory runtime, all 15
translated method units and three Objective-C++ units; device links cap ThinLTO
backends at two. The separately downloaded version 10 package passes source,
digest, privacy, thin ARM64 iOS-device and executable/license-only checks.
Its app metadata differs from build 9 only in CFBundleVersion.

Two local split-screen comparison orders observed about 7.2-7.4% less game-thread
CPU per presented frame and 2.3-3.2% higher software-rendered FPS. Those runs have
natural traffic differences and are not physical or formal causal speedup proof.
A corrected-header session exercised both players' steering and throttle for
three minutes after setup and exited cleanly, but hit barriers and did not
complete a lap. Full-detail Native split-screen FPS, complete races, rare-failure
repair and lifecycle/signing acceptance still require physical evidence.


## Build 12 physical acceptance

The package/build checks pass; these physical checks remain:

1. Use Native resolution, full graphics and two controllers in Split Screen.
   Compare busy/clear sections with the user-accepted build 10 60 FPS baseline,
   complete a race, and check audio/stability.
2. Check both native controller profiles and menu navigation/confirmation.
   With X mapped to downshift, resume Pause with X and hold briefly: gear should
   stay unchanged. Release and press again: downshift should work.
3. Use the small keyboard icon to show/hide the keyboard and supply Return for
   original remapping. Check actual text/name entry when a suitable field occurs.
4. Check relative one-finger movement and tap click, two-finger Escape in menus
   and racing, and three-finger Enter. Check any system-gesture interference.
5. Home/resume should not leave a gesture key or controller button held. Check
   actual OS/audio interruption recovery separately from injected Linux events.

For energy comparisons, match brightness, volume, resolution, controllers,
network state, battery range and starting temperature. Repeat matched-duration
menu-idle, paused and active split-screen sessions, alternating baseline and
candidate order. Record elapsed time, battery start/end, FPS and app CPU.
Battery percentage is coarse; no energy saving is established by host tests.


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

Verified unsigned download: [build 14 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-38995915505-1/NFSIISEGame-development-unsigned.zip).


## Build 14: performance and pacing validation; device acceptance open

The arithmetic copy transaction minimum is reduced from four checked accesses
to two. Every access retains its mapped-span, live-allocation and cooperative
stop checks, the same shared mutex, and existing control/call/declaration
barriers and 32-access/64-line limits. Eleven generator checks, 40,000 mixed
aliased/unaligned CPU/FPU/memory cases under ASan/UBSan, whole-source operation
reconstruction, and exact integrated/tested output matching cover all 15 units.

Four optimized original two-active-player split-screen trials in both orders
observed 8.10-10.18% lower game-worker CPU per displayed frame and 4.34-9.03%
higher presentation FPS on Linux. Scenes and traffic naturally vary, so these
are directional observations, not identical-frame or physical iPad gains.
Linux executable text grows 20.38%; Apple size and device behavior remain open.

Optional 30 FPS pacing now uses an interruptible worker event wait; unrestricted
mode avoids the timer query. Thirty-seven host sanitizer tests pass. A fresh
65-project-unit ASan/UBSan game build and a combined-change split-screen replay
cover both players' gears, pause, held/fresh resume, reset, simulated lifecycle,
and switching the cap off, with clean shutdown and no reported sanitizer error.

Source 3ade3b33bf122323842bb199bee0b4fcf97f1616 passed one sequential standard
Linux169s/Apple520s run38995915505. The new Linux pacing/sampler tests executed;
Apple passed28host tests and40,000 mixed cases each for baseline/ThinLTO. All15
translated units compile ARM64 iOS-device O3/C++14/ThinLTO; two links retain the
two-backend limit. The downloaded version14 thinARM64device Release passes
exact-source,contents,privacy and digest checks. It contains executable,metadata
and two licenses, with no original assets,signing material or sanitizer runtime.
Zero Actions artifacts; no Probe rebuild or extra dispatch.

Physical same-settings split-screen Low Power Mode off/on tests remain needed.
Battery savings,constant Low Power Mode60FPS,real OS/audio recovery,completed
events and the reported Tournament freeze are unverified. A separate optimized
source13 long-run shutdown wait was captured locally; no fix is claimed in14.

## Unreleased worker-error cleanup validation

The complete local sanitizer host suite passes37/37 checks after integration.
Thread tests cover first-error notification after ownership retirement, closed
and reaped handles, concurrent failures, independent owners, cooperative stop
and retained join errors. Exit tests preserve the legacy LIFO/nested/error
behaviour and cover the additional failure-cancellable registry path.

Separate controlled native-session checks exercise normal/explicit exit,
interrupted entry, original callback errors, background cancellation and child
failure. Late child failure during an original exit callback now unwinds at its
existing cancellation checks; a recursive critical-section case confirms the
protected guest write completes before the final release allows cancellation.
These controlled tests are distinct from natural original-data gameplay.

The local original split-screen replay checks both players and simulated
lifecycle recovery before deliberately failing a separate native worker. The
application reports that error and drains with expected error status, rather
than requiring an external stop. A healthy original menu exit still writes
settings. Completed native writes are retained; pending original save logic
cannot be guaranteed after a genuine failure. No new physical freeze, power or
performance acceptance is claimed. Build14 remains the delivered device package.

## Game-thread CPU measurement candidate

The local suite passes39/39 sanitizer checks, including deterministic sampling
cadence, disabled/non-owner paths without clock queries, reset during an in-flight
valid sample, unavailable/regressed clocks, and the real host thread CPU provider.
Eight controlled native-session cases also pass against current runtime objects.
The separate optimized original-data two-player replay passes with clean shutdown;
its CPU values are Linux sanitizer measurements, not iPad performance estimates.

For device comparison, use the same split-screen race, full graphics and Native
resolution, with Lock FPS to30 off. Four-finger tap to show detailed diagnostics.
Allow at least60 submissions after showing diagnostics or changing Low Power Mode.
Record FPS and Game CPU ms/frame in similar busy and clear sections, first with
Low Power Mode off and then on. Game CPU excludes waiting and other workers; a
small value does not establish that the full frame is GPU-bound. Hide diagnostics
for any later controlled battery comparison. Temperature, brightness, audio and
radio use must be comparable; a short percentage plateau cannot establish energy
savings. Device acceptance is still required.


## Build15 Apple/package validation; physical acceptance open

Sourceafd2bbe passed30 Apple host checks,including the real CPU-provider test,
and40,000 baseline plus40,000 ThinLTO mixed state cases. All15 translated units
and eight reviewed native runtime units compile as ARM64 iOS-device Release
O3/C++14/ThinLTO. Both device links retain the two-backend cap. The independently
downloaded version15 package matches source and release digests. It contains
only executable,metadata and two licenses,no assets,signing material,private
computer paths or sanitizer runtime. Zero Actions artifacts were uploaded.
These gates are separate from device performance,stability and battery acceptance.
