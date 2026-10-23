// SPDX-License-Identifier: MIT
/* Display-only normal Player intro. Native front/back COLOR and shared DEPTH
   remain original. All frame borrows are consumed during synchronous begin. */
#include "PlayerIntroRuntimeAbi.h"
#include "PlayerIntroNativePass.h"
#ifndef NFS_PLAYER_INTRO_THREAD_TOKEN
#define NFS_PLAYER_INTRO_THREAD_TOKEN() ((uintptr_t)SDL_GetCurrentThreadID())
#endif
enum {NFS_PLAYER_INTRO_NONE=0,NFS_PLAYER_INTRO_SLOT0=1,NFS_PLAYER_INTRO_TAIL=2,NFS_PLAYER_INTRO_READY=3,NFS_PLAYER_INTRO_PRESENTED=4};
static struct {
 NfsNativeLogoTargetProof target;NfsSoftwareTextGpu caption;NfsPlayerIntroNativePass pass;
 uintptr_t context,thread;uint64_t generation,serial,lastSerial,uploadEvent;
 GLuint color,fbo,nativeColor,nativeDepth;unsigned width,height,phase,tailTriangles;
} nfs_player_intro_renderer;
static void nfs_player_intro_trace(const char*event){static unsigned lines;if(nfs_logo_trace_enabled()&&lines<512){++lines;fprintf(stderr,"PLAYER INTRO NATIVE %s serial=%llu upload=%llu phase=%u selections=%u triangles=%u tail=%u\n",event,(unsigned long long)nfs_player_intro_renderer.serial,(unsigned long long)nfs_player_intro_renderer.uploadEvent,nfs_player_intro_renderer.phase,nfs_player_intro_renderer.pass.selects,nfs_player_intro_renderer.pass.triangles,nfs_player_intro_renderer.tailTriangles);}}
static int nfs_player_intro_context(uint64_t generation,uintptr_t context){return generation&&nfs_player_intro_renderer.generation==generation&&nfs_player_intro_renderer.context==context&&nfs_player_intro_renderer.thread==NFS_PLAYER_INTRO_THREAD_TOKEN()&&g_glCtx&&(uintptr_t)g_glCtx==context&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==generation;}
static int nfs_player_intro_current(void){return nfs_player_intro_context(nfs_player_intro_renderer.generation,nfs_player_intro_renderer.context);}
#define NFS_PLAYER_INTRO_GL(call) do{if(!nfs_player_intro_current())goto lost;call;if(!nfs_player_intro_current())goto lost;}while(0)
static void nfs_player_intro_cancel_pending(void){nfs_player_intro_renderer.phase=NFS_PLAYER_INTRO_NONE;nfs_player_intro_renderer.serial=0;nfs_player_intro_renderer.uploadEvent=0;nfs_player_intro_renderer.tailTriangles=0;nfs_player_intro_pass_fail(&nfs_player_intro_renderer.pass);memset(&nfs_player_intro_renderer.target,0,sizeof nfs_player_intro_renderer.target);nfs_player_intro_renderer.nativeColor=nfs_player_intro_renderer.nativeDepth=0;}
static void nfs_player_intro_forget_gpu(void){nfs_soft_gpu_context_retired(&nfs_player_intro_renderer.caption,nfs_player_intro_renderer.caption.generation,0);nfs_player_intro_renderer.color=nfs_player_intro_renderer.fbo=0;nfs_player_intro_renderer.width=nfs_player_intro_renderer.height=0;}
static void nfs_player_intro_abandon(void){nfs_player_intro_forget_gpu();nfs_player_intro_cancel_pending();}
static int nfs_player_intro_context_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_player_intro_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_player_intro_context_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_player_intro_renderer.generation;t.contextIdentity=nfs_player_intro_renderer.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_player_intro_context_owner;return t;}
static int nfs_player_intro_read_target(NfsNativeLogoTargetProof*out,uint64_t serial){
 GLint fbo=0,vp[4]={0},sc[4]={0},func=0,program=0,ct=0,cn=0,dt=0,dn=0,rbo=0,rw=0,rh=0;
 GLboolean enabled=0,depthMask=0,color[4]={0};GLenum status=0;unsigned w,h;NfsFrameLegacySampling sampling;
 if(!out||!serial||!nfs_player_intro_current()||g_framebufferHeight<=0||g_framebufferHeight>4096||!g_framebuffer||g_renderBuffer!=GR_BUFFER_BACKBUFFER||!g_textureEnabled||g_fogEnabled)return 0;
 h=(unsigned)g_framebufferHeight;w=h*4u/3u;if(w>4096||!nfs_frame_gpu_legacy_sampling(w,h,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||memcmp(g_matrix,sampling.projection,sizeof g_matrix))return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_VIEWPORT,vp));NFS_PLAYER_INTRO_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 if((GLuint)fbo!=g_framebuffer||memcmp(vp,sampling.viewport,sizeof vp)||status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||dt!=GL_RENDERBUFFER||!cn||!dn||(GLuint)cn!=g_framebufferTexture||(GLuint)dn!=g_framebufferDepth)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo));NFS_PLAYER_INTRO_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)dn));NFS_PLAYER_INTRO_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH,&rw));NFS_PLAYER_INTRO_GL(glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT,&rh));NFS_PLAYER_INTRO_GL(glBindRenderbuffer(GL_RENDERBUFFER,(GLuint)rbo));if(rw!=(GLint)w||rh!=(GLint)h)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_DEPTH_FUNC,&func));NFS_PLAYER_INTRO_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_PLAYER_INTRO_GL(glGetBooleanv(GL_COLOR_WRITEMASK,color));NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_DEPTH_TEST));
 if(program!=(GLint)g_shaderProgram||!enabled||depthMask!=(g_depthMask?GL_TRUE:GL_FALSE)||func!=GL_LEQUAL||!color[0]||!color[1]||!color[2]||!color[3]||!nfs_logo_fullbox(vp,w,h))return 0;
 NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(!nfs_logo_fullbox(sc,w,h))return 0;}
 NFS_PLAYER_INTRO_GL(status=glGetError());if(status!=GL_NO_ERROR)return 0;
 memset(out,0,sizeof *out);out->framebuffer=(GLuint)fbo;out->width=w;out->height=h;out->contextGeneration=nfs_player_intro_renderer.generation;/* Color-only commit requires no claimed depth-clear witness. */out->sampling=sampling;return 1;
 lost:nfs_player_intro_abandon();return 0;
}
static int nfs_player_intro_font_prepare(NfsSoftwareTextGpu*s,const NfsSoftwareTextFontView*f){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint status=0,sampler=-1,matrix=-1,maxSize=0;GLboolean exists=0;GLenum error;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_intro_context_stage();int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";
 const char*vertexSource=vertex;const char*fragmentSource=fragment;
 if(!s||!nfs_player_intro_current()||s->generation!=nfs_player_intro_renderer.generation||!f||f->schema!=1||f->width!=1024||f->height!=512||f->bytes!=1024u*512u||!f->alpha||!f->identity)return 0;
 if(s->ownerGeneration){if(s->ownerGeneration!=s->generation||!s->program||!s->texture||s->matrix<0||s->sampler<0||s->fontIdentity!=f->identity||memcmp(s->fontSha256,f->sha256,32))return 0;NFS_PLAYER_INTRO_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_PLAYER_INTRO_GL(exists=glIsTexture(s->texture));return exists!=0;}
 if(s->program||s->texture||!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32))return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));if(maxSize<1024)return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_INTRO_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_PLAYER_INTRO_GL(glShaderSource(vs,1,&vertexSource,NULL));NFS_PLAYER_INTRO_GL(glCompileShader(vs));NFS_PLAYER_INTRO_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_INTRO_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_PLAYER_INTRO_GL(glShaderSource(fs,1,&fragmentSource,NULL));NFS_PLAYER_INTRO_GL(glCompileShader(fs));NFS_PLAYER_INTRO_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_INTRO_GL(program=glCreateProgram());if(!program)goto done;NFS_PLAYER_INTRO_GL(glAttachShader(program,vs));NFS_PLAYER_INTRO_GL(glAttachShader(program,fs));NFS_PLAYER_INTRO_GL(glBindAttribLocation(program,0,"vertex"));NFS_PLAYER_INTRO_GL(glBindAttribLocation(program,1,"color"));NFS_PLAYER_INTRO_GL(glLinkProgram(program));NFS_PLAYER_INTRO_GL(glGetProgramiv(program,GL_LINK_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_INTRO_GL(sampler=glGetUniformLocation(program,"image"));NFS_PLAYER_INTRO_GL(matrix=glGetUniformLocation(program,"matrix"));if(sampler<0||matrix<0)goto done;
 NFS_PLAYER_INTRO_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_INTRO_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_INTRO_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_PLAYER_INTRO_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,1024,512,0,GL_ALPHA,GL_UNSIGNED_BYTE,f->alpha));NFS_PLAYER_INTRO_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_PLAYER_INTRO_GL(glDeleteShader(vs));}if(fs){NFS_PLAYER_INTRO_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_PLAYER_INTRO_GL(glDeleteProgram(program));}if(texture){NFS_PLAYER_INTRO_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;if(!nfs_player_intro_current())goto lost;s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->fontIdentity=f->identity;s->ownerGeneration=s->generation;memcpy(s->fontSha256,sha,32);return 1;
 lost:nfs_player_intro_abandon();return 0;
}
static int nfs_player_intro_color_target(unsigned w,unsigned h){
 GLuint texture=0,fbo=0;GLint maxSize=0,maxViewport[2]={0,0},type=0,name=0;GLenum status=0,error;GLboolean exists=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_intro_context_stage();int ok=0;
 if(nfs_player_intro_renderer.color==g_framebufferTexture||nfs_player_intro_renderer.fbo==g_framebuffer)return 0;
 if(!nfs_player_intro_current()||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));if(w>(unsigned)maxSize||h>(unsigned)maxSize||w>(unsigned)maxViewport[0]||h>(unsigned)maxViewport[1])return 0;
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 if(nfs_player_intro_renderer.color&&nfs_player_intro_renderer.fbo&&nfs_player_intro_renderer.width==w&&nfs_player_intro_renderer.height==h){
  NFS_PLAYER_INTRO_GL(exists=glIsTexture(nfs_player_intro_renderer.color));if(!exists)goto done;NFS_PLAYER_INTRO_GL(exists=glIsFramebuffer(nfs_player_intro_renderer.fbo));if(!exists)goto done;
  NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_intro_renderer.fbo));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_PLAYER_INTRO_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_PLAYER_INTRO_GL(error=glGetError());ok=type==GL_TEXTURE&&(GLuint)name==nfs_player_intro_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
  if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 }
 NFS_PLAYER_INTRO_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_INTRO_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_INTRO_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));NFS_PLAYER_INTRO_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 NFS_PLAYER_INTRO_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_PLAYER_INTRO_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0));NFS_PLAYER_INTRO_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_PLAYER_INTRO_GL(error=glGetError());ok=status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_PLAYER_INTRO_GL(glDeleteFramebuffers(1,&fbo));}if(texture){NFS_PLAYER_INTRO_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;
 if(nfs_player_intro_renderer.fbo){NFS_PLAYER_INTRO_GL(glDeleteFramebuffers(1,&nfs_player_intro_renderer.fbo));}if(nfs_player_intro_renderer.color){NFS_PLAYER_INTRO_GL(glDeleteTextures(1,&nfs_player_intro_renderer.color));}
 nfs_player_intro_renderer.fbo=fbo;nfs_player_intro_renderer.color=texture;nfs_player_intro_renderer.width=w;nfs_player_intro_renderer.height=h;return 1;
 lost:nfs_player_intro_abandon();return 0;
}
static int nfs_player_intro_resource_owner(void*user,const NfsBackgroundResourceOwner*o){
 GLint fbo=0,type=0,name=0;(void)user;if(!o||!nfs_player_intro_context(o->generation,o->contextIdentity)||!nfs_player_intro_renderer.serial||nfs_player_intro_renderer.target.framebuffer!=g_framebuffer)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==g_framebufferTexture;
 lost:return 0;
}
static int nfs_player_intro_cached_tiles(const uint16_t*pixels,const uint32_t*keys,uint32_t ids[300],GLenum*filter){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_player_intro_context_stage();GLint mag=0,min=0;GLboolean exists=0;GLenum error;int ok=0;
 if(!pixels||!keys||!filter||!nfs_player_intro_current())return 0;
 for(unsigned tile=0;tile<300;++tile){uint32_t address=keys[tile];TextureInfo*ti;const unsigned char*bytes;
  if((address&3)||address>TextureMem-2048)return 0;ti=&g_textures[address>>2];
  if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+address||!nfs_logo_native_upload_valid(address,ti))return 0;
  ids[tile]=ti->id;bytes=(const unsigned char*)ti->data;
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t source=pixels[((tile%15)*32+y)*640+(tile/15)*32+x];uint16_t a1555=(uint16_t)(((source&0xf800)>>1)|((source&0x7c0)>>1)|(source&31));uint16_t want,got;a1555=a1555?(uint16_t)(a1555|0x8000):0;want=(uint16_t)((a1555<<1)|(a1555>>15));memcpy(&got,bytes+(y*32+x)*2,2);if(got!=want)return 0;}
 }
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_INTRO_GL(glActiveTexture(GL_TEXTURE0));
 for(unsigned tile=0;tile<300;++tile){NFS_PLAYER_INTRO_GL(exists=glIsTexture(ids[tile]));if(!exists)goto done;NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,ids[tile]));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&mag));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&min));if(tile==0){if(mag!=GL_LINEAR&&mag!=GL_NEAREST)goto done;*filter=(GLenum)mag;}if(mag!=(GLint)*filter||min!=GL_LINEAR)goto done;}
 NFS_PLAYER_INTRO_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;return ok;
 lost:nfs_player_intro_abandon();return 0;
}


