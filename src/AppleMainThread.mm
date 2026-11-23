// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <exception>
#include <CoreFoundation/CoreFoundation.h>
#include <chrono>
#include "MainWaitTiming.h"
namespace nfs {
static_assert(kCFRunLoopRunFinished==1 && kCFRunLoopRunStopped==2 &&
              kCFRunLoopRunTimedOut==3 && kCFRunLoopRunHandledSource==4,
              "Diagnostic return-reason mapping must match CoreFoundation");
static MainWaitCapture wait_capture;
void set_main_wait_capture(bool enabled){wait_capture.enable(enabled);}
MainWaitTiming take_main_wait_timing(){return wait_capture.take();}
static void schedule(MainTask task,void *context){
    if(pthread_main_np()){task(context);return;}
    struct Request {MainTask task;void *context;std::exception_ptr failure;} request{task,context};
    dispatch_sync_f(dispatch_get_main_queue(),&request,[](void *p){
        auto &r=*static_cast<Request *>(p);
        try{r.task(r.context);}catch(...){r.failure=std::current_exception();}
    });
    if(request.failure)std::rethrow_exception(request.failure);
}
MainScheduler apple_main_thread_scheduler(){return schedule;}
void wait_for_main_work(bool capped_frame_rate){
    // UIKit's SDL backend has no native event wait. SDL's fallback sleeps,
    // and a GCD renderer submission cannot wake that sleep. Service the run
    // loop instead: queued synchronous graphics work wakes it immediately.
    // The optional 30 FPS lock leaves longer idle gaps. Continue to poll
    // queued SDL events with a small bound even if no run-loop source fires.
    const CFTimeInterval maximum_wait = capped_frame_rate ? 0.004 : 0.001;
    const auto token=wait_capture.begin();
    if(!token){CFRunLoopRunInMode(kCFRunLoopDefaultMode,maximum_wait,true);return;}
    const auto start=std::chrono::steady_clock::now();
    const auto reason=CFRunLoopRunInMode(kCFRunLoopDefaultMode,maximum_wait,true);
    const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now()-start).count();
    if(elapsed>=0)wait_capture.observe(token,uint64_t(elapsed),int(reason));
}
}
