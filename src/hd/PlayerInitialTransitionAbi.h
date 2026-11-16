// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerCaptionRuntimeAbi.h"
#define NFS_PLAYER_INITIAL_TRANSITION_SCHEMA 1u
/* Exact original P1/P2 initial343 sidebar and caption only; no photo authority. */
typedef struct NfsPlayerInitialTransitionView {
 uint32_t schema,form;uint64_t ownerEpoch,captionSerial,captionLifetime;
 NfsPlayerCaptionWordView caption;
}NfsPlayerInitialTransitionView;
