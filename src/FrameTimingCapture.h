// SPDX-License-Identifier: MIT
#pragma once
#include "FramePresentation.h"
#include <atomic>
#include <mutex>
namespace nfs {
// Fixed storage, one presentation producer; drain/control may run elsewhere.
// Pending records survive enable/disable and window changes until drained.
class FrameTimingCapture {
    std::atomic<bool> enabled_{false};
    std::atomic<uint64_t> epoch_{0},fps_stamp_{0},fps_sequence_{0};
    std::atomic<double> fps_{0};
    std::mutex mutex_;
    NfsFrameTiming records_[1024]{};
    unsigned head_=0,count_=0;
    uint64_t serial_=0,latest_=0,dropped_=0,previous_=0,previous_epoch_=0;
public:
    void capture(bool enabled) {
        enabled_.store(false,std::memory_order_release);
        std::lock_guard<std::mutex> lock(mutex_);
        previous_=0;
        if(enabled) epoch_.fetch_add(1,std::memory_order_relaxed);
        enabled_.store(enabled,std::memory_order_release);
    }
    void reset() {
        epoch_.fetch_add(1,std::memory_order_relaxed);
        fps_sequence_.fetch_add(1,std::memory_order_seq_cst);
        fps_stamp_.store(0,std::memory_order_seq_cst);
        fps_.store(0,std::memory_order_seq_cst);
        fps_sequence_.fetch_add(1,std::memory_order_seq_cst);
    }
    void published(uint64_t now,double fps) {
        fps_sequence_.fetch_add(1,std::memory_order_seq_cst);
        fps_.store(fps,std::memory_order_seq_cst);
        fps_stamp_.store(now,std::memory_order_seq_cst);
        fps_sequence_.fetch_add(1,std::memory_order_seq_cst);
    }
    void presented(uint64_t now) {
        if(!enabled_.load(std::memory_order_acquire)) return;
        std::lock_guard<std::mutex> lock(mutex_);
        if(!enabled_.load(std::memory_order_relaxed)) return;
        const uint64_t epoch=epoch_.load(std::memory_order_relaxed);
        const uint64_t interval=previous_ && previous_epoch_==epoch && now>=previous_ ? now-previous_:0;
        previous_=now;previous_epoch_=epoch;latest_=now;++serial_;
        if(count_==1024){head_=(head_+1)%1024;--count_;++dropped_;}
        records_[(head_+count_)%1024]={serial_,now,interval,epoch};++count_;
    }
    unsigned drain(NfsFrameTiming *out,unsigned capacity) {
        if(!out || !capacity)return 0;
        std::lock_guard<std::mutex> lock(mutex_);
        const unsigned n=capacity<count_?capacity:count_;
        for(unsigned i=0;i<n;++i)out[i]=records_[(head_+i)%1024];
        head_=(head_+n)%1024;count_-=n;return n;
    }
    NfsFrameTimingSnapshot snapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        uint64_t stamp=0;double fps=0;
        // Snapshot is outside presentation, but must never spin indefinitely
        // while holding the ring lock if the FPS owner is preempted.
        for(unsigned attempt=0;attempt<8;++attempt){
            const auto sequence=fps_sequence_.load(std::memory_order_seq_cst);
            if(sequence&1)continue;
            const auto candidate_stamp=fps_stamp_.load(std::memory_order_seq_cst);
            const auto candidate_fps=fps_.load(std::memory_order_seq_cst);
            if(sequence==fps_sequence_.load(std::memory_order_seq_cst)){
                stamp=candidate_stamp;fps=candidate_fps;break;
            }
        }
        return {serial_,latest_,dropped_,epoch_.load(std::memory_order_relaxed),stamp,fps,enabled_.load(std::memory_order_acquire)?1:0};
    }
};
}
