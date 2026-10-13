#include "TimerBridge.h"
#include <cstdio>
#include <atomic>
#include <thread>
#include <future>
#include <stdexcept>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static uintptr_t latest=0;
static bool creation_fails=false,immediate=false;
static std::promise<void> *removal=nullptr;
static unsigned ticks=0;
extern "C" uint32_t nfs_native_add_timer(uintptr_t token){latest=token;if(immediate)nfs_timer_dispatch(token);return creation_fails?0:7;}
extern "C" void nfs_native_remove_timer(uint32_t){if(removal)removal->set_value();}
static uint32_t tick(void *){++ticks;return 3;}
int main(){
    nfs::GameTimerSchedule schedule;
    CHECK(schedule.next(65536,0)==1 && schedule.next(65536,0)==1 && schedule.next(65536,0)==2);
    CHECK(schedule.next(655360,0)==1 && schedule.next(65536,0)==3);
    nfs::GameTimerSchedule catchup;CHECK(catchup.next(65536,2000)==1 && catchup.next(65536,2000)==1 && catchup.next(65536,2000)==2);
    nfs::GameTimerSchedule wrap;CHECK(wrap.next(65536,0xfffffff0u)==16);
    nfs::GameTimerSchedule fraction;for(int i=0;i<5;++i)CHECK(fraction.next(32768,0)==1);CHECK(fraction.next(32768,0)==2);
    nfs::TimerBridge timer;immediate=true;timer.start(tick,nullptr);CHECK(ticks==1);immediate=false;
    const uintptr_t old=latest;CHECK(nfs_timer_dispatch(old)==3 && ticks==2);timer.stop();CHECK(!nfs_timer_dispatch(old) && ticks==2);
    timer.start(tick,nullptr);CHECK(latest!=old && !nfs_timer_dispatch(old));timer.stop();
    creation_fails=true;bool failed=false;try{timer.start(tick,nullptr);}catch(const std::runtime_error &){failed=true;}CHECK(failed && !nfs_timer_dispatch(latest));creation_fails=false;
    timer.start([](void *)->uint32_t{throw std::runtime_error("timer fixture");},nullptr);
    CHECK(!nfs_timer_dispatch(latest));failed=false;try{timer.stop();}catch(const std::runtime_error &){failed=true;}CHECK(failed);
    std::promise<void> entered,release,removed;auto gate=release.get_future().share();
    struct Context {std::promise<void> *entered;std::shared_future<void> gate;} context{&entered,gate};
    timer.start([](void *p){auto &c=*static_cast<Context *>(p);c.entered->set_value();c.gate.wait();return uint32_t(1);},&context);
    const uintptr_t blocked=latest;
    std::thread callback([&]{nfs_timer_dispatch(blocked);});entered.get_future().wait();
    removal=&removed;std::atomic<bool> stopped{false};
    std::thread stopper([&]{timer.stop();stopped=true;});removed.get_future().wait();
    CHECK(!stopped && !nfs_timer_dispatch(blocked));release.set_value();callback.join();stopper.join();removal=nullptr;CHECK(stopped);
    timer.start([](void *p){static_cast<nfs::TimerBridge *>(p)->stop();return uint32_t(1);},&timer);
    CHECK(!nfs_timer_dispatch(latest));failed=false;try{timer.stop();}catch(const std::logic_error &){failed=true;}CHECK(failed);
    std::puts("PASS: original timer cadence/fraction/wrap/resync, immediate callbacks, retired tokens, creation/errors, stop/drain and self-stop rejection");
}
