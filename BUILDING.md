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
