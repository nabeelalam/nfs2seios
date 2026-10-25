// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_SCENE_LAYERS_H
#define NFS_FEEDBACK_MODAL_SCENE_LAYERS_H
#include "ControlsSceneLayers.h"
#include "FeedbackModalWordGpu.h"
#include "FeedbackModalBackdropGpu.h"
#include "FeedbackModalSceneLayerAbi.h"
typedef struct NfsFeedbackModalSceneGpu {
 NfsControlsSceneGpu*parent;NfsFeedbackModalBackdropGpu*backdrop;NfsFeedbackModalWordGpu*words;
} NfsFeedbackModalSceneGpu;
/* Exhaustive additional resources outside this bundle, including original and
   target texture names. At most64 names per namespace. Own parent/popup names
   are independently enumerated and must not occur here. */
typedef struct NfsFeedbackModalSceneAliases {
 const GLuint*programs;size_t programCount;const GLuint*textures;size_t textureCount;
} NfsFeedbackModalSceneAliases;
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared-only checks; both functions borrow the same caller-owned unpublished
   stage. A false result requires fallback/discard, never partial publication. */
int nfs_feedback_modal_scene_preflight(NfsFeedbackModalSceneGpu*,const NfsBackgroundSceneStage*,const NfsFeedbackModalSceneLayers*,const NfsFeedbackModalSceneAliases*);
int nfs_feedback_modal_scene_draw_layers_into_staging(NfsFeedbackModalSceneGpu*,const NfsBackgroundSceneStage*,const NfsFeedbackModalSceneLayers*,const NfsFeedbackModalSceneAliases*);
#ifdef __cplusplus
}
#endif
#endif
