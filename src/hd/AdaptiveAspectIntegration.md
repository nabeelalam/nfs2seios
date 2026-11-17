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

## Local split-screen prototype evidence

A private desktop build verified North Country's two scenery clips and the
shared HUD pass independently. The prototype commits a renderer-supplied
extent immediately before the world frame, following the framebuffer change.
The camera reads the committed extent for both players, including an empty
extent for a contain frame. A newer offer is deferred to the next frame.

The original split-screen replay stayed live through 800x450, 708x492, 800x600,
and 800x450. The 800x600 frame used the normal 4:3 target. Earlier isolated
results also reached the Race Summary, Lap Times, and Main Menu after Quit Race.
The shared target helper is used by the private build. The opt-in paired generator enables the coordinated engine and renderer hooks
for the experimental iPad package.

An aborted local race establishes the menu return route. A normal finish,
camera variants, and physical-device validation remain open.

## Experimental paired preparation

Run the standard engine and renderer preparers first, then prepare the pair
explicitly in the same project:

    python3 scripts/prepare_adaptive_race.py --engine .build/prepared-engine --renderer .build/prepared-renderer --output-dir .build/adaptive-race

For a local experimental Release, point NFS_FULL_MAPPED_ENGINE_DIR to the
pair's engine subdirectory and NFS_RENDERER_SOURCE_DIR to its renderer
subdirectory. Both outputs are required because the engine and renderer share
the camera and framebuffer handoff. The regular preparers leave it disabled.
If a source anchor changes, the command fails during preflight before creating
a destination.
## Renderer-only coverage

The viewport draw scope is shared by the engine and renderer on the calling
thread. Submission captures it before handing work to the renderer thread.
The standalone renderer can read the default scope with no engine callbacks.
A two-translation-unit ASan/UBSan check verified that the reader sees the
current player and nested draw scope, that scope exit restores the parent,
and that another thread starts with an empty scope.
The Release `renderer_pixel_tests` binary linked with the paired renderer and
passed `real_gles_pixels` under Xvfb (2800x1900, software GLES). The larger
virtual display let the test request its 2360x1640 drawable. Desktop GLES
results cover the standalone suite and do not establish Apple device behavior.

## Manual build selection

The reviewed native iPadOS workflow accepts `adaptive_race` for a classic UI
Release. The default is off. Both the Linux gates and the Apple packaging job
use the paired directories emitted by the same source preparer. The classic
base keeps its existing personal-artwork and draft-delivery requirements.
Local preparation with both inputs enabled completed from the pinned sources.
All six patched units matched the previously staged units after line-ending
normalization. The existing workflow and artwork input checks passed.

## iPad preview and local performance checks

Workflow run 43930095947 passed the Linux and Apple checks and produced the
experimental ARM64 preview. Its separately assembled personal install package
still requires physical iPad validation. The bundled renderer predates the
following local optimizations.

- In 4:3 contain mode, a scenery draw with no admitted widescreen target leaves
  the triangle batch untouched and skips the texture-binding query. A 1,000-call
  sanitizer fixture verified the fast path. A local two-controller North
  Country race then exercised wide-to-contain-to-wide resizing and shut down
  cleanly.
- A widescreen clip update now sets the viewport and scissor once. The ordinary
  clip remains the fallback if widescreen layout cannot be calculated. A
  sanitizer fixture verified upper and lower player clips and both fallback
  cases. The standalone GLES pixel suite passed.
  A North Country split-screen replay with both virtual controllers also
  passed live resizes through 708x492, 800x600 and 800x450, with scenery at
  both edges of both views and a clean shutdown. The first attempt timed out
  in the Opponents return; the same executable passed after extending the
  replay's menu wait from five to fifteen seconds.

The session summarizer can compare the proportion of swap intervals over 20 ms
and renderer queue and execution time per call by Low Power Mode. It excludes
the first sample, gaps and observed mode transitions from that comparison.
These measurements require matched race and graphics settings and do not
measure GPU time or energy use.

## Preview artwork storage measurements

The delivered ZIP for run 43930095947 is 502.9 MiB. Its 13 pinned artwork
resources total 489.9 MiB. Slideshow (229.29 MiB), Car Info (108.27 MiB) and
Track Info (43.74 MiB) account for most of that total. The 98 stored slideshow
images average 2.34 MiB, with a 3.23 MiB maximum. Applying outer ZIP deflate
would save about 2.7 MiB.

