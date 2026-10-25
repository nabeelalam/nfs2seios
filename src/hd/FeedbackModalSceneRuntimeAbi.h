// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_SCENE_RUNTIME_ABI_H
#define NFS_FEEDBACK_MODAL_SCENE_RUNTIME_ABI_H
#include "FeedbackModalSceneLayerAbi.h"
#include "FeedbackModalNativeObserverAbi.h"
#define NFS_FEEDBACK_MODAL_SCENE_RUNTIME_SCHEMA 1u
/* Synchronous immutable borrows. Caller retains actual Joined, parent, words
   and authored provider for the entire renderer callback. No field grants GPU
   ownership. Compose consumes the observer's actual one-shot receipt itself. */
typedef struct NfsFeedbackModalSceneView {
 uint32_t schema,width,height;uint64_t serial,modalEpoch;
 NfsFeedbackModalSceneLayers layers;
} NfsFeedbackModalSceneView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_feedback_modal_scene_compose(const NfsFeedbackModalSceneView*,NfsFeedbackModalNativeReceipt*);
#ifdef __cplusplus
}
#endif
#endif
