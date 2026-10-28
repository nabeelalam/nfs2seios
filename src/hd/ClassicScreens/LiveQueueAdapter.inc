// PRIVATE original426950 wrapper: current source ownership gates the complete UI.
#pragma once
#include "LiveGameHooks.inc"
#include "WordDisplay.h"
#include "FullSceneGameHooks.inc"
#include "SceneLease.h"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_ls_live {
extern "C" int nfs_private_loadsave_cancel(uint64_t);
struct QueueErrno {int saved=errno;~QueueErrno()noexcept{errno=saved;}};
struct DrawScope {
 Game&game;std::shared_ptr<const OwnedFrame>frame;std::shared_ptr<const Lease>lease;std::shared_ptr<const nfs_ls_word_display::Lease>wordLease;std::shared_ptr<const nfs_ls_scene_display::Lease>sceneLease;uint64_t serial=0;bool consumed=false;int exceptions=0;
 void clear()noexcept{lease.reset();wordLease.reset();sceneLease.reset();}
 explicit DrawScope(Game&g):game(g){
  QueueErrno preserve;const int saved=errno;try{
   if(nfs_background_scene_live::activeRequest())return;
   frame=LiveIssuer::acquire(g);if(!frame)return;const char*wordFlag=std::getenv("NFS_PRIVATE_LOADSAVE_WORD_ART");
   if(wordFlag&&wordFlag[0]=='1'&&!wordFlag[1]){auto scene=nfs_ls_scene_live::currentScene(g,frame);if(scene)sceneLease=nfs_ls_scene_display::Lease::create(frame,std::move(scene));if(!sceneLease){wordLease=nfs_ls_word_display::Lease::create(frame);if(!wordLease)return;}}
   else{lease=Lease::create(frame);if(!lease)return;}
   serial=nfs_background_scene_live::nextRequest();if(!serial){clear();return;}
   (void)nfs_ls_scene_live::observeScene(g,frame,serial,1);
   nfs::check_worker_stop();const auto keep=lease;const auto wordKeep=wordLease;const auto sceneKeep=sceneLease;const uint64_t request=serial;
   auto accepted=nfs::enqueue_owned_renderer([keep,wordKeep,sceneKeep,request](){const auto v=sceneKeep?sceneKeep->view(request):wordKeep?wordKeep->view(request):keep->view(request);return int32_t(nfs_private_loadsave_begin(&v));});
   if(accepted!=nfs::OwnedRendererAcceptance::Consumed){clear();serial=0;return;}
   consumed=true;nfs_background_scene_live::activeRequest()=serial;exceptions=std::uncaught_exception()?1:0;
  }catch(...){if(serial&&nfs_background_scene_live::activeRequest()==serial)nfs_background_scene_live::activeRequest()=0;clear();errno=saved;throw;}errno=saved;
 }
 ~DrawScope()noexcept(false){
  QueueErrno preserve;const int saved=errno;if(!consumed||!serial){errno=saved;return;}
  if(nfs_background_scene_live::activeRequest()!=serial){clear();errno=saved;return;}
  const auto keep=lease;const auto wordKeep=wordLease;const auto sceneKeep=sceneLease;const uint64_t request=serial;
  if((std::uncaught_exception()?1:0)>exceptions){nfs_background_scene_live::activeRequest()=0;clear();errno=saved;return;}
  try{nfs::check_worker_stop();(void)nfs_ls_scene_live::observeScene(game,frame,serial,2);
   // Both dedicated source and child issuer must still be current. This check
   // precedes reacquireForFinish(), which consumes the child presentation token.
   const bool sceneCurrent=!sceneKeep||sceneKeep->currentFor(frame,nfs_ls_scene_live::currentScene(game,frame));
   auto current=sceneCurrent?LiveIssuer::reacquireForFinish(game,frame):std::shared_ptr<const OwnedFrame>{};nfs::OwnedRendererAcceptance accepted;
   if(current)accepted=nfs::enqueue_owned_renderer([keep,wordKeep,sceneKeep,request](){return int32_t(nfs_private_loadsave_finish(request));});
   else accepted=nfs::enqueue_owned_renderer([keep,wordKeep,sceneKeep,request](){return int32_t(nfs_private_loadsave_cancel(request));});
   if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("LoadSave finish changed renderer owner");nfs_background_scene_live::activeRequest()=0;clear();
  }catch(...){if(serial&&nfs_background_scene_live::activeRequest()==serial)nfs_background_scene_live::activeRequest()=0;clear();errno=saved;throw;}errno=saved;
 }
};
} // namespace nfs_ls_live
