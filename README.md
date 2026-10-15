# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

Build 7 physical feedback reports completed crash-free races at 36-60 displayed
FPS, approximately 200 MB RAM and similar FPS at 480p and native resolution.
Native resolution substantially improves image quality. Controller remapping
works, but a resume press can also activate a driving action.

Build 8 reduces repeated synchronization around bounded original-engine
arithmetic while retaining individual memory-span and allocation checks. It
passed sequential Linux and Apple ARM64 Release validation, including 40,000
mixed arithmetic equivalence cases on both hosts and 27 Apple host tests.
The downloaded unsigned package contains executable, metadata and licenses only.
Local original-data sanitizer tests complete a race and menu return; promising
Linux performance measurements do not establish an iPad FPS improvement.
**Build 8 physical testing and the constant 60 FPS target remain pending.**

Use the [installation guide](INSTALLING.md) for the current download, personal
signing and separately supplied assets. The tappable FPS/CPU-app/RAM/render-size
overlay and 640x480, 960x720, 1280x960 or native-height 4:3 render targets remain.
GPU utilization is unavailable with the current renderer. Real OS/audio
interruption recovery and signing refresh still require acceptance. Controller
input, touch gestures and sharper HUD work follow performance in priority order.

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
