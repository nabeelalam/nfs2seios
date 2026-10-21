# Optional artwork pack format, version 1

This is a byte-oriented little-endian container, separate from original game
assets and source publication. An application must supply a trusted expected
SHA-256 of the **entire file**. The loader verifies that pin before interpreting
metadata. A digest stored in the same file is not a trusted expected pin.
No production content pin or original asset identity catalogue is embedded here.

The file is at most 96 MiB. All 24 sections occur exactly once in the order below,
are tightly contiguous, and consume the entire file. There are no extension
sections, padding bytes, trailing data or implicit native C/C++ structure layouts.
All unlisted reserved bytes must be zero. Integer ranges are checked before
arithmetic or indexing. Signed fields use explicit 32-bit two's-complement values.

## Header and table

The 64-byte header contains:

| Offset | Value |
| --- | --- |
| 0 | Eight ASCII bytes `NFSHDPK1` |
| 8 | u32 version, 1 |
| 12 | u32 header bytes, 64 |
| 16 | u64 total file bytes |
| 24 | u32 section count, 24 |
| 28 | u32 table entry bytes, 32 |
| 32 | u64 table offset, 64 |
| 40 | 24 reserved zero bytes |

The table immediately follows. Each 32-byte entry contains u32 tag, u32 role,
u64 section offset, u64 section bytes and u64 reserved zero. Payload begins at
832. Entries and payloads use this exact order:

1. Tag 1, role 0: source catalogue.
2. Tag 2, roles 0 through 4: five background texture descriptors and pixels.
3. Tag 3, role 0: small-logo texture descriptor and pixels.
4. Tag 4, roles 1 through 12: word descriptors.
5. Tag 5, roles 0 through 4: small-logo region descriptors.

## Source catalogue

This section is 1616 bytes: four u32 values (`0x31544143`, 1, 20, 80), followed by
20 records of 80 bytes. Each record contains:

| Offset | Field |
| --- | --- |
| 0 | u32 kind |
| 4 | u32 role |
| 8 | u32 source span |
| 12 | u32 width |
| 16 | u32 height |
| 20 | u32 palette offset |
| 24 | u32 palette count |
| 28 | u32 maximum index |
| 32 | 16 source header bytes |
| 48 | 32 source SHA-256 bytes |

The first 13 records are kind 1, roles 0 through 12. Palette count and maximum
index are reserved zero for these records. The next five are kind 2, roles 0
through 4. Their header words are decoded little-endian. The last two are kind 3,
map roles 0 and 1: bytes 8 through 47 must be zero. The catalogue must satisfy
`validCatalog`, including complete role assignments, dimensions, span and palette
bounds, distinct source digests, and distinct nonzero map digests.

## Texture descriptors

Each texture section begins with 112 bytes, then tightly packed RGBA8 pixels:

| Offset | Field |
| --- | --- |
| 0 | u32 width |
| 4 | u32 height |
| 8 | u32 schema, 1 |
| 12 | u32 role (background role, or 0 for logo) |
| 16 | u32 premultiplied, 1 |
| 20 | u32 opacity baked, 0 |
| 24 | u32 opaque flag |
| 28 | u32 reserved zero |
| 32 | u64 nonzero artwork identity |
| 40 | u64 pixel bytes, width times height times 4 |
| 48 | 32 SHA-256 bytes of the RGBA payload |
| 80 | 32 original source SHA-256 bytes |
| 112 | RGBA payload |

Dimensions must be nonzero and at most 4096. Every RGB channel must be no greater
than its alpha. Background roles 0 and 1 must be opaque with alpha 255 and source
digests matching their catalogue map. Role 2 is nonopaque and uses scene identity
0. Roles 3 and 4 are nonopaque aggregate word atlases and use a zero source digest;
individual word descriptors supply their source identities. The small-logo atlas
is nonopaque with a zero source digest. Payload hashes are checked independently.

## Word descriptors

Each section is 112 bytes: ten u32 values (schema, source role, texture role, x, y,
width, height, scale, logical width, logical height), followed by a 5-byte
NUL-terminated entry name, three zero bytes, 32 source SHA-256 bytes and 32 nonzero
authored-straight SHA-256 bytes. Names in role order are `nedb`, `neds`, `spdb`,
`spds`, `forb`, `fors`, `bspb`, `bsps`, `bndb`, `bnds`, `bfrb`, `bfrs`.

Schema is 1. Texture role is 3 or 4. Scale is 1 through 16. Logical dimensions and
source digest match the corresponding scene identity; physical dimensions equal
logical dimensions times scale. Rectangles must fit their texture. The
straight-art digest is trusted metadata; it is not reconstructible from the
already premultiplied payload.

## Small-logo regions

Each section is 40 bytes: six u32 values (role, x, y, width, height, scale), then
four i32 values (logical width, logical height, source origin x, source origin y).
Logical dimensions match the corresponding logo identity and physical dimensions
equal logical dimensions times scale. Scale is 1 through 16; rectangles must fit
the logo texture. Source origins match the signed 16-bit halves of source header
word 3. The GPU core still independently checks its supported rendering geometry.

## Lifetime and activation

Successful parsing returns immutable native-owned bytes, descriptors and catalogue.
Provider callbacks allocate nothing, do no I/O and leave outputs unchanged on
invalid role or null output. The pack must be retained for the entire installed
provider lifetime. Catalogue and rendering-provider installation happen before
workers or context/getter use. Loading alone neither installs them nor enables
replacement drawing. No caller-controlled CPU serial, catalogue or texture data
proves GPU context, upload generations, target ownership or suppression authority.
Missing, mismatched or malformed packs preserve the original-art path.
