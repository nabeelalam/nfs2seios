# nfs2seios

Development of a native ARM64 iPadOS port of Need for Speed II Special Edition,
targeting iPad Air M3 and an external controller, without x86 emulation or JIT.
**A playable iPad release is not available yet.**

The project adapts a pinned original-game C++ translation, the existing Glide
GLES2 renderer and SDL3 platform services. Original game assets must be supplied
separately. This repository contains source and public development documentation;
it does not contain the original game executable, owned game assets, signing
credentials or private operational records.

The shared engine has rendered original menus and races locally, with keyboard
acceleration, steering and non-silent game PCM. Host checks cover mapped memory,
file services, audio, renderer boundaries and controller input. An intermittent
audio callback failure remains unresolved. Complete laps, controller gameplay,
sustained stability and the latest physical iPad game build remain unverified.

- [PLAN.md](PLAN.md): mission, progress and remaining acceptance criteria.
- [BUILDING.md](BUILDING.md): preparation and controlled cloud compilation.
- [TESTING.md](TESTING.md): validation scope and device acceptance.
- [CI_POLICY.md](CI_POLICY.md): runner limits, efficient usage and publication gates.
- [INSTALLING.md](INSTALLING.md): unsigned-package and asset import procedure.
- [FEASIBILITY.md](FEASIBILITY.md), [PORTING.md](PORTING.md): source assessment and architecture.

Source dependencies are fetched at reviewed upstream revisions into ignored
directories. The project fetches pinned port support files with their upstream
license; the main GAMEDATA/FEDATA assets are never supplied by this repository.
Publication does not establish Apple SDK compilation or iPad gameplay for the
current revision. The reviewed workflow is provided as an inactive template in
`ci/ipados.yml`. Activation requires workflow-write permission; no cloud builds
are currently enabled. Once activated, it runs only on deliberate manual dispatch.
