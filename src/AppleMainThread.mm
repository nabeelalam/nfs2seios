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
void wait_for_main_work(){
    // UIKit's SDL backend has no native event wait. SDL's fallback sleeps,
    // and a GCD renderer submission cannot wake that sleep. Service the run
    // loop instead: queued synchronous graphics work wakes it immediately.
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,0.001,true);
}
}
