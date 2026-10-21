// SPDX-License-Identifier: MIT
#ifndef NFS_BACKGROUND_SCENE_ART_H
#define NFS_BACKGROUND_SCENE_ART_H
#include <stddef.h>
#include <stdint.h>
#define NFS_BACKGROUND_SCENE_ART_SCHEMA 1u
#define NFS_BACKGROUND_SCENE_TEXTURE_COUNT 5u
#define NFS_BACKGROUND_SCENE_WORD_COUNT 12u
enum {NFS_BG_MAP_BLK4=0,NFS_BG_MAP_BLK6=1,NFS_BG_PANEL=2,NFS_BG_WORD_ATLAS0=3,NFS_BG_WORD_ATLAS1=4};
/* Semantic roles must match the separately trusted CPU source catalogue. */
enum {NFS_BG_WORD_NEDB=1,NFS_BG_WORD_NEDS=2,NFS_BG_WORD_SPDB=3,NFS_BG_WORD_SPDS=4,NFS_BG_WORD_FORB=5,NFS_BG_WORD_FORS=6,NFS_BG_WORD_BSPB=7,NFS_BG_WORD_BSPS=8,NFS_BG_WORD_BNDB=9,NFS_BG_WORD_BNDS=10,NFS_BG_WORD_BFRB=11,NFS_BG_WORD_BFRS=12};
typedef struct NfsBackgroundSceneTexture {const unsigned char*pixels;size_t bytes;uint32_t width,height,schema,role,premultiplied,opacityBaked,opaque;uint64_t identity;unsigned char sha256[32];unsigned char sourceSha256[32];} NfsBackgroundSceneTexture;
typedef struct NfsBackgroundSceneWord {uint32_t schema,sourceRole,textureRole,x,y,width,height,scale,logicalWidth,logicalHeight;char entry[5];unsigned char sourceSha256[32];unsigned char authoredStraightSha256[32];} NfsBackgroundSceneWord;
#ifdef __cplusplus
extern "C" {
#endif
/* Borrowed immutable program-lifetime RO bytes. No allocation, IO, GPU or guest access.
   Invalid role or NULL output returns0 and leaves a non-NULL output byte-for-byte untouched.
   RGB is already floor(originalStraightRGB*A/255); opacity is not baked. */
int nfs_private_background_scene_texture(uint32_t role,NfsBackgroundSceneTexture*out);
int nfs_private_background_scene_word(uint32_t sourceRole,NfsBackgroundSceneWord*out);
#ifdef __cplusplus
}
#endif
#endif
