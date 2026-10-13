#include "EventBridge.h"
#include "FileBridge.h"
#include "GameMemory.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <cstdio>
#include <stdexcept>
#include <thread>
#include <future>
#include <chrono>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size) {
    void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad event array");return p;
}
extern "C" uint32_t nfs_guest_address(const void *p) {
    uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad event pointer");return id;
}
int main() {
    uint8_t bytes[512]{};
    CHECK(reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX && memory.map(0x80000000,bytes,sizeof(bytes)));
    nfs::EventBridge bridge;nfs::bind_event_bridge(&bridge);
    uint32_t a=uint32_t(CreateEventA_wrap(0,0,1,0)),b=uint32_t(CreateEventA_wrap(0,0,1,0));
    CHECK(a && b && a!=b && (a&0xff000000)==0xd0000000);
    const auto wait=[&](bool all,int32_t timeout) {return WaitForMultipleObjects_wrap(2,int32_t(0x80000003u),all,timeout);};
    guest_store<uint32_t>(0x80000003u,a);guest_store<uint32_t>(0x80000007u,b);
    CHECK(wait(false,0)==0 && wait(false,0)==1 && wait(false,0)==258);
    CHECK(SetEvent_wrap(int32_t(a)) && wait(true,0)==258);
    CHECK(SetEvent_wrap(int32_t(b)) && wait(true,0)==0 && wait(false,0)==258);
    CHECK(SetEvent_wrap(int32_t(a)) && SetEvent_wrap(int32_t(a)));
    CHECK(wait(false,0)==0 && wait(false,0)==258); // signals coalesce
    const auto before=std::chrono::steady_clock::now();CHECK(wait(false,12)==258);
    CHECK(std::chrono::steady_clock::now()-before>=std::chrono::milliseconds(12));
    uint32_t manual=uint32_t(CreateEventA_wrap(0,1,1,0));
    CHECK(bridge.wait(&manual,1,false,0)==0 && bridge.wait(&manual,1,false,0)==0);
    CHECK(fclose_wrap(int32_t(manual))==EOF && SetEvent_wrap(int32_t(manual)));
    CHECK(CloseHandle_wrap(int32_t(manual)) && !CloseHandle_wrap(int32_t(manual)) && GetLastError_wrap()==6);
    uint32_t replacement=bridge.create(false,false);CHECK(replacement && replacement!=manual && !bridge.signal(manual));
    CHECK(!bridge.signal(0xe0000100)); // file IDs cannot become event IDs
    guest_store<uint32_t>(0x80000007u,a);CHECK(wait(false,0)==-1 && GetLastError_wrap()==87);
    guest_store<uint32_t>(0x80000007u,b);
    CHECK(!CreateEventA_wrap(0,0,0,1) && GetLastError_wrap()==50);
    CHECK(WaitForMultipleObjects_wrap(65,0,0,0)==-1);
    bool rejected=false;try{WaitForMultipleObjects_wrap(2,int32_t(0x800001fdu),0,0);}catch(const std::out_of_range &){rejected=true;}CHECK(rejected);
    // One auto-reset signal releases exactly one waiter.
    uint32_t single=bridge.create(false,false),r1=UINT32_MAX,r2=UINT32_MAX;
    std::promise<void> p1,p2;auto f1=p1.get_future(),f2=p2.get_future();
    std::thread t1([&]{p1.set_value();r1=bridge.wait(&single,1,false,200);});
    std::thread t2([&]{p2.set_value();r2=bridge.wait(&single,1,false,200);});
    f1.wait();f2.wait();bridge.signal(single);t1.join();t2.join();
    CHECK((r1==0 && r2==258)||(r2==0 && r1==258));
    uint32_t closing=bridge.create(false,false),closed_result=0;
    std::thread t3([&]{closed_result=bridge.wait(&closing,1,false,200);});
    bridge.close(closing);t3.join();CHECK(closed_result==UINT32_MAX);
    uint32_t stopping=bridge.create(false,false),stopped_result=0;
    std::thread t4([&]{stopped_result=bridge.wait(&stopping,1,false,UINT32_MAX);});
    bridge.shutdown();t4.join();CHECK(stopped_result==UINT32_MAX && !bridge.create(false,false));
    nfs::bind_event_bridge(nullptr);
    // Last error is shared across bridges but isolated between native threads.
    nfs::platform_error()=123;uint32_t child_error=0;
    std::thread t5([&]{SetEvent_wrap(0);child_error=nfs::platform_error();});t5.join();
    CHECK(child_error==6 && GetLastError_wrap()==123);
    nfs::EventBridge capacity;uint32_t ids[256];
    for(auto &id:ids){id=capacity.create(false,false);CHECK(id);}
    CHECK(!capacity.create(false,false));CHECK(capacity.close(ids[0]));
    CHECK(capacity.create(false,false)!=ids[0] && !capacity.signal(ids[0]));
    nfs::EventBridge generations;
    const uint32_t oldest=generations.create(false,false);uint32_t latest=oldest;
    for(uint32_t i=1;i<65535;++i){CHECK(generations.close(latest));latest=generations.create(false,false);CHECK((latest&255)==0);}
    CHECK(generations.close(latest));const uint32_t next=generations.create(false,false);
    CHECK((next&255)==1 && !generations.signal(oldest) && !generations.signal(latest));
    std::puts("PASS: event guest ABI, auto/manual reset, wait-any/all, coalescing, timeout, concurrent wakeups, stale handles, close/shutdown, thread-local errors");
}
