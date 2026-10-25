// SPDX-License-Identifier: MIT
/* Observe-only adapter, included in the prepared GLES renderer. It owns its
   registration, target, source lease and receipt; Controls source ownership is
   never consulted. Original calls and the later gamma/display swap are intact. */
#include "FeedbackModalNativeObserverAbi.h"
#include "FeedbackModalNativePass.h"
#ifndef NFS_FEEDBACK_OBSERVER_THREAD_TOKEN
#define NFS_FEEDBACK_OBSERVER_THREAD_TOKEN() ((uintptr_t)SDL_GetCurrentThreadID())
#endif
static struct {
 NfsFeedbackNativePass pass;NfsFeedbackModalNativeView view;
 NfsFeedbackModalNativeReceipt receipt;NfsNativeLogoTargetProof target;
 uintptr_t context,thread;uint64_t generation;unsigned uploadPending,traceLines;
 TextureInfo*uploadTexture;
} nfs_feedback_observer;
static int nfs_feedback_observer_current(void){return nfs_feedback_observer.generation&&nfs_feedback_observer.context&&nfs_feedback_observer.thread==NFS_FEEDBACK_OBSERVER_THREAD_TOKEN()&&g_glCtx&&(uintptr_t)g_glCtx==nfs_feedback_observer.context&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==nfs_feedback_observer.generation;}
static void nfs_feedback_observer_release(void){void*owner=nfs_feedback_observer.view.lifetimeOwner;void(*release)(void*)=nfs_feedback_observer.view.releaseOwner;memset(&nfs_feedback_observer.view,0,sizeof nfs_feedback_observer.view);nfs_feedback_observer.uploadPending=0;nfs_feedback_observer.uploadTexture=NULL;if(owner&&release)release(owner);}
static void nfs_feedback_observer_trace(const char*event){if(!nfs_logo_trace_enabled()||nfs_feedback_observer.traceLines>=128)return;++nfs_feedback_observer.traceLines;fprintf(stderr,"FEEDBACK MODAL NATIVE %s serial=%llu epoch=%llu generation=%llu uploads=%u selections=%u triangles=%u batches=%u reason=%u\n",event,(unsigned long long)nfs_feedback_observer.view.serial,(unsigned long long)nfs_feedback_observer.view.modalEpoch,(unsigned long long)nfs_feedback_observer.generation,nfs_feedback_observer.pass.original.uploads,nfs_feedback_observer.pass.original.selects,nfs_feedback_observer.pass.original.triangles,nfs_feedback_observer.pass.batches,nfs_feedback_observer.pass.reason);}
static void nfs_feedback_observer_cancel(void){nfs_feedback_native_fail(&nfs_feedback_observer.pass,nfs_feedback_observer.pass.reason?nfs_feedback_observer.pass.reason:NFS_FEEDBACK_NATIVE_STATE);if(nfs_feedback_observer.view.serial)nfs_feedback_observer_trace("refused");memset(&nfs_feedback_observer.receipt,0,sizeof nfs_feedback_observer.receipt);nfs_feedback_observer_release();}
static void nfs_feedback_observer_abandon(void){nfs_feedback_native_retired(&nfs_feedback_observer.pass,nfs_feedback_observer.generation);nfs_feedback_observer_cancel();nfs_feedback_observer.generation=0;nfs_feedback_observer.context=nfs_feedback_observer.thread=0;}
/* No compound GL helper is hidden in a guard. No GL cleanup on owner loss. */
#define NFS_FEEDBACK_OBSERVER_GL(call) do{if(!nfs_feedback_observer_current())goto lost;call;if(!nfs_feedback_observer_current())goto lost;}while(0)
static int nfs_feedback_observer_read_target(NfsNativeLogoTargetProof*out,uint64_t serial){
 GLint fbo=0,vp[4]={0},sc[4]={0},func=0,program=0,ct=0,cn=0,dt=0,dn=0,rbo=0,rw=0,rh=0;
 GLboolean enabled=0,depthMask=0,color[4]={0};GLenum status=0;unsigned w,h;NfsFrameLegacySampling sampling;
 if(!out||!serial||!nfs_feedback_observer_current()||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER||!g_textureEnabled||g_fogEnabled)return 0;
 h=(unsigned)g_framebufferHeight;w=h*4u/3u;if(w>4096||!nfs_frame_gpu_legacy_sampling(w,h,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||memcmp(g_matrix,sampling.projection,sizeof g_matrix))return 0;
 NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_VIEWPORT,vp));NFS_FEEDBACK_OBSERVER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 if((GLuint)fbo!=g_framebuffer||memcmp(vp,sampling.viewport,sizeof vp)||status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_FEEDBACK_OBSERVER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_FEEDBACK_OBSERVER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_FEEDBACK_OBSERVER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_FEEDBACK_OBSERVER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||dt!=GL_RENDERBUFFER||!cn||!dn||(GLuint)cn!=g_framebufferTexture||(GLuint)dn!=g_framebufferDepth)return 0;
 NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo));NFS_FEEDBACK_OBSERVER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)dn));NFS_FEEDBACK_OBSERVER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_FEEDBACK_OBSERVER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_FEEDBACK_OBSERVER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo));if(rw!=(GLint)w||rh!=(GLint)h)return 0;
 NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_DEPTH_FUNC,&func));NFS_FEEDBACK_OBSERVER_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_FEEDBACK_OBSERVER_GL(glGetBooleanv(GL_COLOR_WRITEMASK,color));NFS_FEEDBACK_OBSERVER_GL(enabled=glIsEnabled(GL_DEPTH_TEST));
 if(program!=(GLint)g_shaderProgram||!enabled||!depthMask||!g_depthMask||func!=GL_LEQUAL||!color[0]||!color[1]||!color[2]||!color[3]||!nfs_logo_fullbox(vp,w,h))return 0;
 NFS_FEEDBACK_OBSERVER_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(!nfs_logo_fullbox(sc,w,h))return 0;}
 NFS_FEEDBACK_OBSERVER_GL(status=glGetError());if(status!=GL_NO_ERROR)return 0;
 memset(out,0,sizeof *out);out->framebuffer=(GLuint)fbo;out->width=w;out->height=h;out->contextGeneration=nfs_feedback_observer.generation;/* Target observation claims no depth-clear or publication rights. */out->sampling=sampling;return 1;
 lost:nfs_feedback_observer_abandon();return 0;
}

