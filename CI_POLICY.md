# Cloud build and publication policy

Reviewed against official GitHub documentation on 13 October 2026. This is the
operating plan for standard hosted machines, not a claim that the native iPad
game is finished. The workflow in `.github/workflows/ipados.yml` accepts manual
dispatch only. Publishing source does not start cloud jobs. Every dispatch must
identify the revision and specific unresolved Apple SDK or packaging question.

## Verified limits and our bounds

| Resource | GitHub rule | Project policy |
| --- | --- | --- |
| Public standard runner minutes | Free and unlimited | Only manual runs against reviewed main-branch revisions |
| Private minutes | GitHub Free includes 2,000 monthly minutes | Workflow refuses private-repository execution |
| Larger runners | Always paid | Never selected |
| Concurrent standard jobs on Free | 20 total, including at most 5 macOS | One active workflow; Linux completes before macOS starts |
| Standard hosted job duration | At most 6 hours | Linux timeout 15 minutes; macOS timeout 30 minutes |
| Standard macOS ARM runner | 3 M1 cores, 7 GB RAM, 14 GB SSD | macos-15, three compiler tasks; measure before changing parallelism |
| Public Ubuntu standard runner | 4 cores, 16 GB RAM, 14 GB SSD | ubuntu-24.04, three compiler tasks |
| Free artifact allowance | 500 MB, shared with Packages | No automatic uploads; optional IPA payload capped at 64 MiB per run |
| Public artifact retention | Configurable between 1 and 90 days | Optional IPA artifacts expire after one day |
| Cache allowance | Default 10 GB per repository; old entries expire after seven days without access | No caches enabled; never increase the allowance |
| REST API requests | Authenticated user 5,000/hour; workflow token 1,000/hour per repository | Bounded inventory requests; no rapid polling or retry loops |

