// SPDX-License-Identifier: MIT
#pragma once
#include "AudioScenePrepare.h"
#include <atomic>
namespace nfs_audio_scene_cpu {
// One queue handle plus a C renderer retain keep all view borrows alive even
// when worker cancellation prevents the ordered finish command from appending.
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Prepared>prepared_;
 explicit Lease(std::shared_ptr<const Prepared>p):prepared_(std::move(p)){}
 ~Lease()=default;
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*self=static_cast<Lease*>(p);if(self->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete self;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const Prepared>p)noexcept{
  if(!p)return {};try{return std::shared_ptr<const Lease>(new Lease(std::move(p)),[](const Lease*x){release(const_cast<Lease*>(x));});}catch(...){return {};}
 }
 NfsAudioSceneFrameView view(uint64_t serial)const noexcept{auto v=prepared_->view(serial);if(v.schema){v.lifetimeOwner=const_cast<Lease*>(this);v.retainOwner=retain;v.releaseOwner=release;}return v;}
 const std::shared_ptr<const Prepared>&prepared()const noexcept{return prepared_;}
};
} // namespace nfs_audio_scene_cpu
