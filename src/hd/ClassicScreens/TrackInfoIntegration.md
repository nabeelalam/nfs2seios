# Complete TrackInfo static and dynamic integration

The dedicated type11 owner is41C1B0. Its three ordinary405B30 draws select the
back, actual localized heading and sldr resources from one current archive.
The maintained source catalog contains52 exact raw0x78 identities across the
eight original TrackInfo archives. Only metadata and hashes are public.

Before each original call, StaticBitmapScope captures the actual name, flags,
full target and source-derived plane.405B60 resolution binds the source address,
whole bitmap SHA, dimensions, intrinsic signed12 placement and current clip.
Mode0x400 uses the original opaque16-bit copy; mode0x1000 uses the original
zero-keyed16-bit copy. The16-bit dispatch entry is checked at table+12. Replay
must match every word of the complete postdraw plane after all three calls.
Original operations are insertion-only and remain in their original order.

The existing dynamic text collector then reverses only observed text stores.
Its clean plane must equal the completed static replay before optional art can
join the immutable frame. Source resources and owner/locale/target are checked
again at native acquisition. Retirement invalidates an archive-backed frame.

TrackInfoArtProvider installs copied, SHA-checked, immutable PMA RGBA images.
Each entry names one source-catalog index and carries logicalBounds and
focalBounds. A scene acquisition succeeds only when all three exact source
indices from the same archive are installed together. Missing, unknown or
incomplete pages retain their original static imagery; the independently proved
dynamic-text path remains available. A disabled or absent pack is not a complete
static remaster and must not be reported as one.

The optional loader uses a pinned NFSTRKIF v1 file:16-byte outer header
(magic8, version32, exact inflated payload bytes32), then one zlib stream with
no trailing input/output. Payload starts with imageCount32,reserved32. Each
image has sourceIndex32,width32,height32,reserved32, logicalLTRB float32x4,
focalLTRB float32x4, then exact width*height*4 PMA RGBA bytes. Duplicate indices,
nonfinite/inverted bounds, invalid aspect/alpha/hash and oversized payloads are
refused. The file and inflated payload limits are128MiB and384MiB. The startup
pin is deliberately empty until coherent reviewed masters are ready.

The renderer retains only the current three textures. It prepares/draws all
three into its private staging FBO before HD dynamic text, then commits only
after target, generation, owner and original native-pass checks. Failed optional
preparation never publishes a partial static replacement. Context loss forgets
handles without foreign deletion or state restoration; owned retirement releases
the new GPU objects. Ordinary original car/trophy rendering, cursor, timing and
swap operations remain in place.

The640x480 coordinates remain the original source reference. Artwork bounds may
include wider overscan and focal metadata rather than forcing every master into
a4:3 crop. Art uses the retained projection/viewport so the parent aspect-aware
layout can place a wider uncropped master. This module does not independently
change the shared viewport/aspect policy and does not prove widescreen acceptance.

Root integration still needs TrackInfoArtProvider.cpp and TrackInfoArtworkLoader.cpp
in the shared native link sources, existing zlib linkage, an optional startup
load of track-info-ui.ownedz, and optional packaging/resource path wiring. The
classic source/renderer activation wiring was already supplied by root. Artwork
masters, source generation, combined compilation/runtime and iPad acceptance
remain separate; no test, compiler, Game or cloud run was performed for this edit.
