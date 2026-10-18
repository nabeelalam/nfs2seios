# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

Build 10 is the physically accepted split-screen 60 FPS baseline. Build 12
subsequently passed physical feedback for split-screen FPS and relative touch
tracking/gestures. A single-player Tournament freeze on Proving Grounds remains
under investigation; this is not general stability acceptance for every mode.

Build 17 reduces checked-copy lock overhead in six reviewed original helpers.
The verified [build 17 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39093193712-1/NFSIISEGame-development-unsigned.zip) is ready for personal signing.
Twelve bounded word-copy loops and two complete helper scopes retain typed
span checks, per-copy cancellation, original arithmetic and sound ownership.
Private two-order Release comparisons and complete sanitizer checks pass.
Scene differences limit those timings; physical speedup and battery savings
remain unverified. The earlier sound-operation fix is retained; the reported
physical Tournament freeze still needs current-build acceptance.

Four-finger tap shows diagnostics; detailed Game CPU appears after 60 submissions.
For the next performance comparison, use the same split-screen race with Low
Power Mode off/on and Lock FPS to 30 off. Game CPU excludes other workers, GPU
and waiting. Record comparable busy/clear sections rather than assuming the
game thread accounts for the entire frame.

Build 17 retains four-finger diagnostics toggling, initially hidden, an optional
Lock FPS to 30 switch (off by default), an independent keyboard button and Low
Power Mode status. The unused GPU label is removed. The game explicitly requests
fullscreen and opts into Apple Game Mode. Notification filtering is separately
controlled by Gaming Focus. Normal-speed rendering optimizations are retained.

The build 17 sequential run passed Linux validation and 30 Apple host tests,
40, 000 baseline and ThinLTO cases each, and verified ARM64 Release compilation.
The downloaded version 17 package passes exact-source, contents, privacy and
release-digest inspection. It contains only executable, metadata and licenses.
No original game assets, signing material or sanitizer runtime is included.
No probe was rebuilt and no Actions artifact was uploaded.

A bounded local original-data Tournament run used white McLaren F1, Proving
Grounds and all opponents. It continued through blinking-light-tunnel wall
contacts into lap two and shut down cleanly. This did not reproduce the iPad
freeze, complete the four-lap event or establish physical build 17 acceptance.
If frames stall, build 17 attempts to save one NFSIISE-stall.txt in the app's
Documents folder for diagnosis; it does not force recovery.

Use the [installation guide](INSTALLING.md) for the current download, personal
signing and separate game assets. Physical build 17 UI, Game Mode, pacing and
regressions still need testing. Stable 60 FPS in Low Power Mode, controlled
battery savings, lifecycle/signing refresh and sharper HUD artwork remain open.

- [PLAN.md](PLAN.md): mission, progress and remaining acceptance criteria.
- [BUILDING.md](BUILDING.md): preparation and controlled cloud compilation.
- [TESTING.md](TESTING.md): validation scope and device acceptance.
- [CI_POLICY.md](CI_POLICY.md): runner limits, efficient usage and publication gates.
- [INSTALLING.md](INSTALLING.md): unsigned-package and asset import procedure.
- [FEASIBILITY.md](FEASIBILITY.md), [PORTING.md](PORTING.md): source assessment and architecture.

Source dependencies are fetched at reviewed upstream revisions into ignored
directories. The project fetches pinned port support files with their upstream
license; the main GAMEDATA/FEDATA assets are never supplied by this repository.
Source publication alone does not establish Apple SDK compilation or iPad
gameplay. The reviewed workflow is in
`.github/workflows/ipados.yml` and runs only on deliberate manual dispatch.
