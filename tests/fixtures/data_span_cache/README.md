# Bounded data-window compatibility fixture

This preserves the isolated three-window prototype already checked against the actual generated calculation and a hidden two-controller race. It does not replace the delivered frame-only fixture or enable any application feature.

`python scripts/apple_data_cache_probe.py --local-safety-check` verifies the frozen source closure and runs the synthetic oracle under ASan/UBSan and optimized host compilation. Without that flag, it requires an existing native Apple ARM64 host and the pinned iOS 18.5 SDK, then also compiles and links an ARM64 iOS 16 compatibility executable. The iOS executable is inspected, not run. No downloads, game data, app build, timing calibration or workflow dispatch occur.

Production use inherits GuestCopyTransaction's bounded same-thread contract: retain allocation lifetime; no allocation/free, blocking, nested copy helpers or callbacks within the scope. Tests that deliberately free, rebind or retire an engine check defensive fallback only. They do not permit these operations in production or establish concurrent-free safety.

Every canonical access/error/cancellation checkpoint remains. A successful canonical access may populate at most three 64-byte data windows aligned to 16 bytes, in addition to the frame window. Negative probes are remembered, invalid tokens are not refreshed, and bindings are checked before retired revision pointers.
