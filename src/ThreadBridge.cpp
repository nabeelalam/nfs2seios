// SPDX-License-Identifier: MIT
#include "ThreadBridge.h"
#include <atomic>
#include <stdexcept>
namespace nfs {
static std::atomic<uint64_t> next_thread_id{1};
static thread_local uint32_t local_thread_id=0;
static uint32_t allocate_id() {
    const uint64_t id=next_thread_id.fetch_add(1);
    return id<=UINT32_MAX?uint32_t(id):0;
}
uint32_t current_game_thread_id() {
    if(!local_thread_id) local_thread_id=allocate_id();
    return local_thread_id;
}
bool ThreadBridge::valid(uint32_t id) const {
    return (id&0xff000000)==0xc0000000 && slots[id&255].worker &&
        slots[id&255].generation==((id>>8)&65535);
}
void ThreadBridge::run(std::shared_ptr<Worker> worker) {
    local_thread_id=worker->thread_id;
    current_worker_control()=&worker->control;
    {
        std::unique_lock<std::mutex> lock(worker->mutex);
        worker->changed.wait(lock,[&]{return !worker->suspended || worker->cancelled;});
    }
    try {
        // cancelled changes only while suspended and under the worker lock.
        bool cancelled;
        {std::lock_guard<std::mutex> lock(worker->mutex);cancelled=worker->cancelled;}
        if(!cancelled){check_worker_stop();worker->function(worker->context.get());}
    } catch(const WorkerStopped &) {} catch (...) {std::lock_guard<std::mutex> lock(worker->mutex);worker->failure=std::current_exception();}
    try{if(critical_worker_retire())critical_worker_retire()();}catch(...){std::lock_guard<std::mutex> lock(worker->mutex);if(!worker->failure)worker->failure=std::current_exception();}
    current_worker_control()=nullptr;
    worker->context.reset();
    {std::lock_guard<std::mutex> lock(worker->mutex);worker->finished=true;worker->changed.notify_all();}
}
uint32_t ThreadBridge::create(GameFunction function,void *context,void(*destroy)(void *),bool suspended,uint32_t &thread_id) {
    if(!destroy) throw std::invalid_argument("Worker context destructor required");
    std::unique_ptr<void,void(*)(void *)> incoming(context,destroy);
    thread_id=0;
    if(!function || !context || !destroy) return 0;
    std::lock_guard<std::mutex> lock(mutex);
    if(stopping) return 0;
    for(auto it=owned.begin();it!=owned.end();) {
        bool done;
        {std::lock_guard<std::mutex> worker_lock((*it)->mutex);done=(*it)->finished;}
        if(!done){++it;continue;}
        (*it)->thread.join();
        if((*it)->failure && !first_failure) first_failure=(*it)->failure;
        it=owned.erase(it);
    }
    if(owned.size()==256) return 0;
    uint32_t index=256;
    for(uint32_t i=0;i<256;++i) if(!slots[i].worker && slots[i].generation<65535){index=i;break;}
    if(index==256) return 0;
    const uint32_t id=allocate_id();if(!id) return 0;
    auto worker=std::make_shared<Worker>(function,incoming.get(),destroy,id,suspended);
    incoming.release();
    // Retain before spawning: vector allocation failure must not orphan a thread.
    owned.push_back(worker);
    try {worker->thread=std::thread(run,worker);}
    catch (...) {owned.pop_back();throw;}
    auto &slot=slots[index];slot.worker=worker;++slot.generation;
    thread_id=id;
    return 0xc0000000|(slot.generation<<8)|index;
}
uint32_t ThreadBridge::resume(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    if(!valid(id)) return UINT32_MAX;
    auto worker=slots[id&255].worker;
    std::lock_guard<std::mutex> worker_lock(worker->mutex);
    const bool previous=worker->suspended;
    if(worker->cancelled) return UINT32_MAX;
    worker->suspended=false;worker->changed.notify_all();
    return previous?1:0;
}
bool ThreadBridge::close(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    if(!valid(id)) return false;
    slots[id&255].worker.reset();return true;
}
uint32_t ThreadBridge::duplicate(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    if(stopping) return 0;
    std::shared_ptr<Worker> source;
    if(id==0xfffffffeu) {
        const uint32_t current=current_game_thread_id();if(!current) return 0;
        for(auto &slot:slots)if(slot.worker && slot.worker->thread_id==current){source=slot.worker;break;}
        if(!source) for(auto &worker:owned)if(worker->thread_id==current){source=worker;break;}
        if(!source) source=std::make_shared<Worker>(nullptr,nullptr,[](void *){},current,false);
    } else {
        if(!valid(id)) return 0;
        source=slots[id&255].worker;
    }
    for(uint32_t i=0;i<256;++i) {
        auto &slot=slots[i];if(slot.worker || slot.generation==65535) continue;
        slot.worker=source;++slot.generation;
        return 0xc0000000|(slot.generation<<8)|i;
    }
    return 0;
}
bool ThreadBridge::terminate(uint32_t id){
    std::shared_ptr<Worker> worker;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if(id==0xfffffffeu){for(auto &candidate:owned)if(candidate->thread_id==current_game_thread_id()){worker=candidate;break;}}
        else if(valid(id))worker=slots[id&255].worker;
    }
    if(!worker || !worker->function)return false;
    {std::lock_guard<std::mutex> lock(worker->control.mutex);worker->control.stop=true;worker->control.changed.notify_all();}
    {std::lock_guard<std::mutex> lock(worker->mutex);if(worker->suspended)worker->cancelled=true;worker->changed.notify_all();}
    wake_worker_waits();
    if(worker->thread_id==current_game_thread_id()){check_worker_stop();return true;}
    std::unique_lock<std::mutex> lock(worker->mutex);
    worker->changed.wait(lock,[&]{return worker->finished;});return true;
}
void ThreadBridge::join_all() {
    std::vector<std::shared_ptr<Worker>> workers;
    {
        std::lock_guard<std::mutex> lock(mutex);
        for(auto &worker:owned) if(worker->thread.get_id()==std::this_thread::get_id())
            throw std::logic_error("A game worker cannot join itself");
        stopping=true;workers.swap(owned);
        for(auto &slot:slots)slot.worker.reset();
        for(auto &worker:workers){std::lock_guard<std::mutex> worker_lock(worker->mutex);if(worker->suspended){worker->cancelled=true;worker->changed.notify_all();}}
    }
    std::exception_ptr first=first_failure;
    for(auto &worker:workers){
        if(worker->thread.joinable()) worker->thread.join();
        if(worker->failure && !first) first=worker->failure;
    }
    if(first) std::rethrow_exception(first);
}
void ThreadBridge::request_stop_all(){
    {std::lock_guard<std::mutex> guard(mutex);stopping=true;
        for(auto &worker:owned){
            {std::lock_guard<std::mutex> lock(worker->control.mutex);worker->control.stop=true;worker->control.changed.notify_all();}
            {std::lock_guard<std::mutex> lock(worker->mutex);if(worker->suspended)worker->cancelled=true;worker->changed.notify_all();}
        }}
    wake_worker_waits();
}
static ThreadBridge *active=nullptr;
void bind_thread_bridge(ThreadBridge *bridge){active=bridge;}
bool close_thread_handle(uint32_t id){return active && active->close(id);}
ThreadBridge *current_thread_bridge(){return active;}
}
