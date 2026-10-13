// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Linux byteswap.h is unavailable in Apple's SDK. These do not change layouts.
static inline uint16_t bswap_16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t bswap_32(uint32_t x) { return __builtin_bswap32(x); }
static inline uint64_t bswap_64(uint64_t x) { return __builtin_bswap64(x); }
