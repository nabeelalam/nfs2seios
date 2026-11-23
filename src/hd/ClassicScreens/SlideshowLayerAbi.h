// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
#include "HistoryPanoramaAbi.h"
// Immutable renderer borrow. Both image and text live until release(owner).
typedef struct NfsSlideshowLiveView {
 uint32_t schema,width,height,imageWidth,imageHeight;uint64_t request,content;
 const uint16_t*original;size_t pixels;
 const unsigned char*rgba;size_t rgbaBytes;unsigned char imageSha[32];
 // Schema2: optional complete4:3 PMA footer canvas, owned by the same lease.
 uint32_t footerWidth,footerHeight;const unsigned char*footer;size_t footerBytes;unsigned char footerSha[32];
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
 // Schema3 append-only optional CPU panorama, independently retained by its owner.
 // Schema2 callers end immediately before this field. Never native-pass authority.
 NfsHistoryPanoramaView panorama;
}NfsSlideshowLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_slideshow_begin(const NfsSlideshowLiveView*);
int nfs_private_slideshow_finish(uint64_t);
int nfs_private_slideshow_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
