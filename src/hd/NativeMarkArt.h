// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_MARK_ART_H
#define NFS_NATIVE_MARK_ART_H
#include <stddef.h>
#include <stdint.h>
#define NFS_NATIVE_MARK_ART_SCHEMA 1u
#define NFS_NATIVE_MARK_SOURCE_RGBA5551_LE 1u
#define NFS_NATIVE_MARK_SOURCE_EDGE 128u
#define NFS_NATIVE_MARK_MAX_TEXTURES 8u
#define NFS_NATIVE_MARK_MAX_BINDINGS 64u
/* Immutable borrowed program-lifetime canonical128 canvas,512-square RGBA8 PMA.
   Opacity is not baked. Content authentication is the trusted loader's job. */
typedef struct NfsNativeMarkArt {
 const unsigned char*pixels;size_t bytes;
 uint32_t width,height,logicalWidth,logicalHeight,schema,premultiplied,opacityBaked;
 uint64_t identity;unsigned char sha256[32];
} NfsNativeMarkArt;
#endif
