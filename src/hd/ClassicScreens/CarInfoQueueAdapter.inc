// PRIVATE ordinary426950 wrapper. No original render call is skipped.
#pragma once
#include "CarInfoGameHooks.inc"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_car_info_ui_live {
struct QueueErrno {int value=errno;~QueueErrno()noexcept{errno=value;}};
struct DrawScope {
 Game&g;std::shared_ptr<const Frame>frame;std::shared_ptr<const Lease>lease;uint64_t request=0;uint32_t bitmap=0;bool consumed=false,unwinding=false;
 explicit DrawScope(Game&x):g(x){QueueErrno preserve;
  auto*observed=ownerCurrent(g);const bool busy=nfs_background_scene_live::activeRequest()!=0;
  if(observed){const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned lines=0;if(flag&&flag[0]=='1'&&!flag[1]&&lines++<256)std::fprintf(stderr,"CAR INFO DRAW entry bitmap=%08x expected=%08x presenting=%u capturing=%u published=%u failed=%u busy=%u\n",uint32_t(g.eax),observed->published?observed->published->data().target.w[8]:0,observed->presenting,unsigned(observed->capturing),unsigned(bool(observed->published)),unsigned(observed->failed),unsigned(busy));}
  if(busy)return;bitmap=uint32_t(g.eax);frame=Adapter::acquire(g,bitmap);if(!frame)return;lease=Lease::create(frame);if(!lease)return;
  request=nfs_background_scene_live::nextRequest();if(!request){lease.reset();return;}nfs::check_worker_stop();
  auto keep=lease;uint64_t serial=request;auto result=nfs::enqueue_owned_renderer([keep,serial]{auto view=keep->view(serial);return int32_t(nfs_private_car_info_begin(&view));});
  if(result!=nfs::OwnedRendererAcceptance::Consumed){lease.reset();request=0;return;}
  consumed=true;nfs_background_scene_live::activeRequest()=request;unwinding=std::uncaught_exception();
 }
 ~DrawScope()noexcept(false){QueueErrno preserve;if(!consumed||!request)return;
  if(nfs_background_scene_live::activeRequest()!=request){lease.reset();return;}
  if(std::uncaught_exception()&&!unwinding){nfs_background_scene_live::activeRequest()=0;lease.reset();return;}
  try{nfs::check_worker_stop();auto keep=lease;uint64_t serial=request;bool valid=Adapter::reacquire(g,frame,bitmap);
   auto result=nfs::enqueue_owned_renderer([keep,serial,valid]{(void)keep;return int32_t(valid?nfs_private_car_info_finish(serial):nfs_private_car_info_cancel(serial));});
   nfs_background_scene_live::activeRequest()=0;lease.reset();if(result!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Car information renderer owner changed");
  }catch(...){if(nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;lease.reset();throw;}
 }
};
}
