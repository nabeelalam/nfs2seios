// SPDX-License-Identifier: MIT
/* Independent actual renderer-owner child Controls426950 adapter. All original uploads, selects
   and triangles still execute. Source scene joins grant no GL authority. */
#include "ControlsSetupSceneRuntimeAbi.h"
#include "AudioNativePass.h"
#define NFS_CONTROLS_SETUP_SCENE_EXTERNAL_BACKGROUND_IMPLEMENTATION
#include "ControlsSetupSceneLayers.c.inc"
#undef NFS_CONTROLS_SETUP_SCENE_EXTERNAL_BACKGROUND_IMPLEMENTATION
#ifndef NFS_CONTROLS_SETUP_SCENE_THREAD_TOKEN
#define NFS_CONTROLS_SETUP_SCENE_THREAD_TOKEN() ((uintptr_t)SDL_GetCurrentThreadID())
#endif
static struct {
 NfsAudioNativePass pass;NfsControlsSetupSceneFrameView view;NfsNativeLogoTargetProof target;
 NfsSoftwareTextGpu caption,text;NfsNestedCaptionGpu sidebarWordGpu;NfsControlsSetupWordGpu childWordsGpu;
 uintptr_t context,thread;uint64_t generation;unsigned uploadPending,traceLines;
 TextureInfo*uploadTexture;GLuint color,fbo;unsigned width,height;
} nfs_controls_setup_renderer;
static int nfs_controls_setup_context(uint64_t generation,uintptr_t context){return generation&&nfs_controls_setup_renderer.generation==generation&&nfs_controls_setup_renderer.context==context&&nfs_controls_setup_renderer.thread==NFS_CONTROLS_SETUP_SCENE_THREAD_TOKEN()&&g_glCtx&&(uintptr_t)g_glCtx==context&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==generation;}
static int nfs_controls_setup_current(void){return nfs_controls_setup_context(nfs_controls_setup_renderer.generation,nfs_controls_setup_renderer.context);}
static int nfs_controls_setup_word_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_controls_setup_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_controls_setup_word_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_controls_setup_renderer.generation;t.contextIdentity=nfs_controls_setup_renderer.context;t.ownerCurrent=nfs_controls_setup_word_owner;return t;}
/* No GL-containing helper may be hidden inside this macro. */
#define NFS_CONTROLS_SETUP_RENDER_GL(call) do{if(!nfs_controls_setup_current())goto lost;call;if(!nfs_controls_setup_current())goto lost;}while(0)
static void nfs_controls_setup_trace(const char*event,const char*reason){if(!nfs_logo_trace_enabled()||nfs_controls_setup_renderer.traceLines>=2048)return;++nfs_controls_setup_renderer.traceLines;fprintf(stderr,"CONTROLS SETUP NATIVE %s reason=%s serial=%llu content=%llu context=%llu size=%ux%u fbo=%u uploads=%u selections=%u triangles=%u stop=%u\n",event,reason,(unsigned long long)nfs_controls_setup_renderer.view.serial,(unsigned long long)nfs_controls_setup_renderer.view.contentRevision,(unsigned long long)nfs_controls_setup_renderer.generation,nfs_controls_setup_renderer.target.width,nfs_controls_setup_renderer.target.height,nfs_controls_setup_renderer.target.framebuffer,nfs_controls_setup_renderer.pass.uploads,nfs_controls_setup_renderer.pass.selects,nfs_controls_setup_renderer.pass.triangles,nfs_controls_setup_renderer.pass.reason);}
static void nfs_controls_setup_clear_view(void){void*owner=nfs_controls_setup_renderer.view.lifetimeOwner;void(*release)(void*)=nfs_controls_setup_renderer.view.releaseOwner;memset(&nfs_controls_setup_renderer.view,0,sizeof nfs_controls_setup_renderer.view);if(owner&&release)release(owner);}
static void nfs_controls_setup_cancel(const char*why){if(nfs_controls_setup_renderer.view.serial)nfs_controls_setup_trace("canceled",why);nfs_audio_native_fail(&nfs_controls_setup_renderer.pass,NFS_AUDIO_NATIVE_OTHER);nfs_controls_setup_clear_view();nfs_controls_setup_renderer.uploadPending=0;nfs_controls_setup_renderer.uploadTexture=NULL;}
static void nfs_controls_setup_forget_gpu(void){nfs_controls_setup_word_forget(&nfs_controls_setup_renderer.childWordsGpu);nfs_nested_caption_forget(&nfs_controls_setup_renderer.sidebarWordGpu);nfs_soft_gpu_context_retired(&nfs_controls_setup_renderer.caption,nfs_controls_setup_renderer.caption.generation,0);nfs_soft_gpu_context_retired(&nfs_controls_setup_renderer.text,nfs_controls_setup_renderer.text.generation,0);nfs_controls_setup_renderer.color=nfs_controls_setup_renderer.fbo=0;nfs_controls_setup_renderer.width=nfs_controls_setup_renderer.height=0;}
static void nfs_controls_setup_abandon(void){nfs_controls_setup_forget_gpu();nfs_audio_native_retired(&nfs_controls_setup_renderer.pass,nfs_controls_setup_renderer.generation);nfs_controls_setup_cancel("owner-lost");}
/* Context-only guard supports temporary binding changes during resource prepare
   and restoration. It does not grant draw or original-target commit authority. */
