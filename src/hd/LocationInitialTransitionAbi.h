// SPDX-License-Identifier: MIT
#ifndef NFS_LOCATION_INITIAL_TRANSITION_ABI_H
#define NFS_LOCATION_INITIAL_TRANSITION_ABI_H
#include "CompactCaptionWordRuntimeAbi.h"
#define NFS_LOCATION_INITIAL_TRANSITION_SCHEMA 1u
/* Only the original343 sidebar swipe; no Location preview/full-frame authority. */
typedef struct NfsLocationInitialTransitionView {
 uint32_t schema,form;uint64_t ownerEpoch,captionSerial,captionLifetime;
 NfsCompactCaptionWordView caption;
} NfsLocationInitialTransitionView;
#endif
