// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_ART_PROVIDER_H
#define NFS_AUDIO_ART_PROVIDER_H
#include "AudioLayerAbi.h"
#define NFS_AUDIO_ART_PROVIDER_SCHEMA 1u
typedef struct NfsAudioArtProvider {
 uint32_t schema;void*user;
 int (*atlas)(void*,NfsAudioArt*);
} NfsAudioArtProvider;
#ifdef __cplusplus
extern "C" {
#endif
/* Trusted startup-only installation before any getter/worker/context use.
   Descriptor copied; callbacks, user and pixels stay immutable and alive for
   the process lifetime. Completeness is not provenance: the caller must first
   authenticate the whole artwork pack against an independent content pin.
   First getter seals the registry even if missing or its arguments are invalid.
   No hot swap, allocation, IO, GL or guest-memory access is performed here.
   After startup immutable concurrent lookups are supported. Metadata is checked
   here; the owning CPU packet/GPU cold upload separately verifies full digest
   and PMA pixels. Missing/invalid results leave output byte-for-byte unchanged. */
int nfs_hd_install_audio_art_provider(const NfsAudioArtProvider*);
int nfs_hd_audio_art_provider_available(void);
int nfs_hd_audio_art(NfsAudioArt*);
#ifdef __cplusplus
}
#endif
#endif
