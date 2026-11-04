// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_PLAYER_PHOTO_RGBA_PREMULTIPLIED=1};
#define NFS_PLAYER_PHOTO_NO_SOURCE UINT32_MAX
typedef struct NfsPlayerPhotoArtRect {float left,top,right,bottom;}NfsPlayerPhotoArtRect;
/* Twelve complete clean photographs, uniformly contained in an opaque307x146
   classic preview canvas, authored at1228x584. No whole-page/stat-text images. */
typedef struct NfsPlayerPhotoArtImage {uint32_t sourceIndex,format,width,height;NfsPlayerPhotoArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsPlayerPhotoArtImage;
typedef struct NfsPlayerPhotoArtInput {uint32_t schema;size_t count;const NfsPlayerPhotoArtImage*images;}NfsPlayerPhotoArtInput;
typedef struct NfsPlayerPhotoArtView {uint32_t schema,count;uint64_t generation;NfsPlayerPhotoArtImage images[1];void*owner;void(*retain)(void*);void(*release)(void*);}NfsPlayerPhotoArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable all-twelve installation. Acquired views retain pixels. */
int nfs_player_photo_art_install(const NfsPlayerPhotoArtInput*);
void nfs_player_photo_art_clear(void);
uint64_t nfs_player_photo_art_generation(void);
int nfs_player_photo_art_acquire(uint32_t sourceIndex,NfsPlayerPhotoArtView*);
#ifdef __cplusplus
}
#endif
