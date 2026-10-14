// SPDX-License-Identifier: MIT
#pragma once
#include "MainThreadBridge.h"
#include <mutex>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
// Runs witnesses on a distinct owner thread with synchronous queued transport.
class MainThreadHarness {
    struct Request {nfs::MainTask task;void *context;std::promise<void> done;};
    std::mutex mutex;std::condition_variable changed;std::deque<Request *> queue;
    bool stopping=false;std::thread owner;nfs::MainThreadBridge *bridge_instance=nullptr;
    std::promise<void> ready;
    static MainThreadHarness *&active(){static MainThreadHarness *value=nullptr;return value;}
    static void schedule(nfs::MainTask task,void *context){
        auto &self=*active();Request request{task,context};auto done=request.done.get_future();
        {std::lock_guard<std::mutex> lock(self.mutex);self.queue.push_back(&request);self.changed.notify_all();}
        done.get();
    }
    void run(){
        nfs::MainThreadBridge bridge(schedule);nfs::bind_main_thread_bridge(&bridge);bridge_instance=&bridge;ready.set_value();
        for(;;){
            Request *request;
            {std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return stopping || !queue.empty();});if(queue.empty())break;request=queue.front();queue.pop_front();}
            request->task(request->context);request->done.set_value();
        }
        nfs::bind_main_thread_bridge(nullptr);
    }
public:
    MainThreadHarness(){active()=this;owner=std::thread([this]{run();});ready.get_future().wait();}
    ~MainThreadHarness(){{std::lock_guard<std::mutex> lock(mutex);stopping=true;changed.notify_one();}owner.join();active()=nullptr;}
    nfs::MainThreadBridge &bridge(){return *bridge_instance;}
    std::thread::id owner_id()const{return owner.get_id();}
    bool wait_queued(size_t count){std::unique_lock<std::mutex> lock(mutex);return changed.wait_for(lock,std::chrono::seconds(2),[&]{return queue.size()>=count;});}
};
