// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_DIALOG_WORD_RUNTIME_ABI_H
#define NFS_CONTROLS_DIALOG_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_CONTROLS_DIALOG_WORD_SCHEMA 1u
#define NFS_CONTROLS_DIALOG_WORD_IMAGES 5u
#define NFS_CONTROLS_DIALOG_WORD_PLACEMENTS 10u
/* Finite whole-word images only. All zero retains original full640 text.
   Ordered placements replace exact original vertex spans, preserving all
   intervening original ellipses and original shadow/foreground order. */
typedef struct NfsControlsDialogWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsControlsDialogWordImageView;
typedef struct NfsControlsDialogWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsControlsDialogWordPlacement;
typedef struct NfsControlsDialogWordsView {
 uint32_t schema,count;
 NfsControlsDialogWordImageView images[NFS_CONTROLS_DIALOG_WORD_IMAGES];
 NfsControlsDialogWordPlacement placements[NFS_CONTROLS_DIALOG_WORD_PLACEMENTS];
} NfsControlsDialogWordsView;
#endif
