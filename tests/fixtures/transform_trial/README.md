# Transform trial backend

This separate translation unit registers the diagnostic comparison control and
provides the getter used by the private instrumented Game transform method.
It defaults to baseline. Do not add it to normal engine source lists: registration
must mean the linked Game method actually contains both dispatch bodies.

The shared control lives in TransformTrial.h, with no Game class or layout change.
A mode change increments a generation even when another change returns to the
same mode before the next recorder sample. Logs describe requested configuration,
not per-call accounting. CPU/FPS windows may overlap a change; UI changes reset
those existing measurement windows. This mechanism alone establishes no speed or
energy benefit. The original method/body and linked-object pins must be checked
by the private integration recipe before using this backend in a game executable.
