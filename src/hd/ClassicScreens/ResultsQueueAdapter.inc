// PRIVATE ordinary426950 wrapper. No original render call is skipped.
#pragma once
#include "ResultsGameHooks.inc"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_results_ui_live {
struct QueueErrno {int value=errno;~QueueErrno()noexcept{errno=value;}};
struct DrawScope {
 Game&g;std::shared_ptr<const Frame>frame;std::shared_ptr<const Lease>lease;uint64_t request=0;bool consumed=false,unwinding=false;
 explicit DrawScope(Game&x):g(x){QueueErrno preserve;
  auto* state=ownerCurrent(g);
  if(!state||!state->presenting)return;
  if(nfs_background_scene_live::activeRequest()){trace("draw-busy");return;}
  frame=Adapter::acquire(g);if(!frame){trace("draw-frame-unavailable");return;}
  lease=Lease::create(frame);if(!lease){trace("draw-lease-unavailable");return;}
  request=nfs_background_scene_live::nextRequest();if(!request){lease.reset();return;}nfs::check_worker_stop();
  auto keep=lease;uint64_t serial=request;auto result=nfs::enqueue_owned_renderer([keep,serial]{auto view=keep->view(serial);return int32_t(nfs_private_results_begin(&view));});
  if(result!=nfs::OwnedRendererAcceptance::Consumed){trace("renderer-begin-refused",uint32_t(result));lease.reset();request=0;return;}
  trace("renderer-begin-consumed");
  consumed=true;nfs_background_scene_live::activeRequest()=request;unwinding=std::uncaught_exception();
 }
 ~DrawScope()noexcept(false){QueueErrno preserve;if(!consumed||!request)return;
  if(nfs_background_scene_live::activeRequest()!=request){lease.reset();return;}
  if(std::uncaught_exception()&&!unwinding){nfs_background_scene_live::activeRequest()=0;lease.reset();return;}
  try{nfs::check_worker_stop();auto keep=lease;uint64_t serial=request;bool valid=Adapter::reacquire(g,frame);
   auto result=nfs::enqueue_owned_renderer([keep,serial,valid]{(void)keep;return int32_t(valid?nfs_private_results_finish(serial):nfs_private_results_cancel(serial));});
   nfs_background_scene_live::activeRequest()=0;lease.reset();if(result!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Results renderer owner changed");
  }catch(...){if(nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;lease.reset();throw;}
 }
};
// This scope exists only at the original Results cursor call after native commit.
struct TailScope {
 Game&g;
 std::shared_ptr<const Frame> frame;
 NfsResultsCursor cursor{};
 uint32_t descriptor=0,archive=0;
 bool entered=false;
 bool sourceCurrent()const noexcept{
  try{
   auto*s=current(g);
   if(!s||s->published!=frame||s->capturing||s->presenting||s->string||s->word||s->epoch!=cursor.epoch)return false;
   const auto&d=frame->data();
   if(!nfs_logo_capture::same(d.target,target())||d.original!=plane(d.target))return false;
   nfs::GuestCopyTransaction tx;
   return tx.load<uint32_t>(dword_4D40B8)==archive&&(cursor.branding?tx.load<uint32_t>(dword_4D4E44):tx.load<uint32_t>(dword_4D4D30))==descriptor&&
    (cursor.branding||tx.load<int32_t>(dword_4EC4BC)==cursor.edge)&&tx.load<uint32_t>(descriptor+20)==cursor.address;
  }catch(...){return false;}
 }
 explicit TailScope(Game&game,bool branding=false)noexcept:g(game){
  QueueErrno preserve;
  try{
   auto*s=current(g);
   if(!s||!s->published||s->capturing||s->presenting||s->string||s->word)return;
   frame=s->published;
   const auto&d=frame->data();
   cursor.request=nfs_background_scene_live::nextRequest();
   cursor.epoch=d.epoch;cursor.content=d.serial;cursor.game=d.sourceGame;
   cursor.x=int32_t(g.eax);cursor.y=int32_t(g.edx);
   {
    nfs::GuestCopyTransaction tx;
    archive=tx.load<uint32_t>(dword_4D40B8);
    cursor.branding=branding?1u:0u;
    descriptor=branding?tx.load<uint32_t>(dword_4D4E44):tx.load<uint32_t>(dword_4D4D30);
    cursor.edge=branding?256:tx.load<int32_t>(dword_4EC4BC);
    if(!archive||!descriptor||(descriptor&3)||(!branding&&(cursor.edge<1||cursor.edge>64)))return;
    nfs_logo_publisher::checked(descriptor,28);
    const uint32_t lod=branding?0u:3u;
    if(tx.load<uint32_t>(descriptor)!=lod||tx.load<uint32_t>(descriptor+4)!=lod||
       tx.load<uint32_t>(descriptor+8)!=3||tx.load<uint32_t>(descriptor+12)!=(branding?11u:12u)||
       tx.load<uint32_t>(descriptor+24)!=3)return;
    cursor.address=tx.load<uint32_t>(descriptor+20);
    const uint32_t bytes=branding?131072u:2048u;
    if((cursor.address&3)||cursor.address>0x200000u-bytes)return;
   }
   if(!cursor.request||!sourceCurrent()){trace("cursor-source-refused",uint32_t(cursor.content));return;}
   const auto v=cursor;
   entered=nfs::enqueue_owned_renderer([v]{return int32_t(nfs_private_results_cursor_begin(&v));})==nfs::OwnedRendererAcceptance::Consumed;
   trace(entered?"cursor-begin-consumed":"cursor-begin-refused",uint32_t(cursor.content),cursor.address);
  }catch(...){}
 }
 ~TailScope()noexcept{
  QueueErrno preserve;
  if(!entered)return;
  try{
   const auto request=cursor.request;
   const bool valid=sourceCurrent();
   (void)nfs::enqueue_owned_renderer([request,valid]{return int32_t(nfs_private_results_cursor_end(request,valid));});
  }catch(...){}
 }
};
}
