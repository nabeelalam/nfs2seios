// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_MAIN_SOURCE_FRAME_TEST
#include "BackgroundSceneLiveGameHooks.inc"
#include "Main343OriginLifetime.h"
#include "MenuResponsiveDebug.h"
#endif
/* Called only AFTER the existing actual Main native-entry source/copy/upload
   join succeeds, before that same source's renderer begin. It moves the exact
   normal40DC40 source observation earlier; no layout, widget, frame pixel or
   native-source authority is manufactured, and normal observation remains. */
namespace nfs_main_source_frame {
inline bool seed(Game&g,const std::shared_ptr<const nfs_background_scene_cpu::Prepared>&owned)noexcept{
 struct Errno{int value=errno;~Errno(){errno=value;}}preserve;
 namespace live=nfs_background_scene_live;
 auto*s=live::current(g);const auto catalog=nfs_background_scene_runtime::acquireCatalog();
 if(!s||s->invalid||!s->on||s->mainDepth!=1||!s->archiveBound||!s->ownerEpoch||!s->iteration||!s->catalogue||s->catalogue!=catalog||!owned||!owned->valid||s->published!=owned||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=nfs_logo_publisher::threadToken()||uint32_t(g.esi)!=uint32_t(guest_address(off_4D3EC4))||uint32_t(g.eax)!=1||g.edx||g.ebx||uint32_t(g.ecx)!=32)return false;
 const auto life=nfs_main_sidebar_origin::acquireGameLife(s->game);if(!life||!life->current())return false;
 uint32_t x=0,y=0,slot5=0,observed=0;
 try{
  {nfs::GuestCopyTransaction tx;
   if(tx.load<uint32_t>(uint32_t(g.esp))!=UINT32_MAX||tx.load<uint32_t>(dword_4D40B8)!=s->archive)return false;
   x=tx.load<uint32_t>(dword_4D4C7C);y=tx.load<uint32_t>(dword_4D4C80);
   const uint32_t descriptor=tx.load<uint32_t>(dword_4EB5A4+5*4);
   // Only word5 is observed, exactly as40DC40. Do not read a nonexistent eighth
   // descriptor word: the original descriptor is seven words/28bytes.
   if(descriptor&&!(descriptor&3u)&&descriptor<=UINT32_MAX-27u){slot5=tx.load<uint32_t>(descriptor+5*4);observed=!(slot5&3u)&&slot5<=0x200000u-32768u;}
   if(tx.load<uint32_t>(dword_4D40B8)!=s->archive)return false;
  }
  // Host publication is outside the guest lease. Recheck immutable/current
  // source ownership after all reads and before taking the host source mutex.
  if(!life->current()||live::current(g)!=s||s->invalid||s->published!=owned||s->catalogue!=catalog||nfs_background_scene_runtime::acquireCatalog()!=catalog)return false;
  nfs_menu_responsive_source_observation(0x401910u,x,y,slot5,observed);return true;
 }catch(...){return false;}
}
} // namespace nfs_main_source_frame
