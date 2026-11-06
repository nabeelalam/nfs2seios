# Source-bound manufacturer history remaster

This separate optional module covers original owner `411BD0`, exact form
`dword_4D2CBC`, dispatcher type8. CarInfo412670/type7 and Slideshow412300/type12
remain separate. This module changes display only: original statements, calls,
input handling, historical strings, selection rules and frame presentation stay.

`413180`/`off_411990` maps twelve cars to seven manufacturer archives:
McLaren; Ferrari/F355; GT90/Indigo/Mustang; Jaguar; GT1/Esprit; Nazca/Cala;
Isdera. The source catalog stores49 exact entry SHA fingerprints, spans,
dimensions and placements, with indices `archiveIndex*7+role`. Archive order is
hismac,hisfer,hisfrd,hisjag,hisltus,hisita,hisisd. Roles are
back,engl,fren,germ,itly,span,swed. The original locale table at4D4134 selects
roles1,3,2,5,4,5. Its final Spanish selection is retained rather than silently
changing the original language behavior.

Every ordinary frame calls405B30 twice (back then locale title), both flags400h
with the exact opaque RGB565 row dispatcher483438. The collector authenticates
archive directory/name and whole selected entry SHA, dimensions, packed source
position, clip, target identity and callback; it independently replays both
draws and compares the complete640x480 plane. The separate title is an opaque
original patch. A replacement title may be transparent PMA artwork; the new
background replaces the original scene before the new title is composited.

Capture begins after both original static draws and covers40FA90 plus the
later direct caption/timeline text calls. It ends immediately before405C70.
Inverse actual observed glyph stores must join the complete static plane.
Presentation wraps405C70, whose428770 path reaches426B70 and426950. Cursor and
other original later operations are retained. Every other collector stays
inactive because this module owns only the exact type8 form. Font metrics,
text stores and prepared text retain immutable source ownership; nothing is
rendered twice intentionally and a refused proof retains the original frame.

The native renderer retains its own frame lease and context-bound resources,
requires the actual300-tile/600-triangle original pass, prepares a private FBO,
and publishes only after renewed owner/context/target checks. No filename-only
or stale-frame activation is permitted.

## Shared integration required

- Append `history` to the source insertion pipeline after the current stages;
  `scripts/classic_screens/history_hooks.py` writes Methods01/03/10/13 and proves
  byte-exact reversal of all inserted blocks. Regenerate engine and renderer
  together after shared wiring is ready.
- Include `HistoryRenderer.inc` in the shared RendererHooks; declare and forward
  the created, retired(owner), other, upload_begin, upload_done, select and
  triangle functions exactly as the neighboring CarInfo module.
- Compile `HistoryArtProvider.cpp` and `HistoryArtworkLoader.cpp` into activation
  targets with the existing zlib/TextureDigest linkage. Set the exact startup
  feature `NFS_PRIVATE_HISTORY_UI=1` only in the explicit classic feature set.
- Call `nfs_history_art_load_file` after main artwork setup, before the worker;
  clear the provider on corresponding retirement. Empty size/SHA pins disable
  loading until the private reviewed49-image bundle is selected.
- Add the exact pinned resource to personal bundling and injection only after
  its final pixels are reviewed. Never publish private reference/pixel files.

The resource filename is `history-ui.ownedz`; eight-byte magicNFSHIST1,
version1 LE32 and decompressed payload sizeLE32. The zlib payload starts with
image countLE32 and flags0. Each record is48 bytes (`4I8f`: source index,
width,height,flags0,logical rectangle4floats,focal rectangle4floats), followed
by top-first premultiplied RGBA. All49 images are required. Backgrounds must
be opaque, uniform4:3 complete640x480 logical canvases. Each title must keep
its exact original logical rectangle; transparent PMA titles are allowed.
The file limit is64MiB, decoded budget128MiB. Lossless compression and whole
file size/SHA pins remain mandatory.

Current evidence proves insertion reversal and syntax of the four engine TUs,
provider, loader and native renderer. It does not prove shared integration,
resource loading, game publication, widescreen history layout, Apple packaging
or physical iPad behavior. Root must finish those steps before claiming them.
