// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_SCENE_LAYERS_H
#define NFS_CONTROLS_STEERING_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "NestedCaptionGpu.h"
#include "ControlsSteeringWordGpu.h"
#include "ControlsSteeringBarGpu.h"
#include "ControlsSteeringSceneLayerAbi.h"
typedef struct NfsControlsSteeringSceneGpu {
 NfsBackgroundSceneCompositor*background;
 NfsSoftwareTextGpu*sidebarCaption,*text;
 NfsNestedCaptionGpu*sidebarWord;
 NfsControlsSteeringWordGpu*childWords;
 NfsControlsSteeringBarGpu*bar;
} NfsControlsSteeringSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only ordered background, mandatory exact OPTIONS word3 sidebar, and full640
   text with finite authenticated child labels. Intervening original colon spans
   retain source-catalog glyphs. Owner callback
   independently authenticates actual context/thread/generation and unpublished
   staging target. No allocation, original target write, cache or presentation.
   Same-owner failure restores GL state; owner loss forgets metadata without GL.
   Every individual GL operation is checked before and after ownership. */
int nfs_controls_steering_scene_draw_layers_into_staging(NfsControlsSteeringSceneGpu*,const NfsBackgroundSceneStage*,const NfsControlsSteeringSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