static int nfs_player_intro_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,ct=0,cn=0,dt=0,dn=0;(void)user;
 if(!t||!nfs_player_intro_context(t->generation,t->contextIdentity)||t->framebuffer!=nfs_player_intro_renderer.fbo||t->width!=nfs_player_intro_renderer.width||t->height!=nfs_player_intro_renderer.height||nfs_player_intro_renderer.color==nfs_player_intro_renderer.nativeColor)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=t->framebuffer)return 0;
 NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));return ct==GL_TEXTURE&&(GLuint)cn==nfs_player_intro_renderer.color&&dt==GL_RENDERBUFFER&&(GLuint)dn==nfs_player_intro_renderer.nativeDepth;
 lost:return 0;
}
#define NFS_PLAYER_INTRO_LAYER_GL(call) do{if(!nfs_graphics_scene_current(t))return 0;call;if(!nfs_graphics_scene_current(t))return 0;}while(0)
static int nfs_player_intro_draw_layers(NfsGraphicsSceneGpu*gpu,const NfsBackgroundSceneStage*t,const NfsBackgroundSceneLayers*v,const NfsSoftwareTextLayerView*caption){
 NfsBackgroundSceneCompositor*b=gpu->background;NfsLogoVertex logos[18];size_t count=0;float bounds[4];GLint type=0,name=0;GLenum error;
 if(!nfs_bg_stage_layers(v)||!nfs_graphics_scene_font(gpu->sidebarCaption,caption,343,t->generation)||!nfs_graphics_scene_ready(gpu,t)||!nfs_graphics_scene_target(gpu,t)||!nfs_logo_layers(v->logos,3,logos,&count,bounds))return 0;
 NFS_PLAYER_INTRO_LAYER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_INTRO_LAYER_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||(GLuint)name!=nfs_player_intro_renderer.color)return 0;
 for(unsigned i=0;i<8;++i){NFS_PLAYER_INTRO_LAYER_GL(glDisable(nfs_frame_caps[i]));}NFS_PLAYER_INTRO_LAYER_GL(glColorMask(1,1,1,1));NFS_PLAYER_INTRO_LAYER_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_INTRO_LAYER_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_PLAYER_INTRO_LAYER_GL(glDepthMask(GL_FALSE));
 if(!nfs_graphics_scene_quad(b,t,2,0,0,343,480,0,0,1372,1920,v->panelClip,256,1))return 0;
 for(unsigned i=0;i<v->wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->words+i;const NfsBackgroundSceneWord*r=b->regions+w->sourceRole-1;if(!nfs_graphics_scene_quad(b,t,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))return 0;}
 if(count){NFS_PLAYER_INTRO_LAYER_GL(glUseProgram(b->logo.program));NFS_PLAYER_INTRO_LAYER_GL(glUniformMatrix4fv(b->logo.matrix,1,GL_FALSE,t->sampling.projection));NFS_PLAYER_INTRO_LAYER_GL(glUniform1i(b->logo.sampler,0));NFS_PLAYER_INTRO_LAYER_GL(glBindTexture(GL_TEXTURE_2D,b->logo.texture));NFS_PLAYER_INTRO_LAYER_GL(glEnableVertexAttribArray(0));NFS_PLAYER_INTRO_LAYER_GL(glEnableVertexAttribArray(1));NFS_PLAYER_INTRO_LAYER_GL(glDisableVertexAttribArray(2));NFS_PLAYER_INTRO_LAYER_GL(glDisableVertexAttribArray(3));NFS_PLAYER_INTRO_LAYER_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_PLAYER_INTRO_LAYER_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_PLAYER_INTRO_LAYER_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_PLAYER_INTRO_LAYER_GL(error=glGetError());if(error!=GL_NO_ERROR)return 0;}
 return nfs_graphics_scene_text(gpu->sidebarCaption,t,caption);
}

