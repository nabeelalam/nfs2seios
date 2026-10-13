# Validation and acceptance

Keep test evidence scoped to what was executed. Source publication is not build
or gameplay evidence. Local source tests and a previous platform diagnostic do
not establish working gameplay in the current iPad app.

## Host checks

The test sources cover guest address bounds, unaligned and endian access, native
callback identity, original translated routines, heap lifetime, files/mappings,
events, threads, timers, renderer boundaries, audio callback lifecycle, virtual
controller input/reconnect and asset import. Use sanitizers where supported.
Boundary checks must not be weakened merely to let game startup continue.

Public-repository checks also test forbidden-file rejection, secret/path patterns,
safe commit metadata and aggregate artifact-size limits. An independent secret
scanner checks the exact published history in addition to the project policy.

## Original-data local checks

Use a separately supplied, ignored owned copy of the original assets. Record
menu/race behavior, acceleration and steering, callback failures and clean
shutdown. Keep captured graphics, audio and raw diagnostics private. Non-silent
PCM proves generation, not audible correctness. Software rendering results do
not establish Apple GPU performance. Incomplete or forcibly killed runs fail.

## Apple/device acceptance

- Current source compiles and links against the actual Apple device SDK.
- The bundle is ARM64, declares the iOS device platform and targets iPad.
- The package excludes separately supplied main game assets and signing secrets.
- Personal signing and physical launch succeed.
- Imported original assets load, menus work, and complete races are stable.
- Controller axes/buttons, disconnect/reconnect and release states work in-game.
- Original audio sounds correct and remains stable through transitions.
- Background/foreground, graphics context, audio and clean shutdown work.
- Performance and memory use are measured on the physical target.

The intermittent invalid audio callback, sustained races, full laps, controller
gameplay and the latest physical game build remain open. Do not declare the port
complete based on a successful cloud compilation or retry.
