// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_TRACK_BADGE_RGBA_PREMULTIPLIED=1};
#define NFS_TRACK_BADGE_NO_SOURCE UINT32_MAX
typedef struct NfsTrackBadgeArtRect {float left,top,right,bottom;}NfsTrackBadgeArtRect;
/* Eighteen independently registered rec6/rec7 badge canvases (nine sources).
   Premultiplied RGBA at4x on the original256-square staging canvas. */
typedef struct NfsTrackBadgeArtImage {uint32_t sourceIndex,format,width,height;NfsTrackBadgeArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsTrackBadgeArtImage;
typedef struct NfsTrackBadgeArtInput {uint32_t schema;size_t count;const NfsTrackBadgeArtImage*images;}NfsTrackBadgeArtInput;
typedef struct NfsTrackBadgeArtView {uint32_t schema,count;uint64_t generation;NfsTrackBadgeArtImage images[1];void*owner;void(*retain)(void*);void(*release)(void*);}NfsTrackBadgeArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable all-eighteen installation. Acquired views retain pixels. */
int nfs_track_badge_art_install(const NfsTrackBadgeArtInput*);
void nfs_track_badge_art_clear(void);
uint64_t nfs_track_badge_art_generation(void);
int nfs_track_badge_art_acquire(uint32_t sourceIndex,NfsTrackBadgeArtView*);
#ifdef __cplusplus
}
#endif
