// SPDX-License-Identifier: MIT
#include "MainThreadBridge.h"
#include <exception>
#include <stdexcept>
namespace nfs {
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
void MainThreadBridge::wake_renderer(){std::lock_guard<std::mutex> lock(renderer_mutex);renderer_changed.notify_all();}
void MainThreadBridge::execute_renderer(MainTask task,void *context){
    if(!task)throw std::invalid_argument("Missing renderer operation");
    for(;;){
        check_worker_stop();
        {
            std::unique_lock<std::mutex> lock(renderer_mutex);
            if(renderer_suspended && std::this_thread::get_id()==owner)throw std::logic_error("Main thread cannot wait on suspended graphics");
            renderer_changed.wait(lock,[&]{return !renderer_suspended || worker_stop_ready();});
        }
        check_worker_stop();
        struct Pending {MainThreadBridge *bridge;MainTask task;void *context;bool ran=false;} pending{this,task,context};
        execute([](void *value){
            auto &p=*static_cast<Pending *>(value);
            {std::lock_guard<std::mutex> lock(p.bridge->renderer_mutex);if(p.bridge->renderer_suspended)return;}
            p.ran=true;p.task(p.context);
        },&pending);
        if(pending.ran)return;
    }
}
static MainThreadBridge *active=nullptr;
static void wake(){if(active)active->wake_renderer();}
void bind_main_thread_bridge(MainThreadBridge *bridge){active=bridge;renderer_worker_wake()=bridge?wake:nullptr;}
void run_on_main(MainTask task,void *context){if(!active)throw std::logic_error("Main-thread runtime unbound");active->execute(task,context);}
void run_renderer_on_main(MainTask task,void *context){if(!active)throw std::logic_error("Main-thread runtime unbound");active->execute_renderer(task,context);}
void suspend_renderer(bool value){if(!active)throw std::logic_error("Main-thread runtime unbound");active->suspend_renderer(value);}
}
