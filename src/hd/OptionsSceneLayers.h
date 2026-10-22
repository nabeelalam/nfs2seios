// SPDX-License-Identifier: MIT
#ifndef NFS_OPTIONS_SCENE_LAYERS_H
#define NFS_OPTIONS_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "OptionsSceneLayerAbi.h"
typedef struct NfsOptionsSceneGpu {
 NfsBackgroundSceneCompositor*background;
 NfsSoftwareTextGpu*caption;
} NfsOptionsSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only map/panel/words/logos then343caption. No preview or full640text.
   Each GL operation uses trusted actual context/thread/generation and registered
   unpublished staging ownership. No allocation, cache mutation or publication.
   Same-owner failure restores inherited state; loss abandons without GL calls. */
int nfs_options_scene_draw_layers_into_staging(NfsOptionsSceneGpu*,const NfsBackgroundSceneStage*,const NfsOptionsSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
