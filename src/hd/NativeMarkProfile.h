// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_MARK_PROFILE_H
#define NFS_NATIVE_MARK_PROFILE_H
#include <stdint.h>
#define NFS_NATIVE_MARK_SOURCE_RGBA5551_LE 1u
#define NFS_NATIVE_MARK_SOURCE_RGBA4444_LE 2u
/* Exact reviewed upload/art pairs. Unknown combinations retain original artwork.
   No source-content keys or original pixels are part of this format contract. */
typedef struct NfsNativeMarkProfile {
 uint32_t sourceEdge,preparedBytes,artEdge,artBytes,logicalEdge,sourceLod;
} NfsNativeMarkProfile;
static inline int nfs_native_mark_profile(uint32_t encoding,uint32_t edge,NfsNativeMarkProfile*out){
 NfsNativeMarkProfile value;
 if(!out)return 0;
 if(encoding==NFS_NATIVE_MARK_SOURCE_RGBA5551_LE&&edge==128){
  value.sourceEdge=128;value.preparedBytes=32768;value.artEdge=512;value.artBytes=1048576;value.logicalEdge=128;value.sourceLod=1;
 }else if(encoding==NFS_NATIVE_MARK_SOURCE_RGBA4444_LE&&edge==32){
  value.sourceEdge=32;value.preparedBytes=2048;value.artEdge=128;value.artBytes=65536;value.logicalEdge=32;value.sourceLod=3;
 }else return 0;
 *out=value;return 1;
}
#endif
