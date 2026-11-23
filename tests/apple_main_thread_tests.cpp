// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include "MainWaitTiming.h"
#include <dispatch/dispatch.h>
#include <chrono>
#include <cstdlib>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <atomic>
#include <future>
#include <cstdio>
#include <stdexcept>
int main(){
    if(!pthread_main_np())return 1;
    nfs::set_main_wait_capture(true);
    nfs::wait_for_main_work(false);
    const auto ordinary=nfs::take_main_wait_timing();
    if(ordinary.calls!=1 || ordinary.finished+ordinary.stopped+ordinary.timed_out+
       ordinary.handled_source+ordinary.other!=1 || ordinary.max_ns>ordinary.elapsed_ns){
        std::fprintf(stderr,"FAIL: actual Apple wait recording\n");return 1;
    }
    // The recording timer can run inside the call being measured. Taking its
    // sample must invalidate that outer call rather than charge it twice.
    bool sampled=false;
    dispatch_async_f(dispatch_get_main_queue(),&sampled,[](void *context){
        nfs::take_main_wait_timing();*static_cast<bool *>(context)=true;
    });
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!sampled){
        if(std::chrono::steady_clock::now()>deadline)std::_Exit(2);
        nfs::wait_for_main_work(false);
    }
    if(nfs::take_main_wait_timing().calls!=0){
        std::fprintf(stderr,"FAIL: reentrant Apple sample boundary\n");return 1;
    }
    nfs::set_main_wait_capture(false);
    nfs::wait_for_main_work(true);
    if(nfs::take_main_wait_timing().calls!=0){
        std::fprintf(stderr,"FAIL: disabled Apple wait recording\n");return 1;
    }
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
    bool capped=false;
    while(!finished){
        if(std::chrono::steady_clock::now()>deadline)std::_Exit(2);
        nfs::wait_for_main_work(capped);
        capped=!capped;
    }
    worker.join();const bool passed=result.get_future().get();nfs::bind_main_thread_bridge(nullptr);
    if(!passed){std::fprintf(stderr,"FAIL: Apple main queue operation\n");return 1;}
    std::puts("PASS: actual Apple main queue, pthread main identity, 100 borrowed-buffer results, nested dispatch and callback exceptions");
}