Sources: [runner specifications](https://docs.github.com/en/actions/reference/runners/github-hosted-runners),
[Actions limits](https://docs.github.com/en/actions/reference/limits),
[billing and storage](https://docs.github.com/en/billing/concepts/product-billing/github-actions),
[cache rules](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching),
[artifact retention](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/enabling-features-for-your-repository/managing-github-actions-settings-for-a-repository).

The artifact limit is account-wide and shares space with Packages. A per-run size
check cannot prove account-wide headroom. Runner disk is also checked before
compilation, requiring at least 6 GiB free, and the Apple engine compile records
elapsed time and memory statistics to inform the next resource decision. A size
check does not reserve storage. Before selecting publish_ipa, review
current account usage and existing artifacts, including other repositories. If
that cannot be verified, build without uploading. Do not enable billing, increase
quotas or delete existing user artifacts to bypass a limit. One-day retention is
not an immediate cleanup or a refund for previously accrued storage.

## Resource decisions

Prior standard-runner builds demonstrated that this workload can compile on
standard machines. Configuration dominated the measured Apple build; pinned
source downloads were brief. That does not establish current-revision success.
Measure the first reviewed build before adding caches or changing parallelism.
Do not cache owned assets, signing material, complete workspaces or private logs.

## Measured current-revision validation

The first public validation run of revision
`2b8a4a6fd3b751f93781a5178c4088fa490c7e03` passed on standard runners:
Linux 147 seconds, macOS 463 seconds. Apple SDK configuration took 366 seconds;
source preparation took 6 seconds, host checks 44 seconds, and the separate
engine/renderer/link/app steps 7/18/4/4 seconds. Configuration remains the
principal measured cost. The timed engine build reported 136,167,424 bytes of
maximum resident memory for that command, which is not a measurement of total
job peak memory. No Probe rebuild, caches or IPA artifact uploads were used.

Review configuration or generator changes before the next required Apple build;
do not assume they will preserve device bundle behavior. Do not dispatch solely
to repeat this successful Debug validation. An optimized device-test package is
a distinct next validation target, after storage allowance is confirmed.

## Build sequence

1. Validate locally with the existing host checks and relevant original-data WSL
   tests. Original assets remain local and ignored. Decide the precise Apple
   question to resolve: SDK compilation, bundle validation or a specific device
   test. WSL success alone does not prove iPad behavior.
2. Audit source, prospective commit metadata and all publishable history. The
   independent Gitleaks scanner and project publication policy must both pass.
3. Manually dispatch one reviewed public main-branch revision. Record its SHA,
   target and expected evidence. No scheduled builds, automatic push builds,
   automatic PR builds or unattended rerun loops.
4. Linux runs publication gates before fetching dependencies or compiling. It
   then validates actual native linkage, startup failure handling, GLES pixels,
   SDL audio and controller reconnect. macOS waits for success.
5. macOS uses the previously successful Xcode 16.4/iOS 18.5 toolchain, explicitly
   selected by DEVELOPER_DIR. The official [macos-15 image inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-15-Readme.md)
   lists that installation. If it disappears, stop and review a replacement;
   do not silently switch compilers. Record Xcode and SDK versions in every run.
6. Compile the actual mapped engine and native game bundle. The older unmodified
   engine object audit is not repeated by default. The separate Probe is optional
   because its physical platform checks already passed; rebuild it only for a
   relevant platform change. Preserve macOS host checks because platform behavior
   differs from Linux even when the shared code passes there.
7. Package and validate unsigned ARM64 bundles with separately supplied assets.
   Logs remain in normal workflow output instead of redundant stored artifacts.
   Upload IPAs only when explicitly requested and storage headroom is verified.
8. Inspect a failure before spending another run. Repeat only after a source fix,
   an identified transient service failure, or a deliberately changed diagnostic.
   Never rerun an unchanged deterministic failure hoping it resolves itself.

The workflow uses read-only repository permissions, disables persisted checkout
credentials and pins actions to verified commits from their official repositories.
Fork code is not executed by privileged workflows. Sources remain pinned to
reviewed upstream revisions. See [GitHub's security reference](https://docs.github.com/en/actions/reference/security/secure-use).

## Publication boundary

This repository starts from audited source with fresh Git history. Private
development history, operational evidence, captures and personal filesystem
information are excluded. Preserve separately held private work; do not merge
private Git history into this repository.

Use GitHub no-reply commit identities. Both the project publication policy and
independent secret scanner must pass before commits/pushes, and CI repeats the
checks. Configure the repository-local hooks as described in BUILDING.md. Hooks
can be bypassed by clients, so review the exact diff and history too.

Never stage or upload owned game assets, the original EXE, local imports,
captures, raw logs, credentials, signing material or private audit reports.
Forced staging does not bypass the policy check. Findings are reported by
category without printing sensitive matched values.

These checks reduce risk; they do not prove arbitrary future content is safe.
Review source, Git metadata, package inputs and upload storage before publication.

## Latest targeted frame-scheduling validation

A failed Linux startup gate prevented the Apple job from starting. After a
locally validated fixture correction, the targeted optimized build completed
in run 38463370000. The Apple job took 558 seconds: configuration 418 seconds,
host checks 48 seconds, engine compilation 21 seconds and renderer/platform
compilation 35 seconds. Pinned source fetch took five seconds. Its sole IPA
artifact was 1,998,130 bytes with one-day retention; the Probe and caches were
not rebuilt. These figures describe this run, not a guaranteed future budget.

Configuration remains the main cost. [CMake documents alternative generators
for iOS](https://cmake.org/cmake/help/v3.29/manual/cmake-toolchains.7.html#cross-compiling-for-ios-tvos-visionos-or-watchos),
but recommends Xcode; alternatives require explicit CPU and signing handling.
Retain the verified generator for this device test. Evaluate a faster generator
only alongside a necessary future Apple validation, preserving ARM64/device
SDK checks, unsigned bundle resources and package-path verification. No cache
is justified by a five-second fetch.


## Earlier measured ownership optimization validation (build 10)

Source 22f8bcb026669c891e87dc7010bdc9c7a0d0a9bb used one deliberate Release
ThinLTO run 38941504663. Standard Ubuntu finished in 145 seconds before the
standard macOS job began; Apple finished in 449 seconds. Apple compile jobs
and ThinLTO backends remained bounded at two. The Probe was skipped and the
Actions artifacts API confirms zero artifacts. The checked prerelease ZIP is
3,098,903 bytes and contains the separately inspected asset-free version 10 IPA.
No duplicate run, larger runner, cache expansion or billing change was used.
Package/compiler success does not establish physical-device performance.


## Latest measured native validation: build 14

Reviewed source3ade3b3 used one manual Release/ThinLTO dispatch38995915505.
Standard Ubuntu completed in169seconds before standard macOS began; Apple
completed in520seconds. Device links retain two ThinLTO backends. The new
pacing tests executed on Linux; Apple compiler/package gates passed. No Probe,
Actions artifact upload,cache,billing change,larger runner or duplicate run.
The independently downloaded,asset-free version14 ZIP is3,832,064bytes.
This validates compilation and packaging; physical FPS and energy remain open.


## Build15 measurement validation

One manual Release/ThinLTO dispatch39005977672 at reviewed sourceafd2bbe.
Sequential standard Ubuntu164s then standard macOS
669s. No Probe,Actions artifacts,caches,paid runner or billing
change. Apple host30tests,including actual thread CPU provider,passed before
device compilation. Existing compiler/link worker bounds remain. Native package
inspection passed; physical Low Power Mode FPS and energy measurements remain open.


## Build 16 sound-operation validation

One manual Release/ThinLTO dispatch 39075795713 at reviewed source ce9d6de.
Sequential standard Ubuntu 148 seconds then standard macOS
632 seconds. No Probe, Actions artifacts, caches, paid runner
or billing change. Existing compiler/backend bounds retained. Source, download
and package privacy inspected; physical performance/energy remain unverified.

## Build 17 checked-copy validation

One manual dispatch39093193712 at source1b9a561. Standard sequential Ubuntu
135seconds,then standard macOS449seconds.
No Probe,Actions artifacts,caches,paid runner,billing change or duplicate build.
All source/staged/history/metadata/independent secret checks passed;package
privacy and source/digests inspected. Physical energy/FPS remain unverified.
