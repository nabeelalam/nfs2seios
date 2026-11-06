// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_HISTORY_RGBA_PREMULTIPLIED=1};
#define NFS_HISTORY_NO_SOURCE UINT32_MAX
typedef struct NfsHistoryArtRect {float left,top,right,bottom;}NfsHistoryArtRect;
/* Bounds use classic logical coordinates. This pass requires a complete
   640x480 opaque author canvas for every background. Contain wider clean photo
   masters uniformly inside that canvas with a deliberate matte; do not cover-
   crop or stretch. Keep independent wide masters for the adaptive renderer. */
typedef struct NfsHistoryArtImage {uint32_t sourceIndex,format,width,height;NfsHistoryArtRect logicalBounds,focalBounds;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsHistoryArtImage;
typedef struct NfsHistoryArtInput {uint32_t schema;size_t count;const NfsHistoryArtImage*images;}NfsHistoryArtInput;
typedef struct NfsHistoryArtView {uint32_t schema,count;uint64_t generation;NfsHistoryArtImage images[2];void*owner;void(*retain)(void*);void(*release)(void*);}NfsHistoryArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only immutable PMA RGBA installation. All seven backgrounds and forty-two title entries are required together.
   Titles may be transparent PMA artwork at their exact original source bounds.
   No whole-scene reuse or guessed company/locale selection. */
int nfs_history_art_install(const NfsHistoryArtInput*);
void nfs_history_art_clear(void);
uint64_t nfs_history_art_generation(void);
int nfs_history_art_acquire(const uint32_t sourceIndices[2],NfsHistoryArtView*);
#ifdef __cplusplus
}
#endif
