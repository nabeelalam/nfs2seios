# Cold fallback correctness fixture

This MIT-licensed EngineMemory candidate changes only the native-pointer fallback into a noinline free helper. Both variants retain the existing full-span checks, active binding checks, heap validation, error precedence and class layout. Shared headers and the control come from the frozen native_span/candidate fixture; synthetic DATA/BSS contain only zero-filled stand-ins.

The probe compares sanitized native/integer oracles, compiles iOS ARM64 objects and records O3 helper assembly. It does not build the game, upload artifacts, measure FPS or demonstrate an energy improvement. Original game assets are excluded.
