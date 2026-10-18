# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

On the separate HD branch, the inspected [build 21 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39161478449-1/NFSIISEGame-development-unsigned.zip) adds all ten gold speedometer digits and fixes timing-digit 7-9 recognition. Use HUD digits in rendering settings to compare Original and HD while recording a session. Other HUD art remains original. Main is unchanged; device appearance, performance and battery acceptance remain open.

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain original game executables, owned assets, signing credentials
or private operational records.

The main-branch baseline [build 19 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39134676336-1/NFSIISEGame-development-unsigned.zip) is ready for personal signing.
It adds start/stop session recording so a race can be logged without watching
the overlay. Four-finger tap, tap the FPS label, choose Start session recording,
then Stop & save recording and Share log after the race. Recording continues
with the overlay hidden. Files are also available in the app Diagnostics folder.
Low Power Mode, FPS, engine CPU time, app/main CPU work, RAM, battery, brightness,
thermal state and available whole-system core activity are captured about once
a second. The core readings are not per-core attribution of this app, and battery
percentage is not measured energy. Missing values and sampling gaps are explicit.
This diagnostic update claims no FPS or battery improvement; game engine,
renderer, audio ownership, input and import behavior are retained.

Physical build 17 feedback reports Proving Grounds running fine and North
Country split-screen maintaining 60 FPS normally, versus 37-51 FPS in Low Power
Mode. The reported CPU percentage was approximate; no Game CPU ms/frame result
was obtained. Short whole-percentage battery observations suggest a difference
but do not establish precise energy savings or abnormal consumption. Graphics
settings and matching test conditions were not reconfirmed with that feedback.

The build 19 sequential run passed Linux validation and 33 Apple host checks,
including the actual system-core provider, plus two sets of 40,000 mixed cases.
Independent inspection verifies a thin ARM64 iOS-device Release package, version
19, containing only executable, metadata and licenses. No game assets, signing
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
Native resolution, full graphics and Lock FPS to 30 off. Capture separate diagnostic recordings in normal and Low Power Mode, then share
the logs privately with the track, cars and game mode. Actual iPad recorder
controls, export and overhead still need physical testing.

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
