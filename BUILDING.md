# Building

Read [CI_POLICY.md](CI_POLICY.md) before requesting a cloud run. The workflow
uses standard Ubuntu and macOS runners only, with manual dispatch, sequential
validation and bounded timeouts. No automatic push/PR builds or caches are enabled.
The reviewed workflow is `.github/workflows/ipados.yml`. It runs only on manual
dispatch for a specific validation question after local checks pass. Source
publication does not trigger builds. See CI_POLICY.md before dispatch.

## Source preparation

```sh
python3 scripts/prepare_full_mapped.py
python3 scripts/prepare_renderer.py
```

These scripts fetch and verify pinned source revisions and prepare disposable
generated source. They do not fetch the separately supplied main game assets.
Do not commit generated source, dependency checkouts or local tools.

## Host checks

On a supported host with the required compiler and platform libraries:

```sh
cmake -S . -B .build/host -DCMAKE_BUILD_TYPE=Debug -DBOUNDARY_SANITIZERS=ON
cmake --build .build/host --target core_tests mapped_engine_tests -j 3
ctest --test-dir .build/host --output-on-failure
```

Build the remaining test targets before requesting the full host suite. Linux
GLES/SDL checks additionally require the dependencies installed by the workflow
and NFS_RENDERER_PIXELS=ON. Original-data tests use separately supplied assets
in an ignored owned copy. Never upload the files or their private captures.

## Apple build

Dispatch the reviewed workflow for a specific public main-branch commit. Linux
privacy and native validation gates precede the optional macOS build. Select
build_ios deliberately; rebuild the separate Probe only when a platform change
requires it. Xcode 16.4 is explicitly selected on macos-15 and versions are logged.
The workflow compiles the original mapped engine, links native platform services,
and validates unsigned ARM64 iPad bundles. No signing credentials are used.

IPA uploads default to off. Before selecting publish_ipa, verify account-wide
storage headroom. The payload is bounded and expires after one day. Ordinary
workflow logs provide build diagnostics without redundant stored log artifacts.
Apple device validation has passed for the renderer optimization; physical performance remains unverified.

## Publication checks

Install the digest-pinned independent scanner with
`python scripts/setup_secret_scanner.py` on supported Windows/x64 Linux hosts.
Use `python scripts/publication_gate.py --mode staged` before a commit and
`python scripts/publication_gate.py --mode history` before a push. Configure
repository-local Git hooks with `git config --local core.hooksPath .githooks`;
on Unix, make both hook files executable. Use a GitHub no-reply commit identity.
Checks supplement manual review of the exact diff, history and package inputs.

Select `configuration=Release` for a deliberate optimized device-test build,
keeping `include_probe=false` unless the diagnostic changed. Enable `publish_ipa`
only after checking storage headroom; the aggregate upload limit is 64 MiB and
retention is one day. Debug remains available for targeted compiler diagnostics.
The package manifest records the selected configuration.

ThinLTO is an opt-in Release compiler experiment. Select `thin_lto=true` in the
manual workflow, or configure locally with `-DNFS_THIN_LTO=ON
-DNFS_THIN_LTO_JOBS=2` and a Release build. The selected game targets use ThinLTO;
SDL keeps its existing settings. Compiler/linker probes run before the full
build, and the workflow limits compile jobs and linker backend workers to two.
Debug and the default workflow configuration retain their existing compiler
settings. The optional Apple host gate also checks 40,000 mixed integer/FPU
states with ThinLTO enabled. Build 9 passed the Apple SDK build and independent downloaded-package checks.
Physical split-screen FPS and race-stability validation remain required.


## Executable-only development downloads

Select publish_release for a deliberate prerelease download instead of an
expiring Actions artifact. This forces Release and excludes the separate Probe
and Actions artifact uploads. The checked ZIP contains an unsigned IPA and its
manifest; the IPA contains only the executable, app metadata and license notices.
No original data, startup configuration or language files are distributed.
Ordinary SDK validation remains read-only; only the explicit release caller can
publish. No build runs automatically when source is pushed.

Build3 is an update for an existing imported Documents/game installation. Install
over the existing app without deleting it. Its startup files must remain in that
app-owned folder. The newer source importer additionally accepts separately
supplied install.win and text.eng in the selected root beside GAMEDATA/FEDATA,
with optional text.* language files. Those files must be compatible with the
translated engine; do not assume an original CD root contains them. This importer
path has host validation only and is not included in build3 or device-validated.


The production engine link check includes 45 span-routing and lifetime cases.
These exercise EngineMemory itself, including adjacent-allocation rejection,
unaligned access, free/reallocation invalidation and DATA/BSS boundaries. The
mapped-engine unit fixture uses a substitute resolver and cannot replace this
gate. A successful fixed-heap ownership check now returns the existing backing
pointer directly; invalid spans retain their original rejection paths. This
source optimization is included in the independently inspected build 10,
which passed the Apple SDK build and still requires physical split-screen validation.


The heap also tracks conservative 4096-byte logical chunks, independently of
operating-system page size. A flag is true only when an entire chunk belongs to
one live allocation. Single-chunk requests can use that proof directly; partial
chunks and cross-chunk requests retain the existing ownership checks. Freeing
an allocation clears its flags before reuse. Initialization validates the mapping
before metadata allocation and rolls the new mapping back on allocation failure.
The metadata costs 32 KiB for a 128 MiB heap. Bounds, cancellation and guest-copy
serialization remain enabled. Added production cases cover complete chunks,
cross-chunk spans, frees and smaller allocations reusing the same addresses.
The actual C++14 ThinLTO engine and shared runtime pass the production link
suite. Separate address/undefined sanitizer checks cover initialization rejection
and allocation-failure rollback, threaded lifetimes and 12,320 independently
checked ownership cases. Build 10 passes actual Apple compilation and independent
downloaded-package checks. Physical acceptance remains pending.


The next input candidate retains build 10's accepted split-screen rendering
configuration. Controller buttons use native joystick state and frontend input,
without also sending keyboard aliases. Short menu presses and control-recording
presses have separate bounded ownership; closing a menu consumes held buttons
until release so a resume press cannot also shift. Both controller profiles and
analog remapping remain supported. Player one's Back/View button resets the car.

A small keyboard icon below the performance overlay shows or hides the system
keyboard. Use it for names or Return when opening a control-recording prompt.
It stays available for explicit dismissal; automatic field detection is deferred.
Touch works as a relative trackpad: move one finger to move the original cursor,
tap one finger to click, two fingers for Escape, and three fingers for Enter.
Chords commit only after the last finger lifts. Long presses, pans and cancelled
gestures do not also click. This adds menu input, not touch racing buttons.

Local Clang C++14 address/undefined sanitizer checks pass 34 tests. Actual SDL
finger-event injection moves the original cursor and exercises all three tap
actions in the original menus. A two-player original race regression exercises
manual shifts, held/fresh resume, reset and injected background/foreground
recovery, with clean shutdown. These are Linux functional checks, not physical
iPad touch/keyboard acceptance or new iPad FPS or energy measurements. The user
accepted build 10 at split-screen 60 FPS; this input revision needs its own
physical regression. Reported battery drain still needs controlled measurement.
