# Dedicated original intro splash

Owner `408558` receives the `title.qfs` archive loaded by `408450`. The exact
`back` draw and one of six localized copyright draws remain unchanged. Source
capture binds their directory entries, complete raw RGB565 identities, current
language and owned 640x480 target. It independently reconstructs the original
opaque background plus zero-keyed copyright pixels and requires exact final
plane equality before the normal native presentation can carry a new image.

`IntroArtProvider` is optional. Installation copies and hashes the opaque photo
and six alpha masks; a null logo input borrows the exact silver role0 from the
existing program-lifetime immutable HD art provider. The original logo's colored
oval roles are excluded. The logical logo rectangle is supplied separately.
A separately rendered exact PMA logo may instead be provided. Missing or invalid
inputs preserve the original intro; no source bitmap is changed.

The native pass validates the original 300 uploads, 300 selections and 600
triangles before drawing photo, silver logo and white copyright/shadow into a
private FBO. The completed FBO is copied only after target, provider generation,
source and context checks. Context loss drops names without foreign deletes.

## Root wiring

- Run `scripts/classic_screens/intro_hooks.py` as a late classic stage. It emits
  Methods01/03/10 and asserts exact reversal of all numbered source insertions.
- Include `IntroRenderer.inc` once in the combined renderer and register its
  created/retired/other/upload_begin/upload_done/select/triangle callbacks.
- Compile `IntroArtProvider.cpp` and `IntroArtworkLoader.cpp` with targets owning
  `ArtworkActivation.cpp` when bundled classic artwork is enabled, and retain
  the existing ZLIB::ZLIB linkage (including final Apple app targets).
- After successful primary artwork activation, resolve `intro-ui.ownedz` next to
  the bundled personal owner template and call
  `nfs_intro_art_load_file(path,error,capacity)` before starting the game worker.
  Log the returned generic diagnostic if it is unavailable. Add
  `NFS_PRIVATE_INTRO_UI=1` with the classic activation flags.
- The public base omits private intro data. Personal finishing must add the exact
  separately held intro resource and record its hash without modifying existing
  executable/resource bytes. This does not change the shared artwork-pack schema.

## Personal resource

The 16-byte header is `NFSINTRO`, little-endian version1 and payload size.
The remainder is one exact zlib stream. The 72-byte payload header is little
endian `<II4f12I>`: photo width/height, logo L/T/R/B, then six alpha width/height
pairs in EN/DE/FR/ES/IT/SV order. Photo top-first PMA-RGBA pixels follow, then
six top-first A8 planes. The logo is not copied into this file. Full-file SHA,
exact compressed and exact payload sizes are pinned in `IntroArtworkPin.h`.
The loader bounds all data, rejects trailing input/output and copies provider
bytes before releasing its temporary decoded payload.

The current photo master is 1448x1086. No 4K, Apple compile, actual intro-render
or physical-device acceptance is established by the source implementation.


Optional widescreen photograph (pack version2)
-------------------------------------------
The original version1 pack and original photo remain supported. Version2 keeps
the first 72 metadata bytes, appends little-endian wide width/height, retains
all original photo/copyright bytes, then appends opaque wide RGBA. Both images
are independently copied and authenticated by the provider. Use
`scripts/extend_intro_artwork.py` with an explicit original-pack digest and
a separately supplied PNG; no artwork is embedded in public source.

Schema2 input/view adds `widePhotograph`; schema1 input ignores that appended
field. Existing native rendering still uses the original `photograph`. The
wide photograph is drawn by IntroWideRenderer.inc into its own full-drawable
color surface after the exact original title pass. Selection binds the actual
front attachments and context/artwork generations. A separate swap scope
preserves that completed content until native mutation, resize or retirement.
Logo and copyright retain uniform scale and horizontal centering. The internal C structs changed size, so rebuild
renderer/provider consumers together rather than linking old renderer objects
against the new provider. Keep the old active pin until a new package and its
presentation path have been validated.
