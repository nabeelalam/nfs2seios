// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#define NFS_CONTROLS_BINDING_PHRASE_SCHEMA 1u
#define NFS_CONTROLS_BINDING_PHRASE_SLOTS 20u
typedef struct NfsControlsBindingPhraseImageView {unsigned phrase,width,height;const unsigned char*rgba;size_t bytes;unsigned char sha256[32];} NfsControlsBindingPhraseImageView;
typedef struct NfsControlsBindingPhrasePlacement {unsigned slot;uint32_t foreground565,shadow565;size_t vertexFirst,vertexCount;float left,top,right,bottom;} NfsControlsBindingPhrasePlacement;
typedef struct NfsControlsBindingPhrasesView {unsigned schema,imageCount,count;NfsControlsBindingPhraseImageView images[20];NfsControlsBindingPhrasePlacement placements[20];} NfsControlsBindingPhrasesView;
