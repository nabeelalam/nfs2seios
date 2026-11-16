// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include "MenuResponsiveLayout.h"
#define NFS_MENU_CACHED_MAX_WIDGETS 40u
#define NFS_MENU_CACHED_SCHEMA 1u
#define NFS_MENU_CACHED_PLAYER_ONE 0x4D3050u
#define NFS_MENU_CACHED_PLAYER_TWO 0x4D3100u
#define NFS_MENU_CACHED_SETUP 0x4D2790u
#define NFS_MENU_CACHED_OPTIONS 0x4D3A88u
#define NFS_MENU_CACHED_LOCATION 0x4D2A20u
#define NFS_MENU_CACHED_OPPONENT 0x4D3204u
/* Actual measured source rows; enabled semantics come from original423CA0. */
typedef struct NfsMenuCachedWidget {
 uint32_t itemId,itemType,itemFlags,enabled,itemCount;
 int32_t left,top,right,bottom;
} NfsMenuCachedWidget;
/* Typed queue retains this complete immutable snapshot alongside its source
   scene owner. No pointer or numeric tag alone grants renderer authority. */
typedef struct NfsMenuCachedSnapshot {
 uint32_t schema,formTag,descriptor,count; uintptr_t game;
 uint64_t producerEpoch,sourceEpoch,widgetRevision,packetSerial,contentRevision;
 uint32_t cursorX,cursorY;
 NfsMenuCachedWidget widgets[NFS_MENU_CACHED_MAX_WIDGETS];
} NfsMenuCachedSnapshot;
/* Metadata pairing only: live owner/widget admission remains independently required. */
static inline int nfs_menu_cached_packet_matches(const NfsMenuCachedSnapshot*p,
 uint32_t form,uint64_t serial,uint64_t content){
 return p&&p->schema==NFS_MENU_CACHED_SCHEMA&&p->formTag==form&&serial&&content&&
        p->packetSerial==serial&&p->contentRevision==content;
}

#ifdef __cplusplus
extern "C" {
#endif
void nfs_menu_cached_owner_begin(uint32_t,uint32_t,uint64_t,uintptr_t);
void nfs_menu_cached_owner_end(uint32_t,uint64_t,uintptr_t);
void nfs_menu_cached_widgets(uint32_t,uint32_t,uint32_t,uint32_t,const NfsMenuCachedWidget*,uint32_t,uint32_t,uint32_t);
void nfs_menu_cached_source_observation(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
int nfs_menu_cached_snapshot(uint32_t,uint32_t,uint64_t,uintptr_t,uint64_t,uint64_t,NfsMenuCachedSnapshot*);
int nfs_menu_cached_current(const NfsMenuCachedSnapshot*,uint64_t*,float*,float*);
void nfs_menu_cached_publish(const NfsMenuResponsiveLayout*,const NfsMenuCachedSnapshot*);
#ifdef __cplusplus
}
#endif
