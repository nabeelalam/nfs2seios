// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_CLASS_PICTURE_RGBA_PREMULTIPLIED=1};
#define NFS_CLASS_PICTURE_NO_SOURCE UINT32_MAX
typedef struct NfsClassPictureArtRect {float left,top,right,bottom;}NfsClassPictureArtRect;
/* Three class collages on their original306x146
   classic preview canvas, authored at1224x584. No whole-page/stat-text images. */
typedef struct NfsClassPictureArtImage {uint32_t sourceIndex,format,width,height;NfsClassPictureArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsClassPictureArtImage;
typedef struct NfsClassPictureArtInput {uint32_t schema;size_t count;const NfsClassPictureArtImage*images;}NfsClassPictureArtInput;
typedef struct NfsClassPictureArtView {uint32_t schema,count;uint64_t generation;NfsClassPictureArtImage images[1];void*owner;void(*retain)(void*);void(*release)(void*);}NfsClassPictureArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable all-three installation. Acquired views retain pixels. */
int nfs_class_picture_art_install(const NfsClassPictureArtInput*);
void nfs_class_picture_art_clear(void);
uint64_t nfs_class_picture_art_generation(void);
int nfs_class_picture_art_acquire(uint32_t sourceIndex,NfsClassPictureArtView*);
#ifdef __cplusplus
}
#endif
