// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_INTRO_QUEUE_RUNTIME_TEST
#include "PlayerSceneQueueAdapter.inc"
#include "PlayerInitialNativeUploadGameHooks.inc"
#endif
#include "PlayerIntroPrepare.h"
#include "MainThreadBridge.h"
#include "WorkerControl.h"
#include <new>
#include <stdexcept>
namespace nfs_player_intro_live {
namespace scene=nfs_player_scene_live;namespace native=nfs_player_initial_native;namespace live=nfs_player_initial_native_live;namespace cpu=nfs_player_intro_cpu;
// Worker-side scope for exact normal40C375 only: begin before the original
// slot0 draw, phase notifications after slot0/tail, end AFTER original swap.
// Renderer callbacks own every borrowed byte and never read guest Game state.
class IntroFrameScope final{
 enum class Phase{Inactive,Begun,Slot0,Tail,Ended,Cancelled};enum class Operation{Slot0,Tail,End};
 Game&g_;std::shared_ptr<const cpu::Prepared>owned_;std::shared_ptr<const native::Proof>proof_;native::Binding binding_{};std::unique_ptr<native::Plane>actual_;
 uint64_t serial_=0;uint32_t loopForm_=0;Phase phase_=Phase::Inactive;bool beginConsumed_=false;int unwind_=0;
 static bool lexical(Game&g)noexcept{if(uint32_t(g.eax)||g.edx||g.ebx||uint32_t(g.ecx)!=32||uint64_t(uint32_t(g.esp))+4>UINT64_C(0x100000000))return false;try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(uint32_t(g.esp))==UINT32_MAX;}catch(...){return false;}}
 bool workerCurrent()noexcept{
  auto*l=scene::current(g_);if(!l||l->invalid||uint32_t(g_.esi)!=loopForm_||l->formGuest!=loopForm_||!owned_||!proof_||!actual_||!owned_->current())return false;
  native::Binding now{};auto current=live::proofForPresentation(g_,now,*actual_);return current==proof_&&native::sameBinding(binding_,now)&&owned_->proof()==proof_&&proof_->current(now,l->catalogue)&&l==scene::current(g_)&&uint32_t(g_.esi)==loopForm_;
 }
 void cancel()noexcept{
  const auto owned=owned_;const auto proof=proof_;const auto serial=serial_;
  // Cleanup intentionally does not require a now-stale source; exact serial
  // and renderer owner guards are still required by the cancel entry point.
  if(serial&&beginConsumed_&&phase_!=Phase::Ended&&phase_!=Phase::Cancelled)try{nfs::ScopedWorkerStopDeferral cleanup;(void)nfs::enqueue_owned_renderer([owned,proof,serial](){(void)owned;(void)proof;return int32_t(nfs_private_player_intro_cancel(serial));});}catch(...){}
  if(scene::activeRequest()==serial)scene::activeRequest()=0;phase_=Phase::Cancelled;beginConsumed_=false;owned_.reset();proof_.reset();binding_={};actual_.reset();serial_=0;
 }
 bool dispatch(Operation operation){
  if(!beginConsumed_||!serial_)return false;if(scene::activeRequest()!=serial_){cancel();return false;}nfs::check_worker_stop();if(!workerCurrent()){cancel();return false;}
  const auto owned=owned_;const auto proof=proof_;const auto binding=binding_;const auto catalog=owned->catalog();const auto serial=serial_;
  const auto receipt=nfs::enqueue_owned_renderer([owned,proof,binding,catalog,serial,operation](){
   if(!owned||!owned->current()||!proof||!proof->current(binding,catalog))return int32_t(nfs_private_player_intro_cancel(serial));
   switch(operation){case Operation::Slot0:return int32_t(nfs_private_player_intro_slot0_finish(serial));case Operation::Tail:return int32_t(nfs_private_player_intro_tail_finish(serial));case Operation::End:return int32_t(nfs_private_player_intro_end(serial));}return int32_t(0);
  });
  if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Player intro changed renderer owner");return true;
 }
 void begin(){
  auto*l=scene::current(g_);if(!l||l->invalid||!l->pendingInitialSidebar||scene::activeRequest()||uint32_t(g_.esi)!=l->formGuest)return;nfs::check_worker_stop();if(!lexical(g_))return;loopForm_=uint32_t(g_.esi);actual_.reset(new(std::nothrow)native::Plane);if(!actual_)return;
  proof_=live::proofForPresentation(g_,binding_,*actual_);if(!proof_||binding_.sidebar.formGuest!=loopForm_||binding_.sidebar.game!=reinterpret_cast<uintptr_t>(&g_)||!proof_->current(binding_,l->catalogue)){proof_.reset();actual_.reset();return;}
  serial_=nfs_background_scene_live::nextRequest();if(!serial_){proof_.reset();actual_.reset();return;}owned_=cpu::Prepared::create(proof_,binding_,*actual_,l->catalogue,serial_);if(!owned_||!workerCurrent()){cancel();return;}
  const auto owned=owned_;const auto proof=proof_;const auto binding=binding_;const auto catalog=owned->catalog();nfs::check_worker_stop();
  const auto receipt=nfs::enqueue_owned_renderer([owned,proof,binding,catalog](){if(!owned->current()||!proof->current(binding,catalog))return int32_t(0);auto view=owned->view();if(!view.schema||!owned->current()||!proof->current(binding,catalog))return int32_t(0);return int32_t(nfs_private_player_intro_begin(&view));});
  if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){owned_.reset();proof_.reset();actual_.reset();binding_={};serial_=0;return;}beginConsumed_=true;phase_=Phase::Begun;scene::activeRequest()=serial_;nfs::check_worker_stop();
 }
 void end(){if(!beginConsumed_)return;if(phase_!=Phase::Tail){cancel();return;}if(!dispatch(Operation::End))return;phase_=Phase::Ended;beginConsumed_=false;if(scene::activeRequest()==serial_)scene::activeRequest()=0;nfs::check_worker_stop();}
public:
 explicit IntroFrameScope(Game&g):g_(g),unwind_(nfs_logo_publisher::exceptions()){try{begin();}catch(...){cancel();throw;}}
 IntroFrameScope(const IntroFrameScope&)=delete;IntroFrameScope&operator=(const IntroFrameScope&)=delete;
 void slot0Finished(){if(!beginConsumed_)return;try{if(phase_!=Phase::Begun){cancel();return;}if(dispatch(Operation::Slot0)){phase_=Phase::Slot0;nfs::check_worker_stop();}}catch(...){cancel();throw;}}
 void tailFinished(){if(!beginConsumed_)return;try{if(phase_!=Phase::Slot0){cancel();return;}if(dispatch(Operation::Tail)){phase_=Phase::Tail;nfs::check_worker_stop();}}catch(...){cancel();throw;}}
 ~IntroFrameScope()noexcept(false){if(nfs_logo_publisher::exceptions()>unwind_){cancel();return;}try{end();}catch(...){cancel();throw;}}
};
} // namespace nfs_player_intro_live
