// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneLayersAbi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsMenuTransitionView {
 uint64_t serial,content;uint32_t slot,kind,mapRole; /* kind1 map,kind2 sidebar,kind3 complete Main,kind4 initial sidebar */
 uint32_t drawKind; /* 1 original427C70 gradient,2 original427B30 white pass */
 int32_t minimum,maximum,direction,phase,originX,originY,stepX;
 const uint16_t*original565;uint32_t addresses[300];NfsBackgroundSceneLayers layers;
} NfsMenuTransitionView;
int nfs_private_menu_transition_begin(const NfsMenuTransitionView*);
int nfs_private_menu_transition_finish(uint64_t);
#ifdef __cplusplus
}
#endif
