// SPDX-License-Identifier: MIT
#ifndef NFS_BACKGROUND_SCENE_LAYERS_ABI_H
#define NFS_BACKGROUND_SCENE_LAYERS_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
/* Immutable CPU-owned layer description only; no GPU or live-pass authority. */
typedef struct NfsBackgroundSceneLayers {
 uint32_t mapRole,wordCount;int32_t panelClip[4];
 NfsBackgroundSceneWordLayer words[4];NfsSmallLogoLayer logos[3];
} NfsBackgroundSceneLayers;
#endif
