// SPDX-License-Identifier: MIT
#include "EngineThreadMetrics.h"
#include <chrono>
#include <future>
#include <cstdio>
#include <stdexcept>
static int64_t burn_cpu(){
    const auto begin=nfs::current_thread_cpu_ns();
    const auto limit=std::chrono::steady_clock::now()+std::chrono::seconds(4);
    volatile uint64_t value=1;
    for(;;){
        for(unsigned i=0;i<1024;++i)value=value*1664525u+1013904223u;
        const auto now=nfs::current_thread_cpu_ns();
        if(now<begin)throw std::runtime_error("Thread CPU clock unavailable/regressed");
        if(now-begin>=10000000)return now-begin;
        if(std::chrono::steady_clock::now()>limit)throw std::runtime_error("Bounded thread CPU burn timed out");
    }
}
int main(){try{
    if(nfs::current_thread_cpu_ns()<0){std::puts("SKIP current-thread CPU provider unavailable");return 77;}
    const auto own_burn=burn_cpu();
    const auto before=nfs::current_thread_cpu_ns();
    auto child=std::async(std::launch::async,burn_cpu);
    if(child.wait_for(std::chrono::seconds(5))!=std::future_status::ready)throw std::runtime_error("Other-thread clock fixture timed out");
    const auto child_burn=child.get();const auto owner_delta=nfs::current_thread_cpu_ns()-before;
    if(own_burn<10000000 || child_burn<10000000 || owner_delta<0 || owner_delta>=child_burn)throw std::runtime_error("Provider included other worker CPU");
    std::printf("PASS actual current-thread provider:owner_burn_ns=%lld child_burn_ns=%lld waiting_owner_ns=%lld;host evidence only\n",(long long)own_burn,(long long)child_burn,(long long)owner_delta);
    return 0;
}catch(const std::exception &error){std::fprintf(stderr,"%s\n",error.what());return 1;}}
