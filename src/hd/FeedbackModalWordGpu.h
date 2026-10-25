// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_WORD_GPU_H
#define NFS_FEEDBACK_MODAL_WORD_GPU_H
#include "FeedbackModalWordRuntimeAbi.h"
/* GLuint/GLint and the neutral guarded stage types are supplied by the renderer.
   This resource has no scene, source, queue or publication authority. */
typedef struct NfsFeedbackModalWordGpu {
 uint64_t generation;GLuint program,textures[2];GLint matrix,sampler;unsigned char key[32];
} NfsFeedbackModalWordGpu;
/* Caller lists every independently owned/borrowed program and sampled/target
   texture that must remain distinct. Names belong to their own GL namespaces. */
typedef struct NfsFeedbackModalWordGpuAliases {
 const GLuint*programs;size_t programCount;const GLuint*textures;size_t textureCount;
} NfsFeedbackModalWordGpuAliases;
#endif