#undef NFS_PLAYER_INTRO_LAYER_GL
static int nfs_player_intro_native_stamp(NfsAudioNativeStamp*out){
 GLint fbo=0,ct=0,cn=0,dt=0,dn=0;
 if(!out||!nfs_player_intro_current()||!nfs_player_intro_renderer.serial||g_framebuffer!=nfs_player_intro_renderer.target.framebuffer||g_framebufferTexture!=nfs_player_intro_renderer.nativeColor||g_framebufferDepth!=nfs_player_intro_renderer.nativeDepth)return 0;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;
 NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));
 if(ct!=GL_TEXTURE||(GLuint)cn!=nfs_player_intro_renderer.nativeColor||dt!=GL_RENDERBUFFER||(GLuint)dn!=nfs_player_intro_renderer.nativeDepth)return 0;
 out->generation=nfs_player_intro_renderer.generation;out->context=nfs_player_intro_renderer.context;out->thread=nfs_player_intro_renderer.thread;out->framebuffer=g_framebuffer;out->width=nfs_player_intro_renderer.target.width;out->height=nfs_player_intro_renderer.target.height;return 1;
 lost:nfs_player_intro_abandon();return 0;
}
static void nfs_player_intro_renderer_created(void){uint64_t generation=nfs_logo_provenance.context;uintptr_t context=(uintptr_t)g_glCtx;if(!nfs_bg_feature()||!generation||!context||SDL_GL_GetCurrentContext()!=g_glCtx)return;if(nfs_player_intro_renderer.generation!=generation)nfs_player_intro_forget_gpu();nfs_player_intro_cancel_pending();nfs_player_intro_renderer.generation=generation;nfs_player_intro_renderer.context=context;nfs_player_intro_renderer.thread=NFS_PLAYER_INTRO_THREAD_TOKEN();(void)nfs_soft_gpu_context_created(&nfs_player_intro_renderer.caption,generation);}
static void nfs_player_intro_renderer_retired(int owner){
 if(owner&&nfs_player_intro_current()){
  if(nfs_player_intro_renderer.caption.ownerGeneration==nfs_player_intro_renderer.generation){if(nfs_player_intro_renderer.caption.program){NFS_PLAYER_INTRO_GL(glDeleteProgram(nfs_player_intro_renderer.caption.program));}if(nfs_player_intro_renderer.caption.texture){NFS_PLAYER_INTRO_GL(glDeleteTextures(1,&nfs_player_intro_renderer.caption.texture));}}
  if(nfs_player_intro_renderer.fbo){NFS_PLAYER_INTRO_GL(glDeleteFramebuffers(1,&nfs_player_intro_renderer.fbo));}if(nfs_player_intro_renderer.color){NFS_PLAYER_INTRO_GL(glDeleteTextures(1,&nfs_player_intro_renderer.color));}
 }
 lost:nfs_player_intro_forget_gpu();nfs_player_intro_cancel_pending();nfs_player_intro_renderer.generation=0;nfs_player_intro_renderer.context=nfs_player_intro_renderer.thread=0;
}
static void nfs_player_intro_renderer_other(void){nfs_player_intro_cancel_pending();}
static void nfs_player_intro_renderer_state(void){if(nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_TAIL)nfs_player_intro_cancel_pending();}
static void nfs_player_intro_renderer_upload_begin(TextureInfo*ti){(void)ti;nfs_player_intro_cancel_pending();}
static void nfs_player_intro_renderer_select(unsigned tmu,uint32_t address,uint32_t evenOdd,const GrTexInfo*info,TextureInfo*ti){
 if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_SLOT0){NfsAudioNativeStamp stamp;if(!info||!ti||ti->hdDigits||!nfs_player_intro_native_stamp(&stamp)||!nfs_player_intro_pass_select(&nfs_player_intro_renderer.pass,&stamp,nfs_logo_provenance.uploads,tmu,address,ti->id,evenOdd,ti->fmt,ti->size,info->smallLod,info->largeLod,info->aspectRatio))nfs_player_intro_cancel_pending();}
 else if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_READY||nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_PRESENTED)nfs_player_intro_cancel_pending();
}
static NfsAudioNativeVertex nfs_player_intro_vertex(const GrVertex*v){NfsAudioNativeVertex o;memset(&o,0,sizeof o);if(v){o.x=v->x;o.y=v->y;o.r=v->r;o.g=v->g;o.b=v->b;o.a=v->a;o.oow=v->oow;o.sow=v->tmuvtx[0].sow;o.tow=v->tmuvtx[0].tow;}return o;}
static void nfs_player_intro_renderer_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_SLOT0){NfsAudioNativeStamp stamp;NfsAudioNativeVertex x=nfs_player_intro_vertex(a),y=nfs_player_intro_vertex(b),z=nfs_player_intro_vertex(c);if(!a||!b||!c||!nfs_player_intro_native_stamp(&stamp)||!nfs_player_intro_pass_triangle(&nfs_player_intro_renderer.pass,&stamp,nfs_logo_provenance.uploads,&x,&y,&z,g_textureEnabled,g_fogEnabled?1u:0u))nfs_player_intro_cancel_pending();}
 else if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_TAIL){if(!a||!b||!c||a->oow!=1||b->oow!=1||c->oow!=1||g_fogEnabled)nfs_player_intro_cancel_pending();}
 else if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_READY||nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_PRESENTED)nfs_player_intro_cancel_pending();
}
int nfs_private_player_intro_begin(const NfsPlayerIntroFrameView*v){
 NfsNativeLogoTargetProof target,repeat;NfsBackgroundSceneLayers layers;NfsGraphicsSceneGpu gpu;NfsBackgroundSceneStage guard,stage;NfsBgSaved saved;NfsAudioNativeStamp stamp;uint32_t ids[300];GLenum filter=GL_LINEAR,error;int ok=0;
 if(!v||v->schema!=NFS_PLAYER_INTRO_RUNTIME_SCHEMA||(v->form!=0x4d3050&&v->form!=0x4d3100)||!v->requestSerial||v->requestSerial<=nfs_player_intro_renderer.lastSerial||!v->uploadEvent||!v->sourceOwner||!v->sourceBitmap||!v->paddedOriginal565||v->wordsCount!=307200||!nfs_bg_view(&v->inheritedMain)||v->caption.logicalWidth!=343||v->caption.logicalHeight!=480||!nfs_bg_feature()||!nfs_player_intro_current())return 0;
 if(nfs_player_intro_renderer.serial){nfs_player_intro_cancel_pending();return 0;}
 drawTriangles();if(!nfs_player_intro_read_target(&target,v->requestSerial)||!nfs_player_intro_cached_tiles(v->paddedOriginal565,v->keys,ids,&filter))return 0;
 target.sampling.magFilter=filter;nfs_player_intro_renderer.serial=v->requestSerial;nfs_player_intro_renderer.target=target;nfs_player_intro_renderer.nativeColor=g_framebufferTexture;nfs_player_intro_renderer.nativeDepth=g_framebufferDepth;nfs_player_intro_renderer.uploadEvent=v->uploadEvent;
 {NfsBackgroundResourceOwner owner={nfs_player_intro_renderer.generation,nfs_player_intro_renderer.context,nfs_player_intro_resource_owner,NULL};if(!nfs_bg_scene_prepare_owned(&nfs_bg_compositor,&owner))goto refused;}
 if(!nfs_player_intro_font_prepare(&nfs_player_intro_renderer.caption,&v->caption.font)||!nfs_player_intro_color_target(target.width,target.height))goto refused;
 for(unsigned i=0;i<300;++i){if(ids[i]==nfs_player_intro_renderer.color||ids[i]==nfs_player_intro_renderer.caption.texture||ids[i]==nfs_bg_compositor.logo.texture)goto refused;for(unsigned j=0;j<5;++j)if(ids[i]==nfs_bg_compositor.textures[j])goto refused;}
 if(!nfs_player_intro_read_target(&repeat,v->requestSerial))goto refused;repeat.sampling.magFilter=filter;if(memcmp(&repeat,&target,sizeof target))goto refused;
 guard=nfs_player_intro_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_INTRO_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,nfs_player_intro_renderer.color));NFS_PLAYER_INTRO_GL(glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,target.width,target.height));NFS_PLAYER_INTRO_GL(error=glGetError());if(error!=GL_NO_ERROR){if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;goto refused;}
 NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_intro_renderer.fbo));NFS_PLAYER_INTRO_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,nfs_player_intro_renderer.nativeDepth));NFS_PLAYER_INTRO_GL(glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]));
 memset(&stage,0,sizeof stage);stage.generation=nfs_player_intro_renderer.generation;stage.contextIdentity=nfs_player_intro_renderer.context;stage.framebuffer=nfs_player_intro_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_player_intro_stage_owner;
 memset(&layers,0,sizeof layers);layers.mapRole=v->inheritedMain.mapRole;layers.wordCount=v->inheritedMain.wordCount;memcpy(layers.panelClip,v->inheritedMain.panelClip,sizeof layers.panelClip);memcpy(layers.words,v->inheritedMain.words,sizeof layers.words);memcpy(layers.logos,v->inheritedMain.logos,sizeof layers.logos);gpu.background=&nfs_bg_compositor;gpu.sidebarCaption=gpu.text=&nfs_player_intro_renderer.caption;
 ok=nfs_player_intro_draw_layers(&gpu,&stage,&layers,&v->caption);if(!nfs_player_intro_current())goto lost;if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)goto refused;
 if(!nfs_player_intro_read_target(&repeat,v->requestSerial))goto refused;repeat.sampling.magFilter=filter;if(memcmp(&repeat,&target,sizeof target)||!nfs_player_intro_native_stamp(&stamp)||!nfs_player_intro_pass_begin(&nfs_player_intro_renderer.pass,&stamp,v->requestSerial,nfs_logo_provenance.uploads,v->keys,ids))goto refused;
 nfs_player_intro_renderer.phase=NFS_PLAYER_INTRO_SLOT0;nfs_player_intro_renderer.lastSerial=v->requestSerial;nfs_player_intro_trace("begin-accepted");return 1;
 refused:if(!nfs_player_intro_current())goto lost;nfs_player_intro_cancel_pending();return 0;
 lost:nfs_player_intro_abandon();return 0;
}
int nfs_private_player_intro_slot0_finish(uint64_t serial){
 NfsNativeLogoTargetProof target;NfsAudioNativeStamp stamp;
 if(!serial||serial!=nfs_player_intro_renderer.serial||nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_SLOT0||!nfs_player_intro_current())return 0;
 drawTriangles();if(!nfs_player_intro_read_target(&target,serial))goto refused;target.sampling.magFilter=nfs_player_intro_renderer.target.sampling.magFilter;
 if(memcmp(&target,&nfs_player_intro_renderer.target,sizeof target)||!nfs_player_intro_native_stamp(&stamp)||!nfs_player_intro_pass_finish(&nfs_player_intro_renderer.pass,&stamp,nfs_logo_provenance.uploads,serial))goto refused;
 nfs_player_intro_renderer.phase=NFS_PLAYER_INTRO_TAIL;nfs_player_intro_trace("slot0-complete");return 1;
 refused:nfs_player_intro_cancel_pending();return 0;
}

