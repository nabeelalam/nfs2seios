// SPDX-License-Identifier: MIT
#ifndef NFS_SOFTWARE_TEXT_GPU_H
#define NFS_SOFTWARE_TEXT_GPU_H
#include "SoftwareTextVertex.h"
#include "SceneGpuSupport.h"
#include "SoftwareTextLayerAbi.h"
typedef struct NfsSoftwareTextGpu {
 uint64_t generation,lastGeneration,ownerGeneration,fontIdentity;
 GLuint program,texture;GLint sampler,matrix;unsigned char fontSha256[32];
} NfsSoftwareTextGpu;
typedef struct NfsSoftwareTextStage {
 GLuint framebuffer;unsigned width,height;NfsFrameLegacySampling sampling;
 /* Actual original logical surface bounds; only supported 343/640 by480. */
 unsigned logicalWidth,logicalHeight;
} NfsSoftwareTextStage;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_soft_gpu_context_created(NfsSoftwareTextGpu*,uint64_t generation);
void nfs_soft_gpu_context_retired(NfsSoftwareTextGpu*,uint64_t generation,int ownerCurrent);
int nfs_soft_gpu_prepare(NfsSoftwareTextGpu*,const NfsSoftwareTextFontView*);
/* Synchronous borrowed data, no pointers retained. CURRENT unpublished staging
   only. On a late failure discard the entire staging transaction. No original
   render target writes, queue work, owner selection or upload authority here. */
int nfs_soft_gpu_draw_into_staging(NfsSoftwareTextGpu*,const NfsSoftwareTextStage*,const NfsSoftwareTextVertex*,size_t count);
#ifdef __cplusplus
}
#endif
#endif
