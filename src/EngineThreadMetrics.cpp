// SPDX-License-Identifier: MIT
#include "EngineThreadMetrics.h"
#include "EngineCpuWindow.h"
#include <atomic>
#include <mutex>
#include <limits>
#if defined(__APPLE__)
#include <mach/mach.h>
#include <pthread.h>
#elif defined(__linux__)
#include <time.h>
#endif
#ifdef NFS_ENGINE_METRICS_TEST_PROVIDER
extern "C" int64_t nfs_test_engine_cpu_ns();
#endif
namespace nfs {
int64_t current_thread_cpu_ns(){
#ifdef NFS_ENGINE_METRICS_TEST_PROVIDER
    return nfs_test_engine_cpu_ns();
#elif defined(__APPLE__)
    thread_basic_info_data_t info{};mach_msg_type_number_t count=THREAD_BASIC_INFO_COUNT;
    // This is the pthread's borrowed port; no send right is acquired here.
    const auto thread=pthread_mach_thread_np(pthread_self());
    if(thread_info(thread,THREAD_BASIC_INFO,reinterpret_cast<thread_info_t>(&info),&count)!=KERN_SUCCESS || count!=THREAD_BASIC_INFO_COUNT)return -1;
    const auto user=info.user_time,system=info.system_time;
    if(user.seconds<0 || system.seconds<0 || user.microseconds<0 || user.microseconds>=1000000 || system.microseconds<0 || system.microseconds>=1000000)return -1;
    return (int64_t(user.seconds)+system.seconds)*1000000000ll+(int64_t(user.microseconds)+system.microseconds)*1000;
#elif defined(__linux__)
    timespec value{};
    if(clock_gettime(CLOCK_THREAD_CPUTIME_ID,&value) || value.tv_sec<0 || value.tv_nsec<0 || value.tv_nsec>=1000000000)return -1;
    if(uint64_t(value.tv_sec)>uint64_t((std::numeric_limits<int64_t>::max()-value.tv_nsec)/1000000000ll))return -1;
    return int64_t(value.tv_sec)*1000000000ll+value.tv_nsec;
#else
    return -1;
#endif
}
}
namespace {
std::atomic<bool> capture{false};
std::atomic<uint64_t> generation{1};
std::mutex publication;
int64_t latest_ns=-1;
uint64_t sample_id=0;
thread_local bool owner=false;
thread_local nfs::EngineCpuWindow window;
void reset_locked(){generation.fetch_add(1,std::memory_order_release);latest_ns=-1;}
}
extern "C" void nfs_reset_engine_metrics(){std::lock_guard<std::mutex> lock(publication);reset_locked();}
extern "C" void nfs_set_engine_metrics_capture(int enabled){
    capture.store(false,std::memory_order_release);
    {std::lock_guard<std::mutex> lock(publication);reset_locked();}
    capture.store(enabled!=0,std::memory_order_release);
}
extern "C" void nfs_engine_metrics_owner_begin(){owner=true;window.reset();nfs_reset_engine_metrics();}
extern "C" void nfs_engine_metrics_owner_end(){if(owner){owner=false;window.reset();nfs_reset_engine_metrics();}}
extern "C" void nfs_engine_frame_boundary(){
    // Hidden diagnostics and non-owner callers never query a clock or lock.
    if(!capture.load(std::memory_order_acquire) || !owner)return;
    const auto epoch=generation.load(std::memory_order_acquire);
    if(!window.frame_due(epoch))return;
    const auto result=window.observe(nfs::current_thread_cpu_ns());
    // The clock read occurs outside this mutex: reset can invalidate an in-flight
    // sample without waiting for a platform query or publishing stale values.
    std::lock_guard<std::mutex> lock(publication);
    if(capture.load(std::memory_order_acquire) && generation.load(std::memory_order_acquire)==epoch){latest_ns=result;++sample_id;}
}
extern "C" double nfs_engine_cpu_sample(uint64_t *id){
    if(id)*id=0;
    if(!capture.load(std::memory_order_acquire))return -1;
    std::lock_guard<std::mutex> lock(publication);
    if(id)*id=sample_id;
    return latest_ns<0?-1:double(latest_ns)/1000000.0;
}
extern "C" double nfs_engine_cpu_ms_per_frame(){return nfs_engine_cpu_sample(nullptr);}
