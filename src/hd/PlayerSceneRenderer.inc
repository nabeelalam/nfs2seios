// SPDX-License-Identifier: MIT
/* Independent exact Player cached slot1 path. Original427B30 still executes all native
   selects/triangles; COLOR commit finishes before original native GPU text.
   Queued immutable owners retain raw frame borrows during synchronous begin.
   Only GPU staging and scalar metadata survive begin; finish reads no borrow. */
#include "PlayerSceneRuntimeAbi.h"
#ifndef NFS_PLAYER_SCENE_THREAD_TOKEN
#define NFS_PLAYER_SCENE_THREAD_TOKEN() ((uintptr_t)SDL_GetCurrentThreadID())
#endif
static struct {
 NfsNativeLogoTargetProof target;NfsSoftwareTextGpu caption;NfsPlayerCaptionGpu captionWord;
 GLuint preview;unsigned previewWidth,previewHeight,previewReady;unsigned char previewSha[32],coverageSha[32];
 uintptr_t context,thread;uint64_t generation,serial,content;
 unsigned map,words,nativeArmed,traceLines,activeOwnerKind;
 GLuint color,fbo;unsigned width,height;
} nfs_player_renderer;
/* Independent Player resource lifetime. Source-neutral background mechanics
   are reused; public form/tail admission remains exact and separate. */
