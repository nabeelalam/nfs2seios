// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_SCENE_LAYER_ABI_H
#define NFS_FEEDBACK_MODAL_SCENE_LAYER_ABI_H
#include "ControlsSceneLayerAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "FeedbackModalWordRuntimeAbi.h"
#include "FeedbackModalBackdropArtAbi.h"
#define NFS_FEEDBACK_MODAL_SCENE_LAYERS_SCHEMA 1u
/* Synchronous immutable borrows. No source, native or publication capability. */
typedef struct NfsFeedbackModalSceneLayers {
 uint32_t schema;NfsControlsSceneLayers parent;
 NfsSoftwareTextLayerView text;NfsFeedbackModalWordsView words;
 NfsFeedbackModalBackdropArtView backdrop;
} NfsFeedbackModalSceneLayers;
#endif