static int nfs_feedback_observer_stamp(NfsAudioNativeStamp*out){NfsNativeLogoTargetProof target;if(!out||!nfs_feedback_observer.view.serial||!nfs_feedback_observer_read_target(&target,nfs_feedback_observer.view.serial)||memcmp(&target,&nfs_feedback_observer.target,sizeof target))return 0;out->generation=nfs_feedback_observer.generation;out->context=nfs_feedback_observer.context;out->thread=NFS_FEEDBACK_OBSERVER_THREAD_TOKEN();out->framebuffer=target.framebuffer;out->width=target.width;out->height=target.height;return 1;}
static void nfs_feedback_observer_created(void){uint64_t g=nfs_logo_provenance.context;uintptr_t context=(uintptr_t)g_glCtx;if(!g||!context||SDL_GL_GetCurrentContext()!=g_glCtx)return;if(nfs_feedback_observer.generation)nfs_feedback_observer_abandon();nfs_feedback_observer.generation=g;nfs_feedback_observer.context=context;nfs_feedback_observer.thread=NFS_FEEDBACK_OBSERVER_THREAD_TOKEN();if(!nfs_feedback_native_created(&nfs_feedback_observer.pass,g,context,nfs_feedback_observer.thread))nfs_feedback_observer_abandon();}
static void nfs_feedback_observer_retired(int owner){(void)owner;nfs_feedback_observer_abandon();}
static void nfs_feedback_observer_other(void){if(nfs_feedback_observer.view.serial||nfs_feedback_observer.receipt.serial)nfs_feedback_observer_cancel();}
static void nfs_feedback_observer_upload_begin(TextureInfo*ti){NfsAudioNativeStamp stamp;if(!nfs_feedback_observer.view.serial)return;if(nfs_feedback_observer.uploadPending||!ti||!nfs_feedback_observer_stamp(&stamp)||!nfs_audio_native_transition(&nfs_feedback_observer.pass.original,&stamp)||nfs_feedback_observer.pass.original.phase!=1){nfs_feedback_observer_cancel();return;}nfs_feedback_observer.uploadPending=1;nfs_feedback_observer.uploadTexture=ti;}
static void nfs_feedback_observer_upload_done(TextureInfo*ti){
 NfsAudioNativeStamp stamp;uintptr_t offset;uint32_t address;GLboolean exists=0;GLenum error;
 if(!nfs_feedback_observer.view.serial)return;
 if(!nfs_feedback_observer.uploadPending||ti!=nfs_feedback_observer.uploadTexture||!nfs_feedback_observer_stamp(&stamp)){nfs_feedback_observer_cancel();return;}
 nfs_feedback_observer.uploadPending=0;nfs_feedback_observer.uploadTexture=NULL;
 if((uintptr_t)ti<(uintptr_t)g_textures||(uintptr_t)ti-(uintptr_t)g_textures>=sizeof g_textures){nfs_feedback_observer_cancel();return;}
 offset=(uintptr_t)ti-(uintptr_t)g_textures;if(offset%sizeof(TextureInfo)){nfs_feedback_observer_cancel();return;}address=(uint32_t)(offset/sizeof(TextureInfo))*4u;
 if(address>TextureMem-2048||ti->data!=g_textureMem+address||ti->hdDigits){nfs_feedback_observer_cancel();return;}
 NFS_FEEDBACK_OBSERVER_GL(exists=glIsTexture(ti->id));NFS_FEEDBACK_OBSERVER_GL(error=glGetError());
 if(!exists||error!=GL_NO_ERROR||!nfs_feedback_native_upload(&nfs_feedback_observer.pass,&stamp,address,ti->id,ti->fmt,ti->size,(const unsigned char*)ti->data,2048))nfs_feedback_observer_cancel();return;
 lost:nfs_feedback_observer_abandon();
}
static void nfs_feedback_observer_select(unsigned tmu,uint32_t address,uint32_t evenOdd,const GrTexInfo*info,TextureInfo*ti){NfsAudioNativeStamp stamp;if(!nfs_feedback_observer.view.serial)return;if(nfs_feedback_observer.uploadPending||!info||!ti||!nfs_feedback_observer_stamp(&stamp)||!nfs_feedback_native_select(&nfs_feedback_observer.pass,&stamp,tmu,address,ti->id,evenOdd,info->format,ti->size,info->smallLod,info->largeLod,info->aspectRatio))nfs_feedback_observer_cancel();}
static NfsAudioNativeVertex nfs_feedback_observer_vertex(const GrVertex*v){NfsAudioNativeVertex out;out.x=v->x;out.y=v->y;out.r=v->r;out.g=v->g;out.b=v->b;out.a=v->a;out.oow=v->oow;out.sow=v->tmuvtx[0].sow;out.tow=v->tmuvtx[0].tow;return out;}
static void nfs_feedback_observer_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){NfsAudioNativeStamp stamp;NfsAudioNativeVertex av,bv,cv;if(!nfs_feedback_observer.view.serial)return;if(nfs_feedback_observer.uploadPending||!a||!b||!c||!nfs_feedback_observer_stamp(&stamp)){nfs_feedback_observer_cancel();return;}av=nfs_feedback_observer_vertex(a);bv=nfs_feedback_observer_vertex(b);cv=nfs_feedback_observer_vertex(c);if(!nfs_feedback_native_triangle(&nfs_feedback_observer.pass,&stamp,&av,&bv,&cv,g_textureEnabled?1u:0u,g_fogEnabled?1u:0u))nfs_feedback_observer_cancel();}
/* Verify the active GL attribute sources before reading actual SoA data. */
static int nfs_feedback_observer_attribute(GLint location,GLint size,GLenum type,GLboolean normalized,const void*expected){
 GLint enabled=0,actualSize=0,actualType=0,norm=0,stride=0,buffer=0;void*pointer=NULL;if(location<0)return 0;
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&enabled));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_SIZE,&actualSize));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_TYPE,&actualType));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&norm));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&stride));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribiv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&buffer));
 NFS_FEEDBACK_OBSERVER_GL(glGetVertexAttribPointerv((GLuint)location,GL_VERTEX_ATTRIB_ARRAY_POINTER,&pointer));
 return enabled&&actualSize==size&&actualType==(GLint)type&&norm==(GLint)normalized&&!stride&&!buffer&&pointer==expected;
 lost:nfs_feedback_observer_abandon();return 0;
}
/* This callback MUST be after HD/mark sync, immediately before the original draw.
   Initial HD/mark state is deliberately not a begin/select admission condition. */
