# Performance measurement boundary

The next target is sustained split-screen performance with Low Power Mode enabled. The previously delivered UI build is preserved separately. This work does not establish new iPad FPS or battery savings.

## Energy research

Apple describes inefficient or unnecessary redraws as work that can keep the CPU, GPU and display active. Its guidance recommends consistent frame pacing, avoiding obscured drawing, and using lower rates where the experience permits, such as menus. See [Energy Efficiency Guide: Graphics and animations](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/EnergyGuide-iOS/AvoidExtraneousGraphicsAndAnimations.html), reviewed 2026-11-06. This is archived guidance, not an energy measurement of this port.

Apple also recommends reducing computation and other work when Low Power Mode is active: [Responding to power notifications](https://developer.apple.com/documentation/xcode/responding-to-power-notifications). Low Power Mode is an operating-system energy policy; unchanged application CPU percentage does not establish unchanged execution capacity or power consumption.

For this project, preserve the requested 60 FPS race target. First measure real owner-thread CPU windows and presentation intervals. Do not infer battery savings from a smaller texture package, lower resident memory, higher average FPS, or desktop software-renderer results. The user's battery-percentage observations are useful leads, not calibrated power measurements.

## Local recording

The desktop recorder is explicit opt-in, with no recorder thread or capture enabled in ordinary launches. It writes a new JSONL session and separate frame CSV/metadata files; existing destinations are refused. Unsupported desktop battery, Low Power Mode and thermal values remain unknown. Engine CPU samples cover the existing original engine submission windows; presentation samples mark swap submission, not GPU execution duration or the instant a display emits pixels. Preserve sample IDs, timing epochs, capture overflow and writer failure information when summarizing.

Use the same track, both player cars, graphics options, starting profile, controller bindings, input sequence and fixed measurement window for comparisons. Observe these settings before timing. Warm up until fresh complete CPU windows exist, keep setup screenshots outside the measured window, and report race completion separately from completing a 60-second measurement. WSL software rendering is a local regression and CPU investigation tool, not a substitute for final M3 iPad measurements.
