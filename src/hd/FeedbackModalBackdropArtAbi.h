// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_BACKDROP_ART_ABI_H
#define NFS_FEEDBACK_MODAL_BACKDROP_ART_ABI_H
#include <stddef.h>
#include <stdint.h>
#define NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA 1u
#define NFS_FEEDBACK_MODAL_BACKDROP_ART_IMAGES 2u
#define NFS_FEEDBACK_MODAL_BACKDROP_BANNER_WIDTH 1376u
#define NFS_FEEDBACK_MODAL_BACKDROP_BANNER_HEIGHT 188u
#define NFS_FEEDBACK_MODAL_BACKDROP_BANNER_BYTES 1034752u
#define NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_WIDTH 468u
#define NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_HEIGHT 244u
#define NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM_BYTES 456768u
enum {NFS_FEEDBACK_MODAL_BACKDROP_BANNER=1,NFS_FEEDBACK_MODAL_BACKDROP_EMBLEM=2};
/* Borrowed immutable straight RGBA from a separately retained authored provider.
   SHA pins belong to that provider, not to source/native ownership. No original
   pixels, private artwork or candidate hashes are embedded in this interface. */
typedef struct NfsFeedbackModalBackdropArtImage {
 uint32_t schema,role,width,height,straightRgba;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsFeedbackModalBackdropArtImage;
typedef struct NfsFeedbackModalBackdropArtView {
 uint32_t schema,count;NfsFeedbackModalBackdropArtImage images[2];
} NfsFeedbackModalBackdropArtView;
#endif
