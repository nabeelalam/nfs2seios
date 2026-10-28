// SPDX-License-Identifier: MIT
#ifndef NFS_ORIGINAL_PREVIEW_LAYER_ABI_H
#define NFS_ORIGINAL_PREVIEW_LAYER_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_ORIGINAL_PREVIEW_LAYER_SCHEMA 1u
#define NFS_AUTHORED_PREVIEW_LAYER_SCHEMA 2u
/* Schema1 consumers/producers retain the original fields only. Authored display
   fields are read exclusively for explicitly opted-in schema2 views. */
/* Owned immutable RGB565 result of independently replayed original preview calls.
   Bounds come from verified original writes, never the black fill size alone.
   Digest covers canonical little-endian words. Shape/digest do not grant source,
   epoch, context, native-upload or original-target publication authority. */
typedef struct NfsOriginalPreviewLayerView {
 uint32_t schema;int32_t left,top;uint32_t width,height;
 const uint16_t*pixels565;size_t pixelCount;unsigned char sha256[32];
 /* Semantic original-write coverage, strictly0or255; never before!=after. */
 const unsigned char*coverage;size_t coverageCount;unsigned char coverageSha256[32];
 const unsigned char*authoredRGBA;size_t authoredBytes;uint32_t authoredWidth,authoredHeight;unsigned char authoredSha256[32];
} NfsOriginalPreviewLayerView;
#endif
