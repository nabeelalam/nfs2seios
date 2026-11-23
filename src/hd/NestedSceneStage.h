// SPDX-License-Identifier: MIT
#ifndef NFS_NESTED_SCENE_STAGE_H
#define NFS_NESTED_SCENE_STAGE_H
#include "BackgroundSceneCompositor.h"
#include "NestedSceneLayout.h"
/* Synchronous projection-only borrow. Every content-stage check revalidates
   the actual registered source stage. This stack scope never grants ownership
   or permits a different context, attachment, extent or arbitrary projection. */
typedef struct NfsNestedSceneScope {
 const NfsBackgroundSceneStage*source;NfsNestedSceneLayout layout;
} NfsNestedSceneScope;
static inline int nfs_nested_scene_content_current(void*user,const NfsBackgroundSceneStage*t){
 const NfsNestedSceneScope*s=(const NfsNestedSceneScope*)user;
 const NfsBackgroundSceneStage*origin=s?s->source:NULL;
 return origin&&t&&t->generation==origin->generation&&
  t->contextIdentity==origin->contextIdentity&&t->framebuffer==origin->framebuffer&&
  t->width==origin->width&&t->height==origin->height&&
  t->sampling.magFilter==s->layout.content.magFilter&&
  !memcmp(t->sampling.viewport,s->layout.content.viewport,sizeof t->sampling.viewport)&&
  !memcmp(t->sampling.projection,s->layout.content.projection,sizeof t->sampling.projection)&&
  origin->ownerCurrent&&origin->ownerCurrent(origin->ownerUser,origin);
}
static inline int nfs_nested_scene_scope(const NfsBackgroundSceneStage*source,
 NfsNestedSceneScope*scope,NfsBackgroundSceneStage*content){
 NfsNestedSceneLayout layout;
 if(!source||!scope||!content||source==content||!source->ownerCurrent||
    !source->ownerCurrent(source->ownerUser,source)||
    !nfs_nested_scene_layout(source->width,source->height,source->sampling.magFilter,&layout)||
    !nfs_nested_scene_sidebar_sampling(source->width,source->height,&source->sampling))return 0;
 scope->source=source;scope->layout=layout;*content=*source;
 content->sampling=layout.content;content->ownerCurrent=nfs_nested_scene_content_current;
 content->ownerUser=scope;return nfs_nested_scene_content_current(scope,content);
}
#endif
