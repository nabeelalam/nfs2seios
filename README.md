# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

Build 8 physical feedback reports improved FPS, reaching 60 in some sections,
but lower split-screen performance and an intermittent unmapped-engine-span
race failure. Constant split-screen 60 FPS and the crash fix remain open.
Controller remapping works, but a resume press can also activate a driving action.

Build 10 retains bounded ThinLTO and adds a direct heap-pointer return plus
conservative logical-chunk ownership metadata. Bounds, allocation lifetime,
cancellation and copy serialization remain checked. Its single sequential Linux
and Apple run passed, including 45 production span/lifetime cases, all 27 Apple
host tests and 40,000 mixed cases for each compiler configuration. The changed
runtime and all translated units used verified ARM64 device Release ThinLTO.
The downloaded version 10 package was independently inspected for source,
architecture, contents, privacy and digest. It contains executable, metadata
and licenses; original game assets remain separately supplied.
**Build 10 physical FPS and race-stability acceptance remain pending.**

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