/* Called immediately AFTER the original glDrawArrays while its exact shader
   uniforms/attributes/textures are still live, BEFORE native mark reset. No
   native command is suppressed. Flat UI depth is tested against the SAME
   original depth attachment; the mirror never writes it. */
static int nfs_player_intro_array(unsigned index,GLint size,GLenum type,GLboolean normalized,const void*pointer){
 GLint enabled=0,actualSize=0,actualType=0,normal=0,stride=0,buffer=0;void*actual=NULL;
 if(index>3)return 0;
 NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&enabled));NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_SIZE,&actualSize));NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_TYPE,&actualType));NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&normal));NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&stride));NFS_PLAYER_INTRO_GL(glGetVertexAttribiv(index,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&buffer));NFS_PLAYER_INTRO_GL(glGetVertexAttribPointerv(index,GL_VERTEX_ATTRIB_ARRAY_POINTER,&actual));return enabled&&actualSize==size&&(GLenum)actualType==type&&normal==(GLint)normalized&&!stride&&!buffer&&actual==pointer;
 lost:return 0;
}
static void nfs_player_intro_renderer_tail_batch(unsigned count){
 NfsNativeLogoTargetProof target;NfsAudioNativeStamp stamp;NfsBackgroundSceneStage stage;GLint program=0,source=0,dest=0,equation=0,active=0,binding=0;GLboolean depthMask=0,enabled=0;GLenum error;int switched=0;
 if(nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_TAIL)return;
 if(!count||count>MaxTriangles||count!=g_trianglesCount||!nfs_player_intro_current()||g_fogEnabled)goto refused;
 for(unsigned i=0;i<count;++i)for(unsigned j=0;j<3;++j)if(g_vertices[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].q!=1||!isfinite(g_vertices[i].vertex[j].x)||!isfinite(g_vertices[i].vertex[j].y)||g_vertices[i].vertex[j].x<-32768||g_vertices[i].vertex[j].x>32767||g_vertices[i].vertex[j].y<-32768||g_vertices[i].vertex[j].y>32767)goto refused;
 if(!nfs_player_intro_read_target(&target,nfs_player_intro_renderer.serial)||!nfs_player_intro_native_stamp(&stamp))goto refused;target.sampling.magFilter=nfs_player_intro_renderer.target.sampling.magFilter;if(memcmp(&target,&nfs_player_intro_renderer.target,sizeof target))goto refused;
 if(!nfs_player_intro_array((unsigned)g_aPositionLoc,3,GL_FLOAT,GL_FALSE,g_vertices)||!nfs_player_intro_array((unsigned)g_aTexCoordLoc,4,GL_FLOAT,GL_FALSE,g_textureCoord)||!nfs_player_intro_array((unsigned)g_aColorLoc,4,GL_UNSIGNED_BYTE,GL_TRUE,g_colorValues))goto refused;
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));if(program!=(GLint)g_shaderProgram)goto refused;
 NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_STENCIL_TEST));if(enabled)goto refused;NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_CULL_FACE));if(enabled)goto refused;NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_POLYGON_OFFSET_FILL));if(enabled)goto refused;
 NFS_PLAYER_INTRO_GL(enabled=glIsEnabled(GL_BLEND));if(enabled){NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_BLEND_SRC_RGB,&source));NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_BLEND_DST_RGB,&dest));NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_BLEND_EQUATION_RGB,&equation));if(source!=GL_SRC_ALPHA||(dest!=GL_ONE_MINUS_SRC_ALPHA&&dest!=GL_ONE)||equation!=GL_FUNC_ADD)goto refused;}
 NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&active));if(active!=GL_TEXTURE0)goto refused;NFS_PLAYER_INTRO_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));if((GLuint)binding==nfs_player_intro_renderer.color)goto refused;
 NFS_PLAYER_INTRO_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask));NFS_PLAYER_INTRO_GL(error=glGetError());if(error!=GL_NO_ERROR)goto refused;
 memset(&stage,0,sizeof stage);stage.generation=nfs_player_intro_renderer.generation;stage.contextIdentity=nfs_player_intro_renderer.context;stage.framebuffer=nfs_player_intro_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.ownerCurrent=nfs_player_intro_stage_owner;
 NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_intro_renderer.fbo));switched=1;
 if(!nfs_player_intro_stage_owner(NULL,&stage))goto restore_refused;
 NFS_PLAYER_INTRO_GL(glDepthMask(GL_FALSE));NFS_PLAYER_INTRO_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)(count*3)));NFS_PLAYER_INTRO_GL(glDepthMask(depthMask));NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,target.framebuffer));switched=0;NFS_PLAYER_INTRO_GL(error=glGetError());if(error!=GL_NO_ERROR)goto refused;
 if(nfs_player_intro_renderer.tailTriangles>UINT_MAX-count)goto refused;nfs_player_intro_renderer.tailTriangles+=count;return;
 restore_refused:if(switched){NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,target.framebuffer));}
 refused:nfs_player_intro_cancel_pending();return;
 lost:nfs_player_intro_abandon();
}
int nfs_private_player_intro_tail_finish(uint64_t serial){
 NfsNativeLogoTargetProof target;
 if(!serial||serial!=nfs_player_intro_renderer.serial||nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_TAIL||!nfs_player_intro_current())return 0;
 drawTriangles();if(nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_TAIL||!nfs_player_intro_read_target(&target,serial))goto refused;target.sampling.magFilter=nfs_player_intro_renderer.target.sampling.magFilter;if(memcmp(&target,&nfs_player_intro_renderer.target,sizeof target))goto refused;
 nfs_player_intro_renderer.phase=NFS_PLAYER_INTRO_READY;nfs_player_intro_trace("tail-ready");return 1;
 refused:nfs_player_intro_cancel_pending();return 0;
}
/* Presentation runs AFTER the physical front/back swap. It selects a texture
   for the existing display quad, never copies HD pixels into either native
   color buffer. Framebuffer/texture names alone never survive owner retirement. */
