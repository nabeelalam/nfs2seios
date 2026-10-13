// SPDX-License-Identifier: MIT
#include "TimerBridge.h"
#include <condition_variable>
#include <unordered_map>
#include <atomic>
#include <stdexcept>
#include <exception>
namespace nfs {
struct TimerBridge::State {
    std::mutex mutex,tick_mutex;
    std::condition_variable changed;
    Tick tick;void *context;
    uintptr_t token;uint32_t backend=0,running=0;
    bool active=true;
    std::exception_ptr failure;
    State(Tick fn,void *p,uintptr_t id):tick(fn),context(p),token(id){}
};
static std::mutex registry_mutex;
static std::unordered_map<uintptr_t,std::shared_ptr<TimerBridge::State>> registry;
static std::atomic<uintptr_t> next_token{1};
static thread_local uintptr_t callback_token=0;
static uintptr_t allocate_token(){
    uintptr_t value=next_token.load();
    while(value){if(next_token.compare_exchange_weak(value,value==UINTPTR_MAX?0:value+1))return value;}
    throw std::overflow_error("Native timer token space exhausted");
}
void TimerBridge::start(Tick tick,void *context){
    if(callback_token)throw std::logic_error("Timer callbacks cannot start timers");
    if(!tick)throw std::invalid_argument("Timer callback required");
    std::lock_guard<std::mutex> lock(operation);
    if(state)throw std::logic_error("Game timer already started");
    auto pending=std::make_shared<State>(tick,context,allocate_token());
    {std::lock_guard<std::mutex> guard(registry_mutex);if(registry.size()==256)throw std::out_of_range("Native timer capacity");registry.emplace(pending->token,pending);}
    // Registration precedes native creation: an immediate callback is valid.
    state=pending;
    try {pending->backend=nfs_native_add_timer(pending->token);}
    catch (...) {
        {std::lock_guard<std::mutex> guard(registry_mutex);std::lock_guard<std::mutex> state_lock(pending->mutex);pending->active=false;registry.erase(pending->token);}
        std::unique_lock<std::mutex> state_lock(pending->mutex);
        pending->changed.wait(state_lock,[&]{return !pending->running;});state.reset();throw;
    }
    if(!pending->backend){
        {std::lock_guard<std::mutex> guard(registry_mutex);std::lock_guard<std::mutex> state_lock(pending->mutex);pending->active=false;registry.erase(pending->token);}
        std::unique_lock<std::mutex> state_lock(pending->mutex);
        pending->changed.wait(state_lock,[&]{return !pending->running;});state.reset();
        throw std::runtime_error("Native game timer creation failed");
    }
}
void TimerBridge::stop(){
    if(callback_token)throw std::logic_error("Timer callbacks cannot drain timers");
    std::lock_guard<std::mutex> lock(operation);
    auto stopping=state;if(!stopping)return;
    if(callback_token==stopping->token)throw std::logic_error("Timer callback cannot drain itself");
    {
        std::lock_guard<std::mutex> guard(registry_mutex);
        std::lock_guard<std::mutex> state_lock(stopping->mutex);
        stopping->active=false;registry.erase(stopping->token);
    }
    nfs_native_remove_timer(stopping->backend);
    std::unique_lock<std::mutex> state_lock(stopping->mutex);
    stopping->changed.wait(state_lock,[&]{return !stopping->running;});
    auto failure=stopping->failure;state.reset();
    if(failure)std::rethrow_exception(failure);
}
uint32_t GameTimerSchedule::next(uint32_t step,uint32_t now){
    if(step==655360)return 1;
    const uint32_t sum=fraction+step;
    fraction=uint16_t(sum);scheduled+=sum>>16;
    const uint32_t bits=scheduled-now-1;
    const int64_t delay=bits<=INT32_MAX?int64_t(bits):int64_t(bits)-0x100000000ll;
    if(delay<1){if(delay<-1000)scheduled=now+1;return 1;}
    return uint32_t(delay);
}
}
extern "C" uint32_t nfs_timer_dispatch(uintptr_t token){
    std::shared_ptr<nfs::TimerBridge::State> current;
    {
        std::lock_guard<std::mutex> guard(nfs::registry_mutex);
        auto it=nfs::registry.find(token);if(it==nfs::registry.end())return 0;current=it->second;
        std::lock_guard<std::mutex> lock(current->mutex);if(!current->active)return 0;++current->running;
    }
    uint32_t interval=0;
    const uintptr_t previous=nfs::callback_token;nfs::callback_token=token;
    try {
        std::lock_guard<std::mutex> tick_lock(current->tick_mutex);
        bool active;{std::lock_guard<std::mutex> lock(current->mutex);active=current->active;}
        if(active)interval=current->tick(current->context);
    }catch(...){std::lock_guard<std::mutex> lock(current->mutex);if(!current->failure)current->failure=std::current_exception();current->active=false;}
    nfs::callback_token=previous;
    {std::lock_guard<std::mutex> lock(current->mutex);--current->running;current->changed.notify_all();}
    return interval;
}
