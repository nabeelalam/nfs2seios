// SPDX-License-Identifier: MIT
#include "AudioBridge.h"
#include "GameContexts.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <stdexcept>
#include <cstring>
#include <algorithm>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
namespace nfs {
struct AudioBridge::State {
    std::mutex mutex,render;
    std::condition_variable changed;
    std::unique_ptr<void,void(*)(void *)> game{nullptr,nfs_destroy_game_context};
    std::unique_ptr<void,void(*)(void *)> buffer{nullptr,free32};
    uint32_t sample_function,fade_function,guest_buffer=0,running=0;
    uintptr_t token;void *backend=nullptr;
    size_t position=1024;
    bool active=true,resumed=false,suspended=false;
    WorkerControl control;
    std::exception_ptr failure;
    State(uint32_t samples,uint32_t fade,uintptr_t id):sample_function(samples),fade_function(fade),token(id){
        game.reset(nfs_create_game_context());if(!game)throw std::bad_alloc();
        buffer.reset(malloc32(1024));if(!buffer)throw std::bad_alloc();
        guest_buffer=nfs_guest_address(buffer.get());
    }
};
static std::mutex registry_mutex;
static std::unordered_map<uintptr_t,std::shared_ptr<AudioBridge::State>> registry;
static std::atomic<uintptr_t> next_token{1};
static thread_local uintptr_t audio_callback=0;
static uintptr_t token(){auto id=next_token.load();while(id){if(next_token.compare_exchange_weak(id,id==UINTPTR_MAX?0:id+1))return id;}throw std::overflow_error("Audio tokens exhausted");}
void AudioBridge::functions(uint32_t sample,uint32_t fade_function){
    if(audio_callback)throw std::logic_error("Audio callback cannot reconfigure audio");
    nfs_lookup_guest_function(sample);nfs_lookup_guest_function(fade_function);
    std::lock_guard<std::mutex> lock(operation);if(state)throw std::logic_error("Audio functions changed while active");samples=sample;fade=fade_function;
}
void AudioBridge::stop(){
    if(audio_callback)throw std::logic_error("Audio callback cannot drain itself");
    std::lock_guard<std::mutex> lock(operation);retire();
}
void AudioBridge::set_suspended(bool value){
    if(audio_callback)throw std::logic_error("Audio callback cannot change app suspension");
    std::lock_guard<std::mutex> lock(operation);suspended=value;
    auto current=state;if(!current)return;
    {std::lock_guard<std::mutex> guard(current->mutex);current->suspended=value;}
    if(value && current->resumed){
        if(!nfs_native_audio_pause(current->backend))throw std::runtime_error("Audio device pause failed");
        current->resumed=false;
    }
    // Resume only when the original service loop next serves audio. Preserve
    // stream, callback context and the current sample-buffer position.
}
void AudioBridge::retire(){
    auto old=state;if(!old)return;
    {std::lock_guard<std::mutex> guard(registry_mutex);std::lock_guard<std::mutex> state_lock(old->mutex);old->active=false;registry.erase(old->token);}
    {std::lock_guard<std::mutex> control_lock(old->control.mutex);old->control.stop=true;old->control.changed.notify_all();}
    wake_worker_waits();
    if(old->backend)nfs_native_audio_close(old->backend);
    std::unique_lock<std::mutex> state_lock(old->mutex);old->changed.wait(state_lock,[&]{return !old->running;});
    auto failure=old->failure;state.reset();state_lock.unlock();old.reset();
    if(failure)std::rethrow_exception(failure);
}
bool AudioBridge::start(){
    if(audio_callback)throw std::logic_error("Audio callback cannot start audio");
    std::unique_lock<std::mutex> lock(operation);
    if(state){std::lock_guard<std::mutex> guard(state->mutex);if(state->failure)std::rethrow_exception(state->failure);return state->active;}
    if(!samples || !fade)throw std::logic_error("Audio functions unbound");
    auto pending=std::make_shared<State>(samples,fade,token());
    pending->suspended=suspended;
    {std::lock_guard<std::mutex> guard(registry_mutex);if(registry.size()==16)throw std::out_of_range("Audio device capacity");registry.emplace(pending->token,pending);}
    state=pending;
    try{pending->backend=nfs_native_audio_open(pending->token);}
    catch(...){try{retire();}catch(...){}throw;}
    if(!pending->backend){retire();return false;}
    return true;
}
void AudioBridge::serve(void *game){
    if(audio_callback)throw std::logic_error("Audio callback cannot service audio");
    if(!game)throw std::invalid_argument("Audio service Game context missing");
    std::shared_ptr<State> current;
    {
        std::lock_guard<std::mutex> lock(operation);current=state;if(!current || suspended)return;
        {std::lock_guard<std::mutex> guard(current->mutex);if(current->failure)std::rethrow_exception(current->failure);if(!current->active)return;}
        if(!current->resumed){if(!nfs_native_audio_resume(current->backend))throw std::runtime_error("Audio device resume failed");current->resumed=true;}
    }
    const uint32_t fade_function=current->fade_function;current.reset();
    nfs_dispatch_game_procedure(game,fade_function);
}
static AudioBridge *active=nullptr;
void bind_audio_bridge(AudioBridge *bridge){active=bridge;}
static AudioBridge &current(){if(!active)throw std::logic_error("Audio runtime unbound");return *active;}
}
extern "C" size_t nfs_audio_render(uintptr_t token,void *output,size_t bytes) noexcept{
    if(!output || !bytes || bytes>1048576)return 0;
    std::shared_ptr<nfs::AudioBridge::State> current;bool entered=false;
    const uintptr_t previous=nfs::audio_callback;auto *previous_control=nfs::current_worker_control();
    size_t produced=0;
    try{
        {std::lock_guard<std::mutex> guard(nfs::registry_mutex);auto it=nfs::registry.find(token);if(it==nfs::registry.end())return 0;current=it->second;std::lock_guard<std::mutex> state_lock(current->mutex);if(!current->active || current->suspended)return 0;++current->running;entered=true;}
        nfs::audio_callback=token;nfs::current_worker_control()=&current->control;
        std::lock_guard<std::mutex> render_lock(current->render);
        while(produced<bytes){
            nfs::check_worker_stop();
            {std::lock_guard<std::mutex> lock(current->mutex);if(!current->active || current->suspended)break;}
            if(current->position==1024){
                std::memset(current->buffer.get(),0,1024);
                nfs_dispatch_game_regparm2(current->game.get(),current->sample_function,current->guest_buffer,256);
                nfs::check_worker_stop();
                current->position=0;
            }
            const size_t amount=std::min(bytes-produced,1024-current->position);
            std::memcpy(static_cast<uint8_t *>(output)+produced,static_cast<uint8_t *>(current->buffer.get())+current->position,amount);
            current->position+=amount;produced+=amount;
        }
    }catch(const nfs::WorkerStopped &){}catch(...){if(current){std::lock_guard<std::mutex> lock(current->mutex);current->active=false;if(!current->failure)current->failure=std::current_exception();}}
    nfs::current_worker_control()=previous_control;nfs::audio_callback=previous;
    if(entered){
        // Owner retains State until running reaches zero. Drop our lease first
        // so stop returns only after owned contexts/buffers can be destroyed.
        auto *finished=current.get();current.reset();
        std::lock_guard<std::mutex> lock(finished->mutex);--finished->running;finished->changed.notify_all();
    }
    return produced;
}
extern "C" void nfs_audio_report_error(uintptr_t token,const char *message) noexcept{
    try{
        std::lock_guard<std::mutex> guard(nfs::registry_mutex);auto it=nfs::registry.find(token);if(it==nfs::registry.end())return;
        std::lock_guard<std::mutex> lock(it->second->mutex);
        if(!it->second->failure)it->second->failure=std::make_exception_ptr(std::runtime_error(message?message:"Audio backend error"));
        it->second->active=false;
    }catch(...){}
}
extern "C" int32_t iSNDdllversion_(){return 0x60002;}
extern "C" int32_t iSNDdirectcaps_(int32_t){return 0x23e0f;}
extern "C" int32_t iSNDdirectsetfunctions_wrap(int32_t sample,int32_t,int32_t,int32_t fade,int32_t){nfs::current().functions(uint32_t(sample),uint32_t(fade));return 0;}
extern "C" int32_t iSNDdirectstart_(int32_t,int32_t){const bool ok=nfs::current().start();nfs::platform_error()=ok?0:8;return ok?0:-1;}
extern "C" int32_t iSNDdirectserve_(int32_t context){nfs::current().serve(nfs_resolve_game_context(uint32_t(context)));return 0;}
extern "C" int32_t iSNDdirectstop_(){nfs::current().stop();return 0;}
