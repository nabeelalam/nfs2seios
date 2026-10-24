// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_WORD_RUNTIME_ABI_H
#define NFS_CONTROLS_STEERING_WORD_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_CONTROLS_STEERING_WORD_SCHEMA 1u
#define NFS_CONTROLS_STEERING_WORD_IMAGES 8u
#define NFS_CONTROLS_STEERING_WORD_PLACEMENTS 16u
/* Finite whole-word images only. All zero retains original full640 text. Count is variable; images are IDs1..8.
   Ordered placements replace exact original vertex spans, preserving all
   intervening original ellipses and original shadow/foreground order. */
typedef struct NfsControlsSteeringWordImageView {
 uint32_t word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
} NfsControlsSteeringWordImageView;
typedef struct NfsControlsSteeringWordPlacement {
 uint32_t word,color565;size_t vertexFirst,vertexCount;
 float left,top,right,bottom;
} NfsControlsSteeringWordPlacement;
typedef struct NfsControlsSteeringWordsView {
 uint32_t schema,count;
 NfsControlsSteeringWordImageView images[NFS_CONTROLS_STEERING_WORD_IMAGES];
 NfsControlsSteeringWordPlacement placements[NFS_CONTROLS_STEERING_WORD_PLACEMENTS];
} NfsControlsSteeringWordsView;
#endif
