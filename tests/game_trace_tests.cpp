// SPDX-License-Identifier: MIT
#include "GameTrace.h"
#include <thread>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <cstdio>
int main(){
    nfs_trace_guest_call(0xf0001111,0x10002222);
    if(nfs::sampled_game_calls().find("0xf0001111@0x10002222")==std::string::npos)return 1;
    // More than the fixed slot capacity must remain safe; leases are reusable.
    for(unsigned round=0;round<2;++round){
        std::atomic<unsigned> ready{0};std::atomic<bool> release{false};std::vector<std::thread> workers;
        for(unsigned i=0;i<24;++i)workers.emplace_back([&,i]{
            for(unsigned n=0;n<1000;++n)nfs_trace_guest_call(0xf0010000+i,0x10000000+i);
            ++ready;while(!release.load())std::this_thread::yield();
        });
        while(ready.load()!=24)std::this_thread::yield();
        const auto sample=nfs::sampled_game_calls();
        const bool present=sample.find("0xf001")!=std::string::npos;
        release=true;for(auto &worker:workers)worker.join();
        if(!present || nfs::sampled_game_calls().find("0xf001")!=std::string::npos)return 2;
    }
    std::puts("PASS: bounded cross-thread original call samples, overflow and slot reuse");
}
