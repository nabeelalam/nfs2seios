# Feedback lookup ownership

The original target reader obtains Y-row offsets from descriptor slot 11 and X-column offsets from slot 12. The backdrop verifier protects 480 Y entries and 640 X entries for Main, and 60 Y entries and 186 X entries for the popup scratch image.

Two boundary regressions place a reported dispatch slot in the last Main or scratch X entries. The earlier verifier accepts that inconsistent evidence because its X span ends too early. The corrected verifier rejects both cases. Optimized and ASan/UBSan checks pass all 141 synthetic backdrop checks, all 199 dependent text checks, and exact private captured-frame replay over 168 rows, 22,482 stores and 307,200 pixels with nine corruption negatives. Original data remains separate from these source tests.

These are CPU replay and ownership-bound checks. They do not grant live source ownership, GPU publication, or physical-device acceptance.
