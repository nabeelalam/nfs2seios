// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_LOGO_PROVENANCE_H
#define NFS_NATIVE_LOGO_PROVENANCE_H
#include <stdint.h>
#include <string.h>
#include <limits.h>
/* No GPU or guest pointers. Every counter is actual renderer-owned observation. */
typedef struct NfsLogoProvenance {
 uint64_t context,uploads,clearEvent,pendingSerial,pendingContext,pendingUploads,pendingClear,lastSerial;
 unsigned alive,disabled,clearValid,active,selects,triangles,selected;
 uint32_t clearFbo,pendingFbo,addresses[300],ids[300];
} NfsLogoProvenance;
static int nfs_logo_inc(uint64_t*p){if(*p==UINT64_MAX)return 0;++*p;return 1;}
static void nfs_logo_p_cancel(NfsLogoProvenance*p){p->active=0;p->pendingSerial=0;}
static void nfs_logo_p_created(NfsLogoProvenance*p){nfs_logo_p_cancel(p);p->clearValid=0;p->alive=0;if(!nfs_logo_inc(&p->context)){p->disabled=1;return;}p->alive=1;}
static void nfs_logo_p_retired(NfsLogoProvenance*p){nfs_logo_p_cancel(p);p->clearValid=0;p->alive=0;}
static void nfs_logo_p_upload(NfsLogoProvenance*p){nfs_logo_p_cancel(p);if(!nfs_logo_inc(&p->uploads))p->disabled=1;}
static void nfs_logo_p_contaminate(NfsLogoProvenance*p){nfs_logo_p_cancel(p);p->clearValid=0;}
static void nfs_logo_p_clear(NfsLogoProvenance*p,uint32_t fbo,int full,int write,uint16_t depth){
 nfs_logo_p_cancel(p);p->clearValid=0;if(!nfs_logo_inc(&p->clearEvent)){p->disabled=1;return;}
 if(!p->disabled&&p->alive&&full&&write&&depth==65535){p->clearFbo=fbo;p->clearValid=1;}}
static int nfs_logo_p_begin(NfsLogoProvenance*p,uint64_t serial,uint32_t fbo,const uint32_t*addresses,const uint32_t*ids){
 unsigned i,j;nfs_logo_p_cancel(p);if(p->disabled||!p->alive||!serial||serial<=p->lastSerial||!p->clearValid||p->clearFbo!=fbo||!addresses||!ids)return 0;
 for(i=0;i<300;++i){if((addresses[i]&3)||addresses[i]>0x200000u-2048u||!ids[i])return 0;for(j=0;j<i;++j)if(ids[i]==ids[j]||(addresses[i]<addresses[j]+2048u&&addresses[j]<addresses[i]+2048u))return 0;}
 memcpy(p->addresses,addresses,sizeof p->addresses);memcpy(p->ids,ids,sizeof p->ids);p->pendingSerial=serial;p->pendingContext=p->context;p->pendingUploads=p->uploads;p->pendingClear=p->clearEvent;p->pendingFbo=fbo;p->selects=p->triangles=0;p->selected=UINT_MAX;p->active=1;p->lastSerial=serial;return 1;}
static void nfs_logo_p_select(NfsLogoProvenance*p,unsigned tmu,uint32_t addr,uint32_t id,unsigned fmt,unsigned size){
 if(!p->active)return;if(p->selects>=300||p->triangles!=p->selects*2||tmu||addr!=p->addresses[p->selects]||id!=p->ids[p->selects]||fmt!=11||size!=32){nfs_logo_p_cancel(p);return;}p->selected=p->selects++;}
static void nfs_logo_p_triangle(NfsLogoProvenance*p,int exact){
 if(!p->active){p->clearValid=0;return;}if(!exact||p->triangles>=600||p->selected!=p->triangles/2||p->selects!=p->triangles/2+1){nfs_logo_p_contaminate(p);return;}++p->triangles;}
static int nfs_logo_p_finish(NfsLogoProvenance*p,uint64_t serial,uint32_t fbo){
 int ok=p->active&&serial==p->pendingSerial&&p->context==p->pendingContext&&p->uploads==p->pendingUploads&&p->clearEvent==p->pendingClear&&fbo==p->pendingFbo&&p->selects==300&&p->triangles==600;
 nfs_logo_p_cancel(p);p->clearValid=0;return ok;}
#endif
