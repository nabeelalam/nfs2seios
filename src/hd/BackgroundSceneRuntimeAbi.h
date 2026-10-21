// SPDX-License-Identifier: MIT
#ifndef NFS_BACKGROUND_SCENE_RUNTIME_ABI_H
#define NFS_BACKGROUND_SCENE_RUNTIME_ABI_H
#include <stddef.h>
#include <stdint.h>
#include "SmallLogoLayerAbi.h"
#define NFS_BACKGROUND_SCENE_RUNTIME_SCHEMA 1u
#define NFS_BACKGROUND_SCENE_MAX_WORDS 4u
/* Values match immutable BackgroundSceneArt texture roles. */
enum {NFS_BACKGROUND_MAP_BLK4=0,NFS_BACKGROUND_MAP_BLK6=1};
/* Only the exact original40C047 off4D3EC4 branch is supported initially. */
enum {NFS_BACKGROUND_TAIL_MAIN_SKIP=1};
typedef struct NfsBackgroundSceneWordLayer {
 uint32_t sourceRole,scalar256;
 int32_t left,top,clip[4];
} NfsBackgroundSceneWordLayer;
/* Borrowed native-owned immutable data, valid for synchronous owner callback only.
   Producer must prove CURRENT original full640 map + scene source/order/owner before queueing.
   These fields NEVER assert GL context,FBO,depth,upload generation or permission to suppress.
   Renderer derives those from its actual current owner/hooks and preserves original600triangles. */
typedef struct NfsBackgroundSceneFrameView {
 uint32_t schema,width,height,mapRole,wordCount,tailKind;
 uint64_t serial,contentRevision;
 size_t planeWords;
 const uint16_t*original565;
 uint32_t rendererAddress[300];
 /* Supported fixed main panel at0,0 of343x480; clip is normalized fullscene coordinates. */
 int32_t panelClip[4];
 NfsBackgroundSceneWordLayer words[NFS_BACKGROUND_SCENE_MAX_WORDS];
 NfsSmallLogoLayer logos[3];
} NfsBackgroundSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_background_scene_begin(const NfsBackgroundSceneFrameView*view);
int nfs_private_background_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif
