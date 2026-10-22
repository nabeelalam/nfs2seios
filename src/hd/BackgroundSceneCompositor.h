// SPDX-License-Identifier: MIT
#ifndef NFS_BACKGROUND_SCENE_COMPOSITOR_H
#define NFS_BACKGROUND_SCENE_COMPOSITOR_H
#include "BackgroundSceneRuntimeAbi.h"
#include "BackgroundSceneArt.h"
#include "NativeSceneTarget.h"
#include "SmallLogoGpu.h"
#include "SoftwareTextGpu.h"
#include "LocationSceneRuntimeAbi.h"
typedef struct NfsBackgroundSceneImageKey {
 uint64_t generation,logoIdentity;GLuint program,copyProgram,textures[5],logoProgram,logoTexture,color,fbo;
 uint64_t identities[5];unsigned char sha256[5][32],logoSha256[32];
 unsigned width,height;NfsFrameLegacySampling sampling;
 uint32_t mapRole,wordCount,tailKind;
 GLuint captionProgram,captionTexture;uint64_t captionIdentity;size_t captionVertices;
 unsigned char captionFontSha256[32],captionVerticesSha256[32];int32_t panelClip[4];NfsBackgroundSceneWordLayer words[4];NfsSmallLogoLayer logos[3];
} NfsBackgroundSceneImageKey;
typedef struct NfsBackgroundSceneCompositor {
 NfsSmallLogoGpu logo;
 NfsSoftwareTextGpu caption;
 uint64_t generation,lastGeneration,ownerGeneration,lastSerial,pendingSerial,nextImageRevision,pendingImageRevision,lastImageRevision,cacheHits,cacheMisses;
 GLuint program,copyProgram,textures[5],color[2],fbo[2];GLint matrix,sampler,opacity,copySampler;
 NfsBackgroundSceneTexture art[5];NfsBackgroundSceneWord regions[12];
 unsigned width[2],height[2],imageReady[2],pendingSlot,nextSlot,lastBeginKind,pendingTailKind;
 uint64_t imageRevision[2];NfsBackgroundSceneImageKey keys[2];NfsNativeLogoTargetProof pendingTarget;
} NfsBackgroundSceneCompositor;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_bg_scene_context_created(NfsBackgroundSceneCompositor*,uint64_t generation);
void nfs_bg_scene_context_retired(NfsBackgroundSceneCompositor*,uint64_t generation,int ownerCurrent);
int nfs_bg_scene_prepare(NfsBackgroundSceneCompositor*);
/* Owner FIFO only, original queue already flushed. No caller pointer survives begin.
   Caller separately proves CURRENT original map/scene/native300upload and600triangle pass.
   Target proof retains exact current context/target/depth pass; black fields are ignored.
   begin initializes full owned COLOR from opaque authored map, then ordered panel/words/logos.
   No prebase copy, clean565 tiles, depth allocation or original target write.
   finish COLOR-only before40DC40 labels/cursor; arbitrary late GPU fault cannot roll back. */
int nfs_bg_scene_begin(NfsBackgroundSceneCompositor*,const NfsBackgroundSceneFrameView*,const NfsNativeLogoTargetProof*);
int nfs_bg_scene_finish(NfsBackgroundSceneCompositor*,uint64_t serial,const NfsNativeLogoTargetProof*);
int nfs_bg_scene_begin_location(NfsBackgroundSceneCompositor*,const NfsLocationSceneFrameView*,const NfsNativeLogoTargetProof*);
int nfs_bg_scene_finish_location(NfsBackgroundSceneCompositor*,uint64_t serial,const NfsNativeLogoTargetProof*);
void nfs_bg_scene_cancel(NfsBackgroundSceneCompositor*);
#ifdef __cplusplus
}
#endif
#endif
