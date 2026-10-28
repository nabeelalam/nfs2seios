// PRIVATE exact426050->426B70 native upload boundary; original execution continues.
#pragma once
#include "StatsGameHooks.inc"
#include "StatsPrepare.h"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <exception>
#include <stdexcept>
namespace nfs_stats_ui_live {
struct DrawScope {
 Game&game;std::shared_ptr<const nfs_stats_ui::Frame>frame;std::shared_ptr<const nfs_stats_ui::Prepared>display;uint64_t request=0;bool consumed=false,stageSent=false;int exceptions=0;
 explicit DrawScope(Game&g):game(g){Errno preserve;try{
  if(nfs_background_scene_live::activeRequest())return;finishFrame(g);frame=acquireCurrent(g);if(!frame)return;display=nfs_stats_ui::Prepared::create(frame);if(!display)return;request=nfs_background_scene_live::nextRequest();if(!request){display.reset();return;}nfs::check_worker_stop();const auto keep=display;const uint64_t id=request;auto accepted=nfs::enqueue_owned_renderer([keep,id](){const auto v=keep->view(id);return int32_t(nfs_private_stats_ui_begin(&v));});if(accepted!=nfs::OwnedRendererAcceptance::Consumed){request=0;display.reset();return;}consumed=true;exceptions=nfs_logo_publisher::exceptions();nfs_background_scene_live::activeRequest()=request;
 }catch(...){if(request&&nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;display.reset();throw;}}
 void stage(){Errno preserve;if(!consumed||!request||stageSent)return;const auto keep=display;const auto now=acquireCurrent(game);const bool valid=display&&display->currentFor(now);const auto id=request;auto accepted=nfs::enqueue_owned_renderer([keep,id,valid](){return int32_t(valid?nfs_private_stats_ui_stage(id):nfs_private_stats_ui_cancel(id));});if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Statistics staging changed renderer owner");stageSent=true;}
 ~DrawScope()noexcept(false){Errno preserve;if(!consumed||!request)return;if(nfs_background_scene_live::activeRequest()!=request){display.reset();return;}if(nfs_logo_publisher::exceptions()>exceptions){nfs_background_scene_live::activeRequest()=0;display.reset();return;}try{nfs::check_worker_stop();const auto keep=display;const auto now=acquireCurrent(game);const bool current=stageSent&&display&&display->currentFor(now);const uint64_t id=request;auto accepted=nfs::enqueue_owned_renderer([keep,id,current](){return int32_t(current?nfs_private_stats_ui_commit(id):nfs_private_stats_ui_cancel(id));});if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Statistics finish changed renderer owner");nfs_background_scene_live::activeRequest()=0;display.reset();}catch(...){if(nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;display.reset();throw;}}
};
}
