// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_CAR_INFO_RGBA_PREMULTIPLIED=1};
#define NFS_CAR_INFO_NO_SOURCE UINT32_MAX
typedef struct NfsCarInfoArtRect {float left,top,right,bottom;}NfsCarInfoArtRect;
/* Bounds use classic logical coordinates. This pass requires a complete
   640x480 opaque author canvas for every background. Contain wider clean photo
   masters uniformly inside that canvas with a deliberate matte; do not cover-
   crop or stretch. Keep independent wide masters for the adaptive renderer. */
typedef struct NfsCarInfoArtImage {uint32_t sourceIndex,format,width,height;NfsCarInfoArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsCarInfoArtImage;
typedef struct NfsCarInfoArtInput {uint32_t schema;size_t count;const NfsCarInfoArtImage*images;}NfsCarInfoArtInput;
typedef struct NfsCarInfoArtView {uint32_t schema,count;uint64_t generation;NfsCarInfoArtImage images[2];void*owner;void(*retain)(void*);void(*release)(void*);}NfsCarInfoArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable PMA RGBA installation. All twelve backgrounds are
   required together; an
   omitted authored technical overlay is supplied only from the source-proved
   original overlay retained by the immutable live frame. No whole-scene reuse. */
int nfs_car_info_art_install(const NfsCarInfoArtInput*);
void nfs_car_info_art_clear(void);
uint64_t nfs_car_info_art_generation(void);
int nfs_car_info_art_acquire(const uint32_t sourceIndices[2],NfsCarInfoArtView*);
#ifdef __cplusplus
}
#endif
