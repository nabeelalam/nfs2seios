// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_MARK_PROVIDER_H
#define NFS_NATIVE_MARK_PROVIDER_H
#include "NativeMarkArt.h"
#define NFS_NATIVE_MARK_PROVIDER_SCHEMA 1u
typedef struct NfsNativeMarkProvider {
 uint32_t schema;void*user;
 int (*lookup)(void*,uint32_t,uint32_t,const uint8_t[32],NfsNativeMarkArt*);
} NfsNativeMarkProvider;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only nonconcurrent installation before getter/worker/context use.
   Struct copied; user/callback/data immutable for program lifetime. Installation
   validates callback completeness, not provenance. A trusted whole-pack loader
   authenticates content first. First getter seals even when absent. No hot swap.
   After startup immutable concurrent lookups are supported; no allocation/IO/GL.
   Missing/invalid lookup returns0 and leaves output byte-for-byte unchanged. */
int nfs_hd_install_native_mark_provider(const NfsNativeMarkProvider*provider);
int nfs_hd_native_mark_provider_available(void);
int nfs_hd_native_mark_art(uint32_t sourceEncoding,uint32_t sourceEdge,
                          const uint8_t preparedDigest[32],NfsNativeMarkArt*out);
#ifdef __cplusplus
}
#endif
#endif
