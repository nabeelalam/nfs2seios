// SPDX-License-Identifier: MIT
/* Include once at renderer end. All optional work requires trusted art installed
   before context creation. Original draws and later labels/cursor remain intact. */
#include "HdArtProvider.h"
#include "NativeSceneProvenance.h"
#include "BackgroundSceneCompositor.c.inc"
static NfsBackgroundSceneCompositor nfs_bg_compositor;
static int nfs_bg_feature(void){static int cached=-1;if(cached<0){const char*p=getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");cached=p&&p[0]=='1'&&!p[1]&&nfs_hd_art_provider_available();}return cached;}
static int nfs_logo_feature(void);
static int nfs_logo_trace_enabled(void);
static unsigned nfs_logo_trace_lines;
static void nfs_logo_native_upload(void);
static NfsLogoProvenance nfs_logo_provenance;
static void nfs_logo_cancel_all(void){if(nfs_bg_feature())nfs_bg_scene_cancel(&nfs_bg_compositor);}
/* Current native RGB-black fact is consumed by ANY native triangle. Pending pass proof is separate. */
static struct {uint64_t context,epoch;uint32_t fbo;unsigned valid;} nfs_logo_black_clear;
/* Bounded current native-upload cache. Eviction declines optional HD; no sampled-run authority. */
typedef struct {uint64_t context,revision;uint32_t address,id;unsigned valid;unsigned char bytes[2048];}NfsLogoUploadWitness;
static NfsLogoUploadWitness nfs_logo_uploads[1024];static unsigned nfs_logo_upload_next;
static void nfs_logo_native_state(void){if(!nfs_logo_feature())return;if(nfs_logo_provenance.active&&nfs_logo_provenance.triangles)nfs_logo_p_contaminate(&nfs_logo_provenance);else nfs_logo_p_cancel(&nfs_logo_provenance);nfs_logo_cancel_all();}

static int nfs_logo_feature(void){return nfs_bg_feature();}
static void nfs_logo_native_created(void){if(!nfs_logo_feature())return;nfs_logo_black_clear.valid=0;nfs_logo_p_created(&nfs_logo_provenance);if(nfs_bg_feature()&&nfs_logo_provenance.alive)nfs_bg_scene_context_created(&nfs_bg_compositor,nfs_logo_provenance.context);}
static void nfs_logo_native_retired(int owner){if(!nfs_logo_feature())return;nfs_logo_black_clear.valid=0;if(nfs_bg_feature())nfs_bg_scene_context_retired(&nfs_bg_compositor,nfs_logo_provenance.context,owner);nfs_logo_p_retired(&nfs_logo_provenance);}
static uint32_t nfs_logo_texture_address(const TextureInfo*ti){return(uint32_t)(ti-g_textures)*4u;}
static void nfs_logo_native_upload_begin(TextureInfo*ti){unsigned i;uint32_t address;if(!nfs_logo_feature())return;address=nfs_logo_texture_address(ti);for(i=0;i<1024;++i)if(nfs_logo_uploads[i].valid&&nfs_logo_uploads[i].address==address)nfs_logo_uploads[i].valid=0;nfs_logo_native_upload();}
static void nfs_logo_native_upload_done(TextureInfo*ti){
 NfsLogoUploadWitness*w;uint32_t address;if(!nfs_logo_feature())return;
 if(!nfs_logo_provenance.alive||nfs_logo_provenance.disabled||!g_glCtx||SDL_GL_GetCurrentContext()!=g_glCtx||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->size!=32||!ti->id||ti->hdDigits)return;
 address=nfs_logo_texture_address(ti);if(address>TextureMem-2048||ti->data!=g_textureMem+address||glGetError()!=GL_NO_ERROR)return;
 w=&nfs_logo_uploads[nfs_logo_upload_next++%1024];w->valid=0;w->context=nfs_logo_provenance.context;w->revision=nfs_logo_provenance.uploads;w->address=address;w->id=ti->id;memcpy(w->bytes,ti->data,2048);w->valid=1;
}
static int nfs_logo_native_upload_valid(uint32_t address,const TextureInfo*ti){unsigned i;for(i=0;i<1024;++i){const NfsLogoUploadWitness*w=&nfs_logo_uploads[i];if(w->valid&&w->address==address&&w->id==ti->id&&w->context==nfs_logo_provenance.context&&w->revision&&w->revision<=nfs_logo_provenance.uploads&&!memcmp(w->bytes,ti->data,2048))return 1;}return 0;}
static void nfs_logo_native_upload(void){if(!nfs_logo_feature())return;if(nfs_logo_provenance.active&&nfs_logo_provenance.triangles)nfs_logo_p_contaminate(&nfs_logo_provenance);nfs_logo_p_upload(&nfs_logo_provenance);nfs_logo_cancel_all();}
static void nfs_logo_native_other(void){if(!nfs_logo_feature())return;nfs_logo_black_clear.valid=0;nfs_logo_p_contaminate(&nfs_logo_provenance);nfs_logo_cancel_all();}
static int nfs_logo_fullbox(const GLint*b,unsigned w,unsigned h){return b[0]==0&&b[1]==0&&b[2]>0&&b[3]>0&&(uint64_t)(unsigned)b[2]>=w&&(uint64_t)(unsigned)b[3]>=h;}
static unsigned nfs_logo_target_failbits;static GLint nfs_logo_target_mag,nfs_logo_target_min,nfs_logo_observed_binding;static unsigned nfs_logo_old_texture_gate;
/* Native FBO authority deliberately does not depend on the texture selected before tile0.
   presentFrontSurface unbinds unit0; cached 427B30 selects tile0 only AFTER begin. */
static int nfs_logo_native_target_base(unsigned*w,unsigned*h,NfsFrameLegacySampling*sampling){
 GLint viewport[4],fbo=0,active=0,binding=0,mag=0,min=0,colorType=0,colorName=0,depthType=0,depthName=0,rbo=0,rw=0,rh=0;NfsFrameLegacySampling expected;unsigned width,height;
 if(!g_glCtx||SDL_GL_GetCurrentContext()!=g_glCtx||!nfs_logo_provenance.alive||nfs_logo_provenance.disabled||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer)return 0;
 height=(unsigned)g_framebufferHeight;width=height*4u/3u;if(width>4096||!nfs_frame_gpu_legacy_sampling(width,height,linearFiltering?GL_LINEAR:GL_NEAREST,&expected))return 0;
 glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
 if(memcmp(viewport,expected.viewport,sizeof viewport)||memcmp(g_matrix,expected.projection,sizeof g_matrix)||(GLuint)fbo!=g_framebuffer||glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&colorType);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&depthType);
 if(colorType!=GL_TEXTURE||depthType!=GL_RENDERBUFFER)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&colorName);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&depthName);
 if(!colorName||!depthName||(GLuint)colorName!=g_framebufferTexture||(GLuint)depthName!=g_framebufferDepth)return 0;
 glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo);glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)depthName);glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw);glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh);glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo);
 if(rw!=(GLint)width||rh!=(GLint)height||glGetError()!=GL_NO_ERROR)return 0;
 glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);if(binding){glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min);}glActiveTexture((GLenum)active);
 nfs_logo_observed_binding=binding;nfs_logo_old_texture_gate=binding&&mag==(GLint)expected.magFilter&&min==GL_LINEAR;nfs_logo_target_mag=mag;nfs_logo_target_min=min;
 if(w)*w=width;if(h)*h=height;if(sampling)*sampling=expected;return 1;
}
static int nfs_logo_target(NfsNativeLogoTargetProof*t,uint64_t serial){
 GLint fbo=0,view[4],box[4],func=0,program=0;GLboolean mask=0,color[4];unsigned w=0,h=0;
 nfs_logo_target_failbits=0;nfs_logo_target_mag=nfs_logo_target_min=0;
 if(!g_glCtx||SDL_GL_GetCurrentContext()!=g_glCtx){nfs_logo_target_failbits=1;return 0;}if(!nfs_logo_native_target_base(&w,&h,NULL)){nfs_logo_target_failbits=2;return 0;}
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_VIEWPORT,view);glGetIntegerv(GL_SCISSOR_BOX,box);glGetIntegerv(GL_DEPTH_FUNC,&func);glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);glGetBooleanv(GL_COLOR_WRITEMASK,color);
 if(fbo!=(GLint)g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER)nfs_logo_target_failbits|=4;
 if(program!=(GLint)g_shaderProgram)nfs_logo_target_failbits|=8;
 if(!glIsEnabled(GL_DEPTH_TEST)||!mask||!g_depthMask||func!=GL_LEQUAL)nfs_logo_target_failbits|=16;
 if(!nfs_logo_fullbox(view,w,h)||(glIsEnabled(GL_SCISSOR_TEST)&&!nfs_logo_fullbox(box,w,h)))nfs_logo_target_failbits|=32;
 if(!color[0]||!color[1]||!color[2]||!color[3])nfs_logo_target_failbits|=64;
 if(!g_textureEnabled||g_fogEnabled)nfs_logo_target_failbits|=128;
 if(nfs_logo_target_failbits)return 0;
 memset(t,0,sizeof *t);t->framebuffer=(GLuint)fbo;t->width=w;t->height=h;t->contextGeneration=nfs_logo_provenance.context;t->depthClearSerial=serial;t->depthClearProven=1;t->depthClearValue=1;
 memcpy(t->sampling.projection,g_matrix,sizeof g_matrix);memcpy(t->sampling.viewport,view,sizeof view);t->sampling.magFilter=linearFiltering?GL_LINEAR:GL_NEAREST;return 1;
}
static void nfs_logo_native_clear(uint16_t depth,float r,float g,float b){
 GLint fbo=0,view[4],box[4];GLboolean mask=0,rgbMask[4]={0};GLfloat actual[4]={0};unsigned w=0,h=0;int full=0,black=0;if(!nfs_logo_feature())return;nfs_logo_black_clear.valid=0;
 if(g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_native_target_base(&w,&h,NULL)){
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_VIEWPORT,view);glGetIntegerv(GL_SCISSOR_BOX,box);glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);glGetBooleanv(GL_COLOR_WRITEMASK,rgbMask);glGetFloatv(GL_COLOR_CLEAR_VALUE,actual);
 full=fbo==(GLint)g_framebuffer&&w&&h&&nfs_logo_fullbox(view,w,h)&&(!glIsEnabled(GL_SCISSOR_TEST)||nfs_logo_fullbox(box,w,h));
 black=full&&rgbMask[0]&&rgbMask[1]&&rgbMask[2]&&r==0&&g==0&&b==0&&actual[0]==0&&actual[1]==0&&actual[2]==0;}
 if(glGetError()!=GL_NO_ERROR)full=black=0;
 nfs_logo_p_clear(&nfs_logo_provenance,(uint32_t)fbo,full,mask&&g_depthMask,depth);nfs_logo_cancel_all();
 if(black&&nfs_logo_provenance.clearValid){nfs_logo_black_clear.valid=1;nfs_logo_black_clear.context=nfs_logo_provenance.context;nfs_logo_black_clear.epoch=nfs_logo_provenance.clearEvent;nfs_logo_black_clear.fbo=(uint32_t)fbo;}
}
static void nfs_logo_native_select(unsigned tmu,uint32_t address,uint32_t evenOdd,const GrTexInfo*info,TextureInfo*ti){if(!nfs_logo_feature())return;if(nfs_logo_provenance.active&&(!info||evenOdd!=3||info->format!=GR_TEXFMT_ARGB_1555||info->smallLod!=3||info->largeLod!=3||info->aspectRatio!=3)){nfs_logo_native_state();return;}nfs_logo_p_select(&nfs_logo_provenance,tmu,address,ti->id,ti->fmt,ti->size);}
static int nfs_logo_texture_filters(const uint32_t*ids,GLenum expected){
 GLint active=0,binding=0,mag=0,min=0;unsigned i;int ok=1;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
 for(i=0;i<300;++i){if(!glIsTexture(ids[i])){ok=0;break;}glBindTexture(GL_TEXTURE_2D,ids[i]);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min);if(mag!=(GLint)expected||min!=GL_LINEAR){ok=0;break;}}
 glBindTexture(GL_TEXTURE_2D,(GLuint)binding);glActiveTexture((GLenum)active);return ok;
}
static int nfs_logo_vertex(const GrVertex*v,unsigned tile,unsigned corner){
 float x=(float)((tile/15)*32+(corner==1||corner==2?32:0)),y=(float)((tile%15)*32+(corner>=2?32:0));
 return v&&v->x==x+VertexSnap&&v->y==y+VertexSnap&&v->r==255&&v->g==255&&v->b==255&&v->a==255&&v->oow==1&&v->tmuvtx[0].sow==(corner==1||corner==2?255:0)&&v->tmuvtx[0].tow==(corner>=2?255:0);
}
static void nfs_logo_native_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 NfsLogoProvenance*p=&nfs_logo_provenance;unsigned i,k;int exact;if(!nfs_logo_feature())return;nfs_logo_black_clear.valid=0;i=p->triangles/2;k=p->triangles&1;
 exact=p->active&&i<300&&g_textureEnabled&&!g_fogEnabled&&nfs_logo_vertex(a,i,0)&&nfs_logo_vertex(b,i,k?2:1)&&nfs_logo_vertex(c,i,k?3:2);
 nfs_logo_p_triangle(p,exact);if(!p->active)nfs_logo_cancel_all();
}
static int nfs_logo_trace_enabled(void){static int cached=-1;if(cached<0){const char*p=getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");cached=p&&p[0]=='1'&&p[1]==0;}return cached;}

/* Isolated whole-background adapter. Native target/upload/select/triangle authority
   is shared with the verified original-logo adapter, never carried by a CPU enum. */
static NfsNativeLogoTargetProof nfs_bg_pending_target;
static unsigned nfs_bg_trace_lines;
static uint64_t nfs_bg_content;
static uint32_t nfs_bg_map,nfs_bg_words;
static void nfs_bg_trace(const char*event,const char*reason,uint64_t serial,const NfsNativeLogoTargetProof*t){
 if(!nfs_logo_trace_enabled()||nfs_bg_trace_lines>=2048)return;++nfs_bg_trace_lines;
 fprintf(stderr,"BG NATIVE %s reason=%s serial=%llu content=%llu map=%u words=%u context=%llu uploads=%llu clear=%llu fbo=%u size=%ux%u selections=%u triangles=%u image=%llu cache=%u\n",event,reason,(unsigned long long)serial,(unsigned long long)nfs_bg_content,nfs_bg_map,nfs_bg_words,(unsigned long long)nfs_logo_provenance.context,(unsigned long long)nfs_logo_provenance.uploads,(unsigned long long)nfs_logo_provenance.clearEvent,t?t->framebuffer:0,t?t->width:0,t?t->height:0,nfs_logo_provenance.selects,nfs_logo_provenance.triangles,(unsigned long long)nfs_bg_compositor.lastImageRevision,nfs_bg_compositor.lastBeginKind);
}
static int nfs_bg_refuse(const char*why,uint64_t serial,const NfsNativeLogoTargetProof*t){nfs_bg_trace("begin-refused",why,serial,t);return 0;}
int nfs_private_background_scene_begin(const NfsBackgroundSceneFrameView*v){
 uint32_t ids[300];unsigned i,x,y;NfsNativeLogoTargetProof target;
 if(!nfs_bg_feature())return 0;
 nfs_logo_p_cancel(&nfs_logo_provenance);nfs_bg_scene_cancel(&nfs_bg_compositor);
 nfs_bg_content=v?v->contentRevision:0;nfs_bg_map=v?v->mapRole:0;nfs_bg_words=v?v->wordCount:0;
 if(!v||v->schema!=NFS_BACKGROUND_SCENE_RUNTIME_SCHEMA||v->width!=640||v->height!=480||v->planeWords!=307200||!v->original565||!v->serial||!v->contentRevision||v->mapRole>1||v->wordCount>4||v->tailKind!=NFS_BACKGROUND_TAIL_MAIN_SKIP)return nfs_bg_refuse("frame",v?v->serial:0,0);
 if(!nfs_logo_target(&target,v->serial))return nfs_bg_refuse("target",v->serial,0);
 for(i=0;i<300;++i){uint32_t a=v->rendererAddress[i];TextureInfo*ti;const uint8_t*bytes;if((a&3)||a>TextureMem-2048)return nfs_bg_refuse("key-range",v->serial,&target);ti=&g_textures[a>>2];if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+a)return nfs_bg_refuse("native-texture",v->serial,&target);if(!nfs_logo_native_upload_valid(a,ti))return nfs_bg_refuse("upload-witness",v->serial,&target);ids[i]=ti->id;bytes=(const uint8_t*)ti->data;
 for(y=0;y<32;++y)for(x=0;x<32;++x){uint16_t s=v->original565[((i%15)*32+y)*640+(i/15)*32+x],a1555=(uint16_t)(((s&0xf800)>>1)|((s&0x7c0)>>1)|(s&31)),want,got;a1555=a1555?(uint16_t)(a1555|0x8000):0;want=(uint16_t)((a1555<<1)|(a1555>>15));memcpy(&got,bytes+(y*32+x)*2,2);if(got!=want)return nfs_bg_refuse("converted-payload",v->serial,&target);}}
 {GLint active=0,binding=0,mag=0,min=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);glBindTexture(GL_TEXTURE_2D,ids[0]);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min);glBindTexture(GL_TEXTURE_2D,(GLuint)binding);glActiveTexture((GLenum)active);if((mag!=GL_LINEAR&&mag!=GL_NEAREST)||min!=GL_LINEAR)return nfs_bg_refuse("tile0-filter",v->serial,&target);target.sampling.magFilter=(GLenum)mag;}
 if(!nfs_logo_texture_filters(ids,target.sampling.magFilter))return nfs_bg_refuse("tile-filter",v->serial,&target);
 if(!nfs_logo_p_begin(&nfs_logo_provenance,v->serial,target.framebuffer,v->rendererAddress,ids))return nfs_bg_refuse("context-clear-key-serial",v->serial,&target);
 drawTriangles();
 if(!nfs_bg_scene_prepare(&nfs_bg_compositor)||!nfs_bg_scene_begin(&nfs_bg_compositor,v,&target)){
  nfs_logo_p_cancel(&nfs_logo_provenance);nfs_bg_scene_cancel(&nfs_bg_compositor);return nfs_bg_refuse("art-or-core",v->serial,&target);
 }
 nfs_bg_pending_target=target;nfs_bg_trace("begin-accepted","staging-only",v->serial,&target);return 1;
}
int nfs_private_background_scene_finish(uint64_t serial){
 NfsNativeLogoTargetProof target;int ok;if(!nfs_bg_feature())return 0;drawTriangles();
 if(!nfs_logo_target(&target,serial))goto fail;
 target.sampling.magFilter=nfs_bg_pending_target.sampling.magFilter;
 if(memcmp(&target,&nfs_bg_pending_target,sizeof target))goto fail;
 if(!nfs_logo_p_finish(&nfs_logo_provenance,serial,target.framebuffer))goto fail;
 ok=nfs_bg_scene_finish(&nfs_bg_compositor,serial,&target);nfs_bg_trace(ok?"finish-published":"finish-refused",ok?"core-commit":"core",serial,&target);return ok;
 fail:nfs_logo_p_cancel(&nfs_logo_provenance);nfs_bg_scene_cancel(&nfs_bg_compositor);nfs_bg_trace("finish-refused","target-or-original-pass",serial,0);return 0;
}
