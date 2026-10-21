// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_SCENE_TARGET_H
#define NFS_NATIVE_SCENE_TARGET_H
#include "SceneGpuSupport.h"
typedef struct NfsNativeLogoTargetProof {
 GLuint framebuffer;unsigned width,height;uint64_t contextGeneration;
 NfsFrameLegacySampling sampling;
 /* Independently current adapter proof, tied to this frame's serial. Not inferred from GL clear-value. */
 uint64_t depthClearSerial;unsigned depthClearProven;float depthClearValue;
 /* Call-bound current RGB-black proof; historical clear properties are insufficient. */
 uint64_t prebaseColorClearEpoch;unsigned prebaseBlackProven;float prebaseClearRGB[3];
} NfsNativeLogoTargetProof;
#endif
