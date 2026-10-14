# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**An experimental development IPA is available; full iPad acceptance remains open.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

Build 4 launched on the target iPad with original menus, controller input and
game audio. The user reported playable 40-49 displayed race FPS, followed by an
original-memory error during Outback. Build 5 targets a demonstrated audio-state
initialization/decoding conflict; its exact production source passed a complete
two-lap original race, menu return and clean sanitizer shutdown locally, plus
controller steering/braking, pause and synthetic background/foreground recovery.
These host checks do not prove the reported physical error is fixed.

Build 5 passed sequential Linux and Apple ARM64 Release validation. Its checked
unsigned ZIP contains executable, metadata and licenses only. Use the current
[installation guide](INSTALLING.md) for the download, personal signing and
separately supplied assets. Physical build 5 race completion, performance,
real OS/audio-interruption recovery and signing refresh remain unverified.

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
