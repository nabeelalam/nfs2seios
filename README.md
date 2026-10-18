# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain original game executables, owned assets, signing credentials
or private operational records.

The verified [build 18 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39118406383-1/NFSIISEGame-development-unsigned.zip) is ready for personal signing.
It adds battery percentage, a prominent Game CPU time in ms/frame, and per-core
activity for the whole iPad when the system API provides valid counters.
Per-core readings are not this app's usage on each core. Sampling occurs at most
once per second while detailed diagnostics are visible. The game engine,
renderer, audio ownership, input and import behavior are retained from build 17;
this diagnostic update makes no FPS or battery improvement claim.

Four-finger tap shows or hides diagnostics. Tap the overlay for settings and
enable Detailed diagnostics. Game CPU averages need sixty submissions and
exclude other workers, GPU and waiting. App CPU is separately labelled as a
percentage across all cores. Battery percentage also appears in compact view.
Actual per-core API availability and the new layout still need an iPad test.

Physical build 17 feedback reports Proving Grounds running fine and North
Country split-screen maintaining 60 FPS normally, versus 37-51 FPS in Low Power
Mode. The reported CPU percentage was approximate; no Game CPU ms/frame result
was obtained. Short whole-percentage battery observations suggest a difference
but do not establish precise energy savings or abnormal consumption. Graphics
settings and matching test conditions were not reconfirmed with that feedback.

The build 18 sequential run passed Linux validation and 32 Apple host checks,
including the actual system-core provider, plus two sets of 40,000 mixed cases.
Independent inspection verifies a thin ARM64 iOS-device Release package, version
18, containing only executable, metadata and licenses. No game assets, signing
material or sanitizer runtime is included. No probe was rebuilt, no Actions
artifact uploaded and no extra build dispatched for these documentation changes.

Diagnostics start hidden; the optional Lock FPS to 30 switch defaults off. The
keyboard button, touch gestures and controller remapping remain available. The
app requests fullscreen and opts into Apple Game Mode; Gaming Focus separately
controls notification filtering. Stable 60 FPS in Low Power Mode, controlled
battery efficiency, broader stability and OS lifecycle/signing refresh tests,
and sharper HUD artwork remain open.

Use the [installation guide](INSTALLING.md) to update through SideStore with the
same Apple Account and keep using the separately stored original game folder.
For the next comparison, use the same North Country split-screen section,
Native resolution, full graphics and Lock FPS to 30 off. Record FPS and the
explicit Game CPU time in ms/frame in normal and Low Power Mode.

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
