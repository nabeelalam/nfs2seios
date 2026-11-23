// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
enum {NFS_CREDITS_ART_BACKDROP=0,NFS_CREDITS_ART_LOGO=1,NFS_CREDITS_ART_WATERMARK=2,NFS_CREDITS_ART_HEADING_EN=3,NFS_CREDITS_ART_HEADING_DE=4,NFS_CREDITS_ART_HEADING_FR=5,NFS_CREDITS_ART_HEADING_ES=6,NFS_CREDITS_ART_HEADING_IT=7,NFS_CREDITS_ART_HEADING_SV=8,NFS_CREDITS_ART_KENWOOD=9,NFS_CREDITS_ART_Y2=10,NFS_CREDITS_ART_COUNT=11};
typedef struct NfsCreditsArtImage {uint32_t width,height;size_t bytes;const unsigned char*pixels;unsigned char sha256[32];}NfsCreditsArtImage;
typedef struct NfsCreditsArtInput {uint32_t schema;NfsCreditsArtImage images[NFS_CREDITS_ART_COUNT];}NfsCreditsArtInput;
typedef struct NfsCreditsArtView {uint32_t schema;uint64_t generation;NfsCreditsArtImage images[NFS_CREDITS_ART_COUNT];void*owner;void(*retain)(void*);void(*release)(void*);}NfsCreditsArtView;
#ifdef __cplusplus
extern "C" {
#endif
// Optional startup-only install: all eleven roles are copied and authenticated.
// Pixels are premultiplied RGBA; the full-width photographic backdrop is opaque.
// Clear invalidates the published generation while acquired views retain bytes.
int nfs_credits_art_install(const NfsCreditsArtInput*);
void nfs_credits_art_clear(void);
uint64_t nfs_credits_art_generation(void);
// Acquired view owns one reference. All screen layout is independent of pixels.
int nfs_credits_art_acquire(NfsCreditsArtView*);
#ifdef __cplusplus
}
#endif