An in-memory Car Info trial applied reversible row deltas and individually
compressed each of 48 images. Each decoded RGBA row matched the original exactly.
The estimated resource size was 81.14 MiB, including record overhead, compared
with 108.27 MiB today. Filtered chunks averaged 1.69 MiB and peaked at 3.03 MiB.
No resource file, package, or runtime decoder was changed in this trial.

The follow-up added a version 2 decoder and a local converter. The converter
filters each image independently and uses one bounded zlib stream for the
metadata and image bytes. The private candidate measures 85,094,133 bytes
(81.15 MiB), a reduction of 28,433,276 bytes (25.0%). Both versions were read
by the native loader under ASan and UBSan, using their own exact size and SHA
pins. All 48 published image indices, dimensions and decoded pixel hashes
matched. The native build and private assembly tool now pin the verified version 2
candidate for the next package batch. The already delivered IPA uses version 1.

Track Info now shares the lossless row decoder. Its version 2 candidate is
36,501,092 bytes (34.81 MiB), down from 45,860,407 bytes (43.74 MiB).
Native loader checks under ASan and UBSan matched all 52 image indices,
dimensions, and decoded pixel hashes across the two versions. Build and
packaging pins select the verified files for both information packs.

## Pacific Spirit information page

The local replay covered Location, Pacific Spirit Track Info, resizing from
640x480 to 708x492 and 800x450, and returning to Location. The side shader now
limits mirroring to 4.5% of the center image width, fades that strip into the
existing tint, and samples no artwork beyond the strip. GLES2 readback verified
center and side pixels. Actual desktop captures at both new sizes kept the
route map, artwork, headings and cursor visible. Side lettering and long column
streaks were removed; a narrow reflected forest edge is still visible on the
right. The replay waited for the X11 window to be viewable before focusing it
and shut the game down cleanly. These observations do not cover iPadOS.

## Pause controller handoff

When the pause menu refreshes the original joystick state, digital buttons are
masked from that refresh on the engine thread. A controller press that arrives
between the menu's capture and the refresh remains available to the next pause
capture. Analog axes are preserved. The virtual SDL fixture exercised that
ordering on player two, and both the virtual SDL and mapped input tests passed.
The regenerated original engine unit compiled under ASan and UBSan. A physical
controller mapped to downshift still needs an iPad resume check.

## Combined ARM64 preview

Workflow run 43946994663 built commit 3471a48aeb6f52ce44869f45172ea6fa58f565d9.
The Linux gates and Apple device Release job passed. The base manifest reports
an ARM64 iOS device executable requiring personal signing. Local assembly
verified all 13 pinned artwork resources, including the two version 2
information packs. The resulting private ZIP was 489,499,800 bytes, down from
527,290,802 bytes for the preceding widescreen preview. Its uploaded digest
matched the local verification. The temporary unpublished transfer was
removed after private delivery.

The acceptance checks for this revision are SideStore update and launch with
the existing imported data, independent split-screen controls and audio,
Pause resume with the confirm button bound to downshift, widescreen HUD and
menu transitions, and a matched split-screen capture in normal and Low Power
Mode. The build/package evidence does not establish those device results.

## Local audio and input follow-up

The gamepad adapter skips controller enumeration while both slots are occupied.
The two-slot SDL and mapped input tests passed. The audio bridge wakes the
retirement waiter when the inactive stream's final callback completes. Its SDL
worker wakes only when the requested pause state changes. Both callback suites
passed, and a local North Country split-screen replay produced 73.4 seconds of
44.1 kHz stereo PCM using SDL's disk backend. The final 12 seconds had no fully
silent 100 ms blocks and the game shut down cleanly.

Future CI-built iPad diagnostics include the exact source commit in the session
header. The log writer and analyzer reject malformed identifiers while older
logs remain readable. Compact diagnostics retain the battery reading and
engine CPU sampling remains available when recording. These changes followed
the delivered ARM64 preview and still need Apple/device verification.

