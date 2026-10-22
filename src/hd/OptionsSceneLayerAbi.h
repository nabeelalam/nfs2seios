// SPDX-License-Identifier: MIT
#ifndef NFS_OPTIONS_SCENE_LAYER_ABI_H
#define NFS_OPTIONS_SCENE_LAYER_ABI_H
#include "BackgroundSceneLayersAbi.h"
#include "SoftwareTextLayerAbi.h"
/* Immutable synchronous borrows. Source and publication authority are separate. */
typedef struct NfsOptionsSceneLayers {
 NfsBackgroundSceneLayers background;
 NfsSoftwareTextLayerView caption;
} NfsOptionsSceneLayers;
#endif
