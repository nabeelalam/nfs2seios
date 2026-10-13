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
