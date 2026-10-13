// SPDX-License-Identifier: MIT
#pragma once
#include "Glide2x.h"
#ifdef __cplusplus
extern "C" {
#endif
enum {NFS_SURFACE_WIDTH=640,NFS_SURFACE_HEIGHT=480,NFS_SURFACE_STRIDE=1280,NFS_SURFACE_BYTES=614400};
typedef struct {
    BOOL (*read)(GrBuffer_t,GrOriginLocation_t,void *);
    BOOL (*write)(GrBuffer_t,GrOriginLocation_t,const void *);
} NfsSurfaceOperations;
BOOL nfs_glide_query_hardware(GrHwConfiguration *configuration);
BOOL nfs_glide_lfb_lock(GrLock_t,GrBuffer_t,GrLfbWriteMode_t,GrOriginLocation_t,BOOL,GrLfbInfo_t *,NfsSurfaceOperations);
BOOL nfs_glide_lfb_unlock(GrLock_t,GrBuffer_t);
void nfs_glide_lfb_reset(void);
#ifdef __cplusplus
}
#endif
