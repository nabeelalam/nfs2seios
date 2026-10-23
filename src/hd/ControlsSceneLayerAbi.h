// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SCENE_LAYER_ABI_H
#define NFS_CONTROLS_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "ControlsDialogWordRuntimeAbi.h"
/* Synchronous immutable borrows. No target or publication permission. */
typedef struct NfsControlsSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView sidebarCaption,text;
 NfsCompactCaptionWordView sidebarWord;
 NfsControlsDialogWordsView dialogWords;
} NfsControlsSceneLayers;
#endif