static void nfs_feedback_observer_actual_batch(void){
 NfsAudioNativeStamp stamp;NfsFeedbackActualBatch state;NfsFeedbackActualVertex vertices[6];GLint loc=-1,value=0,active=0;GLenum error=0;
 if(!nfs_feedback_observer.view.serial)return;
 if(nfs_feedback_observer.uploadPending||g_trianglesCount!=2||!nfs_feedback_observer_stamp(&stamp))goto refused;
 memset(&state,0,sizeof state);
 NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&value));state.program=(uint32_t)value;
 if(state.program!=g_shaderProgram)goto refused;
#define NFS_FEEDBACK_UNIFORM(name,member,query) do{NFS_FEEDBACK_OBSERVER_GL(loc=glGetUniformLocation(g_shaderProgram,name));if(loc<0)goto refused;NFS_FEEDBACK_OBSERVER_GL(query(g_shaderProgram,loc,&state.member));}while(0)
 NFS_FEEDBACK_UNIFORM("uTextureSampler",sampler,glGetUniformiv);
 NFS_FEEDBACK_OBSERVER_GL(loc=glGetUniformLocation(g_shaderProgram,"uMatrix"));if(loc<0)goto refused;NFS_FEEDBACK_OBSERVER_GL(glGetUniformfv(g_shaderProgram,loc,state.matrix));
 NFS_FEEDBACK_UNIFORM("uTextureEnabled",textureEnabled,glGetUniformfv);
 NFS_FEEDBACK_UNIFORM("uFogEnabled",fogEnabled,glGetUniformfv);
 NFS_FEEDBACK_UNIFORM("uHdActive",hdActive,glGetUniformfv);
 NFS_FEEDBACK_UNIFORM("uMarkActive",markActive,glGetUniformfv);
