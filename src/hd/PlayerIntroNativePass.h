// SPDX-License-Identifier: MIT
#ifndef NFS_PLAYER_INTRO_NATIVE_PASS_H
#define NFS_PLAYER_INTRO_NATIVE_PASS_H
#include "AudioNativePass.h"
/* Independent cached slot0 identity/order proof. Unlike the whole-background
   pass it makes NO claim about clear color or depth. GPU admission owns target,
   payload and vertex observations; this CPU state alone grants no GL authority. */
typedef struct NfsPlayerIntroNativePass {
 NfsAudioNativeStamp stamp;uint64_t serial,uploadRevision;
 uint32_t addresses[300],ids[300];unsigned active,selects,triangles;
} NfsPlayerIntroNativePass;
static int nfs_player_intro_pass_fail(NfsPlayerIntroNativePass*p){if(p)p->active=0;return 0;}
static int nfs_player_intro_pass_current(const NfsPlayerIntroNativePass*p,const NfsAudioNativeStamp*s,uint64_t revision){return p&&p->active&&p->serial&&revision==p->uploadRevision&&nfs_audio_native_stamp_equal(&p->stamp,s);}
static int nfs_player_intro_pass_begin(NfsPlayerIntroNativePass*p,const NfsAudioNativeStamp*s,uint64_t serial,uint64_t revision,const uint32_t*addresses,const uint32_t*ids){
 if(!p||!s||!s->generation||!s->context||!s->thread||!s->framebuffer||!s->width||!s->height||!serial||!revision||!addresses||!ids)return nfs_player_intro_pass_fail(p);
 for(unsigned i=0;i<300;++i){if((addresses[i]&3)||addresses[i]>0x200000u-2048u||!ids[i])return nfs_player_intro_pass_fail(p);for(unsigned j=0;j<i;++j)if(ids[i]==ids[j]||(addresses[i]<addresses[j]+2048u&&addresses[j]<addresses[i]+2048u))return nfs_player_intro_pass_fail(p);}
 memset(p,0,sizeof *p);p->stamp=*s;p->serial=serial;p->uploadRevision=revision;memcpy(p->addresses,addresses,sizeof p->addresses);memcpy(p->ids,ids,sizeof p->ids);p->active=1;return 1;
}
static int nfs_player_intro_pass_select(NfsPlayerIntroNativePass*p,const NfsAudioNativeStamp*s,uint64_t revision,unsigned tmu,uint32_t address,uint32_t id,unsigned evenOdd,unsigned format,unsigned size,unsigned small,unsigned large,unsigned aspect){
 if(!nfs_player_intro_pass_current(p,s,revision)||p->selects>=300||p->triangles!=p->selects*2||tmu||evenOdd!=3||format!=11||size!=32||small!=3||large!=3||aspect!=3||address!=p->addresses[p->selects]||id!=p->ids[p->selects])return nfs_player_intro_pass_fail(p);++p->selects;return 1;
}
static int nfs_player_intro_pass_triangle(NfsPlayerIntroNativePass*p,const NfsAudioNativeStamp*s,uint64_t revision,const NfsAudioNativeVertex*a,const NfsAudioNativeVertex*b,const NfsAudioNativeVertex*c,unsigned textured,unsigned fog){
 if(!nfs_player_intro_pass_current(p,s,revision)||p->triangles>=600||p->selects!=p->triangles/2+1||!textured||fog)return nfs_player_intro_pass_fail(p);unsigned tile=p->triangles/2,half=p->triangles&1;
 if(!nfs_audio_native_corner(a,tile,0)||!nfs_audio_native_corner(b,tile,half?2:1)||!nfs_audio_native_corner(c,tile,half?3:2))return nfs_player_intro_pass_fail(p);++p->triangles;return 1;
}
static int nfs_player_intro_pass_finish(NfsPlayerIntroNativePass*p,const NfsAudioNativeStamp*s,uint64_t revision,uint64_t serial){if(!nfs_player_intro_pass_current(p,s,revision)||p->serial!=serial||p->selects!=300||p->triangles!=600)return nfs_player_intro_pass_fail(p);p->active=0;return 1;}
#endif
