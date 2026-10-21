// SPDX-License-Identifier: MIT
#ifndef NFS_SCENE_GPU_SUPPORT_H
#define NFS_SCENE_GPU_SUPPORT_H
#include <stdint.h>
#include <stddef.h>
#define NFS_FRAME_MAX_DIMENSION 4096u
#define NFS_FRAME_MAX_PIXELS 16777216u
typedef struct NfsFrameLegacySampling {GLenum magFilter;GLfloat projection[16];GLint viewport[4];} NfsFrameLegacySampling;
typedef struct NfsSoftwareTextGpuAttrib {GLint enabled,size,type,normalized,stride,buffer;void*pointer;GLfloat current[4];} NfsSoftwareTextGpuAttrib;
typedef struct NfsFrameSaved {
 GLint program,active,texture,buffer,fbo,rbo,viewport[4],pack,unpack;
 GLint sr,dr,sa,da,eqr,eqa;GLboolean mask[4],caps[8];
 NfsSoftwareTextGpuAttrib attributes[4];
} NfsFrameSaved;
int nfs_frame_gpu_legacy_sampling(unsigned,unsigned,GLenum,NfsFrameLegacySampling*);
#endif
