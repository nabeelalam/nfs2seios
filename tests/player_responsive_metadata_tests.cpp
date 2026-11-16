// SPDX-License-Identifier: MIT
#include "MenuResponsiveCached.h"
#include <cmath>
#include <initializer_list>
#include <cstdio>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %s\n",#x);return 1;}}while(0)
int main(){
 for(uint32_t form:{NFS_MENU_CACHED_PLAYER_ONE,NFS_MENU_CACHED_PLAYER_TWO}){
  const uint32_t descriptor=form+0x1000;const uintptr_t game=0x7000;const uint64_t producer=7;
  nfs_menu_cached_owner_begin(form,descriptor,producer,game);
  NfsMenuCachedWidget widget{0,1,0,1,1,100,100,200,160};
  nfs_menu_cached_widgets(form,descriptor,150,130,&widget,1,0,0);
  NfsMenuCachedSnapshot snapshot{};CHECK(nfs_menu_cached_snapshot(form,descriptor,producer,game,11,12,&snapshot));CHECK(snapshot.formTag==form&&snapshot.count==1);
  CHECK(nfs_menu_cached_packet_matches(&snapshot,form,11,12));CHECK(!nfs_menu_cached_packet_matches(&snapshot,form,12,12));CHECK(!nfs_menu_cached_packet_matches(&snapshot,form,11,13));
  NfsMenuCachedSnapshot wrong=snapshot;wrong.formTag=form==NFS_MENU_CACHED_PLAYER_ONE?NFS_MENU_CACHED_PLAYER_TWO:NFS_MENU_CACHED_PLAYER_ONE;uint64_t epoch=0;float x=0,y=0;CHECK(!nfs_menu_cached_current(&wrong,&epoch,&x,&y));CHECK(nfs_menu_cached_current(&snapshot,&epoch,&x,&y));
  NfsMenuResponsiveLayout shown{};shown.schema=shown.active=shown.cursorValid=1;shown.revision=1;shown.ownerEpoch=epoch;shown.drawableWidth=1280;shown.drawableHeight=720;shown.logicalWidth=480.f*1280/720;shown.delta=shown.logicalWidth-640;shown.sourceCursorX=150;shown.sourceCursorY=130;
  nfs_menu_cached_publish(&shown,&snapshot);NfsMenuResponsivePointer pointer{};CHECK(nfs_menu_responsive_pointer_snapshot(&pointer));CHECK(std::fabs(pointer.x-(150+shown.delta*.5f))<.001f&&pointer.y==130);
  int32_t dx=0,dy=0;int accepted=0;nfs_menu_responsive_pointer_route(0,0,1,&dx,&dy,&accepted);CHECK(accepted&&dx==0&&dy==0);
  nfs_menu_responsive_pointer_route(-250,0,1,&dx,&dy,&accepted);CHECK(!accepted); // outside centered measured controls
  nfs_menu_cached_owner_end(form,producer,game);CHECK(!nfs_menu_responsive_pointer_snapshot(&pointer));CHECK(!nfs_menu_cached_current(&snapshot,&epoch,&x,&y));
  shown.revision=2;nfs_menu_cached_publish(&shown,&snapshot);CHECK(!nfs_menu_responsive_pointer_snapshot(&pointer));
 }
 std::puts("PASS both Player responsive owners: immutable pairing, exact form/epoch rows, centered cursor/inverse hits, padding rejection, stale owner revocation");return 0;
}
