// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SCENE_LAYERS_H
#define NFS_CONTROLS_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "NestedCaptionGpu.h"
#include "ControlsDialogWordGpu.h"
#include "ControlsSceneLayerAbi.h"
typedef struct NfsControlsSceneGpu {
 NfsBackgroundSceneCompositor*background;
 NfsSoftwareTextGpu*sidebarCaption,*text;
 NfsNestedCaptionGpu*sidebarWord;
 NfsControlsDialogWordGpu*dialogWords;
} NfsControlsSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only ordered background, mandatory exact Controls word3 sidebar, and full640 text with five authored words. Owner callback
   independently authenticates actual context/thread/generation and unpublished
   staging target. No allocation, original target write, cache or presentation.
   Same-owner failure restores GL state; owner loss forgets metadata without GL.
   Every individual GL operation is checked before and after ownership. */
int nfs_controls_scene_draw_layers_into_staging(NfsControlsSceneGpu*,const NfsBackgroundSceneStage*,const NfsControlsSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
