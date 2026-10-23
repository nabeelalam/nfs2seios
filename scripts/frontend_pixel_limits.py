"""Bounded source chunks for the unchanged eight-MiB frontend atlas."""
# SPDX-License-Identifier: MIT
# The finite classic-word atlas requires79run-aligned chunks. Retain headroom
# while keeping every generated source below the existing1MiB file gate.
FRONTEND_PIXEL_PART_BYTES = 65520
FRONTEND_PIXEL_MAX_PARTS = 96
