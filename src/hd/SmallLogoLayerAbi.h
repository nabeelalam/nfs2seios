// SPDX-License-Identifier: MIT
#ifndef NFS_SMALL_LOGO_LAYER_ABI_H
#define NFS_SMALL_LOGO_LAYER_ABI_H
#include <stdint.h>
#define NFS_SMALL_LOGO_LAYER_SCHEMA 1u
#define NFS_SMALL_LOGO_MAX_LAYERS 3u
/* Semantic source-driven layers in original order. Full coefficient is256;
   animated next-inset coefficient is0..251, divided by256 for PM RGBA.
   Positions/clips use logical game coordinates; UVs come only from pinned art
   regions. No guest pointers, resource generations or ownership authority. */
typedef struct NfsSmallLogoLayer {
 uint32_t role,scalar256;
 float left,top,right,bottom;
 float clip[4];
} NfsSmallLogoLayer;
#endif