static int nfs_controls_setup_context_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_controls_setup_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_controls_setup_context_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_controls_setup_renderer.generation;t.contextIdentity=nfs_controls_setup_renderer.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_controls_setup_context_owner;return t;}
/* Query original renderer-owned target without calling the legacy compound
   target reader. Every query and temporary renderbuffer bind is guarded. */
static int nfs_controls_setup_read_target(NfsNativeLogoTargetProof*out,uint64_t serial){
 GLint fbo=0,vp[4]={0},sc[4]={0},func=0,program=0,ct=0,cn=0,dt=0,dn=0,rbo=0,rw=0,rh=0;
 GLboolean enabled=0,depthMask=0,color[4]={0};GLenum status=0;unsigned w,h;NfsFrameLegacySampling sampling;
 if(!out||!serial||!nfs_controls_setup_current()||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER||!g_textureEnabled||g_fogEnabled)return 0;
 h=(unsigned)g_framebufferHeight;w=h*4u/3u;if(w>4096||!nfs_frame_gpu_legacy_sampling(w,h,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||memcmp(g_matrix,sampling.projection,sizeof g_matrix))return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_VIEWPORT,vp));NFS_CONTROLS_SETUP_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 if((GLuint)fbo!=g_framebuffer||memcmp(vp,sampling.viewport,sizeof vp)||status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||dt!=GL_RENDERBUFFER||!cn||!dn||(GLuint)cn!=g_framebufferTexture||(GLuint)dn!=g_framebufferDepth)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo));NFS_CONTROLS_SETUP_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)dn));NFS_CONTROLS_SETUP_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_CONTROLS_SETUP_RENDER_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_CONTROLS_SETUP_RENDER_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo));if(rw!=(GLint)w||rh!=(GLint)h)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_DEPTH_FUNC,&func));NFS_CONTROLS_SETUP_RENDER_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_CONTROLS_SETUP_RENDER_GL(glGetBooleanv(GL_COLOR_WRITEMASK,color));NFS_CONTROLS_SETUP_RENDER_GL(enabled=glIsEnabled(GL_DEPTH_TEST));
 if(program!=(GLint)g_shaderProgram||!enabled||!depthMask||!g_depthMask||func!=GL_LEQUAL||!color[0]||!color[1]||!color[2]||!color[3]||!nfs_logo_fullbox(vp,w,h))return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(!nfs_logo_fullbox(sc,w,h))return 0;}
 NFS_CONTROLS_SETUP_RENDER_GL(status=glGetError());if(status!=GL_NO_ERROR)return 0;
 memset(out,0,sizeof *out);out->framebuffer=(GLuint)fbo;out->width=w;out->height=h;out->contextGeneration=nfs_controls_setup_renderer.generation;/* Color-only commit requires no claimed depth-clear witness. */out->sampling=sampling;return 1;
 lost:nfs_controls_setup_abandon();return 0;
}
static int nfs_controls_setup_stamp(NfsAudioNativeStamp*out){NfsNativeLogoTargetProof target;if(!out||!nfs_controls_setup_renderer.view.serial||!nfs_controls_setup_read_target(&target,nfs_controls_setup_renderer.view.serial)||memcmp(&target,&nfs_controls_setup_renderer.target,sizeof target))return 0;out->generation=nfs_controls_setup_renderer.generation;out->context=nfs_controls_setup_renderer.context;out->thread=NFS_CONTROLS_SETUP_SCENE_THREAD_TOKEN();out->framebuffer=target.framebuffer;out->width=target.width;out->height=target.height;return 1;}
static void nfs_controls_setup_renderer_created(void){uint64_t g=nfs_logo_provenance.context;uintptr_t context=(uintptr_t)g_glCtx;if(!nfs_bg_feature()||!g||!context||SDL_GL_GetCurrentContext()!=g_glCtx)return;nfs_controls_setup_renderer.generation=g;nfs_controls_setup_renderer.context=context;nfs_controls_setup_renderer.thread=NFS_CONTROLS_SETUP_SCENE_THREAD_TOKEN();nfs_controls_setup_renderer.uploadPending=0;nfs_controls_setup_clear_view();(void)nfs_audio_native_created(&nfs_controls_setup_renderer.pass,g,context,nfs_controls_setup_renderer.thread);(void)nfs_soft_gpu_context_created(&nfs_controls_setup_renderer.caption,g);(void)nfs_soft_gpu_context_created(&nfs_controls_setup_renderer.text,g);}
static void nfs_controls_setup_renderer_retired(int owner){
 {NfsBackgroundSceneStage guard=nfs_controls_setup_word_stage();if(owner&&nfs_controls_setup_current()){if(nfs_controls_setup_word_alias(&nfs_controls_setup_renderer.childWordsGpu,&nfs_bg_compositor,&nfs_controls_setup_renderer.caption,&nfs_controls_setup_renderer.text,&nfs_controls_setup_renderer.sidebarWordGpu,nfs_controls_setup_renderer.color,g_framebufferTexture)||!nfs_controls_setup_word_retire(&nfs_controls_setup_renderer.childWordsGpu,&guard,1))goto lost;}else nfs_controls_setup_word_forget(&nfs_controls_setup_renderer.childWordsGpu);}

 {NfsBackgroundSceneStage guard=nfs_controls_setup_word_stage();if(owner&&nfs_controls_setup_current()){if(nfs_nested_caption_alias(&nfs_controls_setup_renderer.sidebarWordGpu,&nfs_bg_compositor,&nfs_controls_setup_renderer.caption,&nfs_controls_setup_renderer.text,0,0,nfs_controls_setup_renderer.color,g_framebufferTexture)||!nfs_nested_caption_retire(&nfs_controls_setup_renderer.sidebarWordGpu,&guard,1))goto lost;}else nfs_nested_caption_forget(&nfs_controls_setup_renderer.sidebarWordGpu);}
 if(owner&&nfs_controls_setup_current()){
  if(nfs_controls_setup_renderer.caption.generation==nfs_controls_setup_renderer.generation&&nfs_controls_setup_renderer.caption.ownerGeneration==nfs_controls_setup_renderer.generation){
  if(nfs_controls_setup_renderer.caption.program){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteProgram(nfs_controls_setup_renderer.caption.program));}
  if(nfs_controls_setup_renderer.caption.texture){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&nfs_controls_setup_renderer.caption.texture));}
  }
  if(nfs_controls_setup_renderer.text.generation==nfs_controls_setup_renderer.generation&&nfs_controls_setup_renderer.text.ownerGeneration==nfs_controls_setup_renderer.generation){
  if(nfs_controls_setup_renderer.text.program){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteProgram(nfs_controls_setup_renderer.text.program));}
  if(nfs_controls_setup_renderer.text.texture){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&nfs_controls_setup_renderer.text.texture));}
  }
  if(nfs_controls_setup_renderer.fbo){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteFramebuffers(1,&nfs_controls_setup_renderer.fbo));}
  if(nfs_controls_setup_renderer.color){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&nfs_controls_setup_renderer.color));}
 }
 lost:nfs_controls_setup_forget_gpu();nfs_audio_native_retired(&nfs_controls_setup_renderer.pass,nfs_controls_setup_renderer.generation);nfs_controls_setup_cancel("context-retired");nfs_controls_setup_renderer.generation=0;nfs_controls_setup_renderer.context=nfs_controls_setup_renderer.thread=0;
}
static void nfs_controls_setup_renderer_other(void){if(nfs_controls_setup_renderer.view.serial)nfs_controls_setup_cancel("unknown-operation");}
static void nfs_controls_setup_renderer_upload_begin(TextureInfo*ti){NfsAudioNativeStamp stamp;if(!nfs_controls_setup_renderer.view.serial)return;if(nfs_controls_setup_renderer.uploadPending||!nfs_controls_setup_stamp(&stamp)||!nfs_audio_native_transition(&nfs_controls_setup_renderer.pass,&stamp)||nfs_controls_setup_renderer.pass.phase!=1||!ti){nfs_controls_setup_cancel("upload-begin");return;}nfs_controls_setup_renderer.uploadPending=1;nfs_controls_setup_renderer.uploadTexture=ti;}
static void nfs_controls_setup_renderer_upload_done(TextureInfo*ti){
 NfsAudioNativeStamp stamp;uintptr_t offset;uint32_t address;GLboolean exists=0;GLenum error;
 if(!nfs_controls_setup_renderer.view.serial)return;
 if(!nfs_controls_setup_renderer.uploadPending||ti!=nfs_controls_setup_renderer.uploadTexture||!nfs_controls_setup_stamp(&stamp)){nfs_controls_setup_cancel("upload-owner-pair");return;}
 nfs_controls_setup_renderer.uploadPending=0;nfs_controls_setup_renderer.uploadTexture=NULL;
 if((uintptr_t)ti<(uintptr_t)g_textures||(uintptr_t)ti-(uintptr_t)g_textures>=sizeof g_textures){nfs_controls_setup_cancel("upload-descriptor-range");return;}
 offset=(uintptr_t)ti-(uintptr_t)g_textures;if(offset%sizeof(TextureInfo)){nfs_controls_setup_cancel("upload-descriptor-alignment");return;}address=(uint32_t)(offset/sizeof(TextureInfo))*4u;
 if(address>TextureMem-2048||ti->data!=g_textureMem+address||ti->hdDigits){nfs_controls_setup_cancel("upload-span");return;}
 NFS_CONTROLS_SETUP_RENDER_GL(exists=glIsTexture(ti->id));NFS_CONTROLS_SETUP_RENDER_GL(error=glGetError());
 if(!exists||error!=GL_NO_ERROR||!nfs_audio_native_upload(&nfs_controls_setup_renderer.pass,&stamp,address,ti->id,ti->fmt,ti->size,(const unsigned char*)ti->data,2048))nfs_controls_setup_cancel("upload-payload");return;
 lost:nfs_controls_setup_abandon();
}
static void nfs_controls_setup_renderer_select(unsigned tmu,uint32_t address,uint32_t evenOdd,const GrTexInfo*info,TextureInfo*ti){NfsAudioNativeStamp stamp;if(!nfs_controls_setup_renderer.view.serial)return;if(nfs_controls_setup_renderer.uploadPending||!info||!ti||!nfs_controls_setup_stamp(&stamp)||!nfs_audio_native_select(&nfs_controls_setup_renderer.pass,&stamp,tmu,address,ti->id,evenOdd,info->format,ti->size,info->smallLod,info->largeLod,info->aspectRatio))nfs_controls_setup_cancel("selection");}
static NfsAudioNativeVertex nfs_controls_setup_vertex(const GrVertex*v){NfsAudioNativeVertex out;out.x=v->x;out.y=v->y;out.r=v->r;out.g=v->g;out.b=v->b;out.a=v->a;out.oow=v->oow;out.sow=v->tmuvtx[0].sow;out.tow=v->tmuvtx[0].tow;return out;}
static void nfs_controls_setup_renderer_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){NfsAudioNativeStamp stamp;NfsAudioNativeVertex av,bv,cv;if(!nfs_controls_setup_renderer.view.serial)return;if(nfs_controls_setup_renderer.uploadPending||!a||!b||!c||!nfs_controls_setup_stamp(&stamp)){nfs_controls_setup_cancel("triangle-owner");return;}av=nfs_controls_setup_vertex(a);bv=nfs_controls_setup_vertex(b);cv=nfs_controls_setup_vertex(c);if(!nfs_audio_native_triangle(&nfs_controls_setup_renderer.pass,&stamp,&av,&bv,&cv,g_textureEnabled?1u:0u,g_fogEnabled?1u:0u))nfs_controls_setup_cancel("triangle");}
/* Guarded ready probes and cold shader/font allocation; immutable provider SHA
   must match the exact pin uploaded for this resource generation. */
