// SPDX-License-Identifier: MIT
/* Exact Options cached slot1 adapter. Original427B30 still executes all native
   selects/triangles; COLOR commit finishes before original native GPU text.
   Queued immutable owners retain raw frame borrows during synchronous begin.
   Only GPU staging and scalar metadata survive begin; finish reads no borrow. */
#include "OptionsSceneRuntimeAbi.h"
#define NFS_OPTIONS_SCENE_EXTERNAL_GRAPHICS_IMPLEMENTATION
#include "OptionsSceneLayers.c.inc"
#undef NFS_OPTIONS_SCENE_EXTERNAL_GRAPHICS_IMPLEMENTATION
#ifndef NFS_OPTIONS_SCENE_THREAD_TOKEN
#define NFS_OPTIONS_SCENE_THREAD_TOKEN() ((uintptr_t)SDL_GetCurrentThreadID())
#endif
static struct {
 NfsNativeLogoTargetProof target;NfsSoftwareTextGpu caption;
 uintptr_t context,thread;uint64_t generation,serial,content;
 unsigned map,words,nativeArmed,traceLines;
 GLuint color,fbo;unsigned width,height;
} nfs_options_renderer;
static int nfs_options_context(uint64_t generation,uintptr_t context){return generation&&nfs_options_renderer.generation==generation&&nfs_options_renderer.context==context&&nfs_options_renderer.thread==NFS_OPTIONS_SCENE_THREAD_TOKEN()&&g_glCtx&&(uintptr_t)g_glCtx==context&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==generation;}
static int nfs_options_current(void){return nfs_options_context(nfs_options_renderer.generation,nfs_options_renderer.context);}
#define NFS_OPTIONS_RENDER_GL(call) do{if(!nfs_options_current())goto lost;call;if(!nfs_options_current())goto lost;}while(0)
static void nfs_options_trace(const char*event,const char*reason){if(!nfs_logo_trace_enabled()||nfs_options_renderer.traceLines>=2048)return;++nfs_options_renderer.traceLines;fprintf(stderr,"OPTIONS NATIVE %s reason=%s serial=%llu content=%llu map=%u words=%u context=%llu size=%ux%u fbo=%u selections=%u triangles=%u\n",event,reason,(unsigned long long)nfs_options_renderer.serial,(unsigned long long)nfs_options_renderer.content,nfs_options_renderer.map,nfs_options_renderer.words,(unsigned long long)nfs_options_renderer.generation,nfs_options_renderer.target.width,nfs_options_renderer.target.height,nfs_options_renderer.target.framebuffer,nfs_logo_provenance.selects,nfs_logo_provenance.triangles);}
static void nfs_options_cancel(const char*reason){if(nfs_options_renderer.serial)nfs_options_trace("canceled",reason);if(nfs_options_renderer.nativeArmed)nfs_logo_p_cancel(&nfs_logo_provenance);nfs_options_renderer.nativeArmed=0;nfs_options_renderer.serial=0;memset(&nfs_options_renderer.target,0,sizeof nfs_options_renderer.target);}
static void nfs_options_forget_gpu(void){nfs_soft_gpu_context_retired(&nfs_options_renderer.caption,nfs_options_renderer.caption.generation,0);nfs_options_renderer.color=nfs_options_renderer.fbo=0;nfs_options_renderer.width=nfs_options_renderer.height=0;}
static void nfs_options_abandon(void){nfs_options_forget_gpu();nfs_options_cancel("owner-lost");}
static int nfs_options_context_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_options_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_options_context_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_options_renderer.generation;t.contextIdentity=nfs_options_renderer.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_options_context_owner;return t;}
static int nfs_options_read_target(NfsNativeLogoTargetProof*out,uint64_t serial){
 GLint fbo=0,vp[4]={0},sc[4]={0},func=0,program=0,ct=0,cn=0,dt=0,dn=0,rbo=0,rw=0,rh=0;
 GLboolean enabled=0,depthMask=0,color[4]={0};GLenum status=0;unsigned w,h;NfsFrameLegacySampling sampling;
 if(!out||!serial||!nfs_options_current()||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER||!g_textureEnabled||g_fogEnabled)return 0;
 h=(unsigned)g_framebufferHeight;w=h*4u/3u;if(w>4096||!nfs_frame_gpu_legacy_sampling(w,h,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||memcmp(g_matrix,sampling.projection,sizeof g_matrix))return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_VIEWPORT,vp));NFS_OPTIONS_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 if((GLuint)fbo!=g_framebuffer||memcmp(vp,sampling.viewport,sizeof vp)||status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||dt!=GL_RENDERBUFFER||!cn||!dn||(GLuint)cn!=g_framebufferTexture||(GLuint)dn!=g_framebufferDepth)return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo));NFS_OPTIONS_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)dn));NFS_OPTIONS_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_OPTIONS_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_OPTIONS_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo));if(rw!=(GLint)w||rh!=(GLint)h)return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_DEPTH_FUNC,&func));NFS_OPTIONS_RENDER_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_OPTIONS_RENDER_GL(glGetBooleanv(GL_COLOR_WRITEMASK,color));NFS_OPTIONS_RENDER_GL(enabled=glIsEnabled(GL_DEPTH_TEST));
 if(program!=(GLint)g_shaderProgram||!enabled||!depthMask||!g_depthMask||func!=GL_LEQUAL||!color[0]||!color[1]||!color[2]||!color[3]||!nfs_logo_fullbox(vp,w,h))return 0;
 NFS_OPTIONS_RENDER_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(!nfs_logo_fullbox(sc,w,h))return 0;}
 NFS_OPTIONS_RENDER_GL(status=glGetError());if(status!=GL_NO_ERROR)return 0;
 memset(out,0,sizeof *out);out->framebuffer=(GLuint)fbo;out->width=w;out->height=h;out->contextGeneration=nfs_options_renderer.generation;/* Color-only commit requires no claimed depth-clear witness. */out->sampling=sampling;return 1;
 lost:nfs_options_abandon();return 0;
}
static void nfs_options_renderer_created(void){uint64_t generation=nfs_logo_provenance.context;uintptr_t context=(uintptr_t)g_glCtx;if(!nfs_bg_feature()||!generation||!context||SDL_GL_GetCurrentContext()!=g_glCtx)return;nfs_options_renderer.generation=generation;nfs_options_renderer.context=context;nfs_options_renderer.thread=NFS_OPTIONS_SCENE_THREAD_TOKEN();nfs_options_cancel("context-created");(void)nfs_soft_gpu_context_created(&nfs_options_renderer.caption,generation);}
static void nfs_options_renderer_retired(int owner){
 if(owner&&nfs_options_current()){
  if(nfs_options_renderer.caption.generation==nfs_options_renderer.generation&&nfs_options_renderer.caption.ownerGeneration==nfs_options_renderer.generation){
   if(nfs_options_renderer.caption.program){NFS_OPTIONS_RENDER_GL(glDeleteProgram(nfs_options_renderer.caption.program));}
   if(nfs_options_renderer.caption.texture){NFS_OPTIONS_RENDER_GL(glDeleteTextures(1,&nfs_options_renderer.caption.texture));}
  }
  if(nfs_options_renderer.fbo){NFS_OPTIONS_RENDER_GL(glDeleteFramebuffers(1,&nfs_options_renderer.fbo));}
  if(nfs_options_renderer.color){NFS_OPTIONS_RENDER_GL(glDeleteTextures(1,&nfs_options_renderer.color));}
 }
 lost:nfs_options_forget_gpu();nfs_options_cancel("context-retired");nfs_options_renderer.generation=0;nfs_options_renderer.context=nfs_options_renderer.thread=0;
}
static int nfs_options_font_prepare(NfsSoftwareTextGpu*s,const NfsSoftwareTextFontView*f){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint status=0,sampler=-1,matrix=-1,maxSize=0;GLboolean exists=0;GLenum error;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_options_context_stage();int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";
 const char*vertexSource=vertex;const char*fragmentSource=fragment;
 if(!s||!nfs_options_current()||s->generation!=nfs_options_renderer.generation||!f||f->schema!=1||f->width!=1024||f->height!=512||f->bytes!=1024u*512u||!f->alpha||!f->identity)return 0;
 if(s->ownerGeneration){if(s->ownerGeneration!=s->generation||!s->program||!s->texture||s->matrix<0||s->sampler<0||s->fontIdentity!=f->identity||memcmp(s->fontSha256,f->sha256,32))return 0;NFS_OPTIONS_RENDER_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_OPTIONS_RENDER_GL(exists=glIsTexture(s->texture));return exists!=0;}
 if(s->program||s->texture||!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32))return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));if(maxSize<1024)return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_OPTIONS_RENDER_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_OPTIONS_RENDER_GL(glShaderSource(vs,1,&vertexSource,NULL));NFS_OPTIONS_RENDER_GL(glCompileShader(vs));NFS_OPTIONS_RENDER_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_OPTIONS_RENDER_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_OPTIONS_RENDER_GL(glShaderSource(fs,1,&fragmentSource,NULL));NFS_OPTIONS_RENDER_GL(glCompileShader(fs));NFS_OPTIONS_RENDER_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_OPTIONS_RENDER_GL(program=glCreateProgram());if(!program)goto done;NFS_OPTIONS_RENDER_GL(glAttachShader(program,vs));NFS_OPTIONS_RENDER_GL(glAttachShader(program,fs));NFS_OPTIONS_RENDER_GL(glBindAttribLocation(program,0,"vertex"));NFS_OPTIONS_RENDER_GL(glBindAttribLocation(program,1,"color"));NFS_OPTIONS_RENDER_GL(glLinkProgram(program));NFS_OPTIONS_RENDER_GL(glGetProgramiv(program,GL_LINK_STATUS,&status));if(!status)goto done;
 NFS_OPTIONS_RENDER_GL(sampler=glGetUniformLocation(program,"image"));NFS_OPTIONS_RENDER_GL(matrix=glGetUniformLocation(program,"matrix"));if(sampler<0||matrix<0)goto done;
 NFS_OPTIONS_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_OPTIONS_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_OPTIONS_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_OPTIONS_RENDER_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_OPTIONS_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,1024,512,0,GL_ALPHA,GL_UNSIGNED_BYTE,f->alpha));NFS_OPTIONS_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_OPTIONS_RENDER_GL(glDeleteShader(vs));}if(fs){NFS_OPTIONS_RENDER_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_OPTIONS_RENDER_GL(glDeleteProgram(program));}if(texture){NFS_OPTIONS_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;if(!nfs_options_current())goto lost;s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->fontIdentity=f->identity;s->ownerGeneration=s->generation;memcpy(s->fontSha256,sha,32);return 1;
 lost:nfs_options_abandon();return 0;
}
static int nfs_options_color_target(unsigned w,unsigned h){
 GLuint texture=0,fbo=0;GLint maxSize=0,maxViewport[2]={0,0},type=0,name=0;GLenum status=0,error;GLboolean exists=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_options_context_stage();int ok=0;
 if(nfs_options_renderer.color==g_framebufferTexture||nfs_options_renderer.fbo==g_framebuffer)return 0;
 if(!nfs_options_current()||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));if(w>(unsigned)maxSize||h>(unsigned)maxSize||w>(unsigned)maxViewport[0]||h>(unsigned)maxViewport[1])return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 if(nfs_options_renderer.color&&nfs_options_renderer.fbo&&nfs_options_renderer.width==w&&nfs_options_renderer.height==h){
  NFS_OPTIONS_RENDER_GL(exists=glIsTexture(nfs_options_renderer.color));if(!exists)goto done;NFS_OPTIONS_RENDER_GL(exists=glIsFramebuffer(nfs_options_renderer.fbo));if(!exists)goto done;
  NFS_OPTIONS_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_options_renderer.fbo));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_OPTIONS_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_OPTIONS_RENDER_GL(error=glGetError());ok=type==GL_TEXTURE&&(GLuint)name==nfs_options_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
  if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 }
 NFS_OPTIONS_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_OPTIONS_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_OPTIONS_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_OPTIONS_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_OPTIONS_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));NFS_OPTIONS_RENDER_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 NFS_OPTIONS_RENDER_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;NFS_OPTIONS_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_OPTIONS_RENDER_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0));NFS_OPTIONS_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_OPTIONS_RENDER_GL(error=glGetError());ok=status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_OPTIONS_RENDER_GL(glDeleteFramebuffers(1,&fbo));}if(texture){NFS_OPTIONS_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;
 if(nfs_options_renderer.fbo){NFS_OPTIONS_RENDER_GL(glDeleteFramebuffers(1,&nfs_options_renderer.fbo));}if(nfs_options_renderer.color){NFS_OPTIONS_RENDER_GL(glDeleteTextures(1,&nfs_options_renderer.color));}
 nfs_options_renderer.fbo=fbo;nfs_options_renderer.color=texture;nfs_options_renderer.width=w;nfs_options_renderer.height=h;return 1;
 lost:nfs_options_abandon();return 0;
}
static int nfs_options_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;(void)user;
 if(!t||nfs_options_renderer.color==g_framebufferTexture||nfs_options_renderer.fbo==g_framebuffer||!nfs_options_context(t->generation,t->contextIdentity)||t->framebuffer!=nfs_options_renderer.fbo||t->width!=nfs_options_renderer.width||t->height!=nfs_options_renderer.height)return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=t->framebuffer)return 0;NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==nfs_options_renderer.color;
 lost:return 0;
}
static int nfs_options_resource_owner(void*user,const NfsBackgroundResourceOwner*o){
 GLint fbo=0,type=0,name=0;(void)user;if(!o||!nfs_options_context(o->generation,o->contextIdentity)||!nfs_options_renderer.serial||nfs_options_renderer.target.framebuffer!=g_framebuffer)return 0;
 NFS_OPTIONS_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_OPTIONS_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==g_framebufferTexture;
 lost:return 0;
}
static int nfs_options_commit_current(void){NfsBackgroundResourceOwner owner={nfs_options_renderer.generation,nfs_options_renderer.context,nfs_options_resource_owner,NULL};return nfs_options_renderer.color&&nfs_options_renderer.color!=g_framebufferTexture&&nfs_options_renderer.fbo!=g_framebuffer&&nfs_options_resource_owner(NULL,&owner);}
static int nfs_options_copy_color(const NfsNativeLogoTargetProof*target){
 GLenum error;static const GLfloat copy[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};
#define NFS_OPTIONS_COMMIT_GL(call) do{if(!nfs_options_commit_current())return 0;call;if(!nfs_options_commit_current())return 0;}while(0)
 for(unsigned i=0;i<8;++i){NFS_OPTIONS_COMMIT_GL(glDisable(nfs_frame_caps[i]));}NFS_OPTIONS_COMMIT_GL(glColorMask(1,1,1,1));NFS_OPTIONS_COMMIT_GL(glActiveTexture(GL_TEXTURE0));NFS_OPTIONS_COMMIT_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_OPTIONS_COMMIT_GL(glDepthMask(GL_FALSE));NFS_OPTIONS_COMMIT_GL(glViewport(0,0,target->width,target->height));NFS_OPTIONS_COMMIT_GL(glUseProgram(nfs_bg_compositor.copyProgram));NFS_OPTIONS_COMMIT_GL(glUniform1i(nfs_bg_compositor.copySampler,0));NFS_OPTIONS_COMMIT_GL(glBindTexture(GL_TEXTURE_2D,nfs_options_renderer.color));NFS_OPTIONS_COMMIT_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_OPTIONS_COMMIT_GL(glDisableVertexAttribArray(i));}NFS_OPTIONS_COMMIT_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy));NFS_OPTIONS_COMMIT_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_OPTIONS_COMMIT_GL(error=glGetError());return error==GL_NO_ERROR;
