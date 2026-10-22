# Owned software-menu text packets

These helpers prepare optional replacement text for the original logical
640 by 480 RGB565 software surface. They do not read guest memory, observe game
calls, access files, queue renderer commands or activate replacements. A passing
packet test does not establish game, graphics, device or performance acceptance.

## Font input and ownership

`SoftwareFontCatalog.h` accepts two roles of 96 glyphs and a 1024 by 512 R8 atlas.
Metrics and kerning use the existing 16.16 advances; glyph coverage has one
transparent texel around every nonempty ink box. Construction checks dimensions,
numeric bounds, coverage length and those gutters, then copies metadata and
pixels into a private immutable owner. Inputs must remain readable and immutable
throughout construction. No font bytes or original artwork are embedded here.

The startup owner must authenticate font identity, provenance and licensing.
Shape validation and a caller-supplied numeric identity do not establish these.
A future renderer cache must retain the immutable font owner and authenticate
its content; numeric identity alone is not a reusable GPU resource proof.

`SoftwareTextLayout.h` preserves the bounded layout, kerning, fitting,
alignment, clipping and transparent sampling border of the tested predecessor.
ASCII 32 through 126 and the original supported byte 224 have explicit slots.
Unsupported bytes, unterminated strings, invalid roles or nonfinite geometry
reject the replacement. `SoftwareTextVertices.h` expands RGB565 tint and
prevalidates the complete batch before changing output vertices or its count.

## Original-call proof

`SoftwareFramePacket.h` owns the initial surface, the independently replayed
original result and replacement vertices. The builder accepts only an explicit
font owner, a valid surface identity and an increasing nonzero serial. Each text
command retains its entry metadata; only the measured advance is supplied on
exit. Every observed original pixel store must match its recorded prior value,
computed value and actual result. Completion requires closed commands, bounded
counts, matching identity and equality of the entire replayed original plane.
A failed build cannot publish a partially valid packet.

The producer must independently validate the actual owner, lifetime, selected
surface, clip, format, addresses and every interior lookup-table entry using the
existing checked guest-memory interface. The endpoint geometry checks in
`SurfaceIdentity` are not a replacement for that complete validation. Its
numeric fields are correlation data, never permission to dereference addresses.
Original game calls and stores must still execute normally.

Before consumption, compare the packet against the *current producer's*
accepted serial, current surface identity and complete current original plane.
Do not manufacture acceptance by taking these values from the candidate itself.
Retain the shared immutable packet by value in deferred work. Builder reuse,
caller buffer mutation or owner retirement cannot modify a completed packet or
release its font prematurely. A builder has one owner; it is not a concurrent
collector.

## Integration still required

The eventual adapter must observe the real original calls, preserve pristine
underlay ownership, order all non-text operations, invalidate on retirement and
prove the current presentation matches the packet. Graphics code must separately
prove context, target, upload and original sampling state, including inherited
scissor restrictions, then preserve later widgets and cursor ordering. Missing
or unsupported state must retain the original rendering path. Neither a CPU
serial nor successful layout establishes GPU residency or context ownership.

The optional `hd_software_frame_packets` host test constructs its own fonts,
coverage, planes and stores. It checks original replay, independent geometry and
color expectations, malformed input, capacity boundaries and retained ownership.
It requires no game archives, private traces or extracted original pixels.
