// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_LOCATION_PHOTO_RGBA_PREMULTIPLIED=1,NFS_LOCATION_PHOTO_SOURCE_COUNT=8};
#define NFS_LOCATION_PHOTO_NO_SOURCE UINT32_MAX
typedef struct NfsLocationPhotoArtRect {float left,top,right,bottom;}NfsLocationPhotoArtRect;
/* Eight complete clean photographs, uniformly contained in an native-coverage307x146/147
   classic preview canvas, authored at1228x584/588. No whole-page/stat-text images. */
typedef struct NfsLocationPhotoArtImage {uint32_t sourceIndex,format,width,height;NfsLocationPhotoArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsLocationPhotoArtImage;
typedef struct NfsLocationPhotoArtInput {uint32_t schema;size_t count;const NfsLocationPhotoArtImage*images;}NfsLocationPhotoArtInput;
typedef struct NfsLocationPhotoArtView {uint32_t schema,count;uint64_t generation;NfsLocationPhotoArtImage images[1];void*owner;void(*retain)(void*);void(*release)(void*);}NfsLocationPhotoArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable all-eight installation. Acquired views retain pixels. */
int nfs_location_photo_art_install(const NfsLocationPhotoArtInput*);
void nfs_location_photo_art_clear(void);
uint64_t nfs_location_photo_art_generation(void);
int nfs_location_photo_art_acquire(uint32_t sourceIndex,NfsLocationPhotoArtView*);
#ifdef __cplusplus
}
#endif
