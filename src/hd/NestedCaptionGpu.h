// SPDX-License-Identifier: MIT
#ifndef NFS_NESTED_CAPTION_GPU_H
#define NFS_NESTED_CAPTION_GPU_H
#include "CompactCaptionWordRuntimeAbi.h"
typedef struct NfsNestedCaptionGpu {uint64_t generation;GLuint program,texture;GLint matrix,sampler;unsigned word,traceLines;} NfsNestedCaptionGpu;
#endif
