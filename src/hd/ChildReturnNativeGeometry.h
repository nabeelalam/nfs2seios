// SPDX-License-Identifier: MIT
#pragma once
#include "WideTransitionGeometry.h"
#include "AudioNativePass.h"
/* Geometry only: no source, lifetime or native-upload authority. */
static int nfs_child_return_motion(unsigned route,unsigned frame,const NfsTransitionMotion*m,uint32_t rgba){if(!m||m->originX||m->originY||m->stepX!=32||m->direction)return 0;if(route==1)return frame<12&&m->minimum==140&&m->maximum==230&&m->phase==140+10*(int)frame&&rgba==0;if(route==2)return frame<19&&m->minimum==0&&m->maximum==255&&m->phase==-1&&rgba==0xff8c8c8cu;return 0;}
static int nfs_child_return_native_corner(const NfsAudioNativeVertex*v,unsigned tile,unsigned corner,unsigned route,const NfsTransitionMotion*m){if(!v||!m||tile>=300||corner>=4)return 0;unsigned column=tile/15,row=tile%15;int right=corner==1||corner==2;float gray=route==1?(float)nfs_transition_gray(m,column,right):140.f;return v->x==(float)(column*32+(right?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP&&v->y==(float)(row*32+(corner>=2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP&&v->r==gray&&v->g==gray&&v->b==gray&&v->a==255&&v->oow==1&&v->sow==(right?255:0)&&v->tow==(corner>=2?255:0);}
