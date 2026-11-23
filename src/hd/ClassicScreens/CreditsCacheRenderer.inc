// SPDX-License-Identifier: MIT
#include "CreditsCacheAbi.h"
#include "CreditsWholeGpu.inc"
typedef struct NfsCreditsNativeTarget {GLuint fbo,color,depth;unsigned width,height;GLint viewport[4],scissor[4];}NfsCreditsNativeTarget;
typedef struct NfsCreditsNativeCache {NfsCreditsCacheView view;uint32_t ids[300];unsigned witness[300];uint64_t revision[300];}NfsCreditsNativeCache;
typedef struct NfsCreditsDrawReceipt {uint64_t request;uintptr_t domain;unsigned valid;int32_t x,y;float clip[4];NfsCreditsNativeTarget target;}NfsCreditsDrawReceipt;
static struct {NfsCreditsNativeCache cache[2];NfsCreditsWholeGpu gpu;NfsCreditsCacheDrawView draw;NfsCreditsDrawReceipt base,partner;NfsCreditsNativeTarget target;uint64_t generation;uintptr_t context,thread;unsigned active,slot,selects,triangles,swapObserver,traceLines[2];uint64_t uploads;}nfs_cc;
static int nfs_cc_context(void){return nfs_cc.generation&&nfs_credits_context(nfs_cc.generation,nfs_cc.context)&&nfs_cc.thread==(uintptr_t)SDL_GetCurrentThreadID();}
static int nfs_cc_current(void){const NfsCreditsCacheView*v=&nfs_cc.cache[nfs_cc.slot].view;return nfs_cc_context()&&v->owner&&v->current&&v->current(v->owner);}
static int nfs_cc_context_stage(void*u,const NfsBackgroundSceneStage*t){return u==&nfs_cc&&t&&t->generation==nfs_cc.generation&&t->contextIdentity==nfs_cc.context&&nfs_cc_context();}
static void nfs_cc_trace(const char*event){if(nfs_cc.slot>1||nfs_cc.traceLines[nfs_cc.slot]>=96||!nfs_logo_trace_enabled())return;++nfs_cc.traceLines[nfs_cc.slot];fprintf(stderr,"CREDITS CACHE %s request=%llu slot=%u selects=%u triangles=%u origin=%d,%d\n",event,(unsigned long long)nfs_cc.draw.request,nfs_cc.slot,nfs_cc.selects,nfs_cc.triangles,nfs_cc.draw.x,nfs_cc.draw.y);}
static void nfs_cc_release_cache(NfsCreditsNativeCache*c){NfsCreditsCacheView v=c->view;memset(c,0,sizeof *c);if(v.owner&&v.release)v.release(v.owner);}
int nfs_private_credits_cache_reset(void){nfs_cc.active=0;nfs_cc.base.valid=nfs_cc.partner.valid=0;nfs_cwg_revoke(&nfs_cc.gpu);nfs_cc_release_cache(nfs_cc.cache);nfs_cc_release_cache(nfs_cc.cache+1);return 1;}
static void nfs_credits_cached_created(void){(void)nfs_private_credits_cache_reset();memset(&nfs_cc,0,sizeof nfs_cc);nfs_cc.generation=nfs_logo_provenance.context;nfs_cc.context=(uintptr_t)g_glCtx;nfs_cc.thread=(uintptr_t)SDL_GetCurrentThreadID();}
static void nfs_credits_cached_retired(int owner){nfs_cwg_retire(&nfs_cc.gpu,owner&&nfs_cc_context());(void)nfs_private_credits_cache_reset();memset(&nfs_cc,0,sizeof nfs_cc);}
static void nfs_credits_cached_swap_begin(void){nfs_cc.swapObserver=1;}
static void nfs_credits_cached_swap_end(void){nfs_cc.swapObserver=0;}
static void nfs_credits_cached_other(int painting){if(nfs_cc.active){nfs_cc.active=0;nfs_cc_trace("native-state-changed");}if(!nfs_cc.swapObserver)nfs_cwg_revoke(&nfs_cc.gpu);if(painting&&!nfs_cc.swapObserver)nfs_cc.base.valid=nfs_cc.partner.valid=0;}
static void nfs_credits_cached_upload(TextureInfo*ti){(void)ti;if(!nfs_cc.active&&!nfs_cc.base.valid&&!nfs_cc.gpu.ready)return;nfs_cc.active=0;nfs_cc.base.valid=nfs_cc.partner.valid=0;nfs_cwg_revoke(&nfs_cc.gpu);}
#define NFS_CC_GL(call) do{if(!nfs_cc_current())goto lost;call;if(!nfs_cc_current())goto lost;}while(0)
static int nfs_cc_target(NfsCreditsNativeTarget*out,const float*clip){GLint type=0,name=0,oldRbo=0,rw=0,rh=0,bound=0;GLenum error=0;int rebound=0;float expected[16]={0},ratio=0;GLint sx=0,sy=0,ex=0,ey=0;
 if(!out||!clip||!nfs_cc_current()||g_renderBuffer!=GR_BUFFER_BACKBUFFER||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer)return 0;
 for(unsigned i=0;i<4;++i)if(!isfinite(clip[i])||clip[i]<0||clip[i]>(i%2?480:640))return 0;if(clip[0]>=clip[2]||clip[1]>=clip[3])return 0;
 memset(out,0,sizeof *out);out->fbo=g_framebuffer;out->color=g_framebufferTexture;out->depth=g_framebufferDepth;out->height=(unsigned)g_framebufferHeight;out->width=out->height*4u/3u;if(!out->color||!out->depth||out->width>4096)return 0;
 ratio=g_framebufferHeight/480.f;sx=(GLint)(clip[0]*ratio);sy=(GLint)(clip[1]*ratio);ex=(GLint)(clip[2]*ratio+.5f);ey=(GLint)(clip[3]*ratio+.5f);if(sx>=ex||sy>=ey)return 0;
 expected[0]=2*ratio/(ex-sx);expected[5]=-2*ratio/(ey-sy);expected[10]=-2;expected[12]=-(float)(ex+sx)/(ex-sx);expected[13]=(float)(ey+sy)/(ey-sy);expected[14]=expected[15]=1;
 for(unsigned i=0;i<16;++i)if(!isfinite(g_matrix[i])||fabsf(g_matrix[i]-expected[i])>0.00001f)return 0;
 NFS_CC_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&bound));if((GLuint)bound!=out->fbo)return 0;
 NFS_CC_GL(glGetIntegerv(GL_VIEWPORT,out->viewport));NFS_CC_GL(glGetIntegerv(GL_SCISSOR_BOX,out->scissor));
 {const GLint box[4]={sx,(GLint)out->height-ey,ex-sx,ey-sy};if(memcmp(box,out->viewport,sizeof box)||memcmp(box,out->scissor,sizeof box))return 0;}
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||(GLuint)name!=out->color)return 0;
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_RENDERBUFFER||(GLuint)name!=out->depth)return 0;
 NFS_CC_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&oldRbo));rebound=1;NFS_CC_GL(glBindRenderbuffer(GL_RENDERBUFFER,out->depth));NFS_CC_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_CC_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_CC_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)oldRbo));rebound=0;NFS_CC_GL(error=glGetError());return rw==(GLint)out->width&&rh==(GLint)out->height&&error==GL_NO_ERROR;
 lost:if(rebound&&nfs_cc_context())glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)oldRbo);return 0;
}
static int nfs_cc_same_target(const NfsCreditsNativeTarget*a,const NfsCreditsNativeTarget*b){return a->fbo==b->fbo&&a->color==b->color&&a->depth==b->depth&&a->width==b->width&&a->height==b->height;}
static int nfs_cc_texture_current(NfsCreditsNativeCache*c,unsigned i){const uint32_t address=c->view.addresses[i];if(address>TextureMem-2048||(address&3)||c->witness[i]>=1024)return 0;TextureInfo*ti=g_textures+(address>>2);return ti->id==c->ids[i]&&ti->size==32&&ti->fmt==GR_TEXFMT_ARGB_1555&&!ti->hdDigits&&ti->data==g_textureMem+address&&nfs_native_upload_receipt_current(nfs_logo_uploads+c->witness[i],nfs_cc.generation,c->revision[i],address,ti->id,ti->fmt,(const unsigned char*)ti->data);}
static int nfs_cc_cache_bind(const NfsCreditsCacheView*v){NfsCreditsNativeCache*c=nfs_cc.cache+v->slot;
 if(c->view.owner==v->owner&&c->view.content==v->content){for(unsigned i=0;i<300;++i)if(!nfs_cc_texture_current(c,i))return 0;return 1;}
 nfs_cc_release_cache(c);v->retain(v->owner);c->view=*v;nfs_cc.slot=v->slot;
 for(unsigned i=0;i<300;++i){const uint32_t a=v->addresses[i];if((a&3)||a>TextureMem-2048)goto refuse;TextureInfo*ti=g_textures+(a>>2);if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+a)goto refuse;
  for(unsigned j=0;j<i;++j)if(ti->id==c->ids[j]||(uint64_t)a<(uint64_t)v->addresses[j]+2048&&(uint64_t)v->addresses[j]<(uint64_t)a+2048)goto refuse;
  c->witness[i]=1024;for(unsigned j=0;j<1024;++j){const NfsLogoUploadWitness*w=nfs_logo_uploads+j;if(w->valid&&w->context==nfs_cc.generation&&w->address==a&&w->id==ti->id&&w->format==GR_TEXFMT_ARGB_1555&&w->revision&&nfs_native_upload_receipt_current(w,nfs_cc.generation,w->revision,a,ti->id,ti->fmt,(const unsigned char*)ti->data)){c->witness[i]=j;c->revision[i]=w->revision;break;}}if(c->witness[i]==1024)goto refuse;
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){const uint16_t raw=v->original565[((i%15)*32+y)*640+(i/15)*32+x];uint16_t packed=(uint16_t)(((raw&0xf800)>>1)|((raw&0x7c0)>>1)|(raw&31));if(packed)packed|=0x8000;const uint16_t expected=(uint16_t)((packed<<1)|(packed>>15));uint16_t actual=0;memcpy(&actual,(const uint8_t*)ti->data+2*(y*32+x),2);if(expected!=actual)goto refuse;}
  c->ids[i]=ti->id;{GLboolean live=0;NFS_CC_GL(live=glIsTexture(ti->id));if(!live)goto refuse;}
 }
 return 1;
 refuse:nfs_cc_release_cache(c);return 0;
 lost:nfs_cc_release_cache(c);return 0;
}
int nfs_private_credits_cache_begin(const NfsCreditsCacheDrawView*v){NfsCreditsNativeTarget target;const NfsCreditsCacheView*c=v?&v->cache:NULL;nfs_cc.active=0;
 if(!c||c->schema!=1||c->slot>1||c->locale>=6||!c->content||!c->sourceDomain||!c->owner||!c->retain||!c->release||!c->current||!c->current(c->owner)||!c->original565||c->pixels!=640*480||!v->request||v->x||v->step!=32||(c->slot?v->y!=0:v->y<= -100||v->y>=450)||!nfs_cc_context())return 0;
 if(c->slot==1){nfs_cc.base.valid=nfs_cc.partner.valid=0;nfs_cwg_revoke(&nfs_cc.gpu);}else if(!nfs_cc.base.valid||nfs_cc.base.domain!=c->sourceDomain)return 0;
 drawTriangles();nfs_cc.slot=c->slot;if(!nfs_cc_cache_bind(c)||!nfs_cc_target(&target,v->clip))return 0;if(!c->slot&&!nfs_cc_same_target(&target,&nfs_cc.base.target))return 0;
 nfs_cc.draw=*v;nfs_cc.target=target;nfs_cc.selects=nfs_cc.triangles=0;nfs_cc.uploads=nfs_logo_provenance.uploads;nfs_cc.active=1;return 1;
}
static int nfs_cc_vertex(const GrVertex*v,unsigned tile,unsigned corner){const unsigned right=corner==1||corner==2;return v&&v->x==(float)(nfs_cc.draw.x+(int)(tile/15)*32+(right?32:0))+VertexSnap&&v->y==(float)(nfs_cc.draw.y+(int)(tile%15)*32+(corner>=2?32:0))+VertexSnap&&v->r==255&&v->g==255&&v->b==255&&v->a==255&&v->oow==1&&v->tmuvtx[0].sow==(right?255:0)&&v->tmuvtx[0].tow==(corner>=2?255:0);}
static void nfs_credits_cached_select(unsigned tmu,uint32_t address,uint32_t parity,const GrTexInfo*info,TextureInfo*ti){if(!nfs_cc.active)return;const unsigned i=nfs_cc.selects;NfsCreditsNativeCache*c=nfs_cc.cache+nfs_cc.slot;if(!nfs_cc_current()||i>=300||nfs_cc.triangles!=i*2||tmu||parity!=3||!info||!ti||info->format!=GR_TEXFMT_ARGB_1555||info->smallLod!=3||info->largeLod!=3||info->aspectRatio!=3||address!=c->view.addresses[i]||ti->id!=c->ids[i]||!nfs_cc_texture_current(c,i)){nfs_cc.active=0;return;}++nfs_cc.selects;}
static void nfs_credits_cached_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){if(!nfs_cc.active){if(nfs_cc.gpu.ready)nfs_cwg_revoke(&nfs_cc.gpu);return;}const unsigned i=nfs_cc.triangles/2,half=nfs_cc.triangles&1;if(!nfs_cc_current()||i>=300||nfs_cc.selects!=i+1||!g_textureEnabled||g_fogEnabled||!nfs_cc_vertex(a,i,0)||!nfs_cc_vertex(b,i,half?2:1)||!nfs_cc_vertex(c,i,half?3:2)){nfs_cc.active=0;return;}++nfs_cc.triangles;}
int nfs_private_credits_cache_finish(uint64_t request){NfsCreditsNativeTarget target;GLenum error=0;int okay=0;if(!request||request!=nfs_cc.draw.request)return 0;drawTriangles();
 if(!nfs_cc.active||!nfs_cc_current()||nfs_cc.selects!=300||nfs_cc.triangles!=600||nfs_cc.uploads!=nfs_logo_provenance.uploads||!nfs_cc_target(&target,nfs_cc.draw.clip)||memcmp(&target,&nfs_cc.target,sizeof target))goto done;
 NFS_CC_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 {NfsCreditsDrawReceipt*r=nfs_cc.slot?&nfs_cc.base:&nfs_cc.partner;memset(r,0,sizeof *r);r->request=request;r->domain=nfs_cc.draw.cache.sourceDomain;r->x=nfs_cc.draw.x;r->y=nfs_cc.draw.y;memcpy(r->clip,nfs_cc.draw.clip,sizeof r->clip);r->target=target;r->valid=1;okay=1;nfs_cc_trace("native300600-verified");}
 done:nfs_cc.active=0;return okay;
 lost:nfs_cc.active=0;return 0;
}
int nfs_private_credits_cache_cancel(uint64_t request){if(request&&request==nfs_cc.draw.request)nfs_cc.active=0;return 1;}
int nfs_private_credits_whole_submit(const NfsCreditsWholeView*v){static const float full[4]={0,0,640,480};NfsCreditsNativeTarget target;NfsBackgroundSceneStage guard;int w=0,h=0;
 if(!v||!nfs_cc_context()||!nfs_cc.base.valid||v->sourceDomain!=nfs_cc.base.domain||v->nativeBaseRequest!=nfs_cc.base.request||!v->owner||!v->current||!v->current(v->owner)||(v->hasPartner?(!nfs_cc.partner.valid||v->sourceDomain!=nfs_cc.partner.domain||v->nativePartnerRequest!=nfs_cc.partner.request):v->nativePartnerRequest!=0))return 0;
 nfs_cc.slot=1;drawTriangles();if(!nfs_cc_target(&target,full)||!nfs_cc_same_target(&target,&nfs_cc.base.target)||!SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w!=winWidth||h!=winHeight)return 0;
 if(v->locale!=nfs_cc.cache[1].view.locale)return 0;if(v->hasPartner){if(memcmp(&v->partnerClip,nfs_cc.partner.clip,sizeof nfs_cc.partner.clip))return 0;for(unsigned i=0;i<2;++i){NfsCreditsWholeRect expected=nfs_cc.cache[0].view.sprite[i];expected.left+=nfs_cc.partner.x;expected.right+=nfs_cc.partner.x;expected.top+=nfs_cc.partner.y;expected.bottom+=nfs_cc.partner.y;if(memcmp(&expected,v->partner+i,sizeof expected))return 0;}}
 memset(&guard,0,sizeof guard);guard.generation=nfs_cc.generation;guard.contextIdentity=nfs_cc.context;guard.framebuffer=target.fbo;guard.width=target.width;guard.height=target.height;guard.ownerCurrent=nfs_cc_context_stage;guard.ownerUser=&nfs_cc;
 return nfs_cwg_render(&nfs_cc.gpu,v,&guard,(unsigned)w,(unsigned)h);
}
/* The staged image is selected only for the actual completed native front
   attachments. A matching texture number alone does not establish ownership. */
