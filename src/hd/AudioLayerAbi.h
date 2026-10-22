// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_LAYER_ABI_H
#define NFS_AUDIO_LAYER_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_AUDIO_ART_SCHEMA 2u
#define NFS_AUDIO_ART_WIDTH 2048u
#define NFS_AUDIO_ART_HEIGHT 768u
#define NFS_AUDIO_ART_BYTES (2048u*768u*4u)
#define NFS_AUDIO_MAX_BAR_VERTICES 78u
typedef struct NfsAudioBarVertex {float x,y,u,v;} NfsAudioBarVertex;
/* Authenticated immutable provider/packet storage. Shape, identity and this
   self-described digest alone do not authenticate the provider. All borrows
   are synchronous; no GPU helper retains these pointers. Schema2 fixes the
   six authored control regions and their opacity-baked dim variants. */
typedef struct NfsAudioArt {
 const unsigned char*pixels;size_t bytes;
 uint32_t width,height,schema,premultiplied,opacityBaked;
 uint64_t identity;unsigned char sha256[32];
} NfsAudioArt;
#endif
