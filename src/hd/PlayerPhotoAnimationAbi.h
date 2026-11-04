// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsPlayerPhotoTile {uint32_t address,format;const uint16_t*original;const unsigned char*rgba;}NfsPlayerPhotoTile;
typedef struct NfsPlayerPhotoAnimation {uint64_t serial,generation;uint32_t count;const NfsPlayerPhotoTile*tiles;}NfsPlayerPhotoAnimation;
int nfs_private_player_photo_upload(const NfsPlayerPhotoAnimation*);
int nfs_private_player_photo_begin(uint64_t,int32_t,int32_t);
int nfs_private_player_photo_end(uint64_t);
#ifdef __cplusplus
}
#endif
