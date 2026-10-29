// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct NfsLoadingWordSource{char name[5];unsigned width,height,span;int32_t left,top;unsigned char sha[32];unsigned artIndex;}NfsLoadingWordSource;
typedef struct NfsLoadingWordArt{unsigned width,height;const uint16_t*runs;size_t count;}NfsLoadingWordArt;
