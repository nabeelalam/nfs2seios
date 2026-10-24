// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_BINDING_PHRASE_GPU_H
#define NFS_CONTROLS_BINDING_PHRASE_GPU_H
#include "ControlsBindingPhraseRuntimeAbi.h"
/* Context-owned complete phrases only. This cache grants no source or publication authority. */
typedef struct NfsControlsBindingPhraseGpu {
 uint64_t generation;
 GLuint program,textures[NFS_CONTROLS_BINDING_PHRASE_SLOTS];
 unsigned phrases[NFS_CONTROLS_BINDING_PHRASE_SLOTS];
 GLint matrix,sampler;
 size_t textVertexCount;
 unsigned char key[32];
} NfsControlsBindingPhraseGpu;
#endif
