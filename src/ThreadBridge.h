// SPDX-License-Identifier: MIT
#pragma once
#include "GameMemory.h"
#include "WorkerControl.h"
#include <memory>
#include <atomic>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <exception>
namespace nfs {
class ThreadBridge {
    struct FailureMailbox {
        std::mutex mutex;std::exception_ptr first;
        std::atomic<bool> pending{false};
        void publish(std::exception_ptr error){
            if(!error)return;
            std::lock_guard<std::mutex> lock(mutex);
            if(first)return;
            first=error;pending.store(true,std::memory_order_release);
        }
        std::exception_ptr take(){
            if(!pending.load(std::memory_order_acquire))return {};
            if(!pending.exchange(false,std::memory_order_acquire))return {};
            std::lock_guard<std::mutex> lock(mutex);return first;
        }
    };
    std::shared_ptr<FailureMailbox> failures=std::make_shared<FailureMailbox>();
    struct Worker {
        std::mutex mutex;
        std::condition_variable changed;
        std::thread thread;
        std::unique_ptr<void,void(*)(void *)> context;
        GameFunction function;
        uint32_t thread_id;
        bool suspended,cancelled=false,finished=false;
        std::exception_ptr failure;
        WorkerControl control;
        std::shared_ptr<FailureMailbox> failures;
        Worker(GameFunction fn,void *p,void(*destroy)(void *),uint32_t id,bool paused,std::shared_ptr<FailureMailbox> sink)
            :context(p,destroy),function(fn),thread_id(id),suspended(paused),failures(std::move(sink)){}
    };
    struct Slot { std::shared_ptr<Worker> worker;uint32_t generation=0; } slots[256];
    std::mutex mutex;
    std::vector<std::shared_ptr<Worker>> owned;
    bool stopping=false;
    std::exception_ptr first_failure;
    bool valid(uint32_t id) const;
    static void run(std::shared_ptr<Worker> worker);
public:
    // Takes context ownership even on capacity/native thread creation failure.
    uint32_t create(GameFunction function,void *context,void(*destroy)(void *),bool suspended,uint32_t &thread_id);
    uint32_t resume(uint32_t id);
    uint32_t duplicate(uint32_t id);
    bool terminate(uint32_t id);
    bool close(uint32_t id);
    // Single-consumer first-failure notification, independent of handle/reaping.
    // Healthy polling takes no mutex and scans no workers; join_all still reports.
    std::exception_ptr take_failure(){return failures->take();}
    void request_stop_all(); // Non-blocking; main event loop must keep servicing work.
    // Cancel workers that never started, then join. Running game callbacks must
    // receive their game-level quit signal first. May not run on a game worker.
    void join_all();
    // Explicit join_all reports callback failures. Destruction only cleans up.
    ~ThreadBridge() {try {join_all();} catch (...) {}}
};
void bind_thread_bridge(ThreadBridge *bridge);
bool close_thread_handle(uint32_t id);
uint32_t current_game_thread_id();
}
extern "C" int32_t CreateThread_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t ResumeThread_wrap(int32_t);
extern "C" int32_t GetCurrentThreadId_wrap();
extern "C" int32_t GetCurrentThread_wrap();
extern "C" int32_t GetCurrentProcess_wrap();
extern "C" int32_t DuplicateHandle_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t TerminateThread_wrap(int32_t,int32_t);
