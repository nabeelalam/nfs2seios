// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_SCENE_LAYERS_H
#define NFS_CONTROLS_SETUP_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "NestedCaptionGpu.h"
#include "ControlsSetupWordGpu.h"
#include "ControlsBindingPhraseGpu.h"
#include "ControlsSetupSceneLayerAbi.h"
typedef struct NfsControlsSetupSceneGpu {
 NfsBackgroundSceneCompositor*background;
 NfsSoftwareTextGpu*sidebarCaption,*text;
 NfsNestedCaptionGpu*sidebarWord;
 NfsControlsSetupWordGpu*childWords;
 NfsControlsBindingPhraseGpu*bindings;
} NfsControlsSetupSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only ordered background, mandatory exact OPTIONS word3 sidebar, and full640
   text with finite authenticated child labels. Intervening dynamic bindings
   retain source-catalog glyphs. Owner callback
   independently authenticates actual context/thread/generation and unpublished
   staging target. No allocation, original target write, cache or presentation.
   Same-owner failure restores GL state; owner loss forgets metadata without GL.
   Every individual GL operation is checked before and after ownership. */
int nfs_controls_setup_scene_draw_layers_into_staging(NfsControlsSetupSceneGpu*,const NfsBackgroundSceneStage*,const NfsControlsSetupSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
