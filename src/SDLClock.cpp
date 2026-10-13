// SPDX-License-Identifier: MIT
#include "ClockWrappers.h"
#include "TimerBridge.h"
#include <SDL3/SDL_timer.h>
#include <ctime>
extern "C" uint64_t nfs_native_ticks(){return SDL_GetTicks();}
extern "C" void nfs_native_delay(uint32_t milliseconds){SDL_Delay(milliseconds);}
extern "C" int64_t nfs_native_unix_seconds(){return int64_t(std::time(nullptr));}
namespace {
Uint32 SDLCALL dispatch_timer(void *opaque,SDL_TimerID,Uint32){
    // This is an opaque native-width token, never a memory address to dereference.
    return nfs_timer_dispatch(reinterpret_cast<uintptr_t>(opaque));
}
}
extern "C" uint32_t nfs_native_add_timer(uintptr_t token){return SDL_AddTimer(1,dispatch_timer,reinterpret_cast<void *>(token));}
extern "C" void nfs_native_remove_timer(uint32_t id){SDL_RemoveTimer(id);}
