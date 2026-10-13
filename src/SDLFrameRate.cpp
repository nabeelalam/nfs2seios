// SPDX-License-Identifier: MIT
#include "FrameRate.h"
#include "FramePresentation.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <atomic>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
extern "C" void nfs_show_frame_rate(void *window,double fps);
#endif
namespace {nfs::FrameRate rate;void *last_window=nullptr;std::atomic<double> last_fps{0};}
extern "C" void nfs_reset_frame_rate(){rate.reset();last_window=nullptr;last_fps=0;}
extern "C" double nfs_frame_rate_value(){return last_fps.load(std::memory_order_relaxed);}
extern "C" void nfs_frame_presented(void *window){
    if(window!=last_window){rate.reset();last_window=window;}
    double fps=0;if(!rate.presented(SDL_GetTicksNS(),fps))return;
    last_fps.store(fps,std::memory_order_relaxed);
#if defined(__APPLE__) && TARGET_OS_IOS
    nfs_show_frame_rate(window,fps);
#else
    char title[80];std::snprintf(title,sizeof(title),"Need for Speed II SE - FPS %.1f",fps);
    SDL_SetWindowTitle(static_cast<SDL_Window *>(window),title);
#endif
    std::fprintf(stderr,"Presentation FPS: %.1f\n",fps);
}
