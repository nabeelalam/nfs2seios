// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_INTRO_RGBA_PREMULTIPLIED=1,NFS_INTRO_ALPHA8=2};
typedef struct NfsIntroImage {uint32_t format,width,height;size_t bytes;const unsigned char*pixels;unsigned char sha[32];}NfsIntroImage;
typedef struct NfsIntroRect {float left,top,right,bottom;}NfsIntroRect;
typedef struct NfsIntroArtInput {uint32_t schema;NfsIntroImage photograph,logo,copyright[6];NfsIntroRect logoRect;NfsIntroImage widePhotograph;}NfsIntroArtInput;
typedef struct NfsIntroArtView {uint32_t schema;uint64_t generation;NfsIntroImage photograph,logo,copyright[6];NfsIntroRect photoRect,logoRect,logoUv;void*owner;void(*retain)(void*);void(*release)(void*);NfsIntroImage widePhotograph;}NfsIntroArtView;
#ifdef __cplusplus
extern "C" {
#endif
/* Startup-only optional installation. Bytes are copied and authenticated;
   replacement is refused until clear(). Copyright order EN,DE,FR,ES,IT,SV.
   Schema1 keeps the original 4:3 photo and ignores the appended wide field.
   Schema2 additionally requires an opaque, authenticated 16:9 photograph
   (one-pixel rounding allowed). Both photos are retained independently;
   ordinary native rendering continues to use the original photo.
   A null input logo borrows the existing authenticated silver-logo atlas role0
   under its program-lifetime immutable HdArtProvider contract;
   a separately rendered exact logo may instead be supplied as full PMA RGBA. */
int nfs_intro_art_install(const NfsIntroArtInput*);
void nfs_intro_art_clear(void);
uint64_t nfs_intro_art_generation(void);
/* Caller owns one successful view reference and must call view.release. */
int nfs_intro_art_acquire(NfsIntroArtView*);
#ifdef __cplusplus
}
#endif
