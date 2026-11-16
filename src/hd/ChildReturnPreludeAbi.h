// SPDX-License-Identifier: MIT
#ifndef NFS_CHILD_RETURN_PRELUDE_ABI_H
#define NFS_CHILD_RETURN_PRELUDE_ABI_H
#include "ChildExitPreludeAbi.h"
/* Actual Escape routes, independently owned. The semantic member reuses only
 * the typed immutable resource transport shape, never preview-change authority.
 * Fade: 12 phases 140..250. Reverse: 19 origins 0..-270. */
enum {NFS_CHILD_RETURN_FADE=1,NFS_CHILD_RETURN_REVERSE=2};
enum {NFS_CHILD_RETURN_CLEAR=1,NFS_CHILD_RETURN_BASE_BEGIN=2,NFS_CHILD_RETURN_BASE_END=3,NFS_CHILD_RETURN_SIDEBAR_BEGIN=4,NFS_CHILD_RETURN_SIDEBAR_END=5,NFS_CHILD_RETURN_PHOTO_BEGIN=6,NFS_CHILD_RETURN_PHOTO_END=7,NFS_CHILD_RETURN_BRANDING_BEGIN=8,NFS_CHILD_RETURN_BRANDING_END=9,NFS_CHILD_RETURN_READY=10,NFS_CHILD_RETURN_END=11};
typedef struct NfsChildReturnPreludeView {
 uint32_t schema,route,frameIndex; int32_t minimum,maximum,direction,phase,originX,originY,stepX; uint32_t rgba;
 NfsChildExitPreludeView semantic;
 const uint16_t*baseOriginal565;size_t baseWords;uint32_t baseKeys[300],baseMapRole;uint64_t baseContent;
}NfsChildReturnPreludeView;
typedef NfsChildExitQuad NfsChildReturnQuad;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_child_return_begin(const NfsChildReturnPreludeView*);
int nfs_private_child_return_stage(uint64_t,unsigned,int32_t,int32_t);
int nfs_private_child_return_quad(const NfsChildReturnQuad*);
int nfs_private_child_return_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
#endif
