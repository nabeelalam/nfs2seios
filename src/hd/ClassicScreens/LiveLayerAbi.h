// PRIVATE complete LoadSave UI; all borrows are held by paired native ownership.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
#include "hd/BackgroundSceneLayersAbi.h"
#include "hd/CompactCaptionWordRuntimeAbi.h"
typedef struct NfsLoadSaveLiveView {
 uint32_t schema,width,height;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
 /* Schema 1 leaves these zero. Schema 2 replaces the whole proven scene. */
 const NfsBackgroundSceneLayers*scene;
 NfsCompactCaptionWordView caption;
 uintptr_t sourceGame;uint64_t sourceEpoch;
} NfsLoadSaveLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_loadsave_begin(const NfsLoadSaveLiveView*);
int nfs_private_loadsave_finish(uint64_t);
#ifdef __cplusplus
}
#endif
