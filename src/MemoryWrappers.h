// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Exact translated-game ABI: all pointers crossing these exports are guest
// addresses. Native allocator hooks and host pointers remain full-width.
extern "C" int32_t malloc_wrap(int32_t size);
extern "C" int32_t calloc_wrap(int32_t count, int32_t size);
extern "C" int32_t free_wrap(int32_t address);
