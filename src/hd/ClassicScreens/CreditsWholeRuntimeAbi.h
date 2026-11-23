// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsArtProvider.h"
#include "hd/SoftwareTextLayerAbi.h"
// Views borrow all pointers and the embedded art view from frame.owner. Retain
// and release frame.owner when escaping; do not release the borrowed art view.
// Immutable complete Credits content only. A renderer must independently prove
// the cached native draws, actual context/target and swap before presentation.
typedef struct NfsCreditsWholeEvent {uint32_t kind;size_t first,count;}NfsCreditsWholeEvent;
typedef struct NfsCreditsWholeRect {float left,top,right,bottom;}NfsCreditsWholeRect;
typedef struct NfsCreditsWholeView {
 uint32_t schema,locale;uint64_t request,content,nativeBaseRequest,nativePartnerRequest;uintptr_t sourceDomain;
 NfsCreditsArtView art;NfsSoftwareTextLayerView text;
 const NfsCreditsWholeEvent*events;size_t eventCount;
 uint32_t hasPartner;NfsCreditsWholeRect partner[2],partnerClip,heading;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(const void*);
}NfsCreditsWholeView;