The assembled preview was also checked directly. It contains one thin ARM64
iOS-device Mach-O executable, identical to the Apple build's executable before
artwork assembly. The executable links OpenGLES and UIKit and contains no
initially writable executable segment. The archive contains 25 entries,
including 13 separate remaster resources, with no original game data folders,
PC executable, or game archive payloads. The segment inspection is static
package evidence and does not establish device rendering or signing results.


### Car Showcase information pages

The authenticated authored Car Showcase frame uses the information-page
presentation pass to extend the edge colors to wide drawables. A narrow
reflected strip blends into warm and cool side fills and keeps lettering
and imagery within the original centered 4:3 content rectangle.

The local Release app displayed the Ford Indigo General page at 708x492
and 800x450, with the full car and both inset photos visible. At 800x450
a relative desktop pointer moved from Next Car to More Stats. A left click
opened Performance, and Escape returned to Player One. The run shut down
cleanly. These checks cover the selected car, aspect ratios and desktop
pointer route. Physical touch and other cars remain to be verified.

### Optional renderer timing

Renderer bridge timing is opt-in. Previously ordinary gameplay collected
three clock readings and three atomic updates per measured bridge execution
until a diagnostic recording was stopped. The iPad recorder enables timing
for its session, and the engine startup probe explicitly enables it for its
reported timing. The bridge test verifies zero counters at startup and
populated counters after opt-in. All 13 renderer bridge and queue regressions
passed locally. The device FPS and battery impact remain unmeasured.


### Lower split-screen record panel

The stock line draw submitted two lower record-plaque bevel strokes with
endpoints separated by an extra two half-viewport heights. The resulting
line ran through Player Two's world at 16:9, an iPad-like ratio, and 4:3.
The adaptive renderer corrects the endpoints of those exact two vertical
strokes and preserves their existing colors and draw order.

An original-data North Country split-screen replay confirmed the lower
record panel and both player views at 800x450, 708x492, and 800x600, including
a return to 800x450 and clean shutdown. Inspection of all three outputs
confirmed that the vertical seam was gone. A fresh source preparation
reproduced the exact renderer used by the replay. The replay covers race
startup, aspect switches and the observed record panel. Physical device
verification and full race completion remain open.


### Single-player normal-world view

The mode-zero, player-zero full-size camera now applies the same matched
horizontal focal and culling adjustment as the two split views. The renderer
admits the full original clip and retains the centered HUD transform. A
clear for the contained rear-view mirror now stays in that subview instead
of spilling across the widened world.

A local original-data North Country single-player run displayed the world,
car, HUD, minimap and mirror through 16:9, an iPad-like ratio, and 4:3 before
returning to 16:9. The first prototype exposed the mirror clear as a cyan
rectangle; the corrected replay showed the mirror contained at both wider
ratios and exited cleanly. This replay covers the default chase camera and
live resizing. Other camera modes, moving race completion and physical
performance remain open.


### Consistent Location cards

All eight Location masters share one 280x133 rectangular card with small
rounded corners and a separate soft offset shadow. A feathered scenery
backing fills transparent master edges, while the entire sharp image is
contained inside the card. The original image identity and native fill,
draw and row replay still authorize the replacement. Their pixel coverage
does not define the visible silhouette of the new card.

The lossless version-two pack is 9,468,676 bytes, down from 10,816,400.
Independent decoding verified eight identical shadow masks at shared
pixels, premultiplied color, and deterministic output. Seven selectable
tracks were observed through the normal local Location menu, including
resizing to 708x492 and 800x450 and clean shutdown. Monolithic Studios
was checked in the decoded pack and contact sheet. The copied local
profile does not offer it in this menu. The animation fallback and
physical iPad verification remain open.


### Slideshow presentation

Authenticated Slideshow frames now fill wider drawables with a soft
continuation of their edge colors. The photo, footer, text and cursor retain
their original proportions. The local Ford Indigo Slideshow opened at
708x492 and displayed a subsequent image at 800x450. Both frames filled the
drawable, and Escape returned to Showcase. The process exited cleanly.
Resizing a stationary desktop slide can leave its old frame in place until
the next input triggers a game presentation. Physical verification is open.


### Location transition route on the current local build