#undef NFS_FEEDBACK_UNIFORM
 /* Texture0 is sampled even if an auxiliary HD unit happened to be active. */
 NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&active));NFS_FEEDBACK_OBSERVER_GL(glActiveTexture(GL_TEXTURE0));NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&value));state.texture0=(uint32_t)value;NFS_FEEDBACK_OBSERVER_GL(glActiveTexture((GLenum)active));
 NFS_FEEDBACK_OBSERVER_GL(state.blendEnabled=glIsEnabled(GL_BLEND)?1u:0u);
#define NFS_FEEDBACK_INTEGER(name,member) do{NFS_FEEDBACK_OBSERVER_GL(glGetIntegerv(name,&value));state.member=(uint32_t)value;}while(0)
 NFS_FEEDBACK_INTEGER(GL_BLEND_EQUATION_RGB,equationRGB);NFS_FEEDBACK_INTEGER(GL_BLEND_EQUATION_ALPHA,equationAlpha);
 NFS_FEEDBACK_INTEGER(GL_BLEND_SRC_RGB,sourceRGB);NFS_FEEDBACK_INTEGER(GL_BLEND_DST_RGB,destinationRGB);
 NFS_FEEDBACK_INTEGER(GL_BLEND_SRC_ALPHA,sourceAlpha);NFS_FEEDBACK_INTEGER(GL_BLEND_DST_ALPHA,destinationAlpha);
