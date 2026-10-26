# Bonus car-value text artwork

Three complete fixed graphicsâ€”`Bomber BFS`, `Tombstone`, and `FZR 2000`â€”extend the accepted Haettenschweiler body style described in [Player text artwork](PLAYER_WORD_ART.md). They use the same logical 24 size, 0.88 horizontal factor and 0.5 logical stroke inset. The bounded source contains whole-word alpha runs, not font software, a reusable alphabet, or original game pixels.

The existing source font-cell identities and widths are compatibility metadata. Admission still requires the authenticated original font page, exact original glyph UV and screen geometry, and the complete ASCII draw-string context. Unsupported text preserves the original rendering. The new values do not change original game text, controls, hit areas, or timing.

The atlas retains its byte-identical 1024x2048 legacy prefix and every old integer layout row. A 72-row bottom extension stores only these three graphics and sampling guards: 1024x2120 RGBA8, 288 KiB additional pixels. The actual uploaded height determines normalized Y coordinates. A 2048-only GPU retains the legacy HD prefix and rejects the unavailable bonus rows safely. Below the legacy texture limit, existing original-font fallback remains available.

Run `python scripts/generate_frontend_pixels.py --check` and `python scripts/generate_frontend_pixels.py --layout-only --check` after changes. The generator authenticates the finite source and source-cell metadata, checks the old prefix/layout identities and the exact appended images, and keeps generated chunks below the existing publication and 96-part limits. Updating artwork, source geometry or the legacy prefix requires explicit review of the pinned identities and tests. Physical runtime and whole-frame visual acceptance remain separate checks.
