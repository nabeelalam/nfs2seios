// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_WORD_GPU_H
#define NFS_CONTROLS_SETUP_WORD_GPU_H
#include "ControlsSetupWordRuntimeAbi.h"
typedef struct NfsControlsSetupWordGpu {
 uint64_t generation;GLuint program,textures[NFS_CONTROLS_SETUP_WORD_IMAGES];
 GLint matrix,sampler;unsigned char key[32];
} NfsControlsSetupWordGpu;
#endif
