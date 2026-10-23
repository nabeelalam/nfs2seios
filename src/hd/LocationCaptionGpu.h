// SPDX-License-Identifier: MIT
#ifndef NFS_LOCATION_CAPTION_GPU_H
#define NFS_LOCATION_CAPTION_GPU_H
#include "CompactCaptionWordRuntimeAbi.h"
typedef struct NfsLocationCaptionGpu {uint64_t generation;GLuint program,texture;GLint matrix,sampler;unsigned word,traceLines;} NfsLocationCaptionGpu;
#endif
