# Adaptive world aspect stage (inactive)

`AdaptiveAspect.h` supplies camera and per-view placement calculations only. It
is not connected to the safe-contain presentation switch, and no current renderer
advertises its required capabilities. This does not implement true widescreen.

## Verified original-engine boundaries

- `46F7A0` calls `46F980(mode=0, player=0)` for the main world view. When
  `512208 == 1`, it also calls mode 0 for player 1, followed by the shared mode 2
  and per-player mode 3 passes. These are useful ownership boundaries, not proof
  that every command produced inside them is world geometry.
- `480730` selects camera/view rectangles. Its view-type-1 path is a 150x70
  preview/mirror-sized view, so it must not receive the race-world correction
  implicitly. `480830` also installs a different camera during the `435C50`
  producer; cockpit/mirror ownership requires separate classification.
- `4808D0` computes float (`433910`) and integer (`4A0704` -> `4A05C8`)
  cameras; `433A40` supplies logical screen clipping. `44EEC0` saves/restores
  camera state around additional views. Persistent global edits can compound a
  correction or be overwritten by that restoration.
- `46FA40` constructs and merges deferred command lists from several producers,
  including `448120`, `45BD70`, `47F310`, `46C710`, `4742A0`, `435C50` and
  `45B580`. It is not an immediate GL world scope.
- `46FCF0` only extracts list head/tail from `5222CC`/`5222C8` and terminates the
  tail's next pointer. It does **not** snapshot camera state or establish a
  renderer viewport boundary.
- `471390` inserts an existing node (EDX) into a sorting bucket. `4713C0` only
  calculates a bucket index. `42A3C0` executes the resulting list through
  `4EC500[node.type]`. Node type alone does not establish world/HUD ownership.
- `42A170` initializes normal/split logical view rectangles; `42A010` selects
  the rectangle and calls `497714` for clipping. The generated renderer's
  `native_grClipWindow` currently scales both axes by framebuffer height / 480.
  `useGameProgram` distinguishes game rendering from presentation, not world
  geometry from HUD.

## Required implementation stages

1. Classify the producer contributions and actual node creation paths before
   enabling an owner. Bind world/player, HUD, cockpit and mirror ownership to a
   node address plus its allocation/frame epoch so sorted/merged lists and node
   reuse cannot transfer stale ownership. The confirmed mode-0 player boundary
   is the initial scope; leave the preview and unclassified routes unchanged.
2. Apply Hor+ once to a fresh, confirmed world camera setup at `4808D0`, before
   CPU projection and visibility consumers generate geometry. Capture original
   float horizontal focal `4D5294`, integer focal `4DAC24` and horizontal angle
   `4DAC30`; retain vertical focal `4D5298`, centers `4D529C/A0`, vertical camera
   parameters and intended logical clipping. Verify all horizontal culling and
   clipping consumers against the corrected camera, including save/restore via
   `44EEC0`. An angle rounded outward is conservative, not a substitute for
   tracing those consumers.
3. At actual node execution (`42A3C0`), enqueue immutable renderer ownership and
   per-player viewport data before the node's Glide commands. Generated geometry
   already reflects the camera active during its production; later renderer
   viewport ownership is a separate contract and must not read mutable guest
   camera globals. Preserve FIFO association and flush pending triangles before
   changing transforms or targets.
4. Provide a native-ratio world target and independent aspect-preserving HUD/UI
   composition. Existing HD native passes assume a 4:3 framebuffer; globally
   widening that target would violate their checks. Preserve interleaved command
   ordering, depth/blend behavior and per-player boundaries when composing the
   world and UI. Do not stretch the final mixed framebuffer or the 640x480 HUD.
5. Only advertise the three capabilities after world projection, separate HUD
   placement and matched culling are all integrated and verified. An incomplete
   renderer must refuse the request without changing original state.

For 16:9, factor = 4/3 and horizontal focal becomes 3/4 of the original. Stacked
split-screen receives the same expansion factor (its original per-view aspect is
8:3). An iPad at 4:3 receives factor = 1. No vertical view is removed.
Narrow/portrait surfaces refuse Hor+ and retain contain until a separate Vert+
path is designed. Wider photo assets can use independent layout; this camera
stage does not crop or resize their authored content.

Remaining uncertainty: producer-level classification, mixed world/HUD lists,
cockpit/mirror behavior, node lifetime/epoch coverage, all culling consumers and
cross-target depth/blend composition still require implementation evidence.
Status: inactive calculation and source investigation only; no true-widescreen
runtime or physical-device acceptance is claimed.
