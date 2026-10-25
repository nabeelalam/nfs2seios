// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_NATIVE_PASS_H
#define NFS_FEEDBACK_MODAL_NATIVE_PASS_H
#include "AudioNativePass.h"
#include <float.h>
/* Source-neutral numeric witness only. The adapter must independently acquire
   the registered renderer/context/target and actual program/uniforms/arrays.
   Batch observations belong AFTER HD/mark sync, immediately BEFORE glDrawArrays.
   They must never be copied from expected metadata or source triangle callbacks.
   Gamma remains in the original later display pass. This grants no GL rights. */
enum {NFS_FEEDBACK_NATIVE_OK=0,NFS_FEEDBACK_NATIVE_SOURCE=1,NFS_FEEDBACK_NATIVE_ORDER=2,
 NFS_FEEDBACK_NATIVE_STATE=3,NFS_FEEDBACK_NATIVE_VERTEX=4,NFS_FEEDBACK_NATIVE_INPUT=5};
enum {NFS_FEEDBACK_GL_ZERO=0,NFS_FEEDBACK_GL_ONE=1,NFS_FEEDBACK_GL_SRC_ALPHA=0x0302,
 NFS_FEEDBACK_GL_ONE_MINUS_SRC_ALPHA=0x0303,NFS_FEEDBACK_GL_FUNC_ADD=0x8006};
