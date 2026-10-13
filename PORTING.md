# Native port architecture

## Source and execution

The preparation scripts fetch reviewed upstream commits into ignored dependency
directories and generate disposable mapped C++ source with checked patches.
Execution uses native functions; there is no x86 instruction loop or JIT.
The original algorithmic behavior is retained wherever possible.

## Address and value boundaries

Original 32-bit values, packed layouts and logical addresses remain explicit.
Native pointers are never truncated. Registered DATA/BSS regions and a bounded
heap translate complete guest spans to native addresses. Typed value copies
handle unaligned operands and shared packed fields. Invalid or freed spans fail.

Callable IDs are separate from data addresses. Native function pointers retain
full width. Registration precedes dispatch threads, and an ordered registry
supports binary lookup with identity, duplicate and capacity checks. Unknown
IDs fail with the failing thread's recent original calls; the optional glibc
native callsite diagnostic is excluded from unsupported platforms.

## Platform services

File operations are confined to app-owned imported asset storage. Original
virtual Windows paths are logical game paths, not the developer's filesystem.
Native mappings preserve access and lifetime rules. Event, thread and timer
services use native ownership and bounded stop/join behavior.

Owned game contexts keep register/stack state distinct. SDL thread-local state
is retired on externally created production workers, while SDK-free host tests
avoid an SDL dependency. Audio uses a dedicated context and registered sample
buffer; pause/resume controls run outside the SDL callback's locking path.

SDL input snapshots feed native controller records. Enumeration, reconnect and
menu translation have host coverage; controller behavior throughout complete
original races still needs validation.

The existing Glide GLES2 backend is adapted through typed guest boundaries and
tested framebuffer transfers. Main-thread scheduling and graphics/audio/input
lifecycle gates handle suspension and wakeup. Physical iPad context and performance
validation remain separate from software-renderer tests.

## Assets, packages and validation

Original assets are supplied separately and imported into owned app storage.
Pinned port support resources and upstream licenses are verified when packaging.
The Game bundle executes the original engine; the Probe is a separate platform
diagnostic. Neither a successful Probe nor a compiled unsigned IPA proves game
completion. Signing material and owned game files never belong in Git or CI uploads.

The intermittent invalid audio callback remains unresolved. Capture and explain
its cause before claiming a fix. Complete laps, sustained races, controller
gameplay and the latest physical game package are still acceptance work.