enum {NFS_PLAYER_OWNER_NONE=0,NFS_PLAYER_OWNER_ONE=5,NFS_PLAYER_OWNER_TWO=6};
static int nfs_player_context(uint64_t generation,uintptr_t context){return generation&&nfs_player_renderer.generation==generation&&nfs_player_renderer.context==context&&nfs_player_renderer.thread==NFS_PLAYER_SCENE_THREAD_TOKEN()&&g_glCtx&&(uintptr_t)g_glCtx==context&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==generation;}
static int nfs_player_current(void){return nfs_player_context(nfs_player_renderer.generation,nfs_player_renderer.context);}
#define NFS_PLAYER_RENDER_GL(call) do{if(!nfs_player_current())goto lost;call;if(!nfs_player_current())goto lost;}while(0)
static void nfs_player_trace(const char*event,const char*reason){if(!nfs_logo_trace_enabled()||nfs_player_renderer.traceLines>=2048)return;++nfs_player_renderer.traceLines;fprintf(stderr,"%s NATIVE %s reason=%s serial=%llu content=%llu map=%u words=%u context=%llu size=%ux%u fbo=%u selections=%u triangles=%u\n",nfs_player_renderer.activeOwnerKind==NFS_PLAYER_OWNER_TWO?"PLAYER2":"PLAYER1",event,reason,(unsigned long long)nfs_player_renderer.serial,(unsigned long long)nfs_player_renderer.content,nfs_player_renderer.map,nfs_player_renderer.words,(unsigned long long)nfs_player_renderer.generation,nfs_player_renderer.target.width,nfs_player_renderer.target.height,nfs_player_renderer.target.framebuffer,nfs_logo_provenance.selects,nfs_logo_provenance.triangles);}
static void nfs_player_cancel(const char*reason){if(nfs_player_renderer.serial)nfs_player_trace("canceled",reason);if(nfs_player_renderer.nativeArmed&&nfs_logo_provenance.active&&nfs_logo_provenance.pendingSerial==nfs_player_renderer.serial&&nfs_logo_provenance.pendingContext==nfs_player_renderer.generation&&nfs_logo_provenance.pendingFbo==nfs_player_renderer.target.framebuffer)nfs_logo_p_cancel(&nfs_logo_provenance);nfs_player_renderer.nativeArmed=0;nfs_player_renderer.serial=0;nfs_player_renderer.activeOwnerKind=NFS_PLAYER_OWNER_NONE;memset(&nfs_player_renderer.target,0,sizeof nfs_player_renderer.target);}
static void nfs_player_forget_gpu(void){nfs_player_caption_forget(&nfs_player_renderer.captionWord);nfs_soft_gpu_context_retired(&nfs_player_renderer.caption,nfs_player_renderer.caption.generation,0);nfs_player_renderer.preview=nfs_player_renderer.previewReady=0;nfs_player_renderer.previewWidth=nfs_player_renderer.previewHeight=0;nfs_player_renderer.color=nfs_player_renderer.fbo=0;nfs_player_renderer.width=nfs_player_renderer.height=0;}
static void nfs_player_abandon(void){nfs_player_forget_gpu();nfs_player_cancel("owner-lost");}
static int nfs_player_context_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_player_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_player_context_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_player_renderer.generation;t.contextIdentity=nfs_player_renderer.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_player_context_owner;return t;}
static int nfs_player_read_target(NfsNativeLogoTargetProof*out,uint64_t serial){
 GLint fbo=0,vp[4]={0},sc[4]={0},func=0,program=0,ct=0,cn=0,dt=0,dn=0,rbo=0,rw=0,rh=0;
 GLboolean enabled=0,depthMask=0,color[4]={0};GLenum status=0;unsigned w,h;NfsFrameLegacySampling sampling;
 if(!out||!serial||!nfs_player_current()||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER||!g_textureEnabled||g_fogEnabled)return 0;
 h=(unsigned)g_framebufferHeight;w=h*4u/3u;if(w>4096||!nfs_frame_gpu_legacy_sampling(w,h,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||memcmp(g_matrix,sampling.projection,sizeof g_matrix))return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_VIEWPORT,vp));NFS_PLAYER_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 if((GLuint)fbo!=g_framebuffer||memcmp(vp,sampling.viewport,sizeof vp)||status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||dt!=GL_RENDERBUFFER||!cn||!dn||(GLuint)cn!=g_framebufferTexture||(GLuint)dn!=g_framebufferDepth)return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo));NFS_PLAYER_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)dn));NFS_PLAYER_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_PLAYER_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_PLAYER_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo));if(rw!=(GLint)w||rh!=(GLint)h)return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_DEPTH_FUNC,&func));NFS_PLAYER_RENDER_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_PLAYER_RENDER_GL(glGetBooleanv(GL_COLOR_WRITEMASK,color));NFS_PLAYER_RENDER_GL(enabled=glIsEnabled(GL_DEPTH_TEST));
 if(program!=(GLint)g_shaderProgram||!enabled||!depthMask||!g_depthMask||func!=GL_LEQUAL||!color[0]||!color[1]||!color[2]||!color[3]||!nfs_logo_fullbox(vp,w,h))return 0;
 NFS_PLAYER_RENDER_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(!nfs_logo_fullbox(sc,w,h))return 0;}
 NFS_PLAYER_RENDER_GL(status=glGetError());if(status!=GL_NO_ERROR)return 0;
 memset(out,0,sizeof *out);out->framebuffer=(GLuint)fbo;out->width=w;out->height=h;out->contextGeneration=nfs_player_renderer.generation;/* Color-only commit requires no claimed depth-clear witness. */out->sampling=sampling;return 1;
 lost:nfs_player_abandon();return 0;
}
static void nfs_player_renderer_created(void){uint64_t generation=nfs_logo_provenance.context;uintptr_t context=(uintptr_t)g_glCtx;if(!nfs_bg_feature()||!generation||!context||SDL_GL_GetCurrentContext()!=g_glCtx)return;nfs_player_renderer.generation=generation;nfs_player_renderer.context=context;nfs_player_renderer.thread=NFS_PLAYER_SCENE_THREAD_TOKEN();nfs_player_cancel("context-created");(void)nfs_soft_gpu_context_created(&nfs_player_renderer.caption,generation);}
static void nfs_player_renderer_retired(int owner){
 if(owner&&nfs_player_current()){
  {NfsBackgroundSceneStage guard=nfs_player_context_stage();if(!nfs_player_caption_retire(&nfs_player_renderer.captionWord,&guard,1))goto lost;}
  if(nfs_player_renderer.caption.generation==nfs_player_renderer.generation&&nfs_player_renderer.caption.ownerGeneration==nfs_player_renderer.generation){
   if(nfs_player_renderer.caption.program){NFS_PLAYER_RENDER_GL(glDeleteProgram(nfs_player_renderer.caption.program));}
   if(nfs_player_renderer.caption.texture){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&nfs_player_renderer.caption.texture));}
  }
  if(nfs_player_renderer.preview){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&nfs_player_renderer.preview));}
  if(nfs_player_renderer.fbo){NFS_PLAYER_RENDER_GL(glDeleteFramebuffers(1,&nfs_player_renderer.fbo));}
  if(nfs_player_renderer.color){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&nfs_player_renderer.color));}
 }
 lost:nfs_player_forget_gpu();nfs_player_cancel("context-retired");nfs_player_renderer.generation=0;nfs_player_renderer.context=nfs_player_renderer.thread=0;
}
static int nfs_player_font_prepare(NfsSoftwareTextGpu*s,const NfsSoftwareTextFontView*f){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint status=0,sampler=-1,matrix=-1,maxSize=0;GLboolean exists=0;GLenum error;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_context_stage();int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";
 const char*vertexSource=vertex;const char*fragmentSource=fragment;
 if(!s||!nfs_player_current()||s->generation!=nfs_player_renderer.generation||!f||f->schema!=1||f->width!=1024||f->height!=512||f->bytes!=1024u*512u||!f->alpha||!f->identity)return 0;
 if(s->ownerGeneration){if(s->ownerGeneration!=s->generation||!s->program||!s->texture||s->matrix<0||s->sampler<0||s->fontIdentity!=f->identity||memcmp(s->fontSha256,f->sha256,32))return 0;NFS_PLAYER_RENDER_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_PLAYER_RENDER_GL(exists=glIsTexture(s->texture));return exists!=0;}
 if(s->program||s->texture||!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32))return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));if(maxSize<1024)return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_RENDER_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_PLAYER_RENDER_GL(glShaderSource(vs,1,&vertexSource,NULL));NFS_PLAYER_RENDER_GL(glCompileShader(vs));NFS_PLAYER_RENDER_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_RENDER_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_PLAYER_RENDER_GL(glShaderSource(fs,1,&fragmentSource,NULL));NFS_PLAYER_RENDER_GL(glCompileShader(fs));NFS_PLAYER_RENDER_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_RENDER_GL(program=glCreateProgram());if(!program)goto done;NFS_PLAYER_RENDER_GL(glAttachShader(program,vs));NFS_PLAYER_RENDER_GL(glAttachShader(program,fs));NFS_PLAYER_RENDER_GL(glBindAttribLocation(program,0,"vertex"));NFS_PLAYER_RENDER_GL(glBindAttribLocation(program,1,"color"));NFS_PLAYER_RENDER_GL(glLinkProgram(program));NFS_PLAYER_RENDER_GL(glGetProgramiv(program,GL_LINK_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_RENDER_GL(sampler=glGetUniformLocation(program,"image"));NFS_PLAYER_RENDER_GL(matrix=glGetUniformLocation(program,"matrix"));if(sampler<0||matrix<0)goto done;
 NFS_PLAYER_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_PLAYER_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_PLAYER_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,1024,512,0,GL_ALPHA,GL_UNSIGNED_BYTE,f->alpha));NFS_PLAYER_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_PLAYER_RENDER_GL(glDeleteShader(vs));}if(fs){NFS_PLAYER_RENDER_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_PLAYER_RENDER_GL(glDeleteProgram(program));}if(texture){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;if(!nfs_player_current())goto lost;s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->fontIdentity=f->identity;s->ownerGeneration=s->generation;memcpy(s->fontSha256,sha,32);return 1;
 lost:nfs_player_abandon();return 0;
}
static int nfs_player_color_target(unsigned w,unsigned h){
 GLuint texture=0,fbo=0;GLint maxSize=0,maxViewport[2]={0,0},type=0,name=0;GLenum status=0,error;GLboolean exists=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_context_stage();int ok=0;
 if(nfs_player_renderer.color==g_framebufferTexture||nfs_player_renderer.fbo==g_framebuffer)return 0;
 if(!nfs_player_current()||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));if(w>(unsigned)maxSize||h>(unsigned)maxSize||w>(unsigned)maxViewport[0]||h>(unsigned)maxViewport[1])return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 if(nfs_player_renderer.color&&nfs_player_renderer.fbo&&nfs_player_renderer.width==w&&nfs_player_renderer.height==h){
  NFS_PLAYER_RENDER_GL(exists=glIsTexture(nfs_player_renderer.color));if(!exists)goto done;NFS_PLAYER_RENDER_GL(exists=glIsFramebuffer(nfs_player_renderer.fbo));if(!exists)goto done;
  NFS_PLAYER_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_renderer.fbo));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_PLAYER_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_PLAYER_RENDER_GL(error=glGetError());ok=type==GL_TEXTURE&&(GLuint)name==nfs_player_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
  if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 }
 NFS_PLAYER_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_PLAYER_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));NFS_PLAYER_RENDER_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 NFS_PLAYER_RENDER_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;NFS_PLAYER_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_PLAYER_RENDER_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0));NFS_PLAYER_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_PLAYER_RENDER_GL(error=glGetError());ok=status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_PLAYER_RENDER_GL(glDeleteFramebuffers(1,&fbo));}if(texture){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;
 /* Logical preview pixels are independent of framebuffer dimensions. Keep
    their live texture and verified cache when replacing only the color stage. */
 if(nfs_player_renderer.fbo){NFS_PLAYER_RENDER_GL(glDeleteFramebuffers(1,&nfs_player_renderer.fbo));}if(nfs_player_renderer.color){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&nfs_player_renderer.color));}
 nfs_player_renderer.fbo=fbo;nfs_player_renderer.color=texture;nfs_player_renderer.width=w;nfs_player_renderer.height=h;return 1;
 lost:nfs_player_abandon();return 0;
}
static int nfs_player_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;(void)user;
 if(!t||nfs_player_renderer.color==g_framebufferTexture||nfs_player_renderer.fbo==g_framebuffer||!nfs_player_context(t->generation,t->contextIdentity)||t->framebuffer!=nfs_player_renderer.fbo||t->width!=nfs_player_renderer.width||t->height!=nfs_player_renderer.height)return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=t->framebuffer)return 0;NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==nfs_player_renderer.color;
 lost:return 0;
}
static int nfs_player_resource_owner(void*user,const NfsBackgroundResourceOwner*o){
 GLint fbo=0,type=0,name=0;(void)user;if(!o||!nfs_player_context(o->generation,o->contextIdentity)||!nfs_player_renderer.serial||nfs_player_renderer.target.framebuffer!=g_framebuffer)return 0;
 NFS_PLAYER_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==g_framebufferTexture;
 lost:return 0;
}
static int nfs_player_commit_current(void){NfsBackgroundResourceOwner owner={nfs_player_renderer.generation,nfs_player_renderer.context,nfs_player_resource_owner,NULL};return nfs_player_renderer.color&&nfs_player_renderer.color!=g_framebufferTexture&&nfs_player_renderer.fbo!=g_framebuffer&&nfs_player_resource_owner(NULL,&owner);}
static int nfs_player_copy_color(const NfsNativeLogoTargetProof*target){
 GLenum error;static const GLfloat copy[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};
#define NFS_PLAYER_COMMIT_GL(call) do{if(!nfs_player_commit_current())return 0;call;if(!nfs_player_commit_current())return 0;}while(0)
 for(unsigned i=0;i<8;++i){NFS_PLAYER_COMMIT_GL(glDisable(nfs_frame_caps[i]));}NFS_PLAYER_COMMIT_GL(glColorMask(1,1,1,1));NFS_PLAYER_COMMIT_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_COMMIT_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_PLAYER_COMMIT_GL(glDepthMask(GL_FALSE));NFS_PLAYER_COMMIT_GL(glViewport(0,0,target->width,target->height));NFS_PLAYER_COMMIT_GL(glUseProgram(nfs_bg_compositor.copyProgram));NFS_PLAYER_COMMIT_GL(glUniform1i(nfs_bg_compositor.copySampler,0));NFS_PLAYER_COMMIT_GL(glBindTexture(GL_TEXTURE_2D,nfs_player_renderer.color));NFS_PLAYER_COMMIT_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_PLAYER_COMMIT_GL(glDisableVertexAttribArray(i));}NFS_PLAYER_COMMIT_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy));NFS_PLAYER_COMMIT_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_PLAYER_COMMIT_GL(error=glGetError());return error==GL_NO_ERROR;
