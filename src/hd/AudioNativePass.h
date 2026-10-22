// SPDX-License-Identifier: MIT
#ifndef NFS_AUDIO_NATIVE_PASS_H
#define NFS_AUDIO_NATIVE_PASS_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#define NFS_AUDIO_NATIVE_PIXELS 307200u
#define NFS_AUDIO_NATIVE_TILES 300u
#define NFS_AUDIO_NATIVE_TILE_BYTES 2048u
#define NFS_AUDIO_NATIVE_VERTEX_SNAP 786432.0f
/* Source-neutral state machine only. Stamps/observations MUST be read from the
   actual current renderer owner/context/target and original download/select/
   triangle calls by a separate trusted adapter. Packet metadata, counters and
   a successful finish never grant GL/FBO/publication or suppression authority.
   Native payload is fixed RGB565_LE TextureInfo.data: original4289F0 allocates
   format index4 (byte_4DD01D[4]=0x0a); native download copies565 unchanged.
   Main preuploaded index3/ARGB1555 is a different source path. No auto-detection or borrowed pointer survives. */
typedef struct NfsAudioNativeStamp {uint64_t generation;uintptr_t context,thread;uint32_t framebuffer,width,height;} NfsAudioNativeStamp;
typedef struct NfsAudioNativeVertex {float x,y,r,g,b,a,oow,sow,tow;} NfsAudioNativeVertex;
enum {NFS_AUDIO_NATIVE_OK=0,NFS_AUDIO_NATIVE_OWNER=1,NFS_AUDIO_NATIVE_ORDER=2,NFS_AUDIO_NATIVE_SOURCE=3,NFS_AUDIO_NATIVE_UPLOAD=4,NFS_AUDIO_NATIVE_SELECT=5,NFS_AUDIO_NATIVE_TRIANGLE=6,NFS_AUDIO_NATIVE_OTHER=7,NFS_AUDIO_NATIVE_SERIAL=8};
typedef struct NfsAudioNativePass {
 uint64_t generation,lastGeneration,lastSerial,serial;uintptr_t context,thread;
 NfsAudioNativeStamp stamp;uint32_t active,phase,uploads,selects,triangles,address,id,reason;
 unsigned char expected[NFS_AUDIO_NATIVE_TILES][NFS_AUDIO_NATIVE_TILE_BYTES];
} NfsAudioNativePass;
static int nfs_audio_native_fail(NfsAudioNativePass*p,uint32_t why){if(p){p->active=0;p->phase=0;p->serial=0;p->reason=why;}return 0;}
static int nfs_audio_native_owner(const NfsAudioNativePass*p,const NfsAudioNativeStamp*s){return p&&s&&p->generation&&s->generation==p->generation&&s->context==p->context&&s->thread==p->thread&&s->context&&s->thread&&s->framebuffer&&s->height&&s->height<=4096&&s->width==s->height*4u/3u&&s->width<=4096;}
static int nfs_audio_native_stamp_equal(const NfsAudioNativeStamp*a,const NfsAudioNativeStamp*b){return a&&b&&a->generation==b->generation&&a->context==b->context&&a->thread==b->thread&&a->framebuffer==b->framebuffer&&a->width==b->width&&a->height==b->height;}
static int nfs_audio_native_created(NfsAudioNativePass*p,uint64_t generation,uintptr_t context,uintptr_t thread){
 if(!p||!generation||generation==UINT64_MAX||generation<=p->lastGeneration||p->generation||!context||!thread)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_OWNER);
 p->generation=p->lastGeneration=generation;p->context=context;p->thread=thread;p->active=p->phase=p->uploads=p->selects=p->triangles=p->address=p->id=p->reason=0;p->serial=0;memset(&p->stamp,0,sizeof p->stamp);return 1;
}
static void nfs_audio_native_retired(NfsAudioNativePass*p,uint64_t generation){if(!p||!generation||generation!=p->generation)return;nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_OWNER);p->generation=0;p->context=p->thread=0;memset(&p->stamp,0,sizeof p->stamp);}
/* Exact original565 -> native RGB565. All six green bits and opaque black
   survive unchanged; this Audio scratch format has no alpha rotation. */
