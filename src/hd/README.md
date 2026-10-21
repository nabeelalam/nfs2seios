# Owned HD scene verification

These headers provide shared C-compatible drawing metadata, native-owned original
rendering records and pure CPU verification. They do not load artwork, access
guest memory, record sessions, touch a renderer or enable visual replacements.
The existing runtime remains unchanged when these helpers are unused.

`BackgroundScenePrepare.h` accepts a **trusted caller-supplied SourceCatalog**:
thirteen scene identities, five logo identities and two map digests, each with
complete role assignments. No original asset identity catalogue is embedded here.
Catalogue validation checks bounds, completeness, distinct roles/digests and
shape consistency. It does not establish provenance or authorize caller data.
An eventual production provider must obtain its catalogue from a separately
reviewed trusted source; a user file or guest pointer is not such authority.

The catalogue and every input record must remain immutable for the entire
synchronous verification call. The caller supplies a distinct, nonaliasing
workspace and output object, normally allocated outside its guest-memory lease.
The verifier allocates nothing, performs no file I/O, retains no input pointers
and does not alter its inputs. Outputs are usable only when `valid` is true.
`Prepared::view()` borrows its owner's storage: the owner must outlive the
synchronous consumer or be retained by value in any queued callback.

The original table checks, palette and RGB565 arithmetic, clipping, full-plane
replay, ownership/event ordering, lifetime comparisons and native-key range
checks are preserved. The deliberate API change replaces fixed private identity
arrays with the explicit catalogue and adds a fail-closed catalogue preflight.
Native keys and CPU serials do not prove GPU residency, context ownership or
permission to suppress original draws; a renderer must prove those separately.

`OriginalUiRecords.h` retains the historical `nfs_logo_capture` namespace only
for verifier source compatibility. It contains no diagnostic Store, sampler,
exporter, environment variable or resource-generation factory.

Configure `NFS_HD_CPU_TESTS=ON` to build the optional CPU test. It generates its
own indexed images, palettes, maps and complete original-call records in memory.
Its independent opaque RGB565 oracle checks all 307200 final pixels, with
malformed catalogue, order, palette, source, target and native-key negatives.
No original game blobs, extracted imagery or original identity hashes are needed.
These checks do not establish physical-device or live-renderer acceptance.
