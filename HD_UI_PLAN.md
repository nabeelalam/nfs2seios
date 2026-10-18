# HD interface and texture work

Development branch: `codex/hd-ui`. Integration branch: `main`.

This branch starts from the accepted build 18 source and documentation on main.
It is an independent workstream. Performance, battery work and session recording
continue on main; no unfinished visual changes are required there. This document
establishes the work, not a claim that replacement artwork is implemented.

## Scope and order

1. Inspect the existing HUD and frontend rendering locally. Identify which paths
   draw speedometer, lap information, map, record text and starting lights. Keep
   source texture resolution distinct from final framebuffer resolution: raising
   the output resolution alone does not add detail to low-resolution artwork.
2. Make a small optional replacement proof for one HUD element. Preserve its
   layout, transparency, palette behavior, animation and split-screen placement.
   Decide the replacement boundary from the inspected draw path before changing
   shared renderer interfaces. Do not start with a broad renderer rewrite.
3. Compare original and replacement output in the Linux pixel harness and an
   actual original-data race. Check both players, multiple output sizes, menus,
   track transitions and context recreation. Keep originals untouched.
4. Expand to the remaining HUD and text only after that proof works. Preserve
   existing timing, game rules, input and accessibility controls. Start lights
   must retain the original sequence and timing.
5. Request one Apple/device test when a meaningful visual milestone passes host
   validation. Measure frame time and memory at native resolution in a busy
   split-screen race before proposing integration into main.

## Keep main easy to integrate

- Keep visual helpers, tests and documentation in dedicated files where possible.
  Make necessary existing-renderer hooks small and explicit. Do not duplicate the
  engine, build system or performance fixes into a separate implementation.
- Keep the current main version of diagnostics, recording, frame pacing, memory
  ownership and input behavior. Resolve conflicts around these contracts rather
  than taking one entire file or assuming the branch version is newer.
- Keep feature commits separate from maintenance merges. Regularly merge
  `origin/main` into this shared branch after main advances. This preserves
  published history. Rebase is an option for unpublished local commits; do not
  force-push the shared branch as routine maintenance.
- After an integration, inspect the diff against main, run relevant existing
  host tests plus the visual checks, and verify that no stale copied performance
  code or unrelated changes remain. Record failures rather than hiding them.
- Keep replacement loading optional with a complete original-art fallback.
  Missing or invalid replacements must preserve the working game. Do not enable
  unfinished visual changes on main. Merge a reviewed feature only after visual,
  performance and actual-device acceptance.

Normal integration procedure, from this branch's separate checkout:

```sh
git fetch origin
git merge origin/main
git diff origin/main...HEAD
```

Before merging, require a clean working tree or preserve unfinished changes in
scoped local commits. Do not reset, clean or overwrite active work. A branch
reduces interference; it cannot guarantee that future edits never conflict.

## Privacy and build boundaries

Original assets, extracted images, game profiles, captures and local diagnostic
logs stay in ignored local storage. Do not publish game data, derivative texture
packs without a separate rights review, private paths or device information.
Original-data tests use copies; original game folders remain unchanged. All
published commits must pass the existing policy and independent secret scans.

Keep existing manual-only CI behavior. Creating, updating or publishing this
branch does not justify a cloud build. Use local host and pixel checks first;
an Apple build must answer a specific unresolved SDK or device question. The
current main-only cloud policy remains unchanged until a branch device test is
actually needed and reviewed.

## Acceptance

- Sharper intended interface elements, with unchanged game behavior and usable
  layout in single-player and split-screen at supported resolutions.
- Correct transparent edges, palette changes and start-light animation; no
  stale replacements after a track or graphics-context change.
- Original-art fallback verified with absent and malformed replacements.
- No meaningful frame-time regression, unbounded resource growth or continuing
  background work. Host measurements do not establish iPad power consumption.
- Physical iPad screenshots and play test before calling the work finalized.

## Current status

Branch and isolated working copy established. Source inspection and one-element
proof are next. No HD renderer, artwork pack, FPS improvement or device behavior
is claimed by this setup commit.
