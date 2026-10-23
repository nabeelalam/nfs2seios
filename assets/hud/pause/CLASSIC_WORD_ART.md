# Fixed classic pause word artwork

The six English pause words are authored whole-word RGBA graphics in two bounded JSON sources. They use the same condensed letter family as the reviewed menu direction, with a silver face, a dark rim, directional bevel highlights and a short shadow. The historical SVGs remain as layout and baseline references. No font file, reusable alphabet or original game pixels are included in these sources.

The original six atlas allocations, source crops, UV selectors, output quads, tint and interaction rules are preserved. Generation authenticates the complete original catalog and both finite word sources before replacing pixels. The pause region remains 704 by 384 inside the existing 1024-square HUD texture. The richer material uses ten run-aligned source parts; each stays below the existing one MiB publication limit. This adds no per-frame material shader work.

Run `python scripts/generate_remaster_pixels.py --check` to verify maintained generated output. The registered `pause_font_runtime_tests`, `hd_pause_word_art_source` and `hd_pause_pixel_chunks` checks cover exact output, bounded decode, unchanged neighbouring pixels, source identity refusal, UV selection and chunk inventory. A local race pause/resume check and physical iPad appearance are separate acceptance steps; source and decoder checks alone do not prove them.
