// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct NfsHistoryPanoramaView {
 uint32_t width,height,sourceIndex;uint64_t baseGeneration;size_t bytes;
 const unsigned char*pixels;unsigned char sha[32];
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
} NfsHistoryPanoramaView;
