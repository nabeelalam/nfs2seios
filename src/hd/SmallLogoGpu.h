// SPDX-License-Identifier: MIT
#ifndef NFS_SMALL_LOGO_GPU_H
#define NFS_SMALL_LOGO_GPU_H
#include "SmallLogoArtAbi.h"
#include "SceneGpuSupport.h"
#include "SmallLogoLayerAbi.h"
typedef struct NfsSmallLogoStage {GLuint framebuffer;unsigned width,height;NfsFrameLegacySampling sampling;} NfsSmallLogoStage;
typedef struct NfsSmallLogoGpu {uint64_t generation,lastGeneration,ownerGeneration,artIdentity;GLuint program,texture;GLint sampler,matrix;NfsSmallLogoRegion regions[5];unsigned char artSHA256[32];} NfsSmallLogoGpu;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_logo_gpu_context_created(NfsSmallLogoGpu*,uint64_t generation);
void nfs_logo_gpu_context_retired(NfsSmallLogoGpu*,uint64_t generation,int ownerCurrent);
int nfs_logo_gpu_prepare(NfsSmallLogoGpu*,const NfsSmallLogoArt*,const NfsSmallLogoRegion regions[5]);
/* Caller owns a CURRENT unpublished staging FBO, dimensions and original queue flush.
   No raw pointer retained. No original target writes. A late draw fault may partially
   change staging: caller must discard the entire transaction on return0. */
int nfs_logo_gpu_draw_into_staging(NfsSmallLogoGpu*,const NfsSmallLogoStage*,const NfsSmallLogoLayer*,size_t count);
#ifdef __cplusplus
}
#endif
#endif
