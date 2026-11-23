// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsVictoryArtView {
 uint32_t schema,aspect,image,width,height;
 uint64_t generation,identity;size_t bytes;
 const unsigned char*pixels;unsigned char rgbaSha256[32],sourceSha256[32];
 void*owner;void(*retain)(void*);void(*release)(void*);
}NfsVictoryArtView;
// Startup-only registration of pinned archive metadata. No pixels decode here.
// Production requires32images. allowPreview is explicit diagnostic-only admission.
int nfs_victory_art_load_file(const char*path,uint64_t fileBytes,const unsigned char metadataSha256[32],int allowPreview,char*error,size_t capacity);
// Finite metadata identity check, also available to the C renderer before registration.
int nfs_victory_art_source_identity_valid(unsigned image,const unsigned char sourceSha256[32]);
void nfs_victory_art_clear(void);
uint64_t nfs_victory_art_generation(void);
// Source is supplied by an independently authenticated engine scene, not a car setting.
// One selected image is cached. Acquired views keep their immutable bytes alive.
// Failure leaves output unchanged; old views survive clear but are no longer current.
int nfs_victory_art_acquire(unsigned aspect,unsigned image,const unsigned char originalSourceSha256[32],NfsVictoryArtView*output);
#ifdef __cplusplus
}
#endif
