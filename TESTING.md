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
complete laps. Brake, shifts and remaining actions need their own checks.

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