A source-traced Location entry and three consecutive track changes used
only wide presentation routes at a stable 708x492 drawable and again at
2360x1640. At the larger extent, entry passed through the 17-frame fade
and 21-frame incoming photo. Each track change showed 21 outgoing frames
followed by its incoming wide presentation. The native route did recur
for 19 frames on the subsequent Escape to Main. At that point the
Location child-return source sealed, the outgoing-sidebar path reported
kind 13, and the kind 7 fallback published before Main resumed wide.
The reduced-size run changed drawable size before Escape, so it does
not contradict the fixed-size exit finding. This is local evidence on
the current source. The reported device transition behavior remains open.

The return fallback was reproduced at 708x492. A temporary local trace
showed that the reverse pass expected the original texture even though the
photo renderer had bound the independently authenticated authored tile.
The historical sidebar recovery now attaches the current photo just as the
fresh source path does. It retains the existing source, lifetime, tile,
geometry and target checks. The same Location -> change track -> Escape
route now publishes all 19 frames through the wide return path and goes
directly to wide Main. The tracing patch was excluded from the source.


### Opponents native-size desktop replay

Two pointer entries, two class changes per entry and both returns to Main
completed at 2360x1640 with clean shutdown. The presentation trace used wide
routes across the captured entries, class changes and returns. The reported
on-device crash did not reproduce. One process-wide WSL resident-memory
sample during the run was approximately 1.73 GiB, which includes desktop
software rendering and is not an iPad footprint measurement. The startup
loader currently holds approximately 478 MiB of decoded Car Info, Track Info
and History artwork before any of those screens are opened. This makes
artwork residency a concrete optimization target independently of the
unconfirmed crash cause.


### History artwork residency

Startup records the private History resource location. The authenticated
History handler loads it before its first frame and releases the visit after
the last queued draw. Overlapping visits retain the same artwork generation.
An isolated test with the pinned pack verified startup with no decoded
generation, two holders, clearing after the final release, and reopening
with a fresh generation. A normal local Ford History route displayed
authored statics and HD text on two visits, returned to Showcase twice,
and shut down cleanly. Process-wide WSL RSS readings were affected by the
software renderer and do not quantify iPad memory savings.

Car Info and Track Info now use the same visit lifetime. Together with
History this moves 500,881,608 decoded bytes out of startup. The loader
checks all existing pins before the first authored frame. The Track Info
visit starts before its outgoing/incoming transition and is kept alive
through the queued return work. The three-pack check verified overlap,
final lease release and fresh generation on reopen for each pinned pack.
A grouped local game run showed Ford Indigo Showcase and Pacific Spirit
Track Info with the remastered graphics, then returned to Location and
shut down cleanly. On-device footprint and cold menu-open timing remain
unmeasured.


### Showcase rotating badge source

All twelve Showcase car archives have a source-verified car6 entry.
They cover seven badge designs and ten distinct exact indexed entries.
The Showcase handler builds that entry through the shared badge staging
and alpha conversion path. The later draw uses the original signed
horizontal width and tint, which produces the edge-on and reverse faces.
A replacement must preserve that animation and use the current archive,
selected car and converted native upload to bind the matching authored mark.
The privately retained source inventory and decoded contact sheet cover all
twelve cars. No new spinning logo artwork is installed yet.

The seven authored brand marks were isolated from the existing restoration
artwork. The Italdesign mark keeps its original geometry and uses the
silver finish seen in the spinning source. The engine's per-car placement
table and original alpha-200 silhouette establish the twelve registered
1024-square canvases. They preserve each logo's aspect ratio and include
a soft shadow. Private receipts pin the exact original entries and the
new canvases. The renderer still uses the original spinning texture until
the registered artwork is wired into its upload and draw path.

Showcase now publishes the twelve registered marks through the shared
animated badge path. The authored texture is bound only after verifying
the native archive entry, selector, placement, converted upload and both
original draw triangles. One ordinary-input desktop visit cycled through
all twelve cars at 708x492. All twelve source identities published, the
captured front and back faces retained the original rotation and tint,
and the game returned to Player One and exited cleanly. The new private
version-two badge pack keeps the eighteen track records byte-for-byte and
adds 1,552,022 compressed bytes and 48 MiB of decoded pixels. It is loaded
at startup with the track marks. Apple and physical tests remain open.

## Startup and Player selection replay

