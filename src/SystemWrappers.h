// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
extern "C" int32_t GetSystemInfo_wrap(int32_t address);
extern "C" int32_t GlobalMemoryStatus_wrap(int32_t address);
// Runtime hooks expose the host page size and the configured game heap budget.
extern "C" uint32_t nfs_native_page_size();
extern "C" uint32_t nfs_game_heap_capacity();
extern "C" uint32_t nfs_game_heap_available();
