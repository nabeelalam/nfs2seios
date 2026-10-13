// SPDX-License-Identifier: MIT
#pragma once
#include <thread>
#include <stdint.h>
#include "WorkerControl.h"
namespace nfs {
using MainTask=void(*)(void *);
using MainScheduler=void(*)(MainTask,void *);
// Construct and bind on the app's main thread before starting game workers.
// A scheduler must execute synchronously, exactly once, on that owner thread.
class MainThreadBridge {
    const std::thread::id owner;
    MainScheduler scheduler;
    std::mutex renderer_mutex;
    std::condition_variable renderer_changed;
    bool renderer_suspended=false;
public:
    explicit MainThreadBridge(MainScheduler schedule):owner(std::this_thread::get_id()),scheduler(schedule){}
    void execute(MainTask task,void *context);
    void execute_renderer(MainTask task,void *context);
    void suspend_renderer(bool suspended);
    void wake_renderer();
};
void bind_main_thread_bridge(MainThreadBridge *bridge);
void run_on_main(MainTask task,void *context);
void run_renderer_on_main(MainTask task,void *context);
void suspend_renderer(bool suspended);
MainScheduler sdl_main_thread_scheduler();
MainScheduler apple_main_thread_scheduler();
// Yield without putting the iOS main dispatch queue behind an SDL sleep.
void wait_for_main_work();
struct RendererTiming {uint64_t calls=0,queue_ns=0,execution_ns=0;};
RendererTiming take_renderer_timing();
template<class F> int32_t renderer_operation(F operation){
    check_worker_stop();
    struct Request {F &operation;int32_t result=0;} request{operation};
    run_renderer_on_main([](void *p){auto &r=*static_cast<Request *>(p);r.result=r.operation();},&request);
    // Do not unwind/free borrowed guest buffers while native work is pending.
    check_worker_stop();return request.result;
}
}
