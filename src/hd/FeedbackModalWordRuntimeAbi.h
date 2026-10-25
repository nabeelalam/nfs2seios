// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_WORD_RUNTIME_ABI_H
#define NFS_FEEDBACK_MODAL_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_FEEDBACK_MODAL_WORD_SCHEMA 1u
#define NFS_FEEDBACK_MODAL_WORD_IMAGES 2u
#define NFS_FEEDBACK_MODAL_WORD_PLACEMENTS 2u
/* Two finite unshadowed popup words. Zero view preserves the original whole
   scene. Data pointers borrow a retained Prepared owner; neither this view nor
   successful artwork decoding supplies current-source or native authority. */
typedef struct NfsFeedbackModalWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsFeedbackModalWordImageView;
typedef struct NfsFeedbackModalWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsFeedbackModalWordPlacement;
typedef struct NfsFeedbackModalWordsView {
 uint32_t schema,count;
 NfsFeedbackModalWordImageView images[NFS_FEEDBACK_MODAL_WORD_IMAGES];
 NfsFeedbackModalWordPlacement placements[NFS_FEEDBACK_MODAL_WORD_PLACEMENTS];
} NfsFeedbackModalWordsView;
#endif
