// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_DIALOG_WORD_GPU_H
#define NFS_AUDIO_DIALOG_WORD_GPU_H
#include "AudioDialogWordRuntimeAbi.h"
typedef struct NfsAudioDialogWordGpu {uint64_t generation;GLuint program,textures[8];GLint matrix,sampler;unsigned char key[32];} NfsAudioDialogWordGpu;
#endif
