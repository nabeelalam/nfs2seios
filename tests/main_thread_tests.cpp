// SPDX-License-Identifier: MIT
#include "MainThreadHarness.h"
#include <atomic>
#include <cstdio>
#include <stdexcept>
#include <vector>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
    MainThreadHarness owner;
    int value=0;
    for(int i=0;i<100;++i)CHECK(nfs::renderer_operation([&]{if(std::this_thread::get_id()!=owner.owner_id())throw std::runtime_error("Wrong render owner");return ++value;})==i+1);
    CHECK(nfs::renderer_operation([]{return nfs::renderer_operation([]{return 42;});})==42);
    std::vector<int> commands;
    int snapshot=17;nfs::renderer_command([snapshot,&commands]{commands.push_back(snapshot);return 0;});snapshot=99;
    CHECK(commands.empty());CHECK(nfs::renderer_operation([&]{return commands.size()==1 && commands[0]==17?1:0;})==1);
    // Two producers admit commands in a known order; either may drain them.
    std::promise<void> first_admitted;
    std::thread first([&]{nfs::renderer_command([&]{commands.push_back(1);return 0;});first_admitted.set_value();});
    std::thread second([&]{first_admitted.get_future().wait();nfs::renderer_command([&]{commands.push_back(2);return 0;});nfs::flush_renderer_commands();});
    first.join();second.join();CHECK(commands.size()==3 && commands[1]==1 && commands[2]==2);
    std::atomic<int> bounded{0};
    for(int i=0;i<400;++i)nfs::renderer_command([&]{++bounded;return 0;});
    CHECK(bounded>0 && bounded<400);nfs::flush_renderer_commands();CHECK(bounded==400);
    // Concurrent producers exceed the queue bound; each producer's order and
    // exactly-once execution must survive drains initiated by other producers.
    std::vector<int> producer_next(4,0);
    std::atomic<bool> ordered{true};std::vector<std::thread> producers;
    for(int producer=0;producer<4;++producer)producers.emplace_back([&,producer]{
        for(int sequence=0;sequence<1000;++sequence)nfs::renderer_command([&,producer,sequence]{
            if(std::this_thread::get_id()!=owner.owner_id() || producer_next[producer]!=sequence)ordered=false;
            ++producer_next[producer];return 0;
        });
        nfs::flush_renderer_commands();
    });
    for(auto &producer:producers)producer.join();
    CHECK(ordered);for(auto count:producer_next)CHECK(count==1000);
    bool queued_failure=false;int skipped=0;
    nfs::renderer_command([]()->int32_t{throw std::runtime_error("Queued render fixture");});
    nfs::renderer_command([&]{++skipped;return 0;});
    try{nfs::flush_renderer_commands();}catch(const std::runtime_error &){queued_failure=true;}
    CHECK(queued_failure && skipped==0);CHECK(nfs::renderer_operation([]{return 9;})==9);
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
    // Permanent retirement: full queue under suspension, pending barrier,
    // captured ownership released, no graphics callbacks executed.
    nfs::run_on_main([](void *){nfs::suspend_renderer(true);},nullptr);
    std::atomic<int> forbidden_calls{0};std::atomic<int> destroyed{0};auto ownership=std::shared_ptr<int>(new int(7),[&](int *p){
        nfs::renderer_command([&]{++forbidden_calls;return 0;});
        if(nfs::renderer_operation([&]{++forbidden_calls;return 1;})!=0)std::terminate();
        delete p;++destroyed;
    });std::weak_ptr<int> borrowed_owner=ownership;
    for(int i=0;i<128;++i)nfs::renderer_command([ownership,&forbidden_calls]{++forbidden_calls;return 0;});
    ownership.reset();CHECK(!borrowed_owner.expired());
    std::promise<int> retired_result;
    std::thread retiring_worker([&]{retired_result.set_value(nfs::renderer_operation([&]{++forbidden_calls;return 99;}));});
    auto retirement=retired_result.get_future();CHECK(retirement.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout);
    nfs::run_on_main([](void *p){static_cast<nfs::MainThreadBridge *>(p)->retire_renderer();},&owner.bridge());
    CHECK(retirement.get()==0);retiring_worker.join();CHECK(borrowed_owner.expired());CHECK(destroyed==1);CHECK(forbidden_calls==0);
    nfs::renderer_command([&]{++forbidden_calls;return 0;});CHECK(nfs::renderer_operation([&]{++forbidden_calls;return 91;})==0);CHECK(forbidden_calls==0);
    // Reject a scheduler that silently runs the task on its caller thread.
    nfs::MainThreadBridge invalid([](nfs::MainTask task,void *p){task(p);});
    std::promise<bool> rejected;std::atomic<bool> ran{false};
    std::thread wrong([&]{try{invalid.execute([](void *p){*static_cast<std::atomic<bool> *>(p)=true;},&ran);rejected.set_value(false);}catch(const std::logic_error &){rejected.set_value(true);}});
    CHECK(rejected.get_future().get());wrong.join();CHECK(!ran);
    const auto timing=nfs::take_renderer_timing();
    CHECK(timing.calls>=100 && timing.queue_ns>0 && timing.execution_ns>0);
    const auto cleared=nfs::take_renderer_timing();
    CHECK(cleared.calls==0 && cleared.queue_ns==0 && cleared.execution_ns==0);
    std::puts("PASS: render owner, synchronous results, nested calls, exception transport, borrowed storage cancellation, suspended/queued render deferral, live lifecycle dispatch, cancellation wake and wrong-thread rejection");
}