#undef NFS_OPTIONS_COMMIT_GL
}
/* Independent source-authenticated cached upload admission. Original slot1 is
   ARGB1555 format11; it is not the per-frame RGB565 scratch-upload protocol. */
static int nfs_options_cached_tiles(const NfsBackgroundSceneFrameView*v,uint32_t ids[300],GLenum*filter){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_options_context_stage();GLint mag=0,min=0;GLboolean exists=0;GLenum error;int ok=0;
 if(!v||!filter||!nfs_options_current())return 0;
 for(unsigned tile=0;tile<300;++tile){uint32_t address=v->rendererAddress[tile];TextureInfo*ti;const unsigned char*bytes;
  if((address&3)||address>TextureMem-2048)return 0;ti=&g_textures[address>>2];
  if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+address||!nfs_logo_native_upload_valid(address,ti))return 0;
  ids[tile]=ti->id;bytes=(const unsigned char*)ti->data;
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t source=v->original565[((tile%15)*32+y)*640+(tile/15)*32+x];uint16_t a1555=(uint16_t)(((source&0xf800)>>1)|((source&0x7c0)>>1)|(source&31));uint16_t want,got;a1555=a1555?(uint16_t)(a1555|0x8000):0;want=(uint16_t)((a1555<<1)|(a1555>>15));memcpy(&got,bytes+(y*32+x)*2,2);if(got!=want)return 0;}
 }
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_OPTIONS_RENDER_GL(glActiveTexture(GL_TEXTURE0));
 for(unsigned tile=0;tile<300;++tile){NFS_OPTIONS_RENDER_GL(exists=glIsTexture(ids[tile]));if(!exists)goto done;NFS_OPTIONS_RENDER_GL(glBindTexture(GL_TEXTURE_2D,ids[tile]));NFS_OPTIONS_RENDER_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag));NFS_OPTIONS_RENDER_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min));if(tile==0){if(mag!=GL_LINEAR&&mag!=GL_NEAREST)goto done;*filter=(GLenum)mag;}if(mag!=(GLint)*filter||min!=GL_LINEAR)goto done;}
 NFS_OPTIONS_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 lost:nfs_options_abandon();return 0;
}
static int nfs_options_begin_refused(const char*reason,uint64_t serial){if(nfs_logo_trace_enabled()&&nfs_options_renderer.traceLines<2048){++nfs_options_renderer.traceLines;fprintf(stderr,"OPTIONS NATIVE begin-refused reason=%s serial=%llu context=%llu\n",reason,(unsigned long long)serial,(unsigned long long)nfs_options_renderer.generation);}return 0;}
int nfs_private_options_scene_begin(const NfsOptionsSceneFrameView*v){
 NfsNativeLogoTargetProof target;NfsOptionsSceneLayers layers;NfsOptionsSceneGpu gpu;NfsBackgroundSceneStage guard,stage;NfsBgSaved saved;uint32_t ids[300];GLenum filter=GL_LINEAR;int ok=0;
 if(!v||v->schema!=NFS_OPTIONS_SCENE_RUNTIME_SCHEMA||v->scene.tailKind!=NFS_BACKGROUND_TAIL_OPTIONS_CAPTION||!nfs_bg_feature()||!nfs_options_current()||!nfs_bg_view_common(&v->scene)||v->caption.logicalWidth!=343||v->caption.logicalHeight!=480)return nfs_options_begin_refused("view-owner",v?v->scene.serial:0);
 if(nfs_options_renderer.serial){nfs_options_cancel("nested-begin");return 0;}
 /* Other cached-scene pending state cannot lend authority to this admission. */
 nfs_logo_p_cancel(&nfs_logo_provenance);nfs_bg_scene_cancel(&nfs_bg_compositor);drawTriangles();
 if(!nfs_options_read_target(&target,v->scene.serial))return nfs_options_begin_refused("native-target",v->scene.serial);
 if(!nfs_options_cached_tiles(&v->scene,ids,&filter))return nfs_options_begin_refused("cached-upload-key-payload-filter",v->scene.serial);
 target.sampling.magFilter=filter;nfs_options_renderer.serial=v->scene.serial;nfs_options_renderer.content=v->scene.contentRevision;nfs_options_renderer.map=v->scene.mapRole;nfs_options_renderer.words=v->scene.wordCount;nfs_options_renderer.target=target;
 {NfsBackgroundResourceOwner owner={nfs_options_renderer.generation,nfs_options_renderer.context,nfs_options_resource_owner,NULL};if(!nfs_bg_scene_prepare_owned(&nfs_bg_compositor,&owner)){if(!nfs_options_current())goto lost;nfs_options_cancel("background-prepare");return 0;}}
 if(!nfs_options_font_prepare(&nfs_options_renderer.caption,&v->caption.font)||!nfs_options_color_target(target.width,target.height)){if(!nfs_options_current())goto lost;nfs_options_cancel("caption-stage-prepare");return 0;}
 if(!nfs_options_read_target(&target,v->scene.serial)){nfs_options_cancel("prepare-target");return 0;}target.sampling.magFilter=filter;if(memcmp(&target,&nfs_options_renderer.target,sizeof target)){nfs_options_cancel("prepare-target-changed");return 0;}
 memset(&layers,0,sizeof layers);layers.background.mapRole=v->scene.mapRole;layers.background.wordCount=v->scene.wordCount;memcpy(layers.background.panelClip,v->scene.panelClip,sizeof layers.background.panelClip);memcpy(layers.background.words,v->scene.words,sizeof layers.background.words);memcpy(layers.background.logos,v->scene.logos,sizeof layers.background.logos);layers.caption=v->caption;
 guard=nfs_options_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_OPTIONS_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_options_renderer.fbo));NFS_OPTIONS_RENDER_GL(glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]));
 memset(&stage,0,sizeof stage);stage.generation=nfs_options_renderer.generation;stage.contextIdentity=nfs_options_renderer.context;stage.framebuffer=nfs_options_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_options_stage_owner;gpu.background=&nfs_bg_compositor;gpu.caption=&nfs_options_renderer.caption;
 ok=nfs_options_scene_draw_layers_into_staging(&gpu,&stage,&layers);if(!nfs_options_current())goto lost;if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok){nfs_options_cancel("ordered-stage");return 0;}
 if(!nfs_options_read_target(&target,v->scene.serial)){nfs_options_cancel("staged-target");return 0;}target.sampling.magFilter=filter;if(memcmp(&target,&nfs_options_renderer.target,sizeof target)){nfs_options_cancel("staged-target-changed");return 0;}
 /* Actual observed native clear/context/key/serial proof arms only after all
    optional staging has restored the original state. No clear is invented. */
 if(!nfs_logo_p_begin(&nfs_logo_provenance,v->scene.serial,target.framebuffer,v->scene.rendererAddress,ids)){nfs_options_cancel("context-clear-key-serial");return 0;}
 nfs_options_renderer.nativeArmed=1;nfs_options_trace("begin-accepted","staging-only-cached-original-pass");return 1;
 lost:nfs_options_abandon();return 0;
}
int nfs_private_options_scene_finish(uint64_t serial){
 NfsNativeLogoTargetProof target;NfsBgSaved saved;NfsBackgroundSceneStage guard;int ok;
 if(!serial||serial!=nfs_options_renderer.serial||!nfs_options_renderer.nativeArmed){nfs_options_cancel("finish-serial");return 0;}if(!nfs_options_current())goto lost;
 /* All actual original background triangles execute before COLOR commit. The
    caller queues this finish before native Options text/status/cursor. */
 drawTriangles();if(!nfs_options_read_target(&target,serial)){nfs_options_cancel("finish-target");return 0;}target.sampling.magFilter=nfs_options_renderer.target.sampling.magFilter;
 if(memcmp(&target,&nfs_options_renderer.target,sizeof target)||!nfs_logo_p_finish(&nfs_logo_provenance,serial,target.framebuffer)){nfs_options_cancel("target-or-original-pass");return 0;}nfs_options_renderer.nativeArmed=0;
 guard=nfs_options_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 ok=nfs_options_copy_color(&target);if(!nfs_options_current())goto lost;if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;
 nfs_options_trace(ok?"finish-published":"finish-refused",ok?"color-only-commit-before-native-text":"copy-error");nfs_options_renderer.serial=0;memset(&nfs_options_renderer.target,0,sizeof nfs_options_renderer.target);return ok;
 lost:nfs_options_abandon();return 0;
}
#undef NFS_OPTIONS_RENDER_GL
