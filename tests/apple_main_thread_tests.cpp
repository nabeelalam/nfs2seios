// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <atomic>
#include <future>
#include <cstdio>
#include <stdexcept>
int main(){
    if(!pthread_main_np())return 1;
    nfs::MainThreadBridge bridge(nfs::apple_main_thread_scheduler());nfs::bind_main_thread_bridge(&bridge);
    std::atomic<bool> finished{false};std::promise<bool> result;
    std::thread worker([&]{
        bool passed=true;
        try{
            for(int i=0;i<100;++i){int borrowed=i;const auto output=nfs::renderer_operation([&]{
                if(!pthread_main_np())throw std::runtime_error("GCD operation outside main thread");
                return nfs::renderer_operation([&]{return borrowed+1;});
            });if(output!=i+1)passed=false;}
            bool captured=false;try{nfs::renderer_operation([]()->int32_t{throw std::runtime_error("Apple callback fixture");});}catch(const std::runtime_error &){captured=true;}
            passed=passed && captured;
        }catch(...){passed=false;}
        result.set_value(passed);finished=true;
    });
    // Service the actual Apple main queue, not an SDL pump or test scheduler.
    while(!finished)CFRunLoopRunInMode(kCFRunLoopDefaultMode,0.01,true);
    worker.join();const bool passed=result.get_future().get();nfs::bind_main_thread_bridge(nullptr);
    if(!passed){std::fprintf(stderr,"FAIL: Apple main queue operation\n");return 1;}
    std::puts("PASS: actual Apple main queue, pthread main identity, 100 borrowed-buffer results, nested dispatch and callback exceptions");
}
