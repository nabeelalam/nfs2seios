// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
// Immutable renderer borrow. Both image and text live until release(owner).
typedef struct NfsSlideshowLiveView {
 uint32_t schema,width,height,imageWidth,imageHeight;uint64_t request,content;
 const uint16_t*original;size_t pixels;
 const unsigned char*rgba;size_t rgbaBytes;unsigned char imageSha[32];
 // Schema2: optional complete4:3 PMA footer canvas, owned by the same lease.
 uint32_t footerWidth,footerHeight;const unsigned char*footer;size_t footerBytes;unsigned char footerSha[32];
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
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
