# Owned-chunk experimental fixture

This source-only fixture retains the original control and tested candidate. The production resolver now matches that candidate; control sources remain frozen for independent comparison. Synthetic zero-filled DATA/BSS declarations preserve mapped byte lengths only. They contain no original game fields, bytes, pictures, or executable code. The codec alias points at an arbitrary synthetic location.

The oracle runs in separate frozen-control, candidate and production executables so each uses one coherent HeapArena definition. It compares exact successful spans, rejection messages, binding transitions, freed/reused allocations and concurrent stable reads. Separate boundary checks cover chunk crossing, capacity, address overflow, partial reuse and unrelated allocation activity.

Run `python3 scripts/apple_owned_chunk_probe.py --local-safety-check` on Linux. Native mode requires ARM macOS, Xcode 16.4 and iOS SDK 18.5. It runs native sanitized executables and compiles real control/candidate EngineMemory sources into ARM64 iOS objects with these synthetic declarations. It does not execute iOS, build an app, time gameplay, or measure energy. Passing it cannot establish iPad speed or battery improvement.
