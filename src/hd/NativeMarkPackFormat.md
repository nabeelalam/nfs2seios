# Optional native menu artwork

The existing artwork pack remains the single authenticated source. Version 1 is unchanged: exactly 24 sections, no native-mark provider. Version 2 uses magic `NFSHDPK2`, version 2, and exactly `25 + M` sections with `1 <= M <= 8`. The total size remains capped at 96 MiB. The caller supplies the trusted whole-file SHA-256 before parsing; internal hashes do not establish trust.

The first 24 section payloads retain their version 1 schemas. The section table is rebuilt for the larger count. All sections are contiguous, ordered, bounded, and consume the complete file; reserved fields must be zero.

Section 24 has tag 6, role 0. Its 16-byte little-endian header contains schema 1, binding count (1–64), record size 64, and zero. Each record contains eight 32-bit fields: schema 1, source encoding 1 (RGBA5551 little-endian), source edge 128, source bytes 32768, authored texture role, logical width 128, logical height 128, and zero. Its final 32 bytes are the nonzero SHA-256 of the complete prepared source canvas. Keys must be unique; several keys may refer to one authored texture.

The final M sections have tag 7 and consecutive roles beginning at zero. Each contains a 112-byte descriptor followed by exactly 1,048,576 RGBA8 bytes. Descriptor fields match the existing texture descriptor layout: width and height 512, schema 1, role, premultiplied 1, opacity-baked 0, opaque 0, reserved zero, nonzero unique 64-bit artwork identity, payload byte count, payload SHA-256, and 32 zero bytes. Every RGB component must be no greater than alpha. The pixels describe the complete logical 128-square canvas; rendering retains the original UVs, geometry, color and blending.

`ArtworkPack::nativeMarkProvider()` returns an empty descriptor for version 1. Version 2 exposes immutable borrowed artwork from the same retained pack. The independent native-mark provider ABI is copied at startup and sealed by its first getter. Missing, invalid and unknown lookups leave the caller's output unchanged. After startup, lookup is read-only and supports concurrent callers. Failure to install optional native marks leaves the valid background provider active.

Original-derived keys and original or authored pixel payloads are external data, not constants in public source. Synthetic tests cover the format and provider behavior. Renderer and physical-device acceptance are separate requirements.

## Mixed cursor and emblem packs

Version 3 uses magic `NFSHDPK3` and version 3 with the same section layout, authentication and reserved-field rules. It permits 1–40 native textures (26–65 sections total) and a whole-file size of at most 128 MiB. Version 1 and version 2 retain their 96 MiB cap; version 2 retains its 1–8 texture cap. The file loader checks the exact magic/version before allocating the larger buffer. Binding count remains capped at 64. Renderer resources are created lazily and reused immutably within a context; 40 full-size emblem textures use at most 40 MiB of pixel storage before driver overhead. Exhausted resources keep original artwork; there is no eviction or change to drawing behavior. Version 2 continues to accept only its original emblem profile. Version 3 accepts exactly these source/art pairs:

| Encoding | Prepared square edge | Prepared bytes | Logical square edge | Authored RGBA8 square edge | Authored bytes |
| --- | --- | --- | --- | --- | --- |
| 1, RGBA5551 little-endian | 128 | 32768 | 128 | 512 | 1048576 |
| 2, RGBA4444 little-endian | 32 | 2048 | 32 | 128 | 65536 |

Each binding must reference artwork of its own profile. Binding uniqueness and lookup use the complete tuple of encoding, source edge and prepared-source SHA-256; the same opaque digest in different profiles cannot alias artwork. Texture identities remain unique across the complete pack. All payload hashes, premultiplied-alpha constraints and exact byte counts remain mandatory.

The renderer recognizes the complete converted source upload. The cursor additionally requires square aspect, TMU 0 and matching LOD 3 on selection. Replacement retains the original draw geometry, UV endpoints, clipping, tint and blending, including the original cursor hotspot. Missing providers, unknown content, invalid profiles and unavailable resources retain original artwork. Host tests do not establish physical iPad acceptance.
