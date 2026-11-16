// SPDX-License-Identifier: MIT
#pragma once
/* Geometry only. This is not a source, underlay or presentation capability.
 * The native renderer uses width=640,height=480; a separately authenticated
 * wide composition may use the drawable aspect without stretching its pixels. */
typedef struct NfsTransitionMotion {
 int minimum,maximum,direction,phase,originX,originY,stepX;
} NfsTransitionMotion;
typedef struct NfsTransitionBand {
 float left,right,top,bottom,u0,u1,grayLeft,grayRight;
} NfsTransitionBand;
static int nfs_transition_gray(const NfsTransitionMotion*n,unsigned column,int right){
 int offset,value;
 if(n->phase==-1)return 255;
 offset=n->direction?(right?228:240)-12*(int)column:(right?12:0)+12*(int)column;
 value=2*n->phase-247+offset;
 return value<n->minimum?n->minimum:value>n->maximum?n->maximum:value;
}
static int nfs_transition_bands(const NfsTransitionMotion*n,unsigned width,unsigned height,NfsTransitionBand out[20]){
 float logical,scale;unsigned k;
 if(!n||!out||!width||!height||width>8192||height>8192||n->minimum<0||n->maximum>255||n->minimum>n->maximum||n->direction<0||n->direction>1||n->phase< -1||n->phase>255||n->originX< -32768||n->originX>32767||n->originY< -32768||n->originY>32767||n->stepX<0||n->stepX>32)return 0;
 logical=480.f*(float)width/(float)height;
 if(logical<640.f||logical>854.f)return 0;
 scale=logical/640.f;
 for(k=0;k<20;++k){NfsTransitionBand b;
  b.left=(float)(n->originX+(int)k*n->stepX)*scale;
  b.right=(float)(n->originX+(int)(k+1)*n->stepX)*scale;
  b.top=(float)n->originY;b.bottom=b.top+480.f;
  b.u0=k/20.f;b.u1=(k+1)/20.f;
  b.grayLeft=nfs_transition_gray(n,k,0)/255.f;
  b.grayRight=nfs_transition_gray(n,k,1)/255.f;out[k]=b;
 }
 return 1;
}
