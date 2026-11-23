// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerIntroWidePreludeAbi.h"
#include "ChildIntroWidePreludeAbi.h"
/* PRIVATE geometry transport after a typed begin entrypoint has admitted its
 * own source. This shape has no construction/validity authority and is never
 * passed from a pack, Game, or another scene's proof. */
typedef struct NfsIntroWideGeometry{uint32_t schema,form,sourceOwner,sourceBitmap;uint64_t requestSerial,uploadEvent;NfsBackgroundSceneFrameView inheritedMain;const uint16_t*paddedOriginal565;size_t wordsCount;uint32_t keys[300];}NfsIntroWideGeometry;
typedef struct NfsIntroWideGpuBorrow{
 uint32_t family,countdown,photoFamily,locationInfoReturn,playerShowcaseReturn;NfsIntroWideGeometry frame;
 NfsPlayerCaptionWordView playerCaption;NfsChildInitialTransitionLayer childInitial;
 NfsOpponentInitialTransitionView opponentInitial;NfsLocationInitialTransitionView locationInitial;
 uint64_t content,photoSerial,ownerEpoch;uintptr_t sourceGame,sourceCatalog;uint64_t sourceLife;uint32_t sourceArchive;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
}NfsIntroWideGpuBorrow;
