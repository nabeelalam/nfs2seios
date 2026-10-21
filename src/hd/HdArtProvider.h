// SPDX-License-Identifier: MIT
#ifndef NFS_HD_ART_PROVIDER_H
#define NFS_HD_ART_PROVIDER_H
#include "BackgroundSceneArt.h"
#include "SmallLogoArtAbi.h"
#define NFS_HD_ART_PROVIDER_SCHEMA 1u
typedef struct NfsHdArtProvider {
 uint32_t schema;void* user;
 int (*texture)(void*,uint32_t,NfsBackgroundSceneTexture*);
 int (*word)(void*,uint32_t,NfsBackgroundSceneWord*);
 int (*logo)(void*,NfsSmallLogoArt*);
 int (*logoRegion)(void*,uint32_t,NfsSmallLogoRegion*);
} NfsHdArtProvider;
#ifdef __cplusplus
extern "C" {
#endif
/* Trusted startup-only installation before any worker/context/getter use.
   The struct is copied, not retained. User, callbacks and every returned byte
   must remain immutable and alive for the entire program. No guest pointers.
   Installation validates completeness, NOT content provenance. Loader must
   separately authenticate its immutable pack before calling this function.
   No provider is the default: all getters return0, leaving output unchanged.
   First getter seals installation even if no provider exists; no hot swap. */
/* Read-only after startup; concurrent immutable getter calls are supported. */
int nfs_hd_art_provider_available(void);
int nfs_hd_install_art_provider(const NfsHdArtProvider*provider);
#ifdef __cplusplus
}
#endif
#endif
