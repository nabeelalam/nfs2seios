// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <exception>
#include <CoreFoundation/CoreFoundation.h>
namespace nfs {
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
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,maximum_wait,true);
}
}