The integrated desktop build presented the opening Main swipe at
2360x1640 for 59 consecutive frames before the first steady wide Main
frame. The original intro was dismissed through ordinary input.

Player One entered, changed cars three times and returned to Main at
708x492 and 800x450. At both sizes the opening, car changes and return
used wide presentation throughout. The trace authenticated both photo
layers and published 100 selects and 200 triangles for each observed
photo animation draw. Captures included the moving photo layers and
steady selection. Both sessions exited cleanly. These tests cover the
desktop renderer. Physical iPad startup, car changes and the reported
Opponents crash remain open for this revision.

## Vertex snapshot copy experiment

The candidate copied all vertices of a Glide line or triangle under one
checked memory transaction and released it before renderer dispatch.
Actual generated standard, adaptive and pointer bridges passed Clang
syntax checks. The GLES pixel suite passed with the adaptive candidate.

Two matched 30-second North Country split-screen comparisons started
with both virtual controllers driving. The binaries shared 96 other
build inputs and used the same 13 verified artwork resources. The
desktop replay used software GLES. All four sessions shut down cleanly.

| Run order | Baseline engine median | Candidate engine median | Baseline swap median | Candidate swap median |
| --- | ---: | ---: | ---: | ---: |
| Baseline, candidate | 14.563 ms | 14.576 ms | 29.594 ms | 29.779 ms |
| Candidate, baseline | 14.419 ms | 14.293 ms | 29.780 ms | 29.648 ms |

The direction changed with run order and the median changes were under
1 percent. The candidate source patch and raw captures were retained
locally. The build sources keep the original vertex copies. The trial
measured desktop submissions, with physical iPad FPS and battery impact
still unknown.

## Display swap duration capture

The diagnostic recorder now measures host time inside successful SDL
display swaps while capture is enabled. The summary reports measured
call count, weighted mean duration and maximum duration by power mode
using stable sample intervals. The existing spacing between submissions
is a separate measurement. Swap-call duration can include waiting on
the display or driver. No GPU timestamps are collected.

The capture ring and JSON serializer passed ASan and UBSan, the Python
summary and intro hook tests passed, and a freshly generated desktop
GLES renderer passed the pixel suite through the new swap wrapper. An
actual swap produced a positive duration with capture enabled. The
following swaps left the capture ring empty after capture was disabled.
The change follows iPad package 43978740494 and still needs Apple
compilation and device data.

## Controller discovery during gameplay

The game loop asks SDL to enumerate controllers immediately on add,
remap or foreground events. It also refreshes when a connected slot
is lost. With a free slot and no event, recovery is retried every
250 ms. Existing controller axes and buttons are still sampled on
each poll, and the two live slots still skip discovery.

The SDL input suite passed with actual virtual joystick add events
and with a replacement device whose add event was discarded. It also
covered both players' state, neutral disconnect and reconnect into the
vacated slot. The game-loop translation unit compiled. Physical
hotplug and the battery impact remain unmeasured.

The desktop recorder now includes host swap duration in its per-frame
CSV. An integrated original-data North Country split-screen replay
recorded 1,024 consecutive race submissions at 800x450 without reported
capture loss. Median submission spacing was 26.649 ms and median host
swap duration was 1.910 ms in this software-rendered replay. Audio
captured during the same race interval contained non-silent samples.
The game resized through 708x492 and 800x600, returned to 800x450 and
exited cleanly. This replay used keyboard input. The separate virtual
controller suite passed multi-controller discovery, disconnect and
reconnect. iPad timing and physical pad checks remain open.

In the steady first 800x450 interval, the slowest tenth of 903
submissions had a median spacing of 33.116 ms and a median host swap
call of 2.180 ms. The other submissions had medians of 26.398 ms and
1.894 ms respectively. The measured host swap occupied a median 6.2
percent of the slowest tenth. For this desktop software-rendered run,
most of the increase was outside the swap call. This does not
attribute the remainder to a specific thread or establish iPad behavior.

The pinned SDL event pump already updates connected joysticks. The
game loop now reuses the pumped state while automatic updating is on.
A newly opened pad still receives an explicit update so held triggers
are available on its first snapshot. Virtual controller callbacks
verified that the duplicate update was skipped and that an explicit
update still ran with SDL automatic updating disabled. The complete
SDL input regression and game-loop compilation passed locally.
