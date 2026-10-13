// SPDX-License-Identifier: MIT
#include "WindowBridge.h"
#include "GuestAccess.h"
#include "GameContexts.h"
#include "PlatformError.h"
#include <stdexcept>
namespace nfs {
WindowBridge::~WindowBridge(){try{shutdown();}catch(...) {}}
uint32_t WindowBridge::create(uint32_t function){
    nfs_lookup_guest_function(function);
    std::lock_guard<std::mutex> operation(operation_mutex);
    {std::lock_guard<std::mutex> lock(mutex);if(stopping || native || generation==65535){platform_error()=8;return 0;}}
    if(!operations.create || !operations.destroy)throw std::logic_error("Window operations unbound");
    void *created=operations.create();if(!created){platform_error()=8;return 0;}
    {
        std::lock_guard<std::mutex> lock(mutex);
        if(!stopping){native=created;handle=0xb1000000u|++generation;callback=function;platform_error()=0;return handle;}
    }
    operations.destroy(created);platform_error()=6;return 0;
}
bool WindowBridge::post(GameMessage message){
    std::lock_guard<std::mutex> lock(mutex);
    if(stopping || !native || (message.window && message.window!=handle)){platform_error()=6;return false;}
    if(messages.size()==1024){platform_error()=8;return false;}
    message.window=handle;messages.push_back(message);changed.notify_all();platform_error()=0;return true;
}
int32_t WindowBridge::get(GameMessage &message,uint32_t window){
    std::unique_lock<std::mutex> lock(mutex);
    if(window && window!=handle){platform_error()=6;return -1;}
    changed.wait(lock,[&]{return stopping || worker_stop_ready() || !messages.empty();});
    check_worker_stop();
    if(stopping){message={};return 0;}
    message=messages.front();messages.pop_front();platform_error()=0;
    return message.message==0x12?0:1; // WM_QUIT is consumed but never dispatched.
}
int32_t WindowBridge::dispatch(void *context,const GameMessage &message){
    uint32_t function;
    {std::lock_guard<std::mutex> lock(mutex);if(!native || message.window!=handle){platform_error()=6;return 0;}function=callback;}
    return nfs_dispatch_game_window(context,function,message.window,message.message,message.wparam,message.lparam);
}
bool WindowBridge::destroy(uint32_t window){
    std::lock_guard<std::mutex> operation(operation_mutex);
    void *old;
    {std::lock_guard<std::mutex> lock(mutex);if(!native || window!=handle){platform_error()=6;return false;}old=native;}
    operations.destroy(old);
    {std::lock_guard<std::mutex> lock(mutex);native=nullptr;handle=callback=0;messages.clear();}
    platform_error()=0;return true;
}
void WindowBridge::stop(){std::lock_guard<std::mutex> lock(mutex);stopping=true;changed.notify_all();}
void WindowBridge::shutdown(){
    stop();
    std::lock_guard<std::mutex> operation(operation_mutex);
    void *old;
    {std::lock_guard<std::mutex> lock(mutex);old=native;}
    if(old)operations.destroy(old);
    {std::lock_guard<std::mutex> lock(mutex);native=nullptr;handle=callback=0;messages.clear();}
}
void WindowBridge::wake_workers(){std::lock_guard<std::mutex> lock(mutex);changed.notify_all();}
static WindowBridge *active=nullptr;
static void wake(){if(active)active->wake_workers();}
void bind_window_bridge(WindowBridge *bridge){active=bridge;window_worker_wake()=bridge?wake:nullptr;}
static WindowBridge &current(){if(!active)throw std::logic_error("Window runtime unbound");return *active;}
}
extern "C" int32_t WrapperCreateWindow(int32_t function){return int32_t(nfs::current().create(uint32_t(function)));}
extern "C" int32_t GetMessageA_wrap(int32_t output,int32_t window,int32_t minimum,int32_t maximum){
    nfs_host_address(uint32_t(output),sizeof(nfs::GameMessage));
    if(minimum || maximum){nfs::platform_error()=50;return -1;}
    nfs::GameMessage message{};const auto result=nfs::current().get(message,uint32_t(window));
    if(result>=0)guest_store(uint32_t(output),message);return result;
}
extern "C" int32_t PostMessageA_wrap(int32_t window,int32_t message,int32_t wparam,int32_t lparam){
    return nfs::current().post({uint32_t(window),uint32_t(message),uint32_t(wparam),uint32_t(lparam),0,0,0});
}
extern "C" int32_t DispatchMessageA_wrap(int32_t context,int32_t input){
    return nfs::current().dispatch(nfs_resolve_game_context(uint32_t(context)),guest_load<nfs::GameMessage>(uint32_t(input)));
}
extern "C" int32_t DestroyWindow_wrap(int32_t window){return PostMessageA_wrap(window,2,0,0);}
extern "C" int32_t DefWindowProcA_wrap(int32_t window,int32_t message,int32_t,int32_t){
    if(message==2)nfs::current().destroy(uint32_t(window));return 0;
}