static uint32_t nfs_player_intro_renderer_presentation_texture(uint32_t frontFbo,uint32_t frontTexture){
 NfsBgSaved saved;NfsBackgroundSceneStage guard;GLint ct=0,cn=0,dt=0,dn=0,minFilter=0,magFilter=0,wrapS=0,wrapT=0;GLenum status=0,error=GL_NO_ERROR;GLboolean exists=0;int ok=0;
 if(nfs_player_intro_renderer.phase!=NFS_PLAYER_INTRO_READY)return frontTexture;
 if(!nfs_player_intro_current()||frontFbo!=nfs_player_intro_renderer.target.framebuffer||frontTexture!=nfs_player_intro_renderer.nativeColor||g_framebufferHeight!=(int)nfs_player_intro_renderer.target.height||g_framebufferDepth!=nfs_player_intro_renderer.nativeDepth||!nfs_player_intro_renderer.color||nfs_player_intro_renderer.color==frontTexture)goto refused;
 guard=nfs_player_intro_context_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;
 NFS_PLAYER_INTRO_GL(exists=glIsTexture(nfs_player_intro_renderer.color));if(!exists)goto restore;
 NFS_PLAYER_INTRO_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,frontTexture));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&minFilter));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&magFilter));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,&wrapS));NFS_PLAYER_INTRO_GL(glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,&wrapT));
 if((minFilter!=GL_LINEAR&&minFilter!=GL_NEAREST)||(magFilter!=GL_LINEAR&&magFilter!=GL_NEAREST))goto restore;
 NFS_PLAYER_INTRO_GL(glBindTexture(GL_TEXTURE_2D,nfs_player_intro_renderer.color));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minFilter));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,magFilter));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrapS));NFS_PLAYER_INTRO_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapT));
 NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,frontFbo));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&dt));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&dn));NFS_PLAYER_INTRO_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(ct!=GL_TEXTURE||(GLuint)cn!=frontTexture||dt!=GL_RENDERBUFFER||(GLuint)dn!=nfs_player_intro_renderer.nativeDepth||status!=GL_FRAMEBUFFER_COMPLETE)goto restore;
 NFS_PLAYER_INTRO_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_intro_renderer.fbo));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&ct));NFS_PLAYER_INTRO_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&cn));NFS_PLAYER_INTRO_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_PLAYER_INTRO_GL(error=glGetError());ok=ct==GL_TEXTURE&&(GLuint)cn==nfs_player_intro_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 restore:if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok)goto refused;nfs_player_intro_renderer.phase=NFS_PLAYER_INTRO_PRESENTED;nfs_player_intro_trace("display-selected");return nfs_player_intro_renderer.color;
 refused:nfs_player_intro_cancel_pending();return frontTexture;
 lost:nfs_player_intro_abandon();return frontTexture;
}
static void nfs_player_intro_renderer_after_present(void){if(nfs_player_intro_renderer.phase==NFS_PLAYER_INTRO_PRESENTED)nfs_player_intro_cancel_pending();}
int nfs_private_player_intro_cancel(uint64_t serial){if(!serial||serial!=nfs_player_intro_renderer.serial||!nfs_player_intro_current())return 0;nfs_player_intro_cancel_pending();return 1;}
int nfs_private_player_intro_end(uint64_t serial){return nfs_private_player_intro_cancel(serial);}
#undef NFS_PLAYER_INTRO_GL
