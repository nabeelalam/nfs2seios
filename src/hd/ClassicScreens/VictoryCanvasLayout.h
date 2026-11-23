// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include "hd/AdaptiveCanvasBounds.h"
typedef struct NfsVictoryCanvasLayout {unsigned aspect;int textViewport[4];float leftU,rightU,topV,bottomV;}NfsVictoryCanvasLayout;
// Keep the original4:3 text composition centered while cropping only authored side extensions.
static int nfs_victory_canvas_layout(const int viewport[4],NfsVictoryCanvasLayout*out){
 if(!viewport||!out||viewport[0]<-16384||viewport[0]>16384||viewport[1]<-16384||viewport[1]>16384||viewport[2]<=0||viewport[3]<=0||viewport[2]>16384||viewport[3]>16384)return 0;
 const int w=viewport[2],h=viewport[3];
 if((int64_t)w*3+3<(int64_t)h*4||(int64_t)w*9>(int64_t)h*16+16)return 0;
 NfsVictoryCanvasLayout layout={0,{viewport[0],viewport[1],w,h},0,1,0,1};
 if((int64_t)w*3>(int64_t)h*4){
  layout.aspect=1;int content=(h*4+1)/3;if(content>w)content=w;
  layout.textViewport[0]+=(w-content)/2;layout.textViewport[2]=content;
  float visible=(float)w*9.f/((float)h*16.f);if(visible>1.f)visible=1.f;
  layout.leftU=(1.f-visible)*.5f;layout.rightU=1.f-layout.leftU;
 }
 *out=layout;return 1;
}

// Use actual source dimensions so rounded canvases are cropped, never stretched.
static int nfs_victory_canvas_layout_image(const int viewport[4],unsigned width,unsigned height,NfsVictoryCanvasLayout*out){
 NfsVictoryCanvasLayout layout;if(!out||!width||!height||width>4096||height>4096||!nfs_victory_canvas_layout(viewport,&layout))return 0;
 if(layout.aspect?!nfs_authored_wide_canvas(width,height):(uint64_t)width*3!=(uint64_t)height*4)return 0;
 const double source=(double)width/height,target=(double)viewport[2]/viewport[3];
 layout.leftU=layout.topV=0;layout.rightU=layout.bottomV=1;
 if(source>target){float visible=(float)(target/source);layout.leftU=(1-visible)*.5f;layout.rightU=1-layout.leftU;}
 else{float visible=(float)(source/target);layout.topV=(1-visible)*.5f;layout.bottomV=1-layout.topV;}
 *out=layout;return 1;
}
