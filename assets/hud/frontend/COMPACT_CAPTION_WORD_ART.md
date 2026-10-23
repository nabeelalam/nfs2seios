# Fixed compact menu caption artwork

Three complete lowercase word images cover `game setup`, `location` and `options`. Their logical boxes retain the original measured widths 75, 52 and 48, right edge 168 and respective top coordinates 126, 162 and 364. Artwork uses the same reviewed condensed letter family as the Player captions, with lighter strokes and the original 24-unit baseline in a 32-unit-high box. Raster scale is four. All edges retain transparent padding.

The JSON contains finite alpha runs and exact RGBA content pins. It includes no font file, reusable alphabet or original game pixels. The generator authenticates the entire selected source before validating the inventory, typed geometry, run bounds, decoded hashes and padding. It validates all outputs before writing. The immutable C++ decoder independently checks the same bounds and full image identities.

`python scripts/generate_compact_caption_words.py --check` verifies the two generated headers. Registered source, generation and C++ artwork tests exercise malformed sources, runs, identity, copied ownership and retained image lifetime.

These images are prepared artwork, not activated scene replacements. Setup and Options now have separate ordinary-route lexical source proofs, while Location still needs an equivalent sealed source observation. Each future display must retain and join the genuine current source proof with its independently validated scene, copy/upload and renderer ownership. Sharing the word `options` with Audio or Graphics never shares another form's authority. The existing font provider and actual displayed caption remain unchanged by this preparatory milestone.
