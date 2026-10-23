# Fixed classic pause word artwork

The six English pause words are authored whole-word RGBA graphics in two bounded JSON sources. They use the same condensed letter family as the reviewed menu direction, with a silver face, a dark rim, directional bevel highlights and a short shadow. The historical SVGs remain as layout and baseline references. No font file, reusable alphabet or original game pixels are included in these sources.

The original six atlas allocations, source crops, UV selectors, output quads, tint and interaction rules are preserved. Generation authenticates the complete original catalog and both finite word sources before replacing pixels. The pause region remains 704 by 384 inside the existing 1024-square HUD texture. The richer material uses ten run-aligned source parts; each stays below the existing one MiB publication limit. This adds no per-frame material shader work.

Run `python scripts/generate_remaster_pixels.py --check` to verify maintained generated output. The registered `pause_font_runtime_tests`, `hd_pause_word_art_source` and `hd_pause_pixel_chunks` checks cover exact output, bounded decode, unchanged neighbouring pixels, source identity refusal, UV selection and chunk inventory. A local race pause/resume check and physical iPad appearance are separate acceptance steps; source and decoder checks alone do not prove them.


## Classic pause header material

The PAUSE title keeps its broad squared lettering on a yellow plate, and the header keeps its repeated yellow/charcoal caution stripes. The new vectors use the pinned OFL Michroma family and authored bevels and gradients. They do not resample original texture pixels. The six accepted body-word images and their allocations remain unchanged; original purple fill, borders, volume sprites, clipping, positions and tint remain in place.

`python scripts/generate_pause_header_pixels.py` produces the bounded `PauseHeaderPixels.inc` and its source/raster receipt. `--check` verifies deterministic regeneration. The title occupies `[240,398..760,510]` and the caution tile `[768,398..920,510]` in the existing 1024-square auxiliary texture. Numeric filtering rows 396–397 and neighbouring letter rows 510–511 remain untouched. Both complete streams are validated before either rectangle is written.

Authenticated original pause source 40 and complete original UV triangles select tags 73 and 74. The title retains its original UV range extending past the source atlas edge. The fragment shader maps those original coordinates to the new rectangles and clamps to their texel centres. It reuses the original triangles, draw order, tint and cached texture; it adds no texture allocation, draw pass or per-frame upload. Tag 72 remains frontend text.

Registered checks cover independent full-plane decoding, malformed second-stream refusal without partial writes, canaries, all four filtering gutter rows, source/font authenticity, complete-triangle selection and actual GLES sampling. The GLES fixture also checks six existing body labels, numeric/frontend behaviour and a neighbouring small-letter edge against an untouched atlas. Actual-game appearance and physical iPad performance remain separate acceptance steps. Original volume-bar caps are still pending their own material remaster.
