// SPDX-License-Identifier: MIT
#pragma once
#include <thread>
#include <stdint.h>
#include <functional>
#include <memory>
#include <deque>
#include "WorkerControl.h"
namespace nfs {
using MainTask=void(*)(void *);
using MainScheduler=void(*)(MainTask,void *);
// Construct and bind on the app's main thread before starting game workers.
// A scheduler must execute synchronously, exactly once, on that owner thread.
class MainThreadBridge {
    struct Command;
    const std::thread::id owner;
    MainScheduler scheduler;
    std::mutex renderer_mutex;
    std::condition_variable renderer_changed;
    bool renderer_suspended=false;
    std::mutex command_mutex;
    std::condition_variable command_changed;
    std::deque<std::shared_ptr<Command>> commands;
    unsigned command_depth=0; // Accessed only by the renderer owner.
    std::shared_ptr<Command> append_command(std::function<int32_t()> operation);
    void drain_commands(const std::shared_ptr<Command> &target);
    void finish_command(const std::shared_ptr<Command> &target,bool borrowed);
public:
    explicit MainThreadBridge(MainScheduler schedule):owner(std::this_thread::get_id()),scheduler(schedule){}
    void execute(MainTask task,void *context);
    void execute_renderer(MainTask task,void *context);
    void suspend_renderer(bool suspended);
    void wake_renderer();
    int32_t renderer_barrier(std::function<int32_t()> operation);
    void enqueue_renderer(std::function<int32_t()> operation);
};
void bind_main_thread_bridge(MainThreadBridge *bridge);
void run_on_main(MainTask task,void *context);
void run_renderer_on_main(MainTask task,void *context);
void suspend_renderer(bool suspended);
int32_t renderer_barrier(std::function<int32_t()> operation);
void enqueue_renderer(std::function<int32_t()> operation);
void flush_renderer_commands();
MainScheduler sdl_main_thread_scheduler();
MainScheduler apple_main_thread_scheduler();
// Yield without putting the iOS main dispatch queue behind an SDL sleep.
void wait_for_main_work();
struct RendererTiming {uint64_t calls=0,queue_ns=0,execution_ns=0;};
RendererTiming take_renderer_timing();
template<class F> int32_t renderer_operation(F operation){
    check_worker_stop();
    const auto result=renderer_barrier([&operation]{return operation();});
    // Do not unwind/free borrowed guest buffers while native work is pending.
    check_worker_stop();return result;
}
// Callers must capture every deferred argument by value, including vertices.
// Borrowed buffers, palette reads, output values and presentation use barriers.
template<class F> void renderer_command(F operation){
    check_worker_stop();enqueue_renderer(std::move(operation));check_worker_stop();
}
}
