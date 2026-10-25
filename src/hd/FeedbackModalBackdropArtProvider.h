// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_BACKDROP_ART_PROVIDER_H
#define NFS_FEEDBACK_MODAL_BACKDROP_ART_PROVIDER_H
#include "FeedbackModalBackdropArtAbi.h"
#define NFS_FEEDBACK_MODAL_BACKDROP_ART_PROVIDER_SCHEMA 1u
typedef struct NfsFeedbackModalBackdropArtProvider {
 uint32_t schema;void*user;
 int (*view)(void*,NfsFeedbackModalBackdropArtView*);
} NfsFeedbackModalBackdropArtProvider;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only, before any getter or worker. Descriptor copied; callback, user
   and borrowed pixels remain immutable and alive for the process lifetime.
   Installation is not concurrent with lookups. Concurrent immutable lookups
   after startup are supported. First getter seals even with invalid arguments
   or no provider; availability is a non-sealing startup query.
   Metadata completeness is not provenance. Authenticate the complete pack with
   an independent pin before installation; full SHA and pixel validation belong
   to the pack/GPU checks. No IO, allocation, GL, guest access or conversion here.
   A missing, failed or malformed result leaves output byte-for-byte unchanged. */
int nfs_hd_install_feedback_modal_backdrop_art_provider(const NfsFeedbackModalBackdropArtProvider*);
int nfs_hd_feedback_modal_backdrop_art_provider_available(void);
int nfs_hd_feedback_modal_backdrop_art(NfsFeedbackModalBackdropArtView*);
#ifdef __cplusplus
}
#endif
#endif
