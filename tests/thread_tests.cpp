#include <chrono>
#include <string>
#include "ThreadBridge.h"
#include "FileBridge.h"
#include "EventBridge.h"
#include "CriticalSectionBridge.h"
#include "ClockWrappers.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <atomic>
#include <future>
#include <cstdio>
#include <stdexcept>
#include <cstring>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;
static std::atomic<unsigned> destroyed{0},executed{0};
static std::atomic<uint32_t> callback_id{0};
extern "C" uint64_t nfs_native_ticks(){return 0;}
extern "C" void nfs_native_delay(uint32_t){}
extern "C" int64_t nfs_native_unix_seconds(){return 0;}
struct Context {int value;uint32_t function_id=0;};
extern "C" void *nfs_host_address(uint32_t address,size_t size){void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad worker span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad worker pointer");return id;}
extern "C" void *nfs_resolve_game_context(uint32_t id){return id?nfs_host_address(id,sizeof(Context)):nullptr;}
extern "C" void nfs_destroy_game_context(void *){++destroyed;}
static void callback(void *p){if(static_cast<Context *>(p)->value!=42)throw std::runtime_error("Bad context");callback_id=uint32_t(GetCurrentThreadId_wrap());++executed;}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){if(id!=0xf0000004)throw std::out_of_range("Bad callback ID");return callback;}
extern "C" void nfs_prepare_game_worker(void *p,uint32_t id){static_cast<Context *>(p)->function_id=id;}
extern "C" void nfs_run_game_worker(void *p){nfs_lookup_guest_function(static_cast<Context *>(p)->function_id)(p);}
int main(){
    alignas(16) uint8_t bytes[128]{};Context ctx{42};
    CHECK(reinterpret_cast<uintptr_t>(&ctx)>UINT32_MAX && memory.map(0x80000000,bytes,sizeof(bytes)) && memory.map(0x90000000,reinterpret_cast<uint8_t *>(&ctx),sizeof(ctx)));
    nfs::ThreadBridge bridge;nfs::bind_thread_bridge(&bridge);
    const uint32_t main_id=uint32_t(GetCurrentThreadId_wrap());CHECK(main_id && GetCurrentThread_wrap()==-2);
    std::memset(bytes,0xcc,sizeof(bytes));
    int32_t handle=CreateThread_wrap(0,0,int32_t(0xf0000004u),int32_t(0x90000000u),4,int32_t(0x80000003u));
    CHECK(handle && (uint32_t(handle)&0xff000000)==0xc0000000 && !executed && !destroyed);
    CHECK(bytes[2]==0xcc && bytes[7]==0xcc);uint32_t child_id=guest_load<uint32_t>(0x80000003u);CHECK(child_id && child_id!=main_id);
    CHECK(ResumeThread_wrap(handle)==1 && ResumeThread_wrap(handle)==0);
    CHECK(CloseHandle_wrap(handle) && !CloseHandle_wrap(handle));
    CHECK(ResumeThread_wrap(handle)==-1 && GetLastError_wrap()==6);
    bridge.join_all();CHECK(executed==1 && destroyed==1 && callback_id==child_id);
    nfs::bind_thread_bridge(nullptr);
    // Closing a suspended handle must retain context until shutdown can retire it.
    nfs::ThreadBridge paused;uint32_t id=0;
    auto h=paused.create(callback,&ctx,nfs_destroy_game_context,true,id);CHECK(h);
    CHECK(paused.close(h));paused.join_all();CHECK(executed==1 && destroyed==2);
    // Native callback failures are joined and reported to the runtime owner.
    nfs::ThreadBridge failure;failure.create([](void *){throw std::runtime_error("fixture");},&ctx,nfs_destroy_game_context,false,id);
    bool reported=false;try{failure.join_all();}catch(const std::runtime_error &){reported=true;}CHECK(reported && destroyed==3);
    // A closed running worker keeps its context until the callback exits.
    nfs::ThreadBridge running;std::promise<void> entered,release;auto gate=release.get_future().share();
    struct Blocking {std::promise<void> *entered;std::shared_future<void> gate;} blocking{&entered,gate};
    const auto slow=[](void *p){auto &b=*static_cast<Blocking *>(p);b.entered->set_value();b.gate.wait();};
    h=running.create(slow,&blocking,nfs_destroy_game_context,false,id);entered.get_future().wait();CHECK(running.close(h) && destroyed==3);
    release.set_value();running.join_all();CHECK(destroyed==4);
    nfs::ThreadBridge errors;nfs::bind_thread_bridge(&errors);
    CHECK(!CreateThread_wrap(0,0,int32_t(0xf0000004u),int32_t(0x90000000u),8,0));
    CHECK(GetLastError_wrap()==50 && destroyed==5);
    bool bad_callback=false;
    try{CreateThread_wrap(0,0,1,int32_t(0x90000000u),0,0);}catch(const std::out_of_range &){bad_callback=true;}
    CHECK(bad_callback && destroyed==6);
    CHECK(GetCurrentProcess_wrap()==-1);
    CHECK(DuplicateHandle_wrap(-1,-2,-1,int32_t(0x80000003u),0,0,2));
    const uint32_t primary=guest_load<uint32_t>(0x80000003u);
    CHECK(primary && DuplicateHandle_wrap(-1,int32_t(primary),-1,int32_t(0x80000007u),0,0,2));
    const uint32_t alias=guest_load<uint32_t>(0x80000007u);CHECK(alias && alias!=primary);
    CHECK(CloseHandle_wrap(int32_t(primary)) && ResumeThread_wrap(int32_t(alias))==0);
    CHECK(!DuplicateHandle_wrap(-1,int32_t(primary),-1,int32_t(0x80000003u),0,0,2) && GetLastError_wrap()==6);
    CHECK(CloseHandle_wrap(int32_t(alias)));
    CHECK(!DuplicateHandle_wrap(-1,1,-1,0,0,0,2) && GetLastError_wrap()==87);
    CHECK(!DuplicateHandle_wrap(1,-2,-1,int32_t(0x80000003u),0,0,2) && GetLastError_wrap()==50);
    bool bad_output=false;try{DuplicateHandle_wrap(-1,-2,-1,int32_t(0x8000007fu),0,0,2);}catch(const std::out_of_range &){bad_output=true;}CHECK(bad_output);
    // An alias of an actual suspended worker resumes the same owned callback.
    uint32_t native_id=0;h=errors.create(callback,&ctx,nfs_destroy_game_context,true,native_id);
    auto worker_alias=errors.duplicate(h);CHECK(worker_alias && errors.close(h) && errors.resume(worker_alias)==1);
    errors.join_all();CHECK(destroyed==7 && executed==2);
    nfs::bind_thread_bridge(nullptr);
    nfs::ThreadBridge stopping;nfs::bind_thread_bridge(&stopping);
    nfs::EventBridge events;nfs::bind_event_bridge(&events);
    const uint32_t event=events.create(false,false);
    struct Waiting {nfs::EventBridge *events;uint32_t event;std::promise<void> *entered;};
    std::promise<void> waiting;Waiting wait_context{&events,event,&waiting};
    h=stopping.create([](void *p){auto &c=*static_cast<Waiting *>(p);c.entered->set_value();c.events->wait(&c.event,1,false,UINT32_MAX);},&wait_context,nfs_destroy_game_context,false,native_id);
    waiting.get_future().wait();CHECK(TerminateThread_wrap(int32_t(h),0)==1 && destroyed==8);
    CHECK(CloseHandle_wrap(int32_t(h)) && !TerminateThread_wrap(int32_t(h),0));
    uint8_t sample[4]{};memory.map(0xa0000000,sample,sizeof(sample));
    h=stopping.create([](void *){for(;;)guest_load<uint32_t>(0xa0000000u);},&ctx,nfs_destroy_game_context,false,native_id);
    CHECK(TerminateThread_wrap(int32_t(h),0)==1 && destroyed==9);
    std::promise<bool> self_result;
    h=stopping.create([](void *p){auto *result=static_cast<std::promise<bool> *>(p);try{TerminateThread_wrap(-2,0);result->set_value(false);}catch(const nfs::WorkerStopped &){result->set_value(true);throw;}},&self_result,nfs_destroy_game_context,false,native_id);
    CHECK(self_result.get_future().get());
    stopping.join_all();CHECK(destroyed==10);
    nfs::bind_event_bridge(nullptr);nfs::bind_thread_bridge(nullptr);
    nfs::ThreadBridge delays;nfs::bind_thread_bridge(&delays);
    std::promise<void> sleeping;
    h=delays.create([](void *p){static_cast<std::promise<void> *>(p)->set_value();SDL_Delay_wrap(60000);throw std::runtime_error("Cancelled delay returned");},&sleeping,nfs_destroy_game_context,false,native_id);
    sleeping.get_future().wait();CHECK(TerminateThread_wrap(int32_t(h),0)==1);
    delays.join_all();CHECK(destroyed==11);
    nfs::ThreadBridge deferred;nfs::bind_thread_bridge(&deferred);
    nfs::CriticalSectionBridge critical;nfs::bind_critical_sections(&critical);
    InitializeCriticalSection_wrap(int32_t(0x80000020u));
    std::promise<bool> released;
    h=deferred.create([](void *p){
        auto *result=static_cast<std::promise<bool> *>(p);
        EnterCriticalSection_wrap(int32_t(0x80000020u));
        EnterCriticalSection_wrap(int32_t(0x80000020u));
        if(!TerminateThread_wrap(-2,0))throw std::runtime_error("Self stop failed");
        guest_store<uint32_t>(0xa0000000u,123);
        LeaveCriticalSection_wrap(int32_t(0x80000020u));
        try{LeaveCriticalSection_wrap(int32_t(0x80000020u));result->set_value(false);}
        catch(const nfs::WorkerStopped &){result->set_value(true);throw;}
    },&released,nfs_destroy_game_context,false,native_id);
    CHECK(released.get_future().get());deferred.join_all();CHECK(destroyed==12);
    CHECK(guest_load<uint32_t>(0xa0000000u)==123);
    EnterCriticalSection_wrap(int32_t(0x80000020u));LeaveCriticalSection_wrap(int32_t(0x80000020u));
    DeleteCriticalSection_wrap(int32_t(0x80000020u));
    nfs::ThreadBridge all_stopping;nfs::bind_thread_bridge(&all_stopping);nfs::bind_event_bridge(&events);
    std::promise<void> bulk_entered;Waiting bulk_context{&events,event,&bulk_entered};
    CHECK(all_stopping.create([](void *p){auto &c=*static_cast<Waiting *>(p);c.entered->set_value();c.events->wait(&c.event,1,false,UINT32_MAX);},&bulk_context,nfs_destroy_game_context,false,native_id));
    bulk_entered.get_future().wait();CHECK(all_stopping.create([](void *){throw std::runtime_error("Suspended worker ran during shutdown");},&ctx,nfs_destroy_game_context,true,native_id));
    all_stopping.request_stop_all();all_stopping.join_all();CHECK(destroyed==14);
    nfs::bind_event_bridge(nullptr);nfs::bind_critical_sections(nullptr);nfs::bind_thread_bridge(nullptr);

    auto poll_failure=[](nfs::ThreadBridge &owner){
        const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        std::exception_ptr result;
        while(!(result=owner.take_failure()) && std::chrono::steady_clock::now()<until)std::this_thread::yield();
        return result;
    };
    auto error_text=[](std::exception_ptr error){
        try{if(error)std::rethrow_exception(error);}catch(const std::exception &e){return std::string(e.what());}
        return std::string();
    };
    auto fail=[](void *){throw std::runtime_error("mailbox worker failure");};
    auto destroy=[](void *){++destroyed;};
    auto noop=[](void *){};
    {
        nfs::ThreadBridge owner;
        for(unsigned i=0;i<1000000;++i)CHECK(!owner.take_failure());
        CHECK(owner.create(noop,&ctx,destroy,false,native_id));owner.join_all();CHECK(!owner.take_failure());
    }
    {
        nfs::ThreadBridge owner;const auto before=destroyed.load();
        auto owned=owner.create(fail,&ctx,destroy,false,native_id);CHECK(owned && owner.close(owned));
        auto error=poll_failure(owner);CHECK(error_text(error)=="mailbox worker failure");
        CHECK(destroyed==before+1 && !owner.take_failure());
        bool threw=false;try{owner.join_all();}catch(const std::runtime_error &e){threw=std::string(e.what())=="mailbox worker failure";}CHECK(threw);
    }
    {
        nfs::ThreadBridge owner;const auto before=destroyed.load();
        CHECK(owner.create(fail,&ctx,destroy,false,native_id));
        const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        while(destroyed==before && std::chrono::steady_clock::now()<until)std::this_thread::yield();
        CHECK(destroyed==before+1);CHECK(owner.create(noop,&ctx,destroy,false,native_id));
        CHECK(error_text(poll_failure(owner))=="mailbox worker failure");
        bool threw=false;try{owner.join_all();}catch(const std::runtime_error &){threw=true;}CHECK(threw && !owner.take_failure());
    }
    {
        nfs::ThreadBridge owner;
        for(unsigned i=0;i<8;++i)CHECK(owner.create(fail,&ctx,destroy,false,native_id));
        CHECK(error_text(poll_failure(owner))=="mailbox worker failure");
        try{owner.join_all();}catch(const std::runtime_error &){}
        CHECK(!owner.take_failure());
    }
    {
        nfs::ThreadBridge cancelled,independent;
        CHECK(cancelled.create([](void *){throw nfs::WorkerStopped();},&ctx,destroy,false,native_id));
        cancelled.join_all();CHECK(!cancelled.take_failure() && !independent.take_failure());
        CHECK(independent.create(fail,&ctx,destroy,false,native_id));
        CHECK(poll_failure(independent));CHECK(!cancelled.take_failure());
        try{independent.join_all();}catch(const std::runtime_error &){}
    }
    {
        nfs::ThreadBridge owner;CHECK(owner.create(fail,&ctx,destroy,true,native_id));
        owner.request_stop_all();owner.join_all();CHECK(!owner.take_failure());
    }
    std::puts("PASS: first worker-failure handoff,closed/reaped handles,post-destruction publication,one-shot concurrent errors,independent bridges,cooperative cancellation and healthy polling");
    std::puts("PASS: worker context/start frame, handle lifetime, failure reporting, stop during events/guest loads/delay, self stop deferred until recursive locks release");
}
