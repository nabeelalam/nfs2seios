// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneLayersAbi.h"
#include "MenuTransitionBrandingAbi.h"
#include "ChildInitialTransitionAbi.h"
#include "OpponentInitialTransitionAbi.h"
#include "LocationInitialTransitionAbi.h"
#include "PlayerInitialTransitionAbi.h"
#include "OriginalPreviewLayerAbi.h"
#include "MenuResponsiveCached.h"
#include "ClassicScreens/TrackInfoArtProvider.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsMenuTransitionView {
 uint32_t responsiveForm,responsiveDescriptor;uint64_t responsiveProducerEpoch;NfsMenuCachedSnapshot responsive; /* actual cached-widget owner, never caption epoch */
 uint64_t sourceLife;uintptr_t sourceGame,sourceCatalog;uint32_t sourceArchive; /* strong upload-packet provenance; no guest reads on renderer */
 uint64_t serial,content;uint32_t slot,kind,mapRole; /* kind1 map,kind2 sidebar,kind3 complete Main,kind4 initial sidebar,kind5 typed initial child sidebar,kind6 Opponents initial sidebar,kind7 Location initial sidebar,kind8 exact Setup/Options initial full cache,kind9 Location initial full cache,kind10 Opponent initial full cache,kind11 authenticated Player initial sidebar */
 NfsTrackInfoEntryArtView trackInfoArt;
 uint32_t trackInfoPhase;
 NfsCompactCaptionWordView outgoingLocationCaption;
 uint32_t locationInfoReturn; /* Location sidebar returning from an information page */
 uint32_t playerShowcaseReturn; /* typed Player sidebar returning from Showcase, with a bare-map destination receipt */
 uint32_t drawKind; /* 1 original427C70 gradient,2 original427B30 white pass */
 int32_t minimum,maximum,direction,phase,originX,originY,stepX;
 const uint16_t*original565;uint32_t addresses[300];NfsBackgroundSceneLayers layers;NfsChildInitialTransitionLayer childInitial;NfsOpponentInitialTransitionView opponentInitial;NfsLocationInitialTransitionView locationInitial;NfsOriginalPreviewLayerView initialFullPreview;NfsPlayerInitialTransitionView playerInitial;
} NfsMenuTransitionView;
int nfs_menu_transition_source_life_current(uintptr_t,uint64_t);
int nfs_private_menu_transition_begin(const NfsMenuTransitionView*);
int nfs_private_menu_transition_finish(uint64_t);
#ifdef __cplusplus
}
#endif
