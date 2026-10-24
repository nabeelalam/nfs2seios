// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_SCENE_LAYER_ABI_H
#define NFS_CONTROLS_SETUP_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "ControlsSetupWordRuntimeAbi.h"
#include "ControlsBindingPhraseRuntimeAbi.h"
/* Synchronous immutable borrows. No target or publication permission. */
typedef struct NfsControlsSetupSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView sidebarCaption,text;
 NfsCompactCaptionWordView sidebarWord;
 NfsControlsSetupWordsView childWords;
 NfsControlsBindingPhrasesView bindingPhrases;
} NfsControlsSetupSceneLayers;
#endif