#undef NFS_FEEDBACK_INTEGER
 if(!nfs_feedback_observer_attribute(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,g_vertices)||!nfs_feedback_observer_attribute(g_aTexCoordLoc,4,GL_FLOAT,GL_FALSE,g_textureCoord)||!nfs_feedback_observer_attribute(g_aColorLoc,4,GL_UNSIGNED_BYTE,GL_TRUE,g_colorValues))goto refused;
 for(unsigned k=0;k<6;++k){unsigned t=k/3,v=k%3;NfsFeedbackActualVertex*out=vertices+k;out->x=g_vertices[t].vertex[v].x;out->y=g_vertices[t].vertex[v].y;out->z=g_vertices[t].vertex[v].z;out->s=g_textureCoord[t].vertex[v].s;out->t=g_textureCoord[t].vertex[v].t;out->marker=g_textureCoord[t].vertex[v].r;out->q=g_textureCoord[t].vertex[v].q;out->r=g_colorValues[t].vertex[v].r;out->g=g_colorValues[t].vertex[v].g;out->b=g_colorValues[t].vertex[v].b;out->a=g_colorValues[t].vertex[v].a;}
 NFS_FEEDBACK_OBSERVER_GL(error=glGetError());if(error!=GL_NO_ERROR||!nfs_feedback_observer_stamp(&stamp)||!nfs_feedback_native_actual_batch(&nfs_feedback_observer.pass,&stamp,&state,vertices,6))goto refused;return;
 refused:nfs_feedback_observer_cancel();return;
 lost:nfs_feedback_observer_abandon();
}
int nfs_private_feedback_modal_observer_begin(const NfsFeedbackModalNativeView*v){
 NfsNativeLogoTargetProof target;NfsAudioNativeStamp stamp;
 if(nfs_feedback_observer.view.serial){nfs_feedback_observer_cancel();return 0;}
 memset(&nfs_feedback_observer.receipt,0,sizeof nfs_feedback_observer.receipt);
 if(!v||v->schema!=1||v->width!=640||v->height!=480||!v->serial||!v->modalEpoch||v->planeWords!=307200||!v->final565||!v->lifetimeOwner||!v->retainOwner||!v->releaseOwner||!nfs_feedback_observer_current())return 0;
 /* Flush only pre-existing original geometry before opening our witness. */
 drawTriangles();if(!nfs_feedback_observer_read_target(&target,v->serial))return 0;
 v->retainOwner(v->lifetimeOwner);nfs_feedback_observer.view=*v;nfs_feedback_observer.target=target;
 if(!nfs_feedback_observer_stamp(&stamp)||!nfs_feedback_native_begin(&nfs_feedback_observer.pass,&stamp,v->serial,v->modalEpoch,g_shaderProgram,target.sampling.projection,v->final565,v->planeWords)){nfs_feedback_observer_cancel();return 0;}return 1;
}
int nfs_private_feedback_modal_observer_finish(uint64_t serial,uint64_t modalEpoch){
 NfsAudioNativeStamp stamp;NfsFeedbackModalNativeReceipt receipt;
 if(!serial||serial!=nfs_feedback_observer.view.serial||modalEpoch!=nfs_feedback_observer.view.modalEpoch){nfs_feedback_observer_cancel();return 0;}
 if(!nfs_feedback_observer_current()){nfs_feedback_observer_abandon();return 0;}
 drawTriangles();if(!nfs_feedback_observer.view.serial||nfs_feedback_observer.uploadPending||!nfs_feedback_observer_stamp(&stamp)||!nfs_feedback_native_finish(&nfs_feedback_observer.pass,&stamp,serial)){nfs_feedback_observer_cancel();return 0;}
 memset(&receipt,0,sizeof receipt);receipt.schema=1;receipt.admitted=1;receipt.serial=serial;receipt.modalEpoch=modalEpoch;receipt.contextGeneration=nfs_feedback_observer.generation;receipt.batches=nfs_feedback_observer.pass.batches;
 nfs_feedback_observer_trace("admitted");nfs_feedback_observer_release();nfs_feedback_observer.receipt=receipt;return 1;
}
void nfs_private_feedback_modal_observer_cancel(uint64_t serial,uint64_t modalEpoch){if(serial&&((serial==nfs_feedback_observer.view.serial&&modalEpoch==nfs_feedback_observer.view.modalEpoch)||(serial==nfs_feedback_observer.receipt.serial&&modalEpoch==nfs_feedback_observer.receipt.modalEpoch)))nfs_feedback_observer_cancel();}
int nfs_private_feedback_modal_observer_take_receipt(uint64_t serial,uint64_t modalEpoch,NfsFeedbackModalNativeReceipt*out){if(!out||!nfs_feedback_observer_current()||!serial||serial!=nfs_feedback_observer.receipt.serial||modalEpoch!=nfs_feedback_observer.receipt.modalEpoch)return 0;*out=nfs_feedback_observer.receipt;memset(&nfs_feedback_observer.receipt,0,sizeof nfs_feedback_observer.receipt);return 1;}
static void nfs_feedback_observer_before_swap(void){if(nfs_feedback_observer.view.serial)(void)nfs_private_feedback_modal_observer_finish(nfs_feedback_observer.view.serial,nfs_feedback_observer.view.modalEpoch);}
#undef NFS_FEEDBACK_OBSERVER_GL
