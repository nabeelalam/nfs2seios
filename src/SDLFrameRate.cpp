// SPDX-License-Identifier: MIT
#include "FrameRate.h"
#include "FramePresentation.h"
#include "FramePacer.h"
#include "WorkerControl.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <chrono>
#include <atomic>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && TARGET_OS_IOS
#include <sys/resource.h>
extern "C" void nfs_show_frame_rate(void *window,double fps,double cpu,int width,int height);
extern "C" void nfs_toggle_performance_overlay(void *window);
extern "C" void nfs_monitor_activity(int state);
#endif
namespace {nfs::FrameRate rate;void *last_window=nullptr;std::atomic<double> last_fps{0};std::atomic<int> render_width{640},render_height{480};uint64_t cpu_wall=0;double cpu_time=0;}
extern "C" void nfs_render_target_changed(int width,int height){render_width=width;render_height=height;}
extern "C" int nfs_render_width(){return render_width.load();}
namespace {std::atomic<int> frame_limit{0};}

extern "C" void nfs_set_frame_limit(int fps){frame_limit.store(fps==30?30:0,std::memory_order_relaxed);}
extern "C" int nfs_frame_limit(){return frame_limit.load(std::memory_order_relaxed);}
extern "C" void nfs_wait_frame_slot(){
    // Never sleep UIKit's main queue or its renderer/audio callbacks.
    static thread_local nfs::FramePacer pacer;
    // The normal unrestricted path needs no clock query or wait.
    if(nfs_frame_limit()!=30){pacer.reset();return;}
    if(SDL_IsMainThread())return;
    for(;;){
        nfs::check_worker_stop();
        // Keep deadlines in the same time domain as condition-variable waits.
        const auto now=uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        const auto wait=pacer.remaining(now,nfs_frame_limit()==30);
        if(!wait){if(nfs_frame_limit()==30)pacer.admitted(now);return;}
        if(auto *control=nfs::current_worker_control()){
            // Stop notifications wake this worker. Deferrals/held engine locks
            // must still suppress unwinding without turning the wait into a spin.
            std::unique_lock<std::mutex> lock(control->mutex);
            control->changed.wait_for(lock,std::chrono::nanoseconds(wait),[]{return nfs::worker_stop_ready() || nfs_frame_limit()!=30;});
        }else SDL_DelayNS(wait);
    }
}
extern "C" void nfs_toggle_diagnostics(void *window){
#if defined(__APPLE__) && TARGET_OS_IOS
    nfs_toggle_performance_overlay(window);
#else
    (void)window;
#endif
}
extern "C" void nfs_diagnostics_activity(int state){
#if defined(__APPLE__) && TARGET_OS_IOS
    nfs_monitor_activity(state);
#else
    (void)state;
#endif
}
extern "C" int nfs_render_height(){return render_height.load();}
extern "C" void nfs_reset_frame_rate(){rate.reset();last_window=nullptr;last_fps=0;cpu_wall=0;cpu_time=0;}
extern "C" double nfs_frame_rate_value(){return last_fps.load(std::memory_order_relaxed);}
extern "C" void nfs_frame_presented(void *window){
    nfs_diagnostics_activity(2);
    if(window!=last_window){rate.reset();last_window=window;}
    double fps=0;if(!rate.presented(SDL_GetTicksNS(),fps))return;
    last_fps.store(fps,std::memory_order_relaxed);
#if defined(__APPLE__) && TARGET_OS_IOS
    const auto now=SDL_GetTicksNS();struct rusage usage{};double cpu=-1;
    if(getrusage(RUSAGE_SELF,&usage)==0){
        const double current=usage.ru_utime.tv_sec+usage.ru_stime.tv_sec+(usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0;
        if(cpu_wall && now>cpu_wall && current>=cpu_time)
            cpu=100.0*(current-cpu_time)/(double(now-cpu_wall)/1000000000.0)/SDL_max(1,SDL_GetNumLogicalCPUCores());
        cpu_wall=now;cpu_time=current;
    }
    nfs_show_frame_rate(window,fps,cpu,nfs_render_width(),nfs_render_height());
#else
    char title[80];std::snprintf(title,sizeof(title),"Need for Speed II SE - FPS %.1f",fps);
    SDL_SetWindowTitle(static_cast<SDL_Window *>(window),title);
#endif
    std::fprintf(stderr,"Presentation FPS: %.1f\n",fps);
}