static int nfs_controls_setup_font_prepare(NfsSoftwareTextGpu*s,const NfsSoftwareTextFontView*f){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint status=0,sampler=-1,matrix=-1,maxSize=0;GLboolean exists=0;GLenum error;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_controls_setup_context_stage();int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";
 const char*vertexSource=vertex;const char*fragmentSource=fragment;
 if(!s||!nfs_controls_setup_current()||s->generation!=nfs_controls_setup_renderer.generation||!f||f->schema!=1||f->width!=1024||f->height!=512||f->bytes!=1024u*512u||!f->alpha||!f->identity)return 0;
 if(s->ownerGeneration){if(s->ownerGeneration!=s->generation||!s->program||!s->texture||s->matrix<0||s->sampler<0||s->fontIdentity!=f->identity||memcmp(s->fontSha256,f->sha256,32))return 0;NFS_CONTROLS_SETUP_RENDER_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_CONTROLS_SETUP_RENDER_GL(exists=glIsTexture(s->texture));return exists!=0;}
 if(s->program||s->texture||!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32))return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));if(maxSize<1024)return 0;
 if(!nfs_controls_setup_scene_save(&saved,&guard))goto lost;
 NFS_CONTROLS_SETUP_RENDER_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glShaderSource(vs,1,&vertexSource,NULL));NFS_CONTROLS_SETUP_RENDER_GL(glCompileShader(vs));NFS_CONTROLS_SETUP_RENDER_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_CONTROLS_SETUP_RENDER_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glShaderSource(fs,1,&fragmentSource,NULL));NFS_CONTROLS_SETUP_RENDER_GL(glCompileShader(fs));NFS_CONTROLS_SETUP_RENDER_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_CONTROLS_SETUP_RENDER_GL(program=glCreateProgram());if(!program)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glAttachShader(program,vs));NFS_CONTROLS_SETUP_RENDER_GL(glAttachShader(program,fs));NFS_CONTROLS_SETUP_RENDER_GL(glBindAttribLocation(program,0,"vertex"));NFS_CONTROLS_SETUP_RENDER_GL(glBindAttribLocation(program,1,"color"));NFS_CONTROLS_SETUP_RENDER_GL(glLinkProgram(program));NFS_CONTROLS_SETUP_RENDER_GL(glGetProgramiv(program,GL_LINK_STATUS,&status));if(!status)goto done;
 NFS_CONTROLS_SETUP_RENDER_GL(sampler=glGetUniformLocation(program,"image"));NFS_CONTROLS_SETUP_RENDER_GL(matrix=glGetUniformLocation(program,"matrix"));if(sampler<0||matrix<0)goto done;
 NFS_CONTROLS_SETUP_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_SETUP_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CONTROLS_SETUP_RENDER_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_CONTROLS_SETUP_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,1024,512,0,GL_ALPHA,GL_UNSIGNED_BYTE,f->alpha));NFS_CONTROLS_SETUP_RENDER_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteShader(vs));}if(fs){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteProgram(program));}if(texture){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_controls_setup_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;if(!nfs_controls_setup_current())goto lost;s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->fontIdentity=f->identity;s->ownerGeneration=s->generation;memcpy(s->fontSha256,sha,32);return 1;
 lost:nfs_controls_setup_abandon();return 0;
}
static int nfs_controls_setup_color_target(unsigned w,unsigned h){
 GLuint texture=0,fbo=0;GLint maxSize=0,maxViewport[2]={0,0},type=0,name=0;GLenum status=0,error;GLboolean exists=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_controls_setup_context_stage();int ok=0;
 if(nfs_controls_setup_renderer.color==g_framebufferTexture||nfs_controls_setup_renderer.fbo==g_framebuffer)return 0;
 if(!nfs_controls_setup_current()||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));if(w>(unsigned)maxSize||h>(unsigned)maxSize||w>(unsigned)maxViewport[0]||h>(unsigned)maxViewport[1])return 0;
 if(!nfs_controls_setup_scene_save(&saved,&guard))goto lost;
 if(nfs_controls_setup_renderer.color&&nfs_controls_setup_renderer.fbo&&nfs_controls_setup_renderer.width==w&&nfs_controls_setup_renderer.height==h){
  NFS_CONTROLS_SETUP_RENDER_GL(exists=glIsTexture(nfs_controls_setup_renderer.color));if(!exists)goto done;NFS_CONTROLS_SETUP_RENDER_GL(exists=glIsFramebuffer(nfs_controls_setup_renderer.fbo));if(!exists)goto done;
  NFS_CONTROLS_SETUP_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_controls_setup_renderer.fbo));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_CONTROLS_SETUP_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_CONTROLS_SETUP_RENDER_GL(error=glGetError());ok=type==GL_TEXTURE&&(GLuint)name==nfs_controls_setup_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
  if(!nfs_controls_setup_scene_restore(&saved,&guard))goto lost;return ok;
 }
 NFS_CONTROLS_SETUP_RENDER_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_SETUP_RENDER_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CONTROLS_SETUP_RENDER_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CONTROLS_SETUP_RENDER_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));NFS_CONTROLS_SETUP_RENDER_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 NFS_CONTROLS_SETUP_RENDER_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;NFS_CONTROLS_SETUP_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_CONTROLS_SETUP_RENDER_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0));NFS_CONTROLS_SETUP_RENDER_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_CONTROLS_SETUP_RENDER_GL(error=glGetError());ok=status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteFramebuffers(1,&fbo));}if(texture){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_controls_setup_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;
 if(nfs_controls_setup_renderer.fbo){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteFramebuffers(1,&nfs_controls_setup_renderer.fbo));}if(nfs_controls_setup_renderer.color){NFS_CONTROLS_SETUP_RENDER_GL(glDeleteTextures(1,&nfs_controls_setup_renderer.color));}
 nfs_controls_setup_renderer.fbo=fbo;nfs_controls_setup_renderer.color=texture;nfs_controls_setup_renderer.width=w;nfs_controls_setup_renderer.height=h;return 1;
 lost:nfs_controls_setup_abandon();return 0;
}
/* Strict registered staging callback. Every observation also guards the real
   context/thread/generation before and after, including inside this callback. */
