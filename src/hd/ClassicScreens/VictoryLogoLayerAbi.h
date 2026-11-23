// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Immutable producer borrows are retained by the owned queue for this lexical tail.
typedef struct NfsVictoryLogoTail {
 uint32_t schema,image,address;
 uint64_t request,parentRequest,content,sourceSerial,sourceEpoch,artGeneration;
 const uint16_t*original;
}NfsVictoryLogoTail;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_victory_logo_begin(const NfsVictoryLogoTail*);
int nfs_private_victory_logo_quad(uint64_t request,const float*fourVertices60BytesEach);
int nfs_private_victory_logo_end(uint64_t request,int sourceValid);
#ifdef __cplusplus
}
#endif