// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_WORD_GPU_H
#define NFS_CONTROLS_STEERING_WORD_GPU_H
#include "ControlsSteeringWordRuntimeAbi.h"
typedef struct NfsControlsSteeringWordGpu {
 uint64_t generation;GLuint program,textures[NFS_CONTROLS_STEERING_WORD_IMAGES];
 GLint matrix,sampler;unsigned char key[32];
} NfsControlsSteeringWordGpu;
#endif
