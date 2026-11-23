# Knockout emblem

The original TKO5/TKOD design is an abstract gold globe wrapped by a curved chequered racing flag. The authored replacement preserves the circular segments, negative-space arcs and lower angular tabs while reconstructing bevels and metallic lighting. It is a new reference-guided image, rather than an enlarged source bitmap.

The full-resolution transparent master is retained privately. `scripts/prepare_knockout_emblem_art.py` emits only authored image data and exact original-source hashes. It places the design inside its original 128-square canvas at offset (22, 0), with the original 85-square artwork extent. The normal and subdued variants each decode to a 512-square premultiplied RGBA texture. Their combined lossless deflate stream occupies 348,998 bytes; both decoded textures occupy 2 MiB. Source parts are individually below 1 MiB.

`SetupEmblemProvider` retains its original pack-provider precedence and four rules emblems, then supplies these two additional exact-source matches. Unknown source identities preserve the original rendering. Responsive geometry retains the original TKO canvas and signed pulse, while entrances and returns keep their independent source/texture proofs.

Actual local Setup Done / Main / Player One / Main replays verify the active authored emblem at both 708x492 and 800x450, including native upload/draw receipts and no post-startup native fallback. The disabled source has an archive-replayed identity and prepared pixels; it is not claimed game-observed. See `evidence/ui-knockout-emblem-remaster-local.json`. Local software rendering does not establish iPad behavior or performance.
