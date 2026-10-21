// SPDX-License-Identifier: MIT
#ifndef NFS_SMALL_LOGO_ART_ABI_H
#define NFS_SMALL_LOGO_ART_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_SMALL_LOGO_ART_SCHEMA 1u
#define NFS_SMALL_LOGO_ART_WIDTH 2048u
#define NFS_SMALL_LOGO_ART_HEIGHT 768u
#define NFS_SMALL_LOGO_ART_BYTES ((size_t)2048u*768u*4u)
enum {NFS_SMALL_LOGO_ROLE_LOGO=0,NFS_SMALL_LOGO_ROLE_GREEN=1,NFS_SMALL_LOGO_ROLE_RED=2,NFS_SMALL_LOGO_ROLE_YELLOW=3,NFS_SMALL_LOGO_ROLE_BLUE=4,NFS_SMALL_LOGO_ROLE_COUNT=5};
typedef struct NfsSmallLogoArt {const unsigned char*pixels;size_t bytes;uint32_t width,height,schema,premultiplied,opacityBaked;uint64_t identity;unsigned char sha256[32];} NfsSmallLogoArt;
typedef struct NfsSmallLogoRegion {uint32_t role,x,y,width,height,scale;int32_t logicalWidth,logicalHeight,sourceOriginX,sourceOriginY;} NfsSmallLogoRegion;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_small_logo_art(NfsSmallLogoArt*out);
int nfs_private_small_logo_region(uint32_t role,NfsSmallLogoRegion*out);
#ifdef __cplusplus
}
#endif
#endif
