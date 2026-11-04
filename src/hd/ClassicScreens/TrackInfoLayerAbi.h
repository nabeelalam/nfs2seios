// Dedicated track-information ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
#include "TrackInfoArtProvider.h"
typedef struct NfsTrackInfoLiveView {
 uint32_t schema,width,height,formType;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;NfsTrackInfoArtView art;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsTrackInfoLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_track_info_begin(const NfsTrackInfoLiveView*);
int nfs_private_track_info_finish(uint64_t);
int nfs_private_track_info_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