static GLuint nfs_credits_wide_present(GLuint front,GLuint color,int w,int h){
 GLint oldFbo=0,type=0,name=0;GLuint result=0;int rebound=0;GLenum error=0;
 if(!nfs_cc.base.valid||!nfs_cc_context()||front!=nfs_cc.base.target.fbo||color!=nfs_cc.base.target.color||w!=winWidth||h!=winHeight||w!=(int)nfs_cc.gpu.width||h!=(int)nfs_cc.gpu.height)return 0;
 nfs_cc.slot=1;if(!nfs_cc_current()||!nfs_cwg_staging_color(&nfs_cc.gpu))return 0;
 NFS_CC_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&oldFbo));
 rebound=1;NFS_CC_GL(glBindFramebuffer(GL_FRAMEBUFFER,front));
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||(GLuint)name!=color)goto restore;
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));
 NFS_CC_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_RENDERBUFFER||(GLuint)name!=nfs_cc.base.target.depth)goto restore;
 NFS_CC_GL(error=glGetError());if(error==GL_NO_ERROR)result=nfs_cwg_staging_color(&nfs_cc.gpu);
 restore:if(rebound&&nfs_cc_context())glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)oldFbo);
 return nfs_cc_current()?result:0;
 lost:if(rebound&&nfs_cc_context())glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)oldFbo);return 0;
}
#undef NFS_CC_GL
