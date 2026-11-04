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

## Continuous production reader and publisher

`BackgroundSceneCatalog.h` exposes
`nfs_background_scene_runtime::installCatalog(shared_ptr<const SourceCatalog>)`.
The startup owner must install a trusted immutable catalogue before workers run,
and relinquish mutable aliases. The first successful worker acquisition freezes
replacement and retains its native owner. Validation alone is not provenance.
A missing catalogue or any feature value other than the exact string `1` for
`NFS_PRIVATE_BACKGROUND_SCENE_HD` leaves these hooks inactive, including guest
reads and collector allocation. No artwork loader or original catalogue is here.

`ContinuousLogoCollector.h` owns one current Group/Copy/upload proof per menu
owner. `ContinuousLogoGameHooks.inc` and `ContinuousNativeUploadHooks.inc` take
bounded copies using the existing checked guest-copy interface. The actual game
continues every original write and upload. Each slot-one overwrite, related free,
unwind or unsupported source invalidates replacement eligibility. Timer-only
returns preserve a completed binding; the source-proven lazy redraw point resets
it before actual artwork changes. Owned TLS storage releases its native data at
worker exit without renderer work.

`BackgroundSceneLiveGameHooks.inc` joins those observations with base, side,
word and tail observations from one menu owner and redraw event sequencer.
Warmup before the verified base cannot poison a future pass; resets after a
retained base still invalidate it. Every presentation validates the current
source plane, owner, upload event and all native keys. CPU epochs are content
correlation only, never GPU/context authority.

`BackgroundSceneQueueAdapter.inc` queues immutable owned snapshots by value.
The original native base draw remains between explicit begin and normal finish;
original labels and cursor stay later. STOP/errors propagate under the existing
renderer policy. After consumed acceptance, cancellation only drops local
ownership and never retries, queues destructor work or borrows guest pointers.
The renderer must independently accept current context, uploads and draw state.

`scripts/background_scene_hooks.py` inserts only asserted scopes/calls into four
generated method units after existing checked-copy/frontend transformations.
Its marked additions reverse to the unchanged original bodies. Related free
invalidation uses the central original window-free entry, covering every caller
before retirement; unrelated or null frees do not invalidate a retained proof.
All four units use this one shared include tree; no prototype paths or exporters
are part of generation.

Synthetic runtime tests exercise the actual base/indexed-side/word reader
scopes and call-bound tile upload scopes. Group/Copy records in that fixture
represent simulated original draws; they are not an execution of the original
game. The real queue harness verifies twelve cached owned publications and
retirement. Actual maintained-engine execution and physical-device acceptance
remain separate integration gates.


## Location preview and caption integration

Location uses a separate schema-2 view with a mandatory immutable original
preview layer. The pure packet replays the observed black fill and indexed rows
against the authenticated map, checks the complete 640x480 result, and owns the
cropped RGB565 pixels and semantic write coverage. The GPU stages that layer
before the remastered panel and caption. Original photographs remain original.

A new verified base disarms the prior redraw and can recover from a failed
transition upload. Side and word draws cannot reset invalid state. Caption
retirement distinguishes a true owner/bitmap release from an independent
scratch descriptor decrementing a still-live shared LUT reference; final,
unreadable or invalid references remain rejection cases. Every publication
still requires the current copy, complete native uploads and actual GPU draw.

The bounded maintained Linux menu test now verifies entry, a visibly changed
track with fresh publications, and stable Main return. This does not establish
physical iPad acceptance, performance, energy use or completion of other forms.


22 October - Audio remaster maintained foundation and original control baseline:
Local UI source commit e465a3867f5d5efcef952aca01b8109df737820a adds immutable original Audio background/bar/text replay records, an authenticated process-lifetime Audio atlas provider, a native bar-only staging helper, optional version4 artwork-pack loading/startup and synthetic regression fixtures. All30 registered Audio plus existing artwork/font/Location tests pass optimized and ASan/UBSan with stable source pins. The CPU fixture passes615289 checks/30 negatives; isolated real GLES passes1089 checks per mode, including owner loss after upload/draw without foreign-context cleanup. Retained historical13 bar passes and5 reset/sidebar passes match their original full output planes; derived per-row compatibility data is explicitly not new production capture authority. Independent pack/startup/link review passes; source policy, independent secret scan and commit hook pass. Nothing pushed and no cloud build.
The private authentic version4 pack is89415304 bytes, SHA88e91ccd0167473edc53a5e52024d80f13e6adcd981d95d8fd0e9f4ffd6692a3, with all37 earlier section payloads unchanged, authored6MB Audio atlas and metadata-only original source identities. Its loader, six actual original sources and retained atlas/source owners pass optimized and ASan/UBSan. It has not been copied into the game or activated.
The bounded actual2186x1640 Main -> Options -> Audio -> three normal volume rows -> Options -> stable Main baseline passes with the prior verified game binary156ee7c4/private12-art pack. Original menu/racing/effects bars each visibly decrease and increase: red pixel counts1000/874/1000,1000/869/1000,1000/872/1000. Short keyboard edges did not change the bars; the original source has an Audio throttle, and paced held normal keys work. The exact dropped-input cause was not instrumented. Earlier mouse-navigation, short-key and150s deadline negatives are retained. Final300s bounded test closes normally viaWM_DELETE0/SDL0; copied configuration, baseline artwork and display restored, exact children stopped and lease absent. Independent actual-file/screenshot review confirms the result. Mouse-held13draw highlights and actual numeric values were not measured.
Checkpoint: .build/hd-audio-foundation-maintained-v1/CHECKPOINT.json. Native Audio HD rendering is not active: exact live owner/pass/source/bar/text capture and ordered native background/bar/text staging with original300/600/context/FIFO proof are next. Existing Main/Location behavior remains the current verified runtime. Physical HD acceptance, other frontends/HUD artwork, final art approval and battery/LPM60 remain open. Full goal remains active.
