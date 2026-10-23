// SPDX-License-Identifier: MIT
#ifndef NFS_GRAPHICS_DIALOG_WORD_GPU_H
#define NFS_GRAPHICS_DIALOG_WORD_GPU_H
#include "GraphicsDialogWordRuntimeAbi.h"
typedef struct NfsGraphicsDialogWordGpu {uint64_t generation;GLuint program,textures[8];GLint matrix,sampler;unsigned char key[32];} NfsGraphicsDialogWordGpu;
#endif