#undef NFS_PLAYER_COMMIT_GL
}
/* Independent source-authenticated cached upload admission. Original slot1 is
   ARGB1555 format11; it is not the per-frame RGB565 scratch-upload protocol. */
static int nfs_player_cached_tiles(const NfsBackgroundSceneFrameView*v,uint32_t ids[300],GLenum*filter){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_context_stage();GLint mag=0,min=0;GLboolean exists=0;GLenum error;int ok=0;
 if(!v||!filter||!nfs_player_current())return 0;
 for(unsigned tile=0;tile<300;++tile){uint32_t address=v->rendererAddress[tile];TextureInfo*ti;const unsigned char*bytes;
  if((address&3)||address>TextureMem-2048)return 0;ti=&g_textures[address>>2];
  if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+address||!nfs_logo_native_upload_valid(address,ti))return 0;
  ids[tile]=ti->id;bytes=(const unsigned char*)ti->data;
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t source=v->original565[((tile%15)*32+y)*640+(tile/15)*32+x];uint16_t a1555=(uint16_t)(((source&0xf800)>>1)|((source&0x7c0)>>1)|(source&31));uint16_t want,got;a1555=a1555?(uint16_t)(a1555|0x8000):0;want=(uint16_t)((a1555<<1)|(a1555>>15));memcpy(&got,bytes+(y*32+x)*2,2);if(got!=want)return 0;}
 }
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_RENDER_GL(glActiveTexture(GL_TEXTURE0));
 for(unsigned tile=0;tile<300;++tile){NFS_PLAYER_RENDER_GL(exists=glIsTexture(ids[tile]));if(!exists)goto done;NFS_PLAYER_RENDER_GL(glBindTexture(GL_TEXTURE_2D,ids[tile]));NFS_PLAYER_RENDER_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag));NFS_PLAYER_RENDER_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min));if(tile==0){if(mag!=GL_LINEAR&&mag!=GL_NEAREST)goto done;*filter=(GLenum)mag;}if(mag!=(GLint)*filter||min!=GL_LINEAR)goto done;}
 NFS_PLAYER_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 lost:nfs_player_abandon();return 0;
}

