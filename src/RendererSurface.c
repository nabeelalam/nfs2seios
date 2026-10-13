// SPDX-License-Identifier: MIT
#include "RendererSurface.h"
#include "GuestRuntimeC.h"
#include <string.h>
extern void *malloc32(size_t);
extern void free32(void *);
static void *locked_pixels;
static GrLock_t locked_type;
static GrBuffer_t locked_buffer;
static GrOriginLocation_t locked_origin;
static NfsSurfaceOperations locked_operations;
BOOL nfs_glide_query_hardware(GrHwConfiguration *configuration){
    if(!configuration)return false;
    memset(configuration,0,sizeof(*configuration));
    // Capabilities of this Glide compatibility backend, not physical Voodoo RAM.
    configuration->num_sst=1;
    configuration->SSTs[0].type=GR_SSTTYPE_VOODOO;
    GrVoodooConfig_t *board=&configuration->SSTs[0].sstBoard.VoodooConfig;
    board->fbRam=4;board->fbiRev=2;board->nTexelfx=1;
    board->tmuConfig[0].tmuRev=1;board->tmuConfig[0].tmuRam=2;
    return true;
}
BOOL nfs_glide_lfb_lock(GrLock_t type,GrBuffer_t buffer,GrLfbWriteMode_t mode,GrOriginLocation_t origin,BOOL pipeline,GrLfbInfo_t *info,NfsSurfaceOperations operations){
    if(locked_pixels || !info || info->size!=sizeof(*info) || pipeline || !operations.read || !operations.write ||
       (type!=GR_LFB_READ_ONLY && type!=GR_LFB_WRITE_ONLY) ||
       (buffer!=GR_BUFFER_FRONTBUFFER && buffer!=GR_BUFFER_BACKBUFFER) ||
       (mode!=GR_LFBWRITEMODE_565 && mode!=GR_LFBWRITEMODE_ANY) ||
       (origin!=GR_ORIGIN_UPPER_LEFT && origin!=GR_ORIGIN_LOWER_LEFT && origin!=GR_ORIGIN_ANY))return false;
    if(origin==GR_ORIGIN_ANY)origin=GR_ORIGIN_UPPER_LEFT;
    void *pixels=malloc32(NFS_SURFACE_BYTES);if(!pixels)return false;
    // Preserve untouched pixels even when the game requests a write-only lock.
    if(!operations.read(buffer,origin,pixels)){free32(pixels);return false;}
    locked_pixels=pixels;locked_type=type;locked_buffer=buffer;locked_origin=origin;locked_operations=operations;
    info->size=sizeof(*info);info->lfbPtr=nfs_guest_address(pixels);
    info->strideInBytes=NFS_SURFACE_STRIDE;info->writeMode=GR_LFBWRITEMODE_565;info->origin=origin;
    return true;
}
BOOL nfs_glide_lfb_unlock(GrLock_t type,GrBuffer_t buffer){
    if(!locked_pixels || type!=locked_type || buffer!=locked_buffer)return false;
    if(type==GR_LFB_WRITE_ONLY && !locked_operations.write(buffer,locked_origin,locked_pixels))return false;
    nfs_glide_lfb_reset();return true;
}
void nfs_glide_lfb_reset(void){if(locked_pixels)free32(locked_pixels);locked_pixels=NULL;locked_operations=(NfsSurfaceOperations){0};}
