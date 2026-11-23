# Optional History panorama resource

This resource is separate from the existing 49-image History pack. It supplies
only a panorama for the authenticated background source; the renderer must keep
the original completed page as its center. No draft is activated by the codec.

A caller supplies a trusted SHA-256 of the complete file and the current base
History artwork generation. The file cannot declare its own trust. The loader
bounds the file at 34 MiB before reading and authenticates it before decoding.

The 32-byte little-endian header contains magic NFSHWID2 (8 bytes), version 2,
background source index, width, height, decoded RGBA byte count and a zero
reserved word. The remainder is one zlib stream. Its decoded bytes are opaque
RGBA8 with dimensions at most 4096 per axis and at most 32 MiB total. The
panorama aspect must be 16:9 within two pixels of horizontal rounding.
The decoded count must equal width times height times four. Both the compressed
input and decoded output must be consumed exactly; trailing bytes, concatenated
streams, truncation, extra decoded bytes and transparent pixels are rejected.

The resource owner copies decoded pixels and records the exact background source
and base generation. A view requires both to remain current. Retaining its memory
does not authorize drawing it after source replacement or retirement. Application
activation, renderer context checks and queued ownership are separate integration
requirements and are not implemented by this codec.

Development version 1 (NFSHWID1, version 1) uses the same header with raw RGBA
instead of a compressed stream. It has the same image and ownership checks.
Shipping authoring should use version 2 and verify byte-exact decoded pixels.

## Authoring and application configuration

`scripts/history_panorama_pack.py` accepts opaque raw RGBA8 bytes, dimensions,
a background source index and a new output path. It writes version 2, verifies
the lossless round trip and reports file/pixel digests without input paths.
It never overwrites an existing output file.

For source IDs 0, 7, 14, 21, 28, 35 and 42, the optional CMake settings are
`NFS_CLASSIC_HISTORY_PANORAMA_<source>` (resource file) and the corresponding
`_SHA256` trusted pin. Pins are generated into build-local metadata read by
application activation. The runtime filename is fixed to
`history-panorama-<source>.ownedz`. Configuration happens before base History
art is resident. Lazy loading is selected by the exact proven page source,
attempted once per base generation and cached with owned source identity.

Deferred public bases compile configured pins but omit personal bytes. A local
file supplied in deferred mode is authenticated without being copied. Personal
post-build injection must supply and authenticate configured resources. Normal
non-deferred builds use the existing companion-resource bundling checks.
No panorama is enabled unless its pin is explicitly configured.
