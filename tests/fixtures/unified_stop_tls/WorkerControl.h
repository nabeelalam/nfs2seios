// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <exception>
namespace nfs {
struct WorkerStopped:std::exception {const char *what()const noexcept override{return "Game worker stopped";}};
struct WorkerControl {std::atomic<bool> stop{false};std::mutex mutex;std::condition_variable changed;};
// One constant-initialized trivial TLS object; existing reference APIs retained.
struct WorkerStopThreadState {WorkerControl *control;unsigned critical,deferrals;};
inline WorkerStopThreadState &worker_stop_thread_state(){
    static thread_local WorkerStopThreadState value{nullptr,0,0};return value;
}
inline WorkerControl *&current_worker_control(){return worker_stop_thread_state().control;}
inline unsigned &owned_critical_depth(){return worker_stop_thread_state().critical;}
inline unsigned &worker_stop_deferrals(){return worker_stop_thread_state().deferrals;}
struct ScopedWorkerStopDeferral {
    ScopedWorkerStopDeferral(){++worker_stop_deferrals();}
    ~ScopedWorkerStopDeferral(){--worker_stop_deferrals();}
    ScopedWorkerStopDeferral(const ScopedWorkerStopDeferral &)=delete;
    ScopedWorkerStopDeferral &operator=(const ScopedWorkerStopDeferral &)=delete;
};
inline bool worker_stop_ready(){auto *control=current_worker_control();return control && control->stop.load() && !owned_critical_depth() && !worker_stop_deferrals();}
inline void check_worker_stop(){if(worker_stop_ready())throw WorkerStopped();}
using Wake=void(*)();
inline Wake &event_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &critical_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &critical_worker_retire(){static Wake value=nullptr;return value;}
inline Wake &window_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &renderer_worker_wake(){static Wake value=nullptr;return value;}
inline void wake_worker_waits(){if(event_worker_wake())event_worker_wake()();if(critical_worker_wake())critical_worker_wake()();if(window_worker_wake())window_worker_wake()();if(renderer_worker_wake())renderer_worker_wake()();}
}
