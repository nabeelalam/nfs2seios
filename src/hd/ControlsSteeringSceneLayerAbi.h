// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_SCENE_LAYER_ABI_H
#define NFS_CONTROLS_STEERING_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "ControlsSteeringWordRuntimeAbi.h"
#include "ControlsSteeringBarRuntimeAbi.h"
/* Synchronous immutable borrows. No target or publication permission. */
typedef struct NfsControlsSteeringSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView sidebarCaption,text;
 NfsCompactCaptionWordView sidebarWord;
 NfsControlsSteeringWordsView childWords;
 NfsControlsSteeringBarView chart;
} NfsControlsSteeringSceneLayers;
#endif