static uint16_t nfs_audio_native_to565(uint16_t source){return source;}
static int nfs_audio_native_begin(NfsAudioNativePass*p,const NfsAudioNativeStamp*now,uint64_t serial,const uint16_t*original565,size_t pixels){
 uintptr_t src=(uintptr_t)original565,owned=(uintptr_t)p;size_t bytes=NFS_AUDIO_NATIVE_PIXELS*2u;
 if(!nfs_audio_native_owner(p,now))return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_OWNER);
 if(p->active)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);
 if(!serial||serial==UINT64_MAX||serial<=p->lastSerial)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_SERIAL);
 if(!original565||pixels!=NFS_AUDIO_NATIVE_PIXELS||src>UINTPTR_MAX-bytes||owned>UINTPTR_MAX-sizeof *p||(src<owned+sizeof *p&&owned<src+bytes))return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_SOURCE);
 for(unsigned tile=0;tile<300;++tile)for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t v=nfs_audio_native_to565(original565[((tile%15)*32+y)*640+(tile/15)*32+x]);size_t at=(y*32+x)*2;p->expected[tile][at]=(unsigned char)v;p->expected[tile][at+1]=(unsigned char)(v>>8);}
 p->stamp=*now;p->serial=p->lastSerial=serial;p->uploads=p->selects=p->triangles=p->address=p->id=p->reason=0;p->phase=1;p->active=1;return 1;
}
static int nfs_audio_native_transition(NfsAudioNativePass*p,const NfsAudioNativeStamp*now){if(!nfs_audio_native_owner(p,now)||!nfs_audio_native_stamp_equal(&p->stamp,now))return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_OWNER);if(!p->active||!p->serial||p->uploads>300||p->selects>300||p->triangles>600)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);return 1;}
static int nfs_audio_native_upload(NfsAudioNativePass*p,const NfsAudioNativeStamp*now,uint32_t address,uint32_t id,uint32_t format,uint32_t size,const unsigned char*actual,size_t bytes){
 if(!nfs_audio_native_transition(p,now))return 0;
 if(p->phase!=1||p->uploads>=300||p->uploads!=p->selects||p->triangles!=p->uploads*2)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);
 if(!actual||bytes!=2048||(address&3)||address>0x200000u-2048u||!id||format!=10||size!=32||memcmp(actual,p->expected[p->uploads],2048))return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_UPLOAD);
 p->address=address;p->id=id;++p->uploads;p->phase=2;return 1;
}
static int nfs_audio_native_select(NfsAudioNativePass*p,const NfsAudioNativeStamp*now,uint32_t tmu,uint32_t address,uint32_t id,uint32_t evenOdd,uint32_t format,uint32_t size,uint32_t smallLod,uint32_t largeLod,uint32_t aspect){
 if(!nfs_audio_native_transition(p,now))return 0;
 if(p->phase!=2||p->selects>=300||p->uploads!=p->selects+1||p->triangles!=p->selects*2)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);
 if(tmu||address!=p->address||id!=p->id||evenOdd!=3||format!=10||size!=32||smallLod!=3||largeLod!=3||aspect!=3)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_SELECT);
 ++p->selects;p->phase=3;return 1;
}
static int nfs_audio_native_corner(const NfsAudioNativeVertex*v,unsigned tile,unsigned corner){float x=(float)((tile/15)*32+(corner==1||corner==2?32:0)),y=(float)((tile%15)*32+(corner>=2?32:0));return v&&v->x==x+NFS_AUDIO_NATIVE_VERTEX_SNAP&&v->y==y+NFS_AUDIO_NATIVE_VERTEX_SNAP&&v->r==255&&v->g==255&&v->b==255&&v->a==255&&v->oow==1&&v->sow==(corner==1||corner==2?255:0)&&v->tow==(corner>=2?255:0);}
static int nfs_audio_native_triangle(NfsAudioNativePass*p,const NfsAudioNativeStamp*now,const NfsAudioNativeVertex*a,const NfsAudioNativeVertex*b,const NfsAudioNativeVertex*c,uint32_t textureEnabled,uint32_t fogEnabled){
 unsigned tile,half;if(!nfs_audio_native_transition(p,now))return 0;
 tile=p->triangles/2;half=p->triangles&1;if(p->phase!=3||tile>=300||p->uploads!=tile+1||p->selects!=tile+1)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);
 if(textureEnabled!=1||fogEnabled||!nfs_audio_native_corner(a,tile,0)||!nfs_audio_native_corner(b,tile,half?2:1)||!nfs_audio_native_corner(c,tile,half?3:2))return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_TRIANGLE);
 ++p->triangles;if(half)p->phase=1;return 1;
}
static int nfs_audio_native_other(NfsAudioNativePass*p,const NfsAudioNativeStamp*now){if(!nfs_audio_native_transition(p,now))return 0;return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_OTHER);}
static int nfs_audio_native_finish(NfsAudioNativePass*p,const NfsAudioNativeStamp*now,uint64_t serial){
 if(!nfs_audio_native_transition(p,now))return 0;
 if(serial!=p->serial||p->phase!=1||p->uploads!=300||p->selects!=300||p->triangles!=600)return nfs_audio_native_fail(p,NFS_AUDIO_NATIVE_ORDER);
 p->active=0;p->phase=0;p->serial=0;p->reason=NFS_AUDIO_NATIVE_OK;return 1;
}
#endif
