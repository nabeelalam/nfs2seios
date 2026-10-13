// SPDX-License-Identifier: MIT
#include "MainThreadHarness.h"
#include <atomic>
#include <cstdio>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
    MainThreadHarness owner;
    int value=0;
    for(int i=0;i<100;++i)CHECK(nfs::renderer_operation([&]{if(std::this_thread::get_id()!=owner.owner_id())throw std::runtime_error("Wrong render owner");return ++value;})==i+1);
    CHECK(nfs::renderer_operation([]{return nfs::renderer_operation([]{return 42;});})==42);
    bool failed=false;try{nfs::renderer_operation([]()->int32_t{throw std::runtime_error("Render failure");});}catch(const std::runtime_error &){failed=true;}CHECK(failed);
    // A stop request must not free borrowed buffers while a native call uses them.
    std::promise<void> entered,release;auto gate=release.get_future().share();std::promise<bool> completed;
    nfs::WorkerControl control;
    std::thread worker([&]{
        nfs::current_worker_control()=&control;
        int borrowed=17;
        try{nfs::renderer_operation([&]{entered.set_value();gate.wait();if(borrowed!=17)throw std::runtime_error("Borrowed render storage lost");borrowed=99;return 0;});completed.set_value(false);}
        catch(const nfs::WorkerStopped &){completed.set_value(borrowed==99);}
        nfs::current_worker_control()=nullptr;
    });
    entered.get_future().wait();control.stop=true;
    auto result=completed.get_future();CHECK(result.wait_for(std::chrono::milliseconds(10))==std::future_status::timeout);
    release.set_value();CHECK(result.get());worker.join();
    nfs::run_on_main([](void *){nfs::suspend_renderer(true);},nullptr);
    std::promise<int> resumed;
    std::thread suspended([&]{resumed.set_value(nfs::renderer_operation([]{return 73;}));});
    auto ready=resumed.get_future();CHECK(ready.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
    // Ordinary lifecycle work remains runnable while renderer workers wait.
    int lifecycle=0;nfs::run_on_main([](void *p){++*static_cast<int *>(p);},&lifecycle);CHECK(lifecycle==1);
    nfs::run_on_main([](void *){nfs::suspend_renderer(false);},nullptr);CHECK(ready.get()==73);suspended.join();
    std::promise<void> lifecycle_entered,release_lifecycle;auto lifecycle_gate=release_lifecycle.get_future().share();
    struct BackgroundTransition {std::promise<void> *entered;std::shared_future<void> release;} transition{&lifecycle_entered,lifecycle_gate};
    std::thread background([&]{nfs::run_on_main([](void *p){auto &t=*static_cast<BackgroundTransition *>(p);t.entered->set_value();t.release.wait();nfs::suspend_renderer(true);},&transition);});
    lifecycle_entered.get_future().wait();std::promise<int> queued_result;
    std::thread queued([&]{queued_result.set_value(nfs::renderer_operation([]{return 91;}));});
    CHECK(owner.wait_queued(1));release_lifecycle.set_value();background.join();
    auto queued_done=queued_result.get_future();CHECK(queued_done.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
    nfs::run_on_main([](void *){nfs::suspend_renderer(false);},nullptr);CHECK(queued_done.get()==91);queued.join();
    nfs::run_on_main([](void *){nfs::suspend_renderer(true);},nullptr);
    nfs::WorkerControl paused_control;std::promise<bool> stopped;
    std::thread cancelled([&]{nfs::current_worker_control()=&paused_control;try{nfs::renderer_operation([]{return 1;});stopped.set_value(false);}catch(const nfs::WorkerStopped &){stopped.set_value(true);}nfs::current_worker_control()=nullptr;});
    auto stop_result=stopped.get_future();CHECK(stop_result.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
    paused_control.stop=true;nfs::wake_worker_waits();CHECK(stop_result.get());cancelled.join();
    nfs::run_on_main([](void *){nfs::suspend_renderer(false);},nullptr);
    // Reject a scheduler that silently runs the task on its caller thread.
    nfs::MainThreadBridge invalid([](nfs::MainTask task,void *p){task(p);});
    std::promise<bool> rejected;std::atomic<bool> ran{false};
    std::thread wrong([&]{try{invalid.execute([](void *p){*static_cast<std::atomic<bool> *>(p)=true;},&ran);rejected.set_value(false);}catch(const std::logic_error &){rejected.set_value(true);}});
    CHECK(rejected.get_future().get());wrong.join();CHECK(!ran);
    std::puts("PASS: render owner, synchronous results, nested calls, exception transport, borrowed storage cancellation, suspended/queued render deferral, live lifecycle dispatch, cancellation wake and wrong-thread rejection");
}
