// SPDX-License-Identifier: MIT
#pragma once
#include "../NativeSceneTarget.h"
#include <string.h>
/* Native clip rounding can produce a viewport one pixel wider than its 4:3
   attachment (e.g. height1640: texture2186, viewport2187). Admit only the exact
   existing native sampling, not arbitrary overscan or a different projection. */
static inline int nfs_intro_sampling_supported(const NfsNativeLogoTargetProof*t){
 NfsFrameLegacySampling expected;
 return t&&nfs_frame_gpu_legacy_sampling(t->width,t->height,t->sampling.magFilter,&expected)&&
  !memcmp(expected.viewport,t->sampling.viewport,sizeof expected.viewport)&&
  !memcmp(expected.projection,t->sampling.projection,sizeof expected.projection);
}