static int nfs_controls_setup_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;(void)user;
 if(!t||nfs_controls_setup_renderer.color==g_framebufferTexture||nfs_controls_setup_renderer.fbo==g_framebuffer||!nfs_controls_setup_context(t->generation,t->contextIdentity)||t->framebuffer!=nfs_controls_setup_renderer.fbo||t->width!=nfs_controls_setup_renderer.width||t->height!=nfs_controls_setup_renderer.height)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=t->framebuffer)return 0;NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==nfs_controls_setup_renderer.color;
 lost:return 0;
}
static int nfs_controls_setup_resource_owner(void*user,const NfsBackgroundResourceOwner*o){
 GLint fbo=0,type=0,name=0;(void)user;if(!o||!nfs_controls_setup_context(o->generation,o->contextIdentity)||!nfs_controls_setup_renderer.view.serial||nfs_controls_setup_renderer.target.framebuffer!=g_framebuffer)return 0;
 NFS_CONTROLS_SETUP_RENDER_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_SETUP_RENDER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==g_framebufferTexture;
 lost:return 0;
}
/* Commit has a stricter guard than resource preparation: each GL operation
   keeps the exact registered original color target bound, without requiring
   the original shader/depth state that the color-only copy intentionally alters. */
static int nfs_controls_setup_commit_current(void){NfsBackgroundResourceOwner owner={nfs_controls_setup_renderer.generation,nfs_controls_setup_renderer.context,nfs_controls_setup_resource_owner,NULL};return nfs_controls_setup_renderer.color&&nfs_controls_setup_renderer.color!=g_framebufferTexture&&nfs_controls_setup_renderer.fbo!=g_framebuffer&&nfs_controls_setup_resource_owner(NULL,&owner);}
static int nfs_controls_setup_copy_color(const NfsNativeLogoTargetProof*target){
 GLenum error;static const GLfloat copy[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};
#define NFS_CONTROLS_SETUP_COMMIT_GL(call) do{if(!nfs_controls_setup_commit_current())return 0;call;if(!nfs_controls_setup_commit_current())return 0;}while(0)
 for(unsigned i=0;i<8;++i){NFS_CONTROLS_SETUP_COMMIT_GL(glDisable(nfs_frame_caps[i]));}NFS_CONTROLS_SETUP_COMMIT_GL(glColorMask(1,1,1,1));NFS_CONTROLS_SETUP_COMMIT_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_SETUP_COMMIT_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CONTROLS_SETUP_COMMIT_GL(glDepthMask(GL_FALSE));NFS_CONTROLS_SETUP_COMMIT_GL(glViewport(0,0,target->width,target->height));NFS_CONTROLS_SETUP_COMMIT_GL(glUseProgram(nfs_bg_compositor.copyProgram));NFS_CONTROLS_SETUP_COMMIT_GL(glUniform1i(nfs_bg_compositor.copySampler,0));NFS_CONTROLS_SETUP_COMMIT_GL(glBindTexture(GL_TEXTURE_2D,nfs_controls_setup_renderer.color));NFS_CONTROLS_SETUP_COMMIT_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_CONTROLS_SETUP_COMMIT_GL(glDisableVertexAttribArray(i));}NFS_CONTROLS_SETUP_COMMIT_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy));NFS_CONTROLS_SETUP_COMMIT_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_CONTROLS_SETUP_COMMIT_GL(error=glGetError());return error==GL_NO_ERROR;
