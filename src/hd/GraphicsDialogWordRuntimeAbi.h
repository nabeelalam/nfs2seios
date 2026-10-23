// SPDX-License-Identifier: MIT
#ifndef NFS_GRAPHICS_DIALOG_WORD_RUNTIME_ABI_H
#define NFS_GRAPHICS_DIALOG_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_GRAPHICS_DIALOG_WORD_SCHEMA 1u
#define NFS_GRAPHICS_DIALOG_WORD_IMAGES 8u
#define NFS_GRAPHICS_DIALOG_WORD_PLACEMENTS 16u
/* Finite whole-word images only. All zero retains original full640 text.
   Ordered placements replace exact original vertex spans, preserving all
   intervening dynamic text and original shadow/foreground order. */
typedef struct NfsGraphicsDialogWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsGraphicsDialogWordImageView;
typedef struct NfsGraphicsDialogWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsGraphicsDialogWordPlacement;
typedef struct NfsGraphicsDialogWordsView {
 uint32_t schema,count;
 NfsGraphicsDialogWordImageView images[NFS_GRAPHICS_DIALOG_WORD_IMAGES];
 NfsGraphicsDialogWordPlacement placements[NFS_GRAPHICS_DIALOG_WORD_PLACEMENTS];
} NfsGraphicsDialogWordsView;
#endif
