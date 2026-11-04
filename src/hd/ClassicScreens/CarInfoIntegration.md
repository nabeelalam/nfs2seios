# Source-bound car information photographs and text

This optional module covers original owner `412670`, exact form `dword_4D2E78`, dispatcher type7. It does not replace the history/slideshow owner411BD0 (type8), videos, arbitrary pictures, or Player thumbnails.

Original source: `off_4D40C0` maps type7 to412670. Car selection is copied by413160 from `off_4119C0` in this exact order: mac, ferr, f355, gt90, indigo, mus, jag, gt1, lotus, nazca, ital, isdera. `mus` is the Mustang MachIII; generic `info.qfs` is not a car. The shared410E80 path installs the selected archive in4D40B8. At the actual frame boundary, selected car is `[esp+168h]`, selected page is `[esp+154h]`.

Atloc41285B the opaque back is drawn by405B30 withflags400h. Page0 has no technical bitmap; pages1/2/3 use gnrl/perf/mech through the one actual common405B30 atloc412892, flags1000h (zero-keyed RGB565). The technical image is361x143 at(242,23); the background is640x480. Dynamic40FA90 follows the bitmap work, followed by426B70 and the4274F0 presentation path. Four physical4274F0 call sites exist in412670. All original calls, stores and branches remain; insertion markers must reverse byte-for-byte to their input.

The collector binds form, selected car/page, archive identity/name, source entry offset, complete selected raw RGB565 header/pixel SHA, geometry, clip and row-dispatch callback. Archive sizes can be identical between different cars, so sizes never identify a car alone. Both original bitmap draws are replayed into complete640x480 planes; inverse actual text stores must join the complete static output before authored pixels can be admitted. Subsequent complete frame boundaries recover from a refused frame. Retirement invalidates archive/target-backed snapshots.

A single immutable provider installation requires all12 background source identities together. Source indices are `carIndex*4+role`, role0back/1gnrl/2perf/3mech. Optional authored technical images must match their exact selected source index. If a technical replacement is absent, the actual source-proved original technical bitmap is converted to immutable zero-keyed PMA RGBA and retained by the frame above the new photo. It is not silently removed or inferred from a different car/page. Dynamic text is rendered last. This retains original technical illustration fidelity until those separate assets are remastered; it does not claim they are alreadyHD.

Background images must be opaque, uniformly scaled4:3 complete author canvases with logical bounds(0,0,640,480). Contain the wide clean photo layer uniformly within that canvas with deliberate authored background material; never cover-crop or nonuniformly stretch a car to satisfy this pass. Keep the independent wide masters and layout/subject bounds for the future adaptive renderer. The current legacy pass does not prove true16:9 world/UI support.

The native pass proves the original300tile uploads/selections and600triangles, stages the chosen background and actual technical overlay plusHD text into a privateFBO, and only then publishes. Provider generation, exact source IDs, image count and GLcontext key authored texture reuse; no multi-megabyte authored image hash/upload occurs on a cache hit. Context ownership, framebuffer attachment, texture aliases and source target must remain current. The borrowed fallback technical image stays owned by the immutable frame lease.

## Root integration required

- Append `car_info` to the classic source insertion pipeline, after earlier hooks; its script writes Methods01/03/10/13 and has insertion reversal assertions.
- Include `CarInfoRenderer.inc` in maintained classic RendererHooks and add its created/retired/other/upload_begin/upload_done/select/triangle forwards/dispatch exactly like other isolated screen modules.
- Add CarInfoArtProvider.cpp and CarInfoArtworkLoader.cpp to activation targets with existingzlib linkage.
- Optional startup loader runs after main artwork setup, before the worker. Enable `NFS_PRIVATE_CAR_INFO_UI=1` only in the explicit classic feature set. Empty pin intentionally disables photo replacement; original clean pixels plus independently provedHD text remain available.
- Optional resource is `car-info-ui.ownedz`, eight-byte magic `NFSCARIF`, version1. Header format and per-image layout match the documented TrackInfo48-byte metadata format; install accepts12required backgrounds plus up to36 optional technical replacements.
- Set the trusted fileSHA/file-byte/payload-byte pin only after the private complete artwork bundle is finalized. Keep original/reference pixels, generated candidates and private receipts out of public source.

No source generation, compiler, Game, cloud build, device observation or activation was performed by this source implementation. Root must generate source and renderer together and perform one combined build/ordinaryflow review before claiming runtime coverage.
