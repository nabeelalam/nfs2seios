// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <exception>
#include <stdexcept>
#include <atomic>
#include <chrono>
#include <algorithm>
namespace nfs {
namespace {
using Clock=std::chrono::steady_clock;
std::atomic<uint64_t> render_calls{0},render_queue_ns{0},render_execution_ns{0};
uint64_t elapsed_ns(Clock::time_point start,Clock::time_point end){
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count());
}
}
RendererTiming take_renderer_timing(){
    RendererTiming result;
    result.calls=render_calls.exchange(0,std::memory_order_relaxed);
    result.queue_ns=render_queue_ns.exchange(0,std::memory_order_relaxed);
    result.execution_ns=render_execution_ns.exchange(0,std::memory_order_relaxed);
    return result;
}
struct MainThreadBridge::Command {
    std::function<int32_t()> operation;
    bool executing=false,completed=false;
    int32_t result=0;
    std::exception_ptr failure;
    explicit Command(std::function<int32_t()> task):operation(std::move(task)){}
};
std::shared_ptr<MainThreadBridge::Command> MainThreadBridge::append_command(std::function<int32_t()> operation){
    if(!operation)throw std::invalid_argument("Missing queued renderer operation");
    auto command=std::make_shared<Command>(std::move(operation));
    for(;;){
        check_worker_stop();
        std::shared_ptr<Command> target;
        {
            std::unique_lock<std::mutex> lock(command_mutex);
            // Bound pending storage even when several producers submit at once.
            if(renderer_retiring.load()){lock.unlock();command->operation={};command->completed=true;return command;}
            if(commands.size()<128){commands.push_back(command);return command;}
            target=commands.back();
        }
        finish_command(target,false);
    }
}
void MainThreadBridge::drain_commands(const std::shared_ptr<Command> &target){
    if(std::this_thread::get_id()!=owner)throw std::logic_error("Queued renderer outside owner thread");
    std::exception_ptr failure;
    for(;;){
        std::shared_ptr<Command> next;
        std::function<int32_t()> operation;
        {
            std::lock_guard<std::mutex> lock(command_mutex);
            if(target->completed)return;
            if(commands.empty())throw std::logic_error("Renderer command missing from queue");
            next=commands.front();commands.pop_front();next->executing=true;
            operation=std::move(next->operation);
        }
        int32_t result=0;
        if(!failure){
            ++command_depth;
            try{result=operation();}catch(...){failure=std::current_exception();}
            --command_depth;
        }
        // Retire captures before waking a caller whose buffers they may borrow.
        operation={};
        {
            std::lock_guard<std::mutex> lock(command_mutex);
            next->result=result;next->failure=failure;
            next->executing=false;next->completed=true;
        }
        command_changed.notify_all();
    }
}
void MainThreadBridge::finish_command(const std::shared_ptr<Command> &target,bool borrowed){
    {
        std::lock_guard<std::mutex> lock(command_mutex);
        if(target->completed){if(target->failure)std::rethrow_exception(target->failure);return;}
    }
    struct Drain {MainThreadBridge *bridge;const std::shared_ptr<Command> &target;} drain{this,target};
    try{
        execute_renderer([](void *p){auto &r=*static_cast<Drain *>(p);r.bridge->drain_commands(r.target);},&drain);
    }catch(...){
        auto failure=std::current_exception();
        std::function<int32_t()> retired;
        {
            std::unique_lock<std::mutex> lock(command_mutex);
            if(borrowed && !target->completed && !target->executing){
                auto position=std::find(commands.begin(),commands.end(),target);
                if(position!=commands.end())commands.erase(position);
                retired=std::move(target->operation);target->completed=true;
            }
            // Another producer's drain can already be using this borrowed task.
            // Cancellation must await its completion before releasing the stack.
            if(borrowed)command_changed.wait(lock,[&]{return target->completed;});
            if(target->completed && target->failure)failure=target->failure;
        }
        retired={};command_changed.notify_all();std::rethrow_exception(failure);
    }
    std::exception_ptr failure;
    {std::lock_guard<std::mutex> lock(command_mutex);if(!target->completed)throw std::logic_error("Renderer drain returned before command completion");failure=target->failure;}
    if(failure)std::rethrow_exception(failure);
}
int32_t MainThreadBridge::renderer_barrier(std::function<int32_t()> operation){
    check_worker_stop();
    // Nested owner calls retain the existing immediate semantics. They cannot
    // interleave later queued work inside an in-flight native operation.
    if(std::this_thread::get_id()==owner && command_depth)return operation();
    auto command=append_command(std::move(operation));finish_command(command,true);
    return command->result;
}
void MainThreadBridge::enqueue_renderer(std::function<int32_t()> operation){
    check_worker_stop();
    if(std::this_thread::get_id()==owner){
        if(command_depth){operation();return;}
        auto command=append_command(std::move(operation));finish_command(command,true);return;
    }
    append_command(std::move(operation));
}
OwnedRendererAcceptance MainThreadBridge::enqueue_owned_renderer(std::function<int32_t()> operation){
    if(std::this_thread::get_id()==owner)return OwnedRendererAcceptance::UseExistingOwnerPath;
    append_command(std::move(operation));
    // No stop check after append: ownership was consumed, including retirement.
    return OwnedRendererAcceptance::Consumed;
}
void MainThreadBridge::execute(MainTask task,void *context){
    if(!task)throw std::invalid_argument("Missing main-thread operation");
    struct Request {MainTask task;void *context;std::thread::id owner;bool completed=false;std::exception_ptr failure;};
    Request request{task,context,owner};
    const auto checked=[](void *p){
        auto &r=*static_cast<Request *>(p);
        try{
            if(r.completed)throw std::logic_error("Repeated main-thread operation");
            r.completed=true;
            if(std::this_thread::get_id()!=r.owner)throw std::logic_error("Operation ran outside main thread");
            r.task(r.context);
        }catch(...){r.failure=std::current_exception();}
    };
    if(std::this_thread::get_id()==owner)checked(&request);
    else{if(!scheduler)throw std::logic_error("Main-thread scheduler unbound");scheduler(checked,&request);}
    if(!request.completed)throw std::logic_error("Main-thread scheduler returned before operation");
    if(request.failure)std::rethrow_exception(request.failure);
}
void MainThreadBridge::suspend_renderer(bool value){
    if(std::this_thread::get_id()!=owner)throw std::logic_error("Renderer suspension needs main-thread owner");
    std::lock_guard<std::mutex> lock(renderer_mutex);renderer_suspended=value;renderer_changed.notify_all();
}
void MainThreadBridge::retire_renderer(){
    if(std::this_thread::get_id()!=owner || command_depth)throw std::logic_error("Retirement requires idle renderer owner");
    std::deque<std::shared_ptr<Command>> retired;
    {
        std::lock_guard<std::mutex> lock(command_mutex);
        renderer_retiring.store(true);
        retired.swap(commands);
        // Borrowed cancellation may not steal captures during retirement.
        for(auto &command:retired)command->executing=true;
    }
    // Captures can have reentrant destructors; release outside bridge locks.
    for(auto &command:retired)command->operation={};
    {
        std::lock_guard<std::mutex> lock(command_mutex);
        for(auto &command:retired){command->result=0;command->executing=false;command->completed=true;}
        renderer_retired.store(true);
    }
    command_changed.notify_all();wake_renderer();
}
void MainThreadBridge::wake_renderer(){std::lock_guard<std::mutex> lock(renderer_mutex);renderer_changed.notify_all();}
void MainThreadBridge::execute_renderer(MainTask task,void *context){
    if(!task)throw std::invalid_argument("Missing renderer operation");
    for(;;){
        check_worker_stop();
        {
            std::unique_lock<std::mutex> lock(renderer_mutex);
            if(renderer_suspended && std::this_thread::get_id()==owner)throw std::logic_error("Main thread cannot wait on suspended graphics");
            renderer_changed.wait(lock,[&]{return renderer_retired.load() || !renderer_suspended || worker_stop_ready();});
        }
        check_worker_stop();
        if(renderer_retired.load())return;
        struct Pending {MainThreadBridge *bridge;MainTask task;void *context;bool ran=false;Clock::time_point submitted=Clock::now();} pending{this,task,context};
        execute([](void *value){
            auto &p=*static_cast<Pending *>(value);
            {std::lock_guard<std::mutex> lock(p.bridge->renderer_mutex);if(p.bridge->renderer_suspended)return;}
            const auto started=Clock::now();
            p.ran=true;p.task(p.context);
            const auto finished=Clock::now();
            render_queue_ns.fetch_add(elapsed_ns(p.submitted,started),std::memory_order_relaxed);
            render_execution_ns.fetch_add(elapsed_ns(started,finished),std::memory_order_relaxed);
            render_calls.fetch_add(1,std::memory_order_relaxed);
        },&pending);
        if(pending.ran)return;
    }
}
static MainThreadBridge *active=nullptr;
static void wake(){if(active)active->wake_renderer();}
void bind_main_thread_bridge(MainThreadBridge *bridge){active=bridge;renderer_worker_wake()=bridge?wake:nullptr;}
void run_on_main(MainTask task,void *context){if(!active)throw std::logic_error("Main-thread runtime unbound");active->execute(task,context);}
int32_t renderer_barrier(std::function<int32_t()> operation){if(!active)throw std::logic_error("Main-thread runtime unbound");return active->renderer_barrier(std::move(operation));}
void enqueue_renderer(std::function<int32_t()> operation){if(!active)throw std::logic_error("Main-thread runtime unbound");active->enqueue_renderer(std::move(operation));}
OwnedRendererAcceptance enqueue_owned_renderer(std::function<int32_t()> operation){
    if(!active)throw std::logic_error("Main-thread runtime unbound");
    return active->enqueue_owned_renderer(std::move(operation));
}
void flush_renderer_commands(){renderer_barrier([]{return 0;});}
void run_renderer_on_main(MainTask task,void *context){if(!task)throw std::invalid_argument("Missing renderer operation");renderer_barrier([=]{task(context);return 0;});}
void suspend_renderer(bool value){if(!active)throw std::logic_error("Main-thread runtime unbound");active->suspend_renderer(value);}
}
