// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
enum {NFS_CHILD_PHOTO_ORIGINAL=1,NFS_CHILD_PHOTO_AUTHORED=2};
typedef struct NfsChildPhotoTile {uint32_t address,format;const uint16_t*original;const unsigned char*rgba;}NfsChildPhotoTile;
typedef struct NfsChildPhotoAnimation {uint64_t serial,generation,sourceLife,epoch;uintptr_t game,sourceCatalog;uint32_t family,form,archive,count,mode;const NfsChildPhotoTile*tiles;void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);}NfsChildPhotoAnimation;
int nfs_private_child_photo_upload(const NfsChildPhotoAnimation*);
int nfs_private_child_photo_begin(uint32_t,uint64_t,int32_t,int32_t);
int nfs_private_child_photo_end(uint32_t,uint64_t);
#ifdef __cplusplus
}
#endif
