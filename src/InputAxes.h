// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
namespace nfs {
// Original calibration encoding: controller in bits 20..31, axis in 16..18,
// upper/lower calibration bytes in 8..15/0..7. Output is a game byte, 0..255.
int32_t calibrated_axis(uint32_t calibration, int32_t axis, bool deadzone);
int32_t read_calibrated_axis(uint32_t calibration, uint32_t guest_axes, bool deadzone);
void set_game_axis_deadzone(bool enabled);
}
extern "C" int32_t processAxes(int32_t calibration);
