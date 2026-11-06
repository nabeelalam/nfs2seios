// PRIVATE ordinary426950 wrapper. No original render call is skipped.
#pragma once
#include "SlideshowPresentation.inc"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_slideshow_live {
struct QueueErrno {int value=errno;~QueueErrno()noexcept{errno=value;}};
struct DrawScope {
 Game&g;std::shared_ptr<const Lease>lease;uint64_t request=0;bool consumed=false,unwinding=false;
 explicit DrawScope(Game&x):g(x){QueueErrno preserve;
  if(nfs_background_scene_live::activeRequest())return;lease=Lease::acquire(g);if(!lease)return;
  request=nfs_background_scene_live::nextRequest();if(!request){lease.reset();return;}nfs::check_worker_stop();
  auto keep=lease;uint64_t serial=request;auto result=nfs::enqueue_owned_renderer([keep,serial]{auto view=keep->view(serial);return int32_t(nfs_private_slideshow_begin(&view));});
  if(result!=nfs::OwnedRendererAcceptance::Consumed){lease.reset();request=0;return;}
  consumed=true;nfs_background_scene_live::activeRequest()=request;unwinding=std::uncaught_exception();
 }
 ~DrawScope()noexcept(false){QueueErrno preserve;if(!consumed||!request)return;
  if(nfs_background_scene_live::activeRequest()!=request){lease.reset();return;}
  if(std::uncaught_exception()&&!unwinding){nfs_background_scene_live::activeRequest()=0;lease.reset();return;}
  try{nfs::check_worker_stop();auto keep=lease;uint64_t serial=request;bool valid=lease->current(g);
   auto result=nfs::enqueue_owned_renderer([keep,serial,valid]{(void)keep;return int32_t(valid?nfs_private_slideshow_finish(serial):nfs_private_slideshow_cancel(serial));});
   nfs_background_scene_live::activeRequest()=0;lease.reset();if(result!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Slideshow renderer owner changed");
  }catch(...){if(nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;lease.reset();throw;}
 }
};
}
