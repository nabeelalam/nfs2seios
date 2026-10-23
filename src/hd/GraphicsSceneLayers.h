// SPDX-License-Identifier: MIT
#ifndef NFS_GRAPHICS_SCENE_LAYERS_H
#define NFS_GRAPHICS_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "NestedCaptionGpu.h"
#include "GraphicsDialogWordGpu.h"
#include "GraphicsSceneLayerAbi.h"
typedef struct NfsGraphicsSceneGpu {
 NfsBackgroundSceneCompositor*background;
 NfsSoftwareTextGpu*sidebarCaption,*text;
 NfsNestedCaptionGpu*sidebarWord;
 NfsGraphicsDialogWordGpu*dialogWords;
} NfsGraphicsSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only ordered background, 343 caption (or exact authored word3), full640 text. Owner callback
   independently authenticates actual context/thread/generation and unpublished
   staging target. No allocation, original target write, cache or presentation.
   Same-owner failure restores GL state; owner loss forgets metadata without GL.
   Every individual GL operation is checked before and after ownership. */
int nfs_graphics_scene_draw_layers_into_staging(NfsGraphicsSceneGpu*,const NfsBackgroundSceneStage*,const NfsGraphicsSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
