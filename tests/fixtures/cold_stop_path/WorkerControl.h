// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <exception>
namespace nfs {
struct WorkerStopped:std::exception {const char *what()const noexcept override{return "Game worker stopped";}};
struct WorkerControl {std::atomic<bool> stop{false};std::mutex mutex;std::condition_variable changed;};
inline WorkerControl *&current_worker_control(){static thread_local WorkerControl *value=nullptr;return value;}
inline unsigned &owned_critical_depth(){static thread_local unsigned value=0;return value;}
inline unsigned &worker_stop_deferrals(){static thread_local unsigned value=0;return value;}
struct ScopedWorkerStopDeferral {
    ScopedWorkerStopDeferral(){++worker_stop_deferrals();}
    ~ScopedWorkerStopDeferral(){--worker_stop_deferrals();}
    ScopedWorkerStopDeferral(const ScopedWorkerStopDeferral &)=delete;
    ScopedWorkerStopDeferral &operator=(const ScopedWorkerStopDeferral &)=delete;
};
// Keep the ordinary (stop=false) path away from the two deferral TLS slots.
// All three original TLS objects/getters and their identities stay unchanged.
#if defined(__clang__) || defined(__GNUC__)
#define NFS_WORKER_STOP_COLD __attribute__((noinline,cold))
#elif defined(_MSC_VER)
#define NFS_WORKER_STOP_COLD __declspec(noinline)
#else
#define NFS_WORKER_STOP_COLD
#endif
inline NFS_WORKER_STOP_COLD bool worker_stop_not_deferred(){
    return !owned_critical_depth() && !worker_stop_deferrals();
}
#undef NFS_WORKER_STOP_COLD
inline bool worker_stop_ready(){
    auto *control=current_worker_control();
    // Deliberately retain the original default sequentially consistent load.
    // Short-circuiting must keep the cold helper off the stop=false path.
    return control && control->stop.load() && worker_stop_not_deferred();
}
inline void check_worker_stop(){if(worker_stop_ready())throw WorkerStopped();}
using Wake=void(*)();
inline Wake &event_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &critical_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &critical_worker_retire(){static Wake value=nullptr;return value;}
inline Wake &window_worker_wake(){static Wake value=nullptr;return value;}
inline Wake &renderer_worker_wake(){static Wake value=nullptr;return value;}
inline void wake_worker_waits(){if(event_worker_wake())event_worker_wake()();if(critical_worker_wake())critical_worker_wake()();if(window_worker_wake())window_worker_wake()();if(renderer_worker_wake())renderer_worker_wake()();}
}
