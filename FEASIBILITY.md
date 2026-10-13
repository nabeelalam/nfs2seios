# Feasibility assessment

Source assessment: original-game translation and native iPadOS portability.

## Candidate assessment

| Candidate | Evidence | Decision |
|---|---|---|
| [zaps166/NFSIISE](https://github.com/zaps166/NFSIISE) at 66cbaf47fd77e833c9331b81f6c1e52aff56913f | SDL2 wrapper, Glide-to-GL/GLES2 renderer; translation submodule at 7607d07ea7471b0547d42fb9bb71e0c339f98a0c | Original behavior reference |
| [Link4Electronics/NFSIISE](https://github.com/Link4Electronics/NFSIISE) at fde04b571533ba20a60c473ab99d2431bb5cfd60 | Split C++ translation, SDL3, CMake, ARM64/Linux work; all 18 engine units compile in local AArch64 check | Provisional implementation base |
| [thiekus/NFSIISE](https://github.com/thiekus/NFSIISE) at b75b70918ee8 (inspected checkout) | Emscripten build uses MEMORY64=0 and SDL2 | Useful portability reference; browser/Wasm is outside the requested native scope |
| [OpenNFS](https://github.com/OpenNFS/OpenNFS) | Source/README inspected; modern engine, Bullet physics; original gameplay and configurable menus still listed as planned | Does not preserve original gameplay closely enough for the initial objective |

The GitHub public forks API was inspected, along with recent commits of several
promising forks. This was a targeted survey, not proof that no other port exists.
Contrary to the original assumption of inactivity, upstream's inspected revision
is dated 1 May 2026. Fork documentation is a claim; local compile evidence is
recorded separately.

## What the 32-bit restriction actually means

The original translation is about 249,000 lines of C++ representing game
operations as compiled functions. `CPU` keeps EAXâ€¦EDI/ESP in 32-bit unions;
`Application::to32i` dereferences numeric addresses, `push32` stores pointer
values in four-byte stack slots, and `call` converts numeric values to native
function pointers. `_data` has packed pointer-bearing tables. C wrapper
declarations pass game arguments as 32-bit integers even when the implementation
expects native pointers or handles.

The original constructor casts `malloc`'s stack pointer to `int32_t`; its source
explicitly rejects non-32-bit hosts. The fork allows 64-bit compilation but
`CPU.h`, `Application.h`, `Memory.cpp`, and `Wrapper.c` still truncate pointers
and allocate memory below 4 GB. CMake adds `-no-pie`. `MemoryTranslate.cpp` does
not generally resolve arbitrary high host pointers; the heap/code paths assume
identity mapping. The fork's AGENTS.md contains historical claims that differ
from the current source: inspect code rather than adopting those claims.

Replacing every game integer with intptr_t would corrupt register arithmetic,
packed layouts, and guest stack conventions. Removing the compiler guard alone
would also leave runtime pointer truncation intact.

## iPadOS blockers

[Apple's memory documentation](https://developer.apple.com/documentation/uikit/optimizing-memory-performance)
states its 64-bit runtime supports PIE. [Apple's linker manual](https://github.com/apple-oss-distributions/ld64/blob/main/doc/man/man1/ld-classic.1)
describes a default 4 GB page-zero reservation for 64-bit executables. Therefore
the Linux strategy of a low-address non-PIE process cannot be assumed to work
in an ordinary iPad app. No device experiment validating low mappings was run.
The selected port must support normal high, randomized host addresses.

Remaining work includes DATA/BSS pointer relocation, typed C ABI adapters,
four-byte game pointers inside structures, indirect function IDs, heap/stack
translation and thread/audio boundaries. This is substantial engine work.

Native AOT C++ translation is compatible in principle with no-JIT/no-CPU-emulator
requirements: no instruction decoder or runtime code generation is proposed.
Whether the complete port preserves original behavior remains unverified.

## Platform paths

- Graphics: `Glide2x.c` and `Glide2x/OpenGL2.c` implement the hardware-accelerated
  Glide wrapper, with an existing GLES2 branch. Reuse it first, with SDL's iOS
  GL backend and correct default framebuffer handling. Apple deprecates OpenGL
  ES; SDK compilation and device rendering must establish usability. Metal in
  the diagnostic only checks platform GPU access and is not a replacement game
  renderer.
- Audio: upstream `EAcsnd.c` requests stereo signed 16-bit samples at 22050 Hz,
  optionally interpolated to 44100 Hz, through SDL audio. The fork uses SDL3
  audio streams and pool buffers. Music decoding/mixing comes from translated
  game routines. Buffer and callback pointers require proper address marshalling.
- Input: `DInput.c` emulates DirectInput using SDL joysticks, axes/buttons/hats,
  with keyboard events for menus. The diagnostic uses Apple's Game Controller
  framework; connecting it to game input is still pending.
- Lifecycle: upstream already translates SDL background/foreground events to
  focus changes, but iOS main-thread rendering and interruption behavior need
  device tests. The current probe has separate UIKit lifecycle handling.
- Assets: upstream requires Special Edition `fedata` and `gamedata` trees in
  lowercase, plus `install.win`, language text and configuration. Game save/config
  paths currently assume desktop/Android locations and must use the app sandbox.
- OS services: Win32 file/thread/event/critical-section shims and sockets exist.
  Embedded structure sizes and pointer lists require explicit guest layouts.
  Network multiplayer is outside initial scope.

## Licensing and delivery

Upstream explicitly says its MIT license covers the wrapper only. The translation
submodule has no separate license file in the inspected checkout. The selected
fork has a root MIT license and broader README statement, but that does not
resolve the upstream translated-engine provenance. Preserve notices; do not
claim a definitive right to publicly distribute the full translated game.
The current repository ships original project code and source-fetch instructions;
its diagnostic does not include the engine or EA assets. Resolve provenance
before releasing the complete engine publicly.

[SideStore's official prerequisites](https://docs.sidestore.io/docs/installation/prerequisites)
support Windows for initial setup, so a Mac is unnecessary. Free accounts have
seven-day signing periods and limited app slots per the
[official FAQ](https://docs.sidestore.io/docs/faq). Actual compatibility with the
user-reported iPadOS 26.5.2 has not been tested; no installable build claim is made.

Conclusion: native ARM64 compilation is demonstrated. A faithful iPadOS port is
plausible but requires substantial address/ABI work and real hardware tests.
