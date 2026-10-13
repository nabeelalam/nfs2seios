// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
extern "C" uint64_t nfs_native_ticks();
extern "C" void nfs_native_delay(uint32_t);
extern "C" int64_t nfs_native_unix_seconds();
extern "C" int32_t SDL_GetTicks_wrap();
extern "C" int32_t SDL_Delay_wrap(int32_t);
extern "C" int32_t time_wrap(int32_t);
extern "C" int32_t timeGetDevCaps_wrap(int32_t,int32_t);
extern "C" int32_t timeBeginPeriod_wrap(int32_t);
extern "C" int32_t timeEndPeriod_wrap(int32_t);
