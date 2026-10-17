// SPDX-License-Identifier: MIT
#include "FramePresentation.h"
#include "WorkerControl.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <thread>
#include <cstdio>
#include <stdexcept>
#include <chrono>
#include <ctime>
static uint64_t steady_ns(){return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
static bool protected_stop(bool critical){
    nfs::WorkerControl control;bool stopped=false;double elapsed=0;
    const auto cpu_begin=std::clock();
    std::thread worker([&]{
        nfs::current_worker_control()=&control;nfs_wait_frame_slot();
        const auto begin=steady_ns();control.stop=true;
        try{
            if(critical){++nfs::owned_critical_depth();nfs_wait_frame_slot();--nfs::owned_critical_depth();}
            else {nfs::ScopedWorkerStopDeferral defer;nfs_wait_frame_slot();}
            nfs::check_worker_stop();
        }catch(const nfs::WorkerStopped &){stopped=true;}
        elapsed=double(steady_ns()-begin)/1e9;
        nfs::owned_critical_depth()=0;nfs::current_worker_control()=nullptr;
    });
    worker.join();const auto cpu_end=std::clock();
    const bool cpu_ok=cpu_begin==std::clock_t(-1) || cpu_end==std::clock_t(-1) || double(cpu_end-cpu_begin)/CLOCKS_PER_SEC<.025;
    return stopped && elapsed>=.025 && elapsed<.2 && cpu_ok;
}
int main(){
    if(!SDL_Init(0))return 1;
    nfs_set_frame_limit(30);
    const auto main_start=steady_ns();nfs_wait_frame_slot();nfs_wait_frame_slot();
    if(steady_ns()-main_start>15000000)return 2;
    std::atomic<unsigned> frames{0};std::atomic<bool> stopped{false};
    nfs::WorkerControl control;
    uint64_t begin=0,end=0;
    std::thread worker([&]{
        nfs::current_worker_control()=&control;
        begin=steady_ns();
        try{while(frames.load()<31){nfs_wait_frame_slot();++frames;}
            end=steady_ns();
            while(true)nfs_wait_frame_slot();
        }catch(const nfs::WorkerStopped &){stopped=true;}
        nfs::current_worker_control()=nullptr;
    });
    const auto deadline=steady_ns()+3000000000ull;
    while(frames.load()<31 && steady_ns()<deadline)SDL_Delay(1);
    {std::lock_guard<std::mutex> lock(control.mutex);control.stop=true;control.changed.notify_all();}
    worker.join();
    const double duration=double(end-begin)/1e9;
    const bool protected_waits=protected_stop(true) && protected_stop(false);
    nfs_set_frame_limit(0);nfs_set_frame_limit(60);
    const bool passed=protected_waits && frames==31 && stopped && duration>=.99 && duration<1.5 && nfs_frame_limit()==0;
    SDL_Quit();
    if(!passed){std::fprintf(stderr,"frame pacing failed: frames=%u seconds=%.3f\n",frames.load(),duration);return 3;}
    std::puts("PASS: actual SDL worker pacing near 30 Hz, main-thread bypass, stop cancellation, held/deferred stop without busy spinning and default uncapped mode");
}
