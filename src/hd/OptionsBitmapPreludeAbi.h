// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneRuntimeAbi.h"
#include "BackgroundSceneLayersAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#define NFS_OPTIONS_BITMAP_PRELUDE_SCHEMA 1u
/* Independent original426950 RGB565 pass; no cached1555 addresses or authority. */
typedef struct NfsOptionsBitmapPreludeView {
 uint32_t schema,phase;uint64_t serial,content;size_t planeWords;
 const uint16_t*original565;NfsBackgroundSceneLayers background;
 NfsCompactCaptionWordView caption;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
}NfsOptionsBitmapPreludeView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_options_bitmap_begin(const NfsOptionsBitmapPreludeView*);
int nfs_private_options_bitmap_finish(uint64_t);
#ifdef __cplusplus
}
#endif
