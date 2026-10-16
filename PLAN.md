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

Build 8 physical feedback reports playable racing and better FPS, with lower
split-screen performance and an intermittent unmapped-engine-span race failure.
Build 9 has passed sequential Linux and Apple checks and independent downloaded
package inspection. Its ThinLTO compiler path retains checked memory access;
physical split-screen FPS and stability acceptance remain pending.

## Remaining sequence

1. Reach constant 60 FPS in full-detail native-resolution split-screen racing
   and fix the intermittent race failure with a verified cause. Use both players;
   remap both original control profiles after selecting split-screen. Preserve
   original pointer/span rejection while investigating. Local measurements select
   candidates; physical iPad tests establish performance and crash acceptance.
2. Then remove controller-to-keyboard aliases and prevent resume presses from
   also triggering car actions. Retain native analog remapping and text entry.
3. Then map touch movement to the cursor, one-finger tap to click, two-finger tap
   to Escape and three-finger tap to Enter.
4. Finally investigate sharper HUD text, speedometer, map and starting lights.
   Original assets remain private and unchanged.
5. Continue scoped controller reconnect, audio, lifecycle, signing refresh and
   complete-race acceptance. Repeat only affected Apple build/package gates when
   relevant source changes justify another controlled standard-runner build.

Completion requires stable, playable original-game behavior on the physical
iPad with the controller and native audio/graphics. A host test or an unsigned
IPA alone is insufficient.
