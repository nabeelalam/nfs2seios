// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_TRACK_INFO_RGBA_PREMULTIPLIED=1};
typedef struct NfsTrackInfoArtRect {float left,top,right,bottom;}NfsTrackInfoArtRect;
/* Bounds use classic logical coordinates. Wider authored masters may expose
   overscan; focalBounds identifies retained scenery/content for aspect-aware
   placement. Do not crop or stretch the master to fake a different aspect. */
typedef struct NfsTrackInfoArtImage {uint32_t sourceIndex,format,width,height;NfsTrackInfoArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsTrackInfoArtImage;
typedef struct NfsTrackInfoArtInput {uint32_t schema;size_t count;const NfsTrackInfoArtImage*images;}NfsTrackInfoArtInput;
typedef struct NfsTrackInfoArtView {uint32_t schema;uint64_t generation;NfsTrackInfoArtImage images[3];void*owner;void(*retain)(void*);void(*release)(void*);}NfsTrackInfoArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable PMA RGBA installation. Missing page assets never
   permit a partial scene: acquire succeeds only for all three exact sources. */
int nfs_track_info_art_install(const NfsTrackInfoArtInput*);
void nfs_track_info_art_clear(void);
uint64_t nfs_track_info_art_generation(void);
int nfs_track_info_art_acquire(const uint32_t sourceIndices[3],NfsTrackInfoArtView*);
#ifdef __cplusplus
}
#endif
