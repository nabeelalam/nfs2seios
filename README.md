# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

The user accepts build 10 as the base performance milestone: split-screen at
60 FPS with no observed drop or crash. This is feedback from their play session,
not proof covering every track or rare path. Reported battery drain has not yet
been measured under controlled conditions.

Build 12 adds native controller menu/input ownership without controller keyboard
aliases, held-resume consumption, a small keyboard toggle, and relative touch
cursor controls. One finger taps to click, two fingers send Escape, and three
send Enter. Nonblocking key holds let the original race input sampler observe
Escape and Enter. Local original-menu and two-player race checks pass.

Its sequential Linux/Apple run passed, including 45 production memory-span cases,
the new input suites, 27 Apple host tests and 40,000 mixed cases each for baseline
and ThinLTO. The downloaded version 12 thin ARM64 device Release package was
independently checked for exact source, contents, privacy and release digest.
It contains executable, metadata and licenses; game assets remain separate.
**Build 12 physical controller, keyboard, touch and FPS regression is pending.**

Use the [installation guide](INSTALLING.md) for the current download, personal
signing and separately supplied assets. The tappable FPS/CPU-app/RAM/render-size
overlay and 640x480, 960x720, 1280x960 or native-height 4:3 render targets remain.
GPU utilization is unavailable with the current renderer. Real OS/audio
interruption recovery and signing refresh still require acceptance. Controller
and touch changes need device acceptance; controlled energy measurement and
sharper HUD investigation remain. No energy or HD artwork improvement is claimed.

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