#undef NFS_CONTROLS_SETUP_COMMIT_GL
}
static int nfs_controls_setup_begin_refused(const char*reason,const NfsControlsSetupSceneFrameView*v){if(nfs_logo_trace_enabled()&&nfs_controls_setup_renderer.traceLines<2048){++nfs_controls_setup_renderer.traceLines;fprintf(stderr,"CONTROLS SETUP NATIVE begin-refused reason=%s serial=%llu content=%llu context=%llu\n",reason,(unsigned long long)(v?v->serial:0),(unsigned long long)(v?v->contentRevision:0),(unsigned long long)nfs_controls_setup_renderer.generation);}return 0;}
int nfs_private_controls_setup_scene_begin(const NfsControlsSetupSceneFrameView*v){
 NfsAudioNativeStamp stamp;NfsNativeLogoTargetProof target;
 if(!v||v->schema!=NFS_CONTROLS_SETUP_SCENE_RUNTIME_SCHEMA||!nfs_controls_setup_word_valid(&v->layers.childWords,&v->layers.text)||nfs_nested_caption_empty(&v->layers.sidebarWord)||!nfs_nested_caption_valid(&v->layers.sidebarWord,3)||v->width!=640||v->height!=480||!v->serial||!v->contentRevision||v->planeWords!=307200||!v->original565||!v->lifetimeOwner||!v->retainOwner||!v->releaseOwner||!nfs_bg_feature()||!nfs_controls_setup_current())return nfs_controls_setup_begin_refused("view-provider-owner",v);
 if(nfs_controls_setup_renderer.view.serial){nfs_controls_setup_cancel("nested-begin");return 0;}drawTriangles();if(!nfs_controls_setup_read_target(&target,v->serial))return nfs_controls_setup_begin_refused("native-target",v);
 v->retainOwner(v->lifetimeOwner);nfs_controls_setup_renderer.view=*v;nfs_controls_setup_renderer.target=target;
 if(!nfs_controls_setup_stamp(&stamp)||!nfs_audio_native_begin(&nfs_controls_setup_renderer.pass,&stamp,v->serial,v->original565,v->planeWords)){nfs_controls_setup_cancel("begin-owner-source-serial");return 0;}
 nfs_controls_setup_trace("begin-accepted","original-pass-witness");return 1;
}
int nfs_private_controls_setup_scene_finish(uint64_t serial){
 NfsAudioNativeStamp stamp;NfsNativeLogoTargetProof target;NfsBackgroundSceneStage guard,stage;NfsControlsSetupSceneGpu gpu;NfsBgSaved saved;int ok=0;
 if(!serial||serial!=nfs_controls_setup_renderer.view.serial){nfs_controls_setup_cancel("finish-serial");return 0;}if(!nfs_controls_setup_current())goto lost;
 /* The existing renderer's original buffered geometry executes in full. */
 drawTriangles();if(!nfs_controls_setup_stamp(&stamp)||nfs_controls_setup_renderer.uploadPending||!nfs_audio_native_finish(&nfs_controls_setup_renderer.pass,&stamp,serial)){nfs_controls_setup_cancel("incomplete-original-pass");return 0;}
 if(!nfs_controls_setup_read_target(&target,serial)||memcmp(&target,&nfs_controls_setup_renderer.target,sizeof target)){nfs_controls_setup_cancel("finish-target");return 0;}
 {NfsBackgroundResourceOwner owner={nfs_controls_setup_renderer.generation,nfs_controls_setup_renderer.context,nfs_controls_setup_resource_owner,NULL};if(!nfs_bg_scene_prepare_owned(&nfs_bg_compositor,&owner)){if(!nfs_controls_setup_current())goto lost;nfs_controls_setup_cancel("background-cold-prepare");return 0;}}
 if(!nfs_controls_setup_read_target(&target,serial)||memcmp(&target,&nfs_controls_setup_renderer.target,sizeof target)){nfs_controls_setup_cancel("background-prepare-target");return 0;}
 if(!nfs_controls_setup_font_prepare(&nfs_controls_setup_renderer.caption,&nfs_controls_setup_renderer.view.layers.sidebarCaption.font)||!nfs_controls_setup_font_prepare(&nfs_controls_setup_renderer.text,&nfs_controls_setup_renderer.view.layers.text.font)||!nfs_controls_setup_color_target(target.width,target.height)){if(!nfs_controls_setup_current())goto lost;nfs_controls_setup_cancel("prepare-font-stage");return 0;}
 {NfsBackgroundSceneStage wordGuard=nfs_controls_setup_word_stage();if(!nfs_nested_caption_empty(&nfs_controls_setup_renderer.view.layers.sidebarWord)){if(nfs_nested_caption_alias(&nfs_controls_setup_renderer.sidebarWordGpu,&nfs_bg_compositor,&nfs_controls_setup_renderer.caption,&nfs_controls_setup_renderer.text,0,0,nfs_controls_setup_renderer.color,g_framebufferTexture)||!nfs_nested_caption_prepare(&nfs_controls_setup_renderer.sidebarWordGpu,&nfs_controls_setup_renderer.view.layers.sidebarWord,3,&wordGuard)){if(!nfs_controls_setup_context(nfs_controls_setup_renderer.generation,nfs_controls_setup_renderer.context)){nfs_controls_setup_abandon();return 0;}nfs_controls_setup_cancel("sidebar-word-prepare");return 0;}}}
 {NfsBackgroundSceneStage wordGuard=nfs_controls_setup_word_stage();if(nfs_controls_setup_word_alias(&nfs_controls_setup_renderer.childWordsGpu,&nfs_bg_compositor,&nfs_controls_setup_renderer.caption,&nfs_controls_setup_renderer.text,&nfs_controls_setup_renderer.sidebarWordGpu,nfs_controls_setup_renderer.color,g_framebufferTexture)||!nfs_controls_setup_word_prepare(&nfs_controls_setup_renderer.childWordsGpu,&nfs_controls_setup_renderer.view.layers.childWords,&nfs_controls_setup_renderer.view.layers.text,&wordGuard)){if(!nfs_controls_setup_current())goto lost;nfs_controls_setup_cancel("child-word-prepare");return 0;}}
 if(!nfs_controls_setup_read_target(&target,serial)||memcmp(&target,&nfs_controls_setup_renderer.target,sizeof target)){nfs_controls_setup_cancel("prepare-target-changed");return 0;}
 guard=nfs_controls_setup_context_stage();if(!nfs_controls_setup_scene_save(&saved,&guard))goto lost;
 NFS_CONTROLS_SETUP_RENDER_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_controls_setup_renderer.fbo));NFS_CONTROLS_SETUP_RENDER_GL(glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]));
 memset(&stage,0,sizeof stage);stage.generation=nfs_controls_setup_renderer.generation;stage.contextIdentity=nfs_controls_setup_renderer.context;stage.framebuffer=nfs_controls_setup_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_controls_setup_stage_owner;gpu.background=&nfs_bg_compositor;gpu.sidebarCaption=&nfs_controls_setup_renderer.caption;gpu.text=&nfs_controls_setup_renderer.text;gpu.sidebarWord=&nfs_controls_setup_renderer.sidebarWordGpu;gpu.childWords=&nfs_controls_setup_renderer.childWordsGpu;
 ok=nfs_controls_setup_scene_draw_layers_into_staging(&gpu,&stage,&nfs_controls_setup_renderer.view.layers);if(!nfs_controls_setup_current())goto lost;if(!nfs_controls_setup_scene_restore(&saved,&guard))goto lost;if(!ok){nfs_controls_setup_cancel("ordered-stage");return 0;}
 if(!nfs_controls_setup_read_target(&target,serial)||memcmp(&target,&nfs_controls_setup_renderer.target,sizeof target)){nfs_controls_setup_cancel("precommit-target");return 0;}
 if(!nfs_controls_setup_scene_save(&saved,&guard))goto lost;
 ok=nfs_controls_setup_copy_color(&target);if(!nfs_controls_setup_current())goto lost;
 if(!nfs_controls_setup_scene_restore(&saved,&guard))goto lost;nfs_controls_setup_trace(ok?"finish-published":"finish-refused",ok?"color-only-commit":"copy-error");nfs_controls_setup_clear_view();return ok;
 lost:nfs_controls_setup_abandon();return 0;
}
#undef NFS_CONTROLS_SETUP_RENDER_GL
