// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_SCENE_LAYERS_H
#define NFS_AUDIO_SCENE_LAYERS_H
#include "BackgroundSceneCompositor.h"
#include "NestedCaptionGpu.h"
#include "AudioBarGpu.h"
#include "AudioDialogWordGpu.h"
#include "AudioSceneLayerAbi.h"
typedef struct NfsAudioSceneGpu {
 NfsBackgroundSceneCompositor*background;NfsAudioBarGpu*bars;
 NfsSoftwareTextGpu*sidebarCaption;NfsSoftwareTextGpu*text;
 NfsNestedCaptionGpu*sidebarWord;
 NfsAudioDialogWordGpu*dialogWords;
} NfsAudioSceneGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* All resources already prepared by their actual owner; caller retains immutable
   art/font/FramePackets throughout this synchronous borrow. Exact layer order:
   map/panel/words/logos,343 caption (or exact authored word3),authored Audio bars,full640 dialog text.
   Caller separately authenticates original scene/pass/owned staging FBO via
   stage.ownerCurrent. No cold prepare,FBO allocation,original backdrop,present
   or suppression authority. Failure means discard the entire staging image.
   Owner loss abandons all resource metadata without foreign GL cleanup. */
int nfs_audio_scene_draw_layers_into_staging(NfsAudioSceneGpu*,const NfsBackgroundSceneStage*,const NfsAudioSceneLayers*);
#ifdef __cplusplus
}
#endif
#endif
