// SPDX-License-Identifier: MIT
#ifndef NFS_PLAYER_CAPTION_RUNTIME_ABI_H
#define NFS_PLAYER_CAPTION_RUNTIME_ABI_H
#include <stdint.h>
#include <stddef.h>
#define NFS_PLAYER_CAPTION_WORD_SCHEMA 1u
/* Two complete lowercase word graphics, never a bitmap alphabet/font.
   All-zero means source-text fallback; any other malformed view declines. */
typedef struct NfsPlayerCaptionWordView {
 uint32_t schema,word,width,height;
 const unsigned char*rgba;size_t bytes;unsigned char sha256[32];
 float left,top,right,bottom;
} NfsPlayerCaptionWordView;
#endif
