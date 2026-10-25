// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_BACKDROP_GPU_H
#define NFS_FEEDBACK_MODAL_BACKDROP_GPU_H
#include "FeedbackModalBackdropArtAbi.h"
/* GL types and neutral owner/stage helpers are supplied by the renderer. */
typedef struct NfsFeedbackModalBackdropGpu {
 uint64_t generation;GLuint program,textures[2];GLint matrix,sampler,kind;unsigned char key[32];
} NfsFeedbackModalBackdropGpu;
typedef struct NfsFeedbackModalBackdropGpuAliases {
 const GLuint*programs;size_t programCount;const GLuint*textures;size_t textureCount;
} NfsFeedbackModalBackdropGpuAliases;
#endif
