// SPDX-License-Identifier: MIT
#ifndef NFS_GRAPHICS_SCENE_LAYER_ABI_H
#define NFS_GRAPHICS_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "GraphicsDialogWordRuntimeAbi.h"
/* Synchronous immutable borrows. No target or publication permission. */
typedef struct NfsGraphicsSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView sidebarCaption,text;
 NfsCompactCaptionWordView sidebarWord;
 NfsGraphicsDialogWordsView dialogWords;
} NfsGraphicsSceneLayers;
#endif
