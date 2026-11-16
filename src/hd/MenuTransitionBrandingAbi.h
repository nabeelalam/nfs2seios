// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsMenuTransitionBrandingSlot {
 uint32_t index,enabled,descriptorGuest,descriptorWords[7];int32_t centerY;
 uint8_t cpuSha256[32],convertedSha256[32]; /* reserved zero; retired CPU scratch is never read */
} NfsMenuTransitionBrandingSlot;
typedef struct NfsMenuTransitionBrandingView {
 uint64_t serial,content,sourceLife;uintptr_t sourceGame,sourceCatalog;
 uint32_t sourceArchive,kind,form,gameMode,slotCount;uint64_t producerEpoch;
 NfsMenuTransitionBrandingSlot slots[6];
} NfsMenuTransitionBrandingView;
typedef struct NfsMenuTransitionBrandingQuad {
 uint64_t serial;uint32_t ordinal,slot;
 uint32_t sourceVertices[4][8],indices[4],nativeVertices[4][15];
} NfsMenuTransitionBrandingQuad;
int nfs_private_menu_branding_begin(const NfsMenuTransitionBrandingView*);
int nfs_private_menu_branding_quad(const NfsMenuTransitionBrandingQuad*);
int nfs_private_menu_branding_end(uint64_t);
void nfs_private_menu_branding_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
