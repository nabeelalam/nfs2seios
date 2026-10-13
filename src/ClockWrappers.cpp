// SPDX-License-Identifier: MIT
#include "ClockWrappers.h"
#include "GuestAccess.h"
#include <mutex>
#include <chrono>
namespace {
std::mutex periods_mutex;
uint32_t periods[1001]{};
}
extern "C" int32_t SDL_GetTicks_wrap(){return int32_t(uint32_t(nfs_native_ticks()));}
extern "C" int32_t SDL_Delay_wrap(int32_t milliseconds){
    auto *control=nfs::current_worker_control();
    if(!control || !milliseconds)nfs_native_delay(uint32_t(milliseconds));
    else {std::unique_lock<std::mutex> lock(control->mutex);control->changed.wait_for(lock,std::chrono::milliseconds(uint32_t(milliseconds)),[]{return nfs::worker_stop_ready();});}
    nfs::check_worker_stop();return 0;
}
extern "C" int32_t time_wrap(int32_t output){
    const int32_t seconds=int32_t(uint32_t(nfs_native_unix_seconds()));
    if(output)guest_store<int32_t>(uint32_t(output),seconds);
    return seconds;
}
extern "C" int32_t timeGetDevCaps_wrap(int32_t output,int32_t size){
    if(!output || size<8)return 97;
    const struct {uint32_t minimum,maximum;} caps{1,1000};
    guest_store(uint32_t(output),caps);return 0;
}
extern "C" int32_t timeBeginPeriod_wrap(int32_t period){
    if(period<1 || period>1000)return 97;
    std::lock_guard<std::mutex> lock(periods_mutex);
    if(periods[period]==UINT32_MAX)return 97;
    ++periods[period];return 0;
}
extern "C" int32_t timeEndPeriod_wrap(int32_t period){
    if(period<1 || period>1000)return 97;
    std::lock_guard<std::mutex> lock(periods_mutex);
    if(!periods[period])return 97;
    --periods[period];return 0;
}
