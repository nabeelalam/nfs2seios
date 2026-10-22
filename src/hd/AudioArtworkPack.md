# Optional Audio artwork pack, version 4

Version 4 retains the version 3 main-menu, Location, native-mark and cursor sections. It appends two sections after the native textures. The application must authenticate the complete file against a separately trusted SHA-256 before parsing or installing providers.

* Tag 8, role 0: 496-byte source metadata catalog. Header: schema 1, count 6, stride 80, reserved zero. Each record contains role, source span, width, height, palette offset/count, signed hot X/Y, the four original header words, and source SHA-256. Roles are audi, abar, laud, raud, laul, raul in that order. No original indexed pixels or palette payload are stored here.
* Tag 9, role 0: 112-byte atlas header followed by 2048 × 768 premultiplied RGBA bytes. Schema 2, premultiplied 1, opacity-baked 1, opaque 0, nonzero identity, byte count, atlas SHA-256 and zero source-SHA field. Unused header words are zero.

All fields are little endian. Section table entries remain 32 bytes with contiguous, ordered spans and zero reserved words. There must be at least one native texture and at most the version 3 native texture limit. The total file limit remains 128 MiB. Versions 1–3 retain their existing limits, section counts and validation rules.

The pack owns its immutable bytes. Startup retains the pack before publishing the optional Audio provider. Existing packs provide no Audio provider; a sealed or unavailable optional provider preserves original rendering. Source metadata is exposed only while its pack is retained. Before any original draw can qualify for replacement, the runtime must independently obtain and copy the six actual sources from the user's archive and authenticate their complete bytes with SourceCatalog. Metadata and an atlas alone never authorize draw suppression or publication.

Audio drawing additionally requires exact current form, owner, pass, target, original reset, sidebar, bar-row replay and text proof. The GPU bar helper draws only into an independently authenticated unpublished native staging target. It supplies no full-screen backdrop and grants no presentation authority. Any failure preserves the original path.

These interfaces are foundations for integration. Component tests do not establish native game activation, physical iPad appearance or energy savings.
