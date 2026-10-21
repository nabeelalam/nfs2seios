# Optional native menu artwork

The existing artwork pack remains the single authenticated source. Version 1 is unchanged: exactly 24 sections, no native-mark provider. Version 2 uses magic `NFSHDPK2`, version 2, and exactly `25 + M` sections with `1 <= M <= 8`. The total size remains capped at 96 MiB. The caller supplies the trusted whole-file SHA-256 before parsing; internal hashes do not establish trust.

The first 24 section payloads retain their version 1 schemas. The section table is rebuilt for the larger count. All sections are contiguous, ordered, bounded, and consume the complete file; reserved fields must be zero.

Section 24 has tag 6, role 0. Its 16-byte little-endian header contains schema 1, binding count (1–64), record size 64, and zero. Each record contains eight 32-bit fields: schema 1, source encoding 1 (RGBA5551 little-endian), source edge 128, source bytes 32768, authored texture role, logical width 128, logical height 128, and zero. Its final 32 bytes are the nonzero SHA-256 of the complete prepared source canvas. Keys must be unique; several keys may refer to one authored texture.

The final M sections have tag 7 and consecutive roles beginning at zero. Each contains a 112-byte descriptor followed by exactly 1,048,576 RGBA8 bytes. Descriptor fields match the existing texture descriptor layout: width and height 512, schema 1, role, premultiplied 1, opacity-baked 0, opaque 0, reserved zero, nonzero unique 64-bit artwork identity, payload byte count, payload SHA-256, and 32 zero bytes. Every RGB component must be no greater than alpha. The pixels describe the complete logical 128-square canvas; rendering retains the original UVs, geometry, color and blending.

`ArtworkPack::nativeMarkProvider()` returns an empty descriptor for version 1. Version 2 exposes immutable borrowed artwork from the same retained pack. The independent native-mark provider ABI is copied at startup and sealed by its first getter. Missing, invalid and unknown lookups leave the caller's output unchanged. After startup, lookup is read-only and supports concurrent callers. Failure to install optional native marks leaves the valid background provider active.

Original-derived keys and original or authored pixel payloads are external data, not constants in public source. Synthetic tests cover the format and provider behavior. Renderer and physical-device acceptance are separate requirements.
