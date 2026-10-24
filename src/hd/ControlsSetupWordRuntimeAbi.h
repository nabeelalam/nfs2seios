// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_WORD_RUNTIME_ABI_H
#define NFS_CONTROLS_SETUP_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_CONTROLS_SETUP_WORD_SCHEMA 1u
#define NFS_CONTROLS_SETUP_WORD_IMAGES 28u
#define NFS_CONTROLS_SETUP_WORD_PLACEMENTS 48u
/* Finite whole-word images only. All zero retains original full640 text. Count is variable; images are IDs1..28.
   Ordered placements replace exact original vertex spans, preserving all
   intervening original ellipses and original shadow/foreground order. */
typedef struct NfsControlsSetupWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsControlsSetupWordImageView;
typedef struct NfsControlsSetupWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsControlsSetupWordPlacement;
typedef struct NfsControlsSetupWordsView {
 uint32_t schema,count;
 NfsControlsSetupWordImageView images[NFS_CONTROLS_SETUP_WORD_IMAGES];
 NfsControlsSetupWordPlacement placements[NFS_CONTROLS_SETUP_WORD_PLACEMENTS];
} NfsControlsSetupWordsView;
#endif
