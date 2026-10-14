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
#include <sys/resource.h>
extern "C" void nfs_show_frame_rate(void *window,double fps,double cpu,int width,int height);
#endif
namespace {nfs::FrameRate rate;void *last_window=nullptr;std::atomic<double> last_fps{0};std::atomic<int> render_width{640},render_height{480};uint64_t cpu_wall=0;double cpu_time=0;}
extern "C" void nfs_render_target_changed(int width,int height){render_width=width;render_height=height;}
extern "C" int nfs_render_width(){return render_width.load();}
extern "C" int nfs_render_height(){return render_height.load();}
extern "C" void nfs_reset_frame_rate(){rate.reset();last_window=nullptr;last_fps=0;cpu_wall=0;cpu_time=0;}
extern "C" double nfs_frame_rate_value(){return last_fps.load(std::memory_order_relaxed);}
extern "C" void nfs_frame_presented(void *window){
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
