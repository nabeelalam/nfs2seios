// SPDX-License-Identifier: MIT
#pragma once
#include "GuestAccess.h"
// The original RefPack decoder loads four lookahead bytes even for a one-byte
// end marker. Preserve the little-endian command bits it actually consumes,
// while requiring only the command's encoded width at the allocation boundary.
inline int32_t nfs_qfs_command(uint32_t address) {
    const uint8_t first = guest_load<uint8_t>(address);
    const unsigned width = first < 0x80 ? 2 : first < 0xc0 ? 3 : first < 0xe0 ? 4 : 1;
    uint32_t value = first;
    for (unsigned i = 1; i < width; ++i)
        value |= uint32_t(guest_load<uint8_t>(address + i)) << (8 * i);
    return static_cast<int32_t>(value);
}
