// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_DIALOG_WORD_GPU_H
#define NFS_CONTROLS_DIALOG_WORD_GPU_H
#include "ControlsDialogWordRuntimeAbi.h"
typedef struct NfsControlsDialogWordGpu {uint64_t generation;GLuint program,textures[5];GLint matrix,sampler;unsigned char key[32];} NfsControlsDialogWordGpu;
#endif
