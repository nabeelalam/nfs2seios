// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_SCENE_LAYER_ABI_H
#define NFS_AUDIO_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "AudioDialogWordRuntimeAbi.h"
#include "AudioLayerAbi.h"
/* Synchronous immutable native-owned borrows; packet/catalog/art owners retain
   every byte. No GL dependency or frame/target permission is encoded here. */
typedef struct NfsAudioSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView sidebarCaption,text;
 const NfsAudioBarVertex*bars;size_t barVertices;
 NfsCompactCaptionWordView sidebarWord;
 NfsAudioDialogWordsView dialogWords;
} NfsAudioSceneLayers;
#endif
