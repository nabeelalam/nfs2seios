// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
namespace nfs {
#if defined(__APPLE__) && TARGET_OS_IOS
MainScheduler sdl_main_thread_scheduler(){return apple_main_thread_scheduler();}
#else
static void schedule(MainTask task,void *context){
    if(!SDL_RunOnMainThread(task,context,true))throw std::runtime_error(SDL_GetError());
}
MainScheduler sdl_main_thread_scheduler(){return schedule;}
#endif
}
