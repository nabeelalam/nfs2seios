// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* All words and vertices are owned by the queued producer until consumed. */
typedef struct NfsTrackBadgeUpload {uint64_t serial,generation,epoch;uint32_t address;const uint16_t*original;const unsigned char*rgba;}NfsTrackBadgeUpload;
int nfs_private_track_badge_upload(const NfsTrackBadgeUpload*);
int nfs_private_track_badge_begin(uint64_t serial,uint64_t epoch);
int nfs_private_track_badge_quad(uint64_t serial,const float*fourVertices60BytesEach);
int nfs_private_track_badge_end(uint64_t serial);
#ifdef __cplusplus
}
#endif
