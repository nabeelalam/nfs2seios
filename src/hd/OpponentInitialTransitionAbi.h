// SPDX-License-Identifier: MIT
#ifndef NFS_OPPONENT_INITIAL_TRANSITION_ABI_H
#define NFS_OPPONENT_INITIAL_TRANSITION_ABI_H
#include "SoftwareTextLayerAbi.h"
#define NFS_OPPONENT_INITIAL_TRANSITION_SCHEMA 1u
typedef struct NfsOpponentInitialTransitionView {uint32_t schema,form;uint64_t ownerEpoch,captionSerial,captionLifetime;NfsSoftwareTextLayerView caption;}NfsOpponentInitialTransitionView;
#endif