typedef struct NfsFeedbackActualVertex {
 float x,y,z,s,t,marker,q;unsigned char r,g,b,a;
} NfsFeedbackActualVertex;
typedef struct NfsFeedbackActualBatch {
 uint32_t program,texture0;int32_t sampler;
 float matrix[16],textureEnabled,fogEnabled,hdActive,markActive;
 uint32_t blendEnabled,equationRGB,equationAlpha,sourceRGB,destinationRGB,sourceAlpha,destinationAlpha;
} NfsFeedbackActualBatch;
typedef struct NfsFeedbackNativePass {
 NfsAudioNativePass original;uint64_t epoch,lastEpoch;uint32_t program,batches,reason;
 float matrix[16];uint32_t textureIds[300];
} NfsFeedbackNativePass;
static int nfs_feedback_native_fail(NfsFeedbackNativePass*p,uint32_t why){if(p){p->reason=why;nfs_audio_native_fail(&p->original,NFS_AUDIO_NATIVE_OTHER);}return 0;}
static int nfs_feedback_native_separate(const NfsFeedbackNativePass*p,const void*data,size_t bytes){
 uintptr_t a=(uintptr_t)data,b=(uintptr_t)p;return p&&data&&bytes&&a<=UINTPTR_MAX-bytes&&b<=UINTPTR_MAX-sizeof *p&&!(a<b+sizeof *p&&b<a+bytes);
}
static int nfs_feedback_native_finite(float x){return x>=-FLT_MAX&&x<=FLT_MAX;}
static int nfs_feedback_native_created(NfsFeedbackNativePass*p,uint64_t generation,uintptr_t context,uintptr_t thread){
 if(!p)return 0;
 if(!nfs_audio_native_created(&p->original,generation,context,thread)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}
 p->program=p->batches=p->reason=0;p->epoch=0;return 1;
}
static void nfs_feedback_native_retired(NfsFeedbackNativePass*p,uint64_t generation){
 if(!p||!generation||generation!=p->original.generation)return;
 nfs_audio_native_retired(&p->original,generation);p->program=p->batches=0;p->epoch=0;p->reason=NFS_FEEDBACK_NATIVE_SOURCE;
}
static int nfs_feedback_native_begin(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,uint64_t serial,uint64_t epoch,uint32_t program,const float*projection,const uint16_t*original565,size_t pixels){
 if(!p)return 0;
 if(!epoch||epoch==UINT64_MAX||epoch<p->lastEpoch||!program||!nfs_feedback_native_separate(p,projection,16*sizeof(float))||!nfs_feedback_native_separate(p,original565,NFS_AUDIO_NATIVE_PIXELS*sizeof(uint16_t)))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_INPUT);
 for(unsigned k=0;k<16;++k)if(!nfs_feedback_native_finite(projection[k]))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_INPUT);
 if(!nfs_audio_native_begin(&p->original,now,serial,original565,pixels)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}
 p->epoch=p->lastEpoch=epoch;p->program=program;p->batches=p->reason=0;memcpy(p->matrix,projection,sizeof p->matrix);memset(p->textureIds,0,sizeof p->textureIds);return 1;
}
static int nfs_feedback_native_upload(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,uint32_t address,uint32_t id,uint32_t format,uint32_t size,const unsigned char*actual,size_t bytes){
 if(!p)return 0;
 // Original downloads flush the previous tile before replacing its texture.
 if(p->batches!=p->original.uploads)return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_ORDER);
 if(!nfs_feedback_native_separate(p,actual,bytes))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_INPUT);
 if(!nfs_audio_native_upload(&p->original,now,address,id,format,size,actual,bytes)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}
 p->textureIds[p->original.uploads-1]=id;return 1;
}
static int nfs_feedback_native_select(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,uint32_t tmu,uint32_t address,uint32_t id,uint32_t evenOdd,uint32_t format,uint32_t size,uint32_t smallLod,uint32_t largeLod,uint32_t aspect){
 if(!p)return 0;
 if(!nfs_audio_native_select(&p->original,now,tmu,address,id,evenOdd,format,size,smallLod,largeLod,aspect)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}return 1;
}
static int nfs_feedback_native_triangle(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,const NfsAudioNativeVertex*a,const NfsAudioNativeVertex*b,const NfsAudioNativeVertex*c,uint32_t textured,uint32_t fogged){
 if(!p)return 0;
 if(!nfs_audio_native_triangle(&p->original,now,a,b,c,textured,fogged)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}return 1;
}
static int nfs_feedback_native_opaque_factors(uint32_t source,uint32_t destination){return(source==NFS_FEEDBACK_GL_ONE||source==NFS_FEEDBACK_GL_SRC_ALPHA)&&(destination==NFS_FEEDBACK_GL_ZERO||destination==NFS_FEEDBACK_GL_ONE_MINUS_SRC_ALPHA);}
static int nfs_feedback_native_blend(const NfsFeedbackActualBatch*s){
 if(s->blendEnabled>1)return 0;
 if(!s->blendEnabled)return 1;
 return s->equationRGB==NFS_FEEDBACK_GL_FUNC_ADD&&s->equationAlpha==NFS_FEEDBACK_GL_FUNC_ADD&&nfs_feedback_native_opaque_factors(s->sourceRGB,s->destinationRGB)&&nfs_feedback_native_opaque_factors(s->sourceAlpha,s->destinationAlpha);
}
static int nfs_feedback_native_actual_corner(const NfsFeedbackActualVertex*v,unsigned tile,unsigned corner){
 const float x=(float)((tile/15)*32+(corner==1||corner==2?32:0)),y=(float)((tile%15)*32+(corner>=2?32:0));
 return v->x==x&&v->y==y&&v->z==1&&v->s==(corner==1||corner==2?255.0f/256.0f:0)&&v->t==(corner>=2?255.0f/256.0f:0)&&v->marker==0&&v->q==1&&v->r==255&&v->g==255&&v->b==255&&v->a==255;
}
static int nfs_feedback_native_actual_batch(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,const NfsFeedbackActualBatch*state,const NfsFeedbackActualVertex*vertices,size_t count){
 static const unsigned corners[6]={0,1,2,0,2,3};if(!p)return 0;
 if(!nfs_audio_native_transition(&p->original,now)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}
 if(p->batches>=300||p->original.phase!=1||p->original.uploads!=p->batches+1||p->original.selects!=p->batches+1||p->original.triangles!=(p->batches+1)*2)return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_ORDER);
 if(count!=6||!nfs_feedback_native_separate(p,state,sizeof *state)||!nfs_feedback_native_separate(p,vertices,6*sizeof *vertices))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_INPUT);
 if(state->program!=p->program||state->texture0!=p->textureIds[p->batches]||state->sampler||state->textureEnabled!=1||state->fogEnabled!=0||state->hdActive!=0||state->markActive!=0||!nfs_feedback_native_blend(state))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_STATE);
 for(unsigned k=0;k<16;++k)if(!nfs_feedback_native_finite(state->matrix[k])||state->matrix[k]!=p->matrix[k])return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_STATE);
 for(unsigned k=0;k<6;++k)if(!nfs_feedback_native_actual_corner(vertices+k,p->batches,corners[k]))return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_VERTEX);
 ++p->batches;return 1;
}
static int nfs_feedback_native_other(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now){if(!p)return 0;
 if(!nfs_audio_native_transition(&p->original,now)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_ORDER);}
static int nfs_feedback_native_finish(NfsFeedbackNativePass*p,const NfsAudioNativeStamp*now,uint64_t serial){
 if(!p)return 0;
 if(p->batches!=300)return nfs_feedback_native_fail(p,NFS_FEEDBACK_NATIVE_ORDER);
 if(!nfs_audio_native_finish(&p->original,now,serial)){p->reason=NFS_FEEDBACK_NATIVE_SOURCE;return 0;}p->reason=NFS_FEEDBACK_NATIVE_OK;return 1;
}
#endif
