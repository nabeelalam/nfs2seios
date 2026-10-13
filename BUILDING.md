# Building

Read [CI_POLICY.md](CI_POLICY.md) before requesting a cloud run. The workflow
uses standard Ubuntu and macOS runners only, with manual dispatch, sequential
validation and bounded timeouts. No automatic push/PR builds or caches are enabled.
The reviewed template is `ci/ipados.yml`; it is currently inactive because the
publishing login lacks workflow-write permission. Activate it only after that
permission is granted and a specific Apple build is needed.

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
The current public workflow has not been run yet.

## Publication checks

Install the digest-pinned independent scanner with
`python scripts/setup_secret_scanner.py` on supported Windows/x64 Linux hosts.
Use `python scripts/publication_gate.py --mode staged` before a commit and
`python scripts/publication_gate.py --mode history` before a push. Configure
repository-local Git hooks with `git config --local core.hooksPath .githooks`;
on Unix, make both hook files executable. Use a GitHub no-reply commit identity.
Checks supplement manual review of the exact diff, history and package inputs.
