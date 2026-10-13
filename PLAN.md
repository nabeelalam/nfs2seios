# Native iPadOS port plan

## Mission and operating requirements

Deliver the original Need for Speed II Special Edition as a native ARM64 iPad
app for iPad Air M3 with controller input, accelerated graphics and game audio.
Use separately supplied original assets; no x86 emulation or JIT. Keep compiler,
linker, package, signing and physical gameplay evidence separate.

Two requirements are part of the goal:

1. Use standard public GitHub Actions efficiently. Build only to answer a
   specific remaining Apple SDK or packaging question after relevant local
   checks pass. Use the bounded, manual, sequential workflow in CI_POLICY.md.
   Avoid duplicate builds and unchanged deterministic failure reruns.
2. Publish only reviewed source and public documentation. Independently audit
   files and Git metadata before every commit or push. Keep credentials, signing
   material, original assets, captures, private evidence and personal filesystem
   information outside the public repository.

## Current shared implementation

- Explicit mapped 32-bit game values with full-width native pointers.
- Original translated functions linked to native platform services.
- File, event, thread, timer, audio, input, renderer and lifecycle boundaries.
- Asset import validates and copies separately supplied files into owned app storage.
- Local original menu/race rendering, keyboard driving and generated game PCM.
- Host regression and sanitizer checks, plus portable ARM64 compile checks.

These are development results. They do not establish current iPad gameplay.

## Remaining sequence

1. Reproduce and fix the intermittent invalid audio callback with an exact
   callsite and verified cause; do not treat successful retries as a fix.
2. Verify sustained original racing, complete laps and controller gameplay.
3. Measure rendering and audio performance with clear host/device scope.
4. After relevant local gates pass, perform a deliberate current-revision Apple
   SDK build and validate the ARM64 device package. Use standard runners only.
5. Personally sign and install the unsigned package, import original assets,
   and verify menus, race behavior, controller reconnect, audio and lifecycle.
6. Resolve physical-device failures and repeat only the affected validation.

Completion requires stable, playable original-game behavior on the physical
iPad with the controller and native audio/graphics. A host test or an unsigned
IPA alone is insufficient.
