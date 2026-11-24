// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Zero-filled span fixture; contains no original game fields or data.
struct __attribute__((packed)) DataLayout { uint8_t zero_layout[69037]; };
alignas(4) extern DataLayout _data;
