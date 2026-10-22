// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_BAR_GPU_H
#define NFS_AUDIO_BAR_GPU_H
#include "AudioLayerAbi.h"
#include "SceneGpuSupport.h"
/* Trusted renderer owner callback checks the ACTUAL current context and owner
   thread. It must be side-effect-free, nonthrowing and alive until retirement.
   This avoids mistaking colliding GL resource names in another context. */
typedef int (*NfsAudioBarOwnerCurrent)(void*,uint64_t,uintptr_t);
typedef struct NfsAudioBarGpu {
 uint64_t generation,lastGeneration,resourceGeneration,artIdentity;
 uintptr_t contextIdentity;NfsAudioBarOwnerCurrent ownerCurrent;void*ownerUser;
 GLuint program,texture;GLint sampler,matrix;unsigned char artSha256[32];
} NfsAudioBarGpu;
typedef struct NfsAudioBarStage {
 uint64_t generation;GLuint framebuffer;unsigned width,height;
 NfsFrameLegacySampling sampling;
} NfsAudioBarStage;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_audio_bar_context_created(NfsAudioBarGpu*,uint64_t,uintptr_t,NfsAudioBarOwnerCurrent,void*);
void nfs_audio_bar_context_retired(NfsAudioBarGpu*,uint64_t,int ownerCurrent);
int nfs_audio_bar_prepare(NfsAudioBarGpu*,const NfsAudioArt*);
/* Only caller-authenticated CURRENT unpublished native staging. The caller
   proves attachment dimensions, original scene/source/native-upload ownership
   and queue order separately. This API never grants publication/suppression.
   Rejects malformed geometry and a clipping inherited scissor before drawing.
   Any late GL failure requires discarding the whole caller staging image.
   No backdrop, FBO allocation, depth write, original-target bind or present. */
int nfs_audio_bar_draw_into_staging(NfsAudioBarGpu*,const NfsAudioBarStage*,const NfsAudioBarVertex*,size_t);
#ifdef __cplusplus
}
#endif
#endif
