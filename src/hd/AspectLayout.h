// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
typedef struct NfsAspectRect {int32_t left,top,width,height;}NfsAspectRect;
/* Pixel rectangles are top-left based. Every admitted pixel stays in the
   drawable; no cover/crop policy or game-camera/FOV change is implied. */
static inline int nfs_aspect_contain(int32_t sourceWidth,int32_t sourceHeight,int32_t width,int32_t height,NfsAspectRect*out){
 if(!out||sourceWidth<=0||sourceHeight<=0||width<=0||height<=0)return 0;
 NfsAspectRect r={0,0,0,0};
 if((int64_t)width*sourceHeight<=(int64_t)height*sourceWidth){r.width=width;r.height=(int32_t)(((int64_t)width*sourceHeight+sourceWidth/2)/sourceWidth);}else{r.height=height;r.width=(int32_t)(((int64_t)height*sourceWidth+sourceHeight/2)/sourceHeight);}
 if(r.width<1)r.width=1;if(r.height<1)r.height=1;if(r.width>width||r.height>height)return 0;
 r.left=(width-r.width)/2;r.top=(height-r.height)/2;*out=r;return 1;
}
static inline int nfs_aspect_viewport(int32_t sourceWidth,int32_t sourceHeight,int32_t width,int32_t height,int contain,NfsAspectRect*out){
 if(contain)return nfs_aspect_contain(sourceWidth,sourceHeight,width,height,out);
 if(!out||sourceWidth<=0||sourceHeight<=0||width<=0||height<=0)return 0;NfsAspectRect r={0,0,width,height};*out=r;return 1;
}
