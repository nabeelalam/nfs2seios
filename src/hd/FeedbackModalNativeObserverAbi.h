// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_NATIVE_OBSERVER_ABI_H
#define NFS_FEEDBACK_MODAL_NATIVE_OBSERVER_ABI_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Private immutable final-popup lease. No source metadata grants renderer rights. */
typedef struct NfsFeedbackModalNativeView {
 uint32_t schema,width,height;uint64_t serial,modalEpoch;
 const uint16_t*final565;size_t planeWords;
 void*lifetimeOwner;void(*retainOwner)(void*);void(*releaseOwner)(void*);
} NfsFeedbackModalNativeView;
typedef struct NfsFeedbackModalNativeReceipt {
 uint32_t schema,admitted,reason,batches;uint64_t serial,modalEpoch,contextGeneration;
} NfsFeedbackModalNativeReceipt;
int nfs_private_feedback_modal_observer_begin(const NfsFeedbackModalNativeView*);
int nfs_private_feedback_modal_observer_finish(uint64_t serial,uint64_t modalEpoch);
/* All entry points run on the registered renderer thread. */
void nfs_private_feedback_modal_observer_cancel(uint64_t serial,uint64_t modalEpoch);
/* One-shot receipt only: never grants staging, publication or suppression rights. */
int nfs_private_feedback_modal_observer_take_receipt(uint64_t serial,uint64_t modalEpoch,NfsFeedbackModalNativeReceipt*);
#ifdef __cplusplus
}
#endif
#endif
