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

The intermittent invalid audio callback, sustained races, full laps, controller
gameplay and the latest physical game build remain open. Do not declare the port
complete based on a successful cloud compilation or retry.

## Current public Apple SDK validation

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

The next candidate adds a tappable diagnostics overlay and 640x480, 960x720,
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
