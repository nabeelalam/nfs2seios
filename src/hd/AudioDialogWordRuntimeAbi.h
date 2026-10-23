// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_DIALOG_WORD_RUNTIME_ABI_H
#define NFS_AUDIO_DIALOG_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_AUDIO_DIALOG_WORD_SCHEMA 2u
#define NFS_AUDIO_DIALOG_WORD_IMAGES 11u
#define NFS_AUDIO_DIALOG_WORD_PLACEMENTS 22u
/* Finite whole-word images only. All zero retains original full640 text.
   Ordered placements replace exact original vertex spans, preserving all
   intervening dynamic text and original shadow/foreground order. */
typedef struct NfsAudioDialogWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsAudioDialogWordImageView;
typedef struct NfsAudioDialogWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsAudioDialogWordPlacement;
typedef struct NfsAudioDialogWordsView {
 uint32_t schema,count;
 NfsAudioDialogWordImageView images[NFS_AUDIO_DIALOG_WORD_IMAGES];
 NfsAudioDialogWordPlacement placements[NFS_AUDIO_DIALOG_WORD_PLACEMENTS];
} NfsAudioDialogWordsView;
#endif