/* Preview texture contains already premultiplied binary coverage. It is drawn
   BELOW authored sidebar/words/logos/caption. No original-pixel mask substitutes
   for modern authored alpha; the producer owns full verified union coverage. */
static int nfs_player_preview_prepare(const NfsOriginalPreviewLayerView*v){
 unsigned char sha[32],maskSha[32],*rgba=NULL;const uint16_t little=1;GLuint texture=0;GLboolean exists=0;GLenum error;int ok=0;unsigned tw=0,th=0;size_t count=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_context_stage();
 if(!nfs_player_current()||!nfs_bg_preview_view(v)||*(const unsigned char*)&little!=1||!nfs_texture_sha256((const unsigned char*)v->pixels565,v->pixelCount*2,sha)||memcmp(sha,v->sha256,32)||!nfs_texture_sha256(v->coverage,v->coverageCount,maskSha)||memcmp(maskSha,v->coverageSha256,32))return 0;
 for(size_t i=0;i<v->coverageCount;++i)if(v->coverage[i]&&v->coverage[i]!=255)return 0;
 tw=v->width;th=v->height;count=v->pixelCount;
 if(v->schema==NFS_AUTHORED_PREVIEW_LAYER_SCHEMA){if(!v->authoredRGBA||v->authoredWidth!=v->width*4||v->authoredHeight!=v->height*4||v->authoredBytes!=(size_t)v->authoredWidth*v->authoredHeight*4||!nfs_texture_sha256(v->authoredRGBA,v->authoredBytes,sha)||memcmp(sha,v->authoredSha256,32))return 0;for(size_t k=0;k<v->authoredBytes;k+=4)if(v->authoredRGBA[k]>v->authoredRGBA[k+3]||v->authoredRGBA[k+1]>v->authoredRGBA[k+3]||v->authoredRGBA[k+2]>v->authoredRGBA[k+3])return 0;tw=v->authoredWidth;th=v->authoredHeight;count=(size_t)tw*th;}
 texture=nfs_player_renderer.preview;if(texture){if(texture==g_framebufferTexture||texture==nfs_player_renderer.color||texture==nfs_player_renderer.caption.texture||texture==nfs_player_renderer.captionWord.texture||texture==nfs_bg_compositor.logo.texture)return 0;for(unsigned i=0;i<5;++i)if(texture==nfs_bg_compositor.textures[i])return 0;NFS_PLAYER_RENDER_GL(exists=glIsTexture(texture));if(!exists)return 0;}
 if(texture&&nfs_player_renderer.previewReady&&nfs_player_renderer.previewWidth==tw&&nfs_player_renderer.previewHeight==th&&!memcmp(sha,nfs_player_renderer.previewSha,32)&&!memcmp(maskSha,nfs_player_renderer.coverageSha,32))return 1;
 rgba=(unsigned char*)malloc(count*4);if(!rgba)return 0;if(v->schema==NFS_AUTHORED_PREVIEW_LAYER_SCHEMA)memcpy(rgba,v->authoredRGBA,v->authoredBytes);else for(size_t i=0;i<v->pixelCount;++i){unsigned p=v->pixels565[i],r=(p>>11)&31,g=(p>>5)&63,b=p&31;unsigned char*out=rgba+4*i;out[3]=v->coverage[i];out[0]=out[3]?(unsigned char)((r<<3)|(r>>2)):0;out[1]=out[3]?(unsigned char)((g<<2)|(g>>4)):0;out[2]=out[3]?(unsigned char)((b<<3)|(b>>2)):0;}
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;nfs_player_renderer.previewReady=0;
 if(!texture){NFS_PLAYER_RENDER_GL(glGenTextures(1,&texture));}if(!texture)goto done;
 NFS_PLAYER_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_RENDER_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_PLAYER_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba));NFS_PLAYER_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(!ok&&texture!=nfs_player_renderer.preview&&texture){NFS_PLAYER_RENDER_GL(glDeleteTextures(1,&texture));}if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;free(rgba);if(!ok)return 0;nfs_player_renderer.preview=texture;nfs_player_renderer.previewWidth=tw;nfs_player_renderer.previewHeight=th;memcpy(nfs_player_renderer.previewSha,sha,32);memcpy(nfs_player_renderer.coverageSha,maskSha,32);nfs_player_renderer.previewReady=1;return 1;
 lost:free(rgba);nfs_player_abandon();return 0;
}
#define NFS_PLAYER_LAYER_GL(call) do{if(!nfs_graphics_scene_current(t))return 0;call;if(!nfs_graphics_scene_current(t))return 0;}while(0)
static int nfs_player_preview_draw(const NfsBackgroundSceneStage*t,const NfsOriginalPreviewLayerView*v){
 NfsBackgroundSceneCompositor*b=&nfs_bg_compositor;GLenum error;const GLfloat l=(GLfloat)v->left,top=(GLfloat)v->top,r=l+v->width,bottom=top+v->height;const GLfloat vertices[]={l,top,0,0,r,top,1,0,l,bottom,0,1,l,bottom,0,1,r,top,1,0,r,bottom,1,1};
 if(!nfs_player_renderer.previewReady||!nfs_player_renderer.preview)return 0;
 NFS_PLAYER_LAYER_GL(glUseProgram(b->program));NFS_PLAYER_LAYER_GL(glUniformMatrix4fv(b->matrix,1,GL_FALSE,t->sampling.projection));NFS_PLAYER_LAYER_GL(glUniform1i(b->sampler,0));NFS_PLAYER_LAYER_GL(glUniform1f(b->opacity,1));NFS_PLAYER_LAYER_GL(glBindTexture(GL_TEXTURE_2D,nfs_player_renderer.preview));NFS_PLAYER_LAYER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,t->sampling.magFilter));NFS_PLAYER_LAYER_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_PLAYER_LAYER_GL(glDisableVertexAttribArray(i));}NFS_PLAYER_LAYER_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,vertices));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_PLAYER_LAYER_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_PLAYER_LAYER_GL(error=glGetError());return error==GL_NO_ERROR;
}
static int nfs_player_draw_layers(NfsGraphicsSceneGpu*gpu,const NfsBackgroundSceneStage*t,const NfsBackgroundSceneLayers*v,const NfsSoftwareTextLayerView*caption,const NfsOriginalPreviewLayerView*preview,const NfsPlayerCaptionWordView*word){
 NfsBackgroundSceneCompositor*b=gpu->background;NfsLogoVertex logos[18];size_t count=0;float bounds[4];GLint type=0,name=0;GLenum error;static const int32_t full[4]={0,0,640,480};
 if(!nfs_bg_stage_layers(v)||!nfs_graphics_scene_font(gpu->sidebarCaption,caption,343,t->generation)||!nfs_graphics_scene_ready(gpu,t)||!nfs_graphics_scene_target(gpu,t)||!nfs_logo_layers(v->logos,3,logos,&count,bounds))return 0;
 NFS_PLAYER_LAYER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_LAYER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||(GLuint)name==nfs_player_renderer.preview)return 0;
 for(unsigned i=0;i<8;++i){NFS_PLAYER_LAYER_GL(glDisable(nfs_frame_caps[i]));}NFS_PLAYER_LAYER_GL(glColorMask(1,1,1,1));NFS_PLAYER_LAYER_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_LAYER_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_PLAYER_LAYER_GL(glDepthMask(GL_FALSE));
 if(!nfs_graphics_scene_quad(b,t,v->mapRole,0,0,640,480,0,0,b->art[v->mapRole].width,b->art[v->mapRole].height,full,256,0)||!nfs_player_preview_draw(t,preview)||!nfs_graphics_scene_quad(b,t,2,0,0,343,480,0,0,1372,1920,v->panelClip,256,1))return 0;
 for(unsigned i=0;i<v->wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->words+i;const NfsBackgroundSceneWord*r=b->regions+w->sourceRole-1;if(!nfs_graphics_scene_quad(b,t,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))return 0;}
 if(count){NFS_PLAYER_LAYER_GL(glUseProgram(b->logo.program));NFS_PLAYER_LAYER_GL(glUniformMatrix4fv(b->logo.matrix,1,GL_FALSE,t->sampling.projection));NFS_PLAYER_LAYER_GL(glUniform1i(b->logo.sampler,0));NFS_PLAYER_LAYER_GL(glBindTexture(GL_TEXTURE_2D,b->logo.texture));NFS_PLAYER_LAYER_GL(glEnableVertexAttribArray(0));NFS_PLAYER_LAYER_GL(glEnableVertexAttribArray(1));NFS_PLAYER_LAYER_GL(glDisableVertexAttribArray(2));NFS_PLAYER_LAYER_GL(glDisableVertexAttribArray(3));NFS_PLAYER_LAYER_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_PLAYER_LAYER_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_PLAYER_LAYER_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_PLAYER_LAYER_GL(error=glGetError());if(error!=GL_NO_ERROR)return 0;}
 return nfs_player_caption_empty(word)?nfs_graphics_scene_text(gpu->sidebarCaption,t,caption):nfs_player_caption_draw(&nfs_player_renderer.captionWord,word,t);
}
#undef NFS_PLAYER_LAYER_GL
static int nfs_player_begin_refused(const char*reason,uint64_t serial){if(nfs_logo_trace_enabled()&&nfs_player_renderer.traceLines<2048){++nfs_player_renderer.traceLines;fprintf(stderr,"PLAYER1 NATIVE begin-refused reason=%s serial=%llu context=%llu\n",reason,(unsigned long long)serial,(unsigned long long)nfs_player_renderer.generation);}return 0;}
static int nfs_player_cached_begin_refused(unsigned ownerKind,const char*reason,uint64_t serial){
 if(ownerKind!=NFS_PLAYER_OWNER_TWO)return nfs_player_begin_refused(reason,serial);
 if(nfs_logo_trace_enabled()&&nfs_player_renderer.traceLines<2048){++nfs_player_renderer.traceLines;fprintf(stderr,"PLAYER2 NATIVE begin-refused reason=%s serial=%llu context=%llu\n",reason,(unsigned long long)serial,(unsigned long long)nfs_player_renderer.generation);}return 0;
}
static int nfs_player_cached_begin(const NfsBackgroundSceneFrameView*scene,const NfsSoftwareTextLayerView*caption,const NfsOriginalPreviewLayerView*preview,const NfsPlayerCaptionWordView*word,unsigned ownerKind){
 NfsNativeLogoTargetProof target;NfsBackgroundSceneLayers layers;NfsGraphicsSceneGpu gpu;NfsBackgroundSceneStage guard,stage;NfsBgSaved saved;uint32_t ids[300];GLenum filter=GL_LINEAR;int ok=0;
 if(!nfs_player_caption_valid(word,ownerKind==NFS_PLAYER_OWNER_ONE?1u:2u)||(ownerKind!=NFS_PLAYER_OWNER_ONE&&ownerKind!=NFS_PLAYER_OWNER_TWO)||!scene||!caption||!nfs_bg_preview_view(preview)||scene->tailKind!=ownerKind||!nfs_bg_feature()||!nfs_player_current()||!nfs_bg_view_common(scene)||caption->logicalWidth!=343||caption->logicalHeight!=480)return nfs_player_cached_begin_refused(ownerKind,"view-owner",scene?scene->serial:0);
 if(nfs_player_renderer.serial||nfs_player_renderer.activeOwnerKind!=NFS_PLAYER_OWNER_NONE){nfs_player_cancel("nested-begin");return 0;}
 /* Other cached-scene pending state cannot lend authority to this admission. */
 nfs_logo_p_cancel(&nfs_logo_provenance);nfs_bg_scene_cancel(&nfs_bg_compositor);drawTriangles();
 if(!nfs_player_read_target(&target,scene->serial))return nfs_player_cached_begin_refused(ownerKind,"native-target",scene->serial);
 if(!nfs_player_cached_tiles(scene,ids,&filter))return nfs_player_cached_begin_refused(ownerKind,"cached-upload-key-payload-filter",scene->serial);
 if(nfs_player_caption_alias(nfs_player_renderer.captionWord.texture,nfs_player_renderer.caption.texture,nfs_player_renderer.preview,nfs_player_renderer.color,ids))return nfs_player_cached_begin_refused(ownerKind,"word-native-alias",scene->serial);
 if(nfs_player_renderer.preview)for(unsigned i=0;i<300;++i)if(nfs_player_renderer.preview==ids[i])return nfs_player_cached_begin_refused(ownerKind,"preview-native-alias",scene->serial);
 target.sampling.magFilter=filter;nfs_player_renderer.activeOwnerKind=ownerKind;nfs_player_renderer.serial=scene->serial;nfs_player_renderer.content=scene->contentRevision;nfs_player_renderer.map=scene->mapRole;nfs_player_renderer.words=scene->wordCount;nfs_player_renderer.target=target;
 {NfsBackgroundResourceOwner owner={nfs_player_renderer.generation,nfs_player_renderer.context,nfs_player_resource_owner,NULL};if(!nfs_bg_scene_prepare_owned(&nfs_bg_compositor,&owner)){if(!nfs_player_current())goto lost;nfs_player_cancel("background-prepare");return 0;}}
 guard=nfs_player_context_stage();
 if(!nfs_player_font_prepare(&nfs_player_renderer.caption,&caption->font)||!nfs_player_caption_prepare(&nfs_player_renderer.captionWord,word,ownerKind==NFS_PLAYER_OWNER_ONE?1u:2u,&guard)||!nfs_player_color_target(target.width,target.height)||!nfs_player_preview_prepare(preview)){if(!nfs_player_current())goto lost;nfs_player_cancel("caption-stage-prepare");return 0;}
 if(!nfs_player_read_target(&target,scene->serial)){nfs_player_cancel("prepare-target");return 0;}target.sampling.magFilter=filter;if(memcmp(&target,&nfs_player_renderer.target,sizeof target)){nfs_player_cancel("prepare-target-changed");return 0;}
 memset(&layers,0,sizeof layers);layers.mapRole=scene->mapRole;layers.wordCount=scene->wordCount;memcpy(layers.panelClip,scene->panelClip,sizeof layers.panelClip);memcpy(layers.words,scene->words,sizeof layers.words);memcpy(layers.logos,scene->logos,sizeof layers.logos);
 guard=nfs_player_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_renderer.fbo));NFS_PLAYER_RENDER_GL(glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]));
 memset(&stage,0,sizeof stage);stage.generation=nfs_player_renderer.generation;stage.contextIdentity=nfs_player_renderer.context;stage.framebuffer=nfs_player_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_player_stage_owner;gpu.background=&nfs_bg_compositor;gpu.sidebarCaption=gpu.text=&nfs_player_renderer.caption;
 ok=nfs_player_draw_layers(&gpu,&stage,&layers,caption,preview,word);if(!nfs_player_current())goto lost;if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok){nfs_player_cancel("ordered-stage");return 0;}
 if(!nfs_player_read_target(&target,scene->serial)){nfs_player_cancel("staged-target");return 0;}target.sampling.magFilter=filter;if(memcmp(&target,&nfs_player_renderer.target,sizeof target)){nfs_player_cancel("staged-target-changed");return 0;}
 /* Actual observed native clear/context/key/serial proof arms only after all
    optional staging has restored the original state. No clear is invented. */
 if(!nfs_logo_p_begin(&nfs_logo_provenance,scene->serial,target.framebuffer,scene->rendererAddress,ids)){nfs_player_cancel("context-clear-key-serial");return 0;}
 nfs_player_renderer.nativeArmed=1;nfs_player_trace("begin-accepted","staging-only-cached-original-pass");return 1;
 lost:nfs_player_abandon();return 0;
}
static int nfs_player_cached_finish(uint64_t serial,unsigned expectedOwnerKind){
 NfsNativeLogoTargetProof target;NfsBgSaved saved;NfsBackgroundSceneStage guard;int ok;
 /* A different public owner may neither commit nor steal this pending image. */
 if(nfs_player_renderer.activeOwnerKind!=expectedOwnerKind)return 0;
 if(!serial||serial!=nfs_player_renderer.serial||!nfs_player_renderer.nativeArmed){nfs_player_cancel("finish-serial");return 0;}if(!nfs_player_current())goto lost;
 /* All actual original background triangles execute before COLOR commit. The
    caller queues this finish before native Options text/status/cursor. */
 drawTriangles();if(!nfs_player_read_target(&target,serial)){nfs_player_cancel("finish-target");return 0;}target.sampling.magFilter=nfs_player_renderer.target.sampling.magFilter;
 if(memcmp(&target,&nfs_player_renderer.target,sizeof target)||!nfs_logo_p_finish(&nfs_logo_provenance,serial,target.framebuffer)){nfs_player_cancel("target-or-original-pass");return 0;}nfs_player_renderer.nativeArmed=0;
 guard=nfs_player_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 ok=nfs_player_copy_color(&target);if(!nfs_player_current())goto lost;if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;
 nfs_player_trace(ok?"finish-published":"finish-refused",ok?"color-only-commit-before-native-text":"copy-error");nfs_player_renderer.serial=0;nfs_player_renderer.activeOwnerKind=NFS_PLAYER_OWNER_NONE;memset(&nfs_player_renderer.target,0,sizeof nfs_player_renderer.target);return ok;
 lost:nfs_player_abandon();return 0;
}

