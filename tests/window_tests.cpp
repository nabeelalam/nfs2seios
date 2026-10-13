// SPDX-License-Identifier: MIT
#include "WindowBridge.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include "GameContexts.h"
#include "PlatformError.h"
#include <cstdio>
#include <cstring>
#include <thread>
#include <future>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;
static int native_window,context;
static unsigned created=0,destroyed=0,dispatched=0;
static nfs::GameMessage seen{};
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto *p=memory.resolve(address,size);if(!p)throw std::out_of_range("window guest span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address;if(!memory.encode(p,address))throw std::out_of_range("window guest pointer");return address;}
static void callback(void *){}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){if(id!=0xf0000004u)throw std::out_of_range("window callback ID");return callback;}
extern "C" void *nfs_resolve_game_context(uint32_t address){return nfs_host_address(address,sizeof(context));}
extern "C" int32_t nfs_dispatch_game_window(void *p,uint32_t function,uint32_t window,uint32_t message,uint32_t wparam,uint32_t lparam){
    if(p!=&context || function!=0xf0000004u)throw std::runtime_error("bad window dispatch");
    ++dispatched;seen={window,message,wparam,lparam,0,0,0};return 123;
}
static void *create(){++created;return &native_window;}
static void destroy(void *p){if(p!=&native_window)throw std::runtime_error("bad native window");++destroyed;}
int main(){
    uint8_t bytes[128];std::memset(bytes,0xcc,sizeof(bytes));
    CHECK(reinterpret_cast<uintptr_t>(&native_window)>UINT32_MAX);
    CHECK(memory.map(0x80000000u,bytes,sizeof(bytes)) && memory.map(0x90000000u,reinterpret_cast<uint8_t *>(&context),sizeof(context)));
    nfs::WindowBridge bridge({create,destroy});nfs::bind_window_bridge(&bridge);
    const auto h=uint32_t(WrapperCreateWindow(int32_t(0xf0000004u)));CHECK(h && created==1);
    CHECK(!WrapperCreateWindow(int32_t(0xf0000004u)));
    CHECK(PostMessageA_wrap(int32_t(h),0x401,int32_t(0xffffffffu),int32_t(0x81234567u)));
    bool bounds=false;try{GetMessageA_wrap(int32_t(0x8000007fu),0,0,0);}catch(const std::out_of_range &){bounds=true;}CHECK(bounds);
    CHECK(GetMessageA_wrap(int32_t(0x80000003u),0,1,2)==-1 && nfs::platform_error()==50);
    CHECK(GetMessageA_wrap(int32_t(0x80000003u),0,0,0)==1);
    auto message=guest_load<nfs::GameMessage>(0x80000003u);
    CHECK(message.window==h && message.message==0x401 && message.wparam==UINT32_MAX && message.lparam==0x81234567u && !message.time && !message.x && !message.y);
    CHECK(bytes[2]==0xcc && bytes[31]==0xcc);
    CHECK(DispatchMessageA_wrap(int32_t(0x90000000u),int32_t(0x80000003u))==123 && dispatched==1 && seen.window==h);
    CHECK(DestroyWindow_wrap(int32_t(h)) && destroyed==0);
    CHECK(GetMessageA_wrap(int32_t(0x80000003u),int32_t(h),0,0)==1);
    message=guest_load<nfs::GameMessage>(0x80000003u);CHECK(message.message==2);
    CHECK(!DefWindowProcA_wrap(int32_t(h),2,0,0) && destroyed==1);
    const auto next=uint32_t(WrapperCreateWindow(int32_t(0xf0000004u)));CHECK(next!=h);
    CHECK(!PostMessageA_wrap(int32_t(h),0x401,0,0));
    CHECK(DispatchMessageA_wrap(int32_t(0x90000000u),int32_t(0x80000003u))==0 && dispatched==1);
    for(unsigned i=0;i<1024;++i)CHECK(PostMessageA_wrap(0,0x401,i,0));
    CHECK(!PostMessageA_wrap(0,0x401,0,0) && nfs::platform_error()==8);
    for(unsigned i=0;i<1024;++i){CHECK(GetMessageA_wrap(int32_t(0x80000003u),0,0,0)==1);CHECK(guest_load<nfs::GameMessage>(0x80000003u).wparam==i);}
    CHECK(PostMessageA_wrap(0,0x12,5,0) && GetMessageA_wrap(int32_t(0x80000003u),0,0,0)==0);
    // Stop a worker blocked in the game message wait, without a polling timer.
    nfs::WorkerControl control;std::promise<void> entering;std::promise<bool> stopped;
    std::thread waiter([&]{nfs::current_worker_control()=&control;entering.set_value();try{nfs::GameMessage m;bridge.get(m,0);stopped.set_value(false);}catch(const nfs::WorkerStopped &){stopped.set_value(true);}nfs::current_worker_control()=nullptr;});
    entering.get_future().wait();control.stop=true;nfs::wake_worker_waits();CHECK(stopped.get_future().get());waiter.join();
    std::promise<int32_t> result;std::thread retiring([&]{nfs::GameMessage m;result.set_value(bridge.get(m,0));});
    bridge.shutdown();CHECK(result.get_future().get()==0);retiring.join();CHECK(destroyed==2);
    CHECK(!WrapperCreateWindow(int32_t(0xf0000004u)) && !PostMessageA_wrap(0,0x401,0,0));
    nfs::bind_window_bridge(nullptr);
    std::puts("PASS: 28-byte unaligned game messages/canaries, native window IDs, callback fields, queued destroy, stale IDs, FIFO capacity, quit and cancellable shutdown");
}
