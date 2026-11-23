// SPDX-License-Identifier: MIT
#ifndef NFS_NESTED_SCENE_LAYOUT_H
#define NFS_NESTED_SCENE_LAYOUT_H
#include "SceneGpuSupport.h"
#include <string.h>
/* Pixel-isotropic 480-unit canvas. The sidebar stays left, interactive content
   stays centred, and original right-side badges remain right anchored.
   This describes geometry only; it grants no frame, resource or GL authority. */
typedef struct NfsNestedSceneLayout {
 unsigned width,height;float logicalWidth,extension;
 NfsFrameLegacySampling sidebar,content,right;
} NfsNestedSceneLayout;
static inline int nfs_nested_scene_layout(unsigned w,unsigned h,GLenum mag,
                                         NfsNestedSceneLayout*out){
 NfsNestedSceneLayout value;float logical;
 if(!out||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||
    (uint64_t)w*h>NFS_FRAME_MAX_PIXELS||(mag!=GL_LINEAR&&mag!=GL_NEAREST))return 0;
 logical=480.f*(float)w/(float)h;
 if(logical<=640.01f||logical>854.f)return 0;
 memset(&value,0,sizeof value);value.width=w;value.height=h;
 value.logicalWidth=logical;value.extension=logical-640.f;
 value.sidebar.magFilter=mag;value.sidebar.viewport[2]=(GLint)w;
 value.sidebar.viewport[3]=(GLint)h;
 value.sidebar.projection[0]=2.f/logical;value.sidebar.projection[5]=-2.f/480.f;
 value.sidebar.projection[10]=-2.f;value.sidebar.projection[12]=-1.f;
 value.sidebar.projection[13]=value.sidebar.projection[14]=value.sidebar.projection[15]=1.f;
 value.content=value.right=value.sidebar;
 value.content.projection[12]=-640.f/logical;
 value.right.projection[12]=-1.f+2.f*value.extension/logical;
 *out=value;return 1;
}
/* Keep this separate from the exact legacy sampling validator. */
static inline int nfs_nested_scene_sidebar_sampling(unsigned w,unsigned h,
                                                   const NfsFrameLegacySampling*p){
 NfsNestedSceneLayout value;
 return p&&nfs_nested_scene_layout(w,h,p->magFilter,&value)&&
        !memcmp(p->viewport,value.sidebar.viewport,sizeof p->viewport)&&
        !memcmp(p->projection,value.sidebar.projection,sizeof p->projection);
}
static inline int nfs_nested_scene_content_sampling(unsigned w,unsigned h,
                                                   const NfsFrameLegacySampling*p){
 NfsNestedSceneLayout value;
 return p&&nfs_nested_scene_layout(w,h,p->magFilter,&value)&&
        !memcmp(p->viewport,value.content.viewport,sizeof p->viewport)&&
        !memcmp(p->projection,value.content.projection,sizeof p->projection);
}
#endif