/* Exact typed Player public boundary. No Location/Options ABI casts or enums. */
int nfs_private_player_scene_begin(const NfsPlayerSceneFrameView*v){
 unsigned kind;if(!v||v->schema!=NFS_PLAYER_SCENE_RUNTIME_SCHEMA)return nfs_player_begin_refused("player-view",v?v->scene.serial:0);
 kind=v->form==0x4d3050?NFS_BACKGROUND_TAIL_PLAYER1_CAPTION:v->form==0x4d3100?NFS_BACKGROUND_TAIL_PLAYER2_CAPTION:0;
 if(!kind||v->scene.tailKind!=kind)return nfs_player_begin_refused("player-form-tail",v->scene.serial);
 return nfs_player_cached_begin(&v->scene,&v->caption,&v->preview,&v->captionWord,kind);
}
int nfs_private_player_scene_finish(uint64_t serial){unsigned kind=nfs_player_renderer.activeOwnerKind;if(kind!=NFS_PLAYER_OWNER_ONE&&kind!=NFS_PLAYER_OWNER_TWO)return 0;return nfs_player_cached_finish(serial,kind);}
int nfs_private_player_scene_cancel(uint64_t serial){
 if(!serial||serial!=nfs_player_renderer.serial||!nfs_player_current()||(nfs_player_renderer.activeOwnerKind!=NFS_PLAYER_OWNER_ONE&&nfs_player_renderer.activeOwnerKind!=NFS_PLAYER_OWNER_TWO))return 0;
 /* CPU-only cancellation may clear only this pending Player image and its
    own matching native arm, never another scene's replacement global pass. */
 nfs_player_cancel("source-proof-revoked");return 1;
}
static void nfs_player_renderer_other(void){if(nfs_player_renderer.serial)nfs_player_cancel("other-original-operation");}
#undef NFS_PLAYER_RENDER_GL
