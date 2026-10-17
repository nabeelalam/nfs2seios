// SPDX-License-Identifier: MIT
#include "FramePresentation.h"
#include "WorkerControl.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <thread>
#include <cstdio>
#include <stdexcept>
int main(){
    if(!SDL_Init(0))return 1;
    nfs_set_frame_limit(30);
    const auto main_start=SDL_GetTicksNS();nfs_wait_frame_slot();nfs_wait_frame_slot();
    if(SDL_GetTicksNS()-main_start>15000000)return 2;
    std::atomic<unsigned> frames{0};std::atomic<bool> stopped{false};
    nfs::WorkerControl control;
    uint64_t begin=0,end=0;
    std::thread worker([&]{
        nfs::current_worker_control()=&control;
        begin=SDL_GetTicksNS();
        try{while(frames.load()<31){nfs_wait_frame_slot();++frames;}
            end=SDL_GetTicksNS();
            while(true)nfs_wait_frame_slot();
        }catch(const nfs::WorkerStopped &){stopped=true;}
        nfs::current_worker_control()=nullptr;
    });
    const auto deadline=SDL_GetTicksNS()+3000000000ull;
    while(frames.load()<31 && SDL_GetTicksNS()<deadline)SDL_Delay(1);
    control.stop=true;worker.join();
    const double duration=double(end-begin)/1e9;
    nfs_set_frame_limit(0);nfs_set_frame_limit(60);
    const bool passed=frames==31 && stopped && duration>=.99 && duration<1.5 && nfs_frame_limit()==0;
    SDL_Quit();
    if(!passed){std::fprintf(stderr,"frame pacing failed: frames=%u seconds=%.3f\n",frames.load(),duration);return 3;}
    std::puts("PASS: actual SDL worker pacing near 30 Hz, main-thread bypass, stop cancellation and default uncapped mode");
}
