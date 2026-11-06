# Cloud build and publication policy

Reviewed against official GitHub documentation on 13 October 2026. This is the
operating plan for standard hosted machines, not a claim that the native iPad
game is finished. Standard-runner billing and the pinned ARM64 Xcode inventory
were rechecked against the official sources on 29 October 2026. The workflow in `.github/workflows/ipados.yml` accepts manual
dispatch only. Publishing source does not start cloud jobs. Every dispatch must
identify the revision and specific unresolved Apple SDK or packaging question.

## Verified limits and our bounds

| Resource | GitHub rule | Project policy |
| --- | --- | --- |
| Public standard runner minutes | Free and unlimited | Only manual runs against reviewed main or codex/hd-ui revisions |
| Private compute | Account-plan allowances are separate; the public-runner exemption does not apply | Workflow refuses private-repository execution |
| Larger runners | Always paid | Never selected |
| Concurrent standard jobs on Free | 20 total, including at most 5 macOS | One active workflow; Linux completes before macOS starts |
| Standard hosted job duration | At most 6 hours | Linux timeout 15 minutes; macOS timeout 30 minutes |
| Standard macOS Intel runner (current Apple build route) | 4 cores, 14 GB RAM, 14 GB SSD | macos-15-intel; ARM64 iOS cross-compilation, at most two compiler tasks and two ThinLTO backend jobs |
| Standard macOS ARM runner | 3 M1 cores, 7 GB RAM, 14 GB SSD | macos-15 remains a documented free alternative; not selected while its observed pool is capacity constrained |
| Public Ubuntu standard runner | 4 cores, 16 GB RAM, 14 GB SSD | ubuntu-24.04, at most two compiler tasks |
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

The named `codex/hd-ui` exception permits a separately audited experimental
device build without merging unfinished visuals into main. Both the Linux gate
and reusable Apple workflow require a public repository and one of these exact
two branches. Other branches and private execution remain excluded. Manual
dispatch, sequential standard machines, timeouts, privacy scans, package bounds,
no caches and no paid resources remain required for the experimental branch.

## Private personal release delivery

Release-asset rules were checked against [official GitHub documentation](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases)
on 30 October 2026: each asset must be below 2 GiB; GitHub documents no total
release-size or release-bandwidth limit. Releases are distinct from Actions
artifacts, Packages and Git LFS.

The public Actions artifact stays capped at 64 MiB and contains the unsigned
base without personal artwork. Authenticated artwork is appended locally and
can make the finished personal package larger than that public artifact. Use a
separate 512 MiB bound for a finished personal ZIP uploaded to the original
private repository's Release. Verify private visibility before and after upload,
verify the uploaded digest, and keep original game data and signing material
out. This does not raise an Actions quota, enable billing, permit private builds
or change the manual-upload requirement. Measure the final package before
upload; never enlarge this bound silently if the finished package exceeds it.

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
3. Manually dispatch one reviewed public main or codex/hd-ui revision. Record its SHA,
   target and expected evidence. No scheduled builds, automatic push builds,
   automatic PR builds or unattended rerun loops.
4. Linux runs publication gates before fetching dependencies or compiling. It
   then validates actual native linkage, startup failure handling, GLES pixels,
   SDL audio and controller reconnect. macOS waits for success.
5. macOS uses the previously successful Xcode 16.4/iOS 18.5 toolchain, explicitly
   selected by DEVELOPER_DIR. The official [macos-15 image inventory](https://github.com/actions/runner-images/blob/main/images/macos/macos-15-arm64-Readme.md)
   lists that installation. If it disappears, stop and review a replacement;
   do not silently switch compilers. Record Xcode and SDK versions in every run.
6. Compile the actual mapped engine and native game bundle. The older unmodified
   engine object audit is not repeated by default. The separate full-engine
   diagnostic app link also runs only when include_probe is selected: the normal
   game bundle links the same complete engine, platform and renderer object
   groups. Linux still executes the memory/link tests, and Apple-specific host
   checks remain required. The separate Probe is optional
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

## Build 22 resource review

The successful reviewed UI run 40941545707 used Linux for 300 seconds and Apple
for 984 seconds. Apple configuration took 567 seconds, engine compilation 90,
renderer/platform compilation 108, the separate diagnostic link 80 and the real
game app build 80. Its diagnostic step compiled one test entry and linked a
separate app; it did not execute those tests on iPad or contribute object files
to the game app. Routine future game builds omit that extra link while keeping
it available with the optional platform diagnostics. The actual game bundle
still compiles, links and passes ARM64/package checks. The next required Apple
build will measure the net saving; no extra run is justified solely to time
this workflow edit. Configuration remains the largest measured cost, and its
SDK checks have not been bypassed. See `evidence/build22-cloud-resource-review.json`.


## Current complete artwork delivery envelope

Reviewed against GitHub's official Releases documentation on 5 November 2026.
The eight private artwork bundles total 215,234,068 bytes. The former 128 MiB
personal-package bound cannot hold these inputs. The personal ZIP bound is now
512 MiB, matching the local injector's existing enforced limit. The injector
also bounds decompressed ZIP members to 1 GiB and the member count to 10,000.
The future finished package must be measured and verified against these bounds.

The reviewed delivery destination is the original private repository's Release.
GitHub documents a limit below 2 GiB per Release asset and no total release-size
or bandwidth limit. Public Actions artifacts retain their 64 MiB base-only bound.
Source and personal artwork use the existing separate publication paths.

Source: [GitHub Release storage and bandwidth quotas](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases#storage-and-bandwidth-quotas).
## 6 November: bounded unpublished base transfer

The prior Apple job of run41884649481 never started a step: GitHub reported that
no hosted runner acquired it and the macOS ARM pool was capacity constrained.
The Linux gate passed. The next grouped build therefore selects the documented
standard `macos-15-intel` runner, retaining Xcode16.4, explicit ARM64 device
output, two compiler jobs, two ThinLTO jobs and the existing timeouts. Host CPU
checks on Intel do not establish ARM runtime acceptance.

Current official references confirm this label is a free standard runner in a
public repository and its image includes the pinned Xcode16.4:
[runner specifications](https://docs.github.com/en/actions/reference/runners/github-hosted-runners),
[macOS15 Intel image](https://github.com/actions/runner-images/blob/main/images/macos/macos-15-Readme.md).

This repository currently has no Actions artifacts, but account-wide Packages
usage cannot be read with the existing login. Do not assume that proves shared
storage headroom or request more credentials. The optional `publish_base_draft`
route avoids Actions artifact upload entirely. It requires classic UI, Release,
the successful Linux gate, no diagnostic rebuild, no public-release request and
no artifact-upload request. The existing write-enabled release job alone creates
a separate UNPUBLISHED draft. Its checked ZIP contains only the unsigned ARM64
executable, app metadata and licenses; configuration/language files and all owned
artwork are excluded. The original public classic-release refusal remains the
default. No command in this route publishes the draft.

The base transfer remains below64MiB. The helper verifies the exact source,
unfinished-package flags, allowlisted members, privacy patterns, executable
identity, draft state before/after upload, server SHA256 and unchanged public
repository visibility. Only source and licenses go to the draft; personal
artwork is injected locally and delivered through the original private repository.
After verified local download, remove only this generated temporary draft.

GitHub documents draft-release listings as restricted to users with push access,
and release assets separately from Actions storage, with each asset below2GiB
and no total release storage/bandwidth limit:
[release API](https://docs.github.com/en/rest/releases/releases),
[release quotas](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases).
This is an opt-in transfer mechanism, not a public finished release or device
acceptance. It never enables billing, selects larger machines, increases quotas
or changes repository visibility.
