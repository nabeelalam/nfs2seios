// SPDX-License-Identifier: MIT
#if defined(__APPLE__)
#include <OpenGLES/ES2/gl.h>
#else
#include <GLES2/gl2.h>
#endif
#include "ControlsSteeringBarGpu.h"
#include "ControlsSteeringBarIdentity.h"
#include "../TextureDigest.h"
#include <math.h>
#include <string.h>
#ifndef NFS_CONTROLS_STEERING_BAR_EXTERNAL_SCENE_IMPLEMENTATION
/* Keep this standalone C translation unit independent of the compositor's
   implementation and avoid exporting a second neutral sampling symbol. */
#define nfs_frame_gpu_legacy_sampling nfs_controls_steering_bar_private_sampling
#include "SceneGpuSupport.inc"
#undef nfs_frame_gpu_legacy_sampling
#endif
static int nfs_sbar_current(const NfsBackgroundSceneStage*t){return t&&t->generation&&t->contextIdentity&&t->ownerCurrent&&t->ownerCurrent(t->ownerUser,t);}
static int nfs_sbar_valid(const NfsControlsSteeringBarView*v){return v&&v->schema==1&&v->level<4&&v->width==1424&&v->height==196&&v->rgba&&v->bytes==NFS_CONTROLS_STEERING_BAR_BYTES&&v->left==280.f&&v->top==280.f&&v->right==636.f&&v->bottom==329.f&&!memcmp(v->sha256,nfs_controls_steering_bar_sha256[v->level],32);}
void nfs_controls_steering_bar_gpu_forget(NfsControlsSteeringBarGpu*s){if(s){memset(s,0,sizeof(*s));s->sampler=s->matrix=-1;}}
typedef struct NfsControlsSteeringBarSaved {NfsFrameSaved common;GLint depthFunc,scissor[4];GLboolean depthMask;GLfloat depthClear;} NfsControlsSteeringBarSaved;
#define NFS_SBAR_STATE_GL(call) do{if(!nfs_sbar_current(t))return 0;call;if(!nfs_sbar_current(t))return 0;}while(0)
static int nfs_sbar_save(NfsControlsSteeringBarSaved*v,const NfsBackgroundSceneStage*t){NfsFrameSaved*s=&v->common;unsigned i;NFS_SBAR_STATE_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&s->program));NFS_SBAR_STATE_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&s->active));NFS_SBAR_STATE_GL(glActiveTexture(GL_TEXTURE0));NFS_SBAR_STATE_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&s->texture));NFS_SBAR_STATE_GL(glActiveTexture(s->active));NFS_SBAR_STATE_GL(glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&s->buffer));NFS_SBAR_STATE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&s->fbo));NFS_SBAR_STATE_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&s->rbo));NFS_SBAR_STATE_GL(glGetIntegerv(GL_VIEWPORT,s->viewport));NFS_SBAR_STATE_GL(glGetIntegerv(GL_PACK_ALIGNMENT,&s->pack));NFS_SBAR_STATE_GL(glGetIntegerv(GL_UNPACK_ALIGNMENT,&s->unpack));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_SRC_RGB,&s->sr));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_DST_RGB,&s->dr));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_SRC_ALPHA,&s->sa));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_DST_ALPHA,&s->da));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_EQUATION_RGB,&s->eqr));NFS_SBAR_STATE_GL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&s->eqa));NFS_SBAR_STATE_GL(glGetBooleanv(GL_COLOR_WRITEMASK,s->mask));
 for(i=0;i<8;++i){NFS_SBAR_STATE_GL(s->caps[i]=glIsEnabled(nfs_frame_caps[i]));}for(i=0;i<4;++i){NfsSoftwareTextGpuAttrib*a=s->attributes+i;NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&a->enabled));NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_SIZE,&a->size));NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_TYPE,&a->type));NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&a->normalized));NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&a->stride));NFS_SBAR_STATE_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&a->buffer));NFS_SBAR_STATE_GL(glGetVertexAttribPointerv(i,GL_VERTEX_ATTRIB_ARRAY_POINTER,&a->pointer));NFS_SBAR_STATE_GL(glGetVertexAttribfv(i,GL_CURRENT_VERTEX_ATTRIB,a->current));}NFS_SBAR_STATE_GL(glGetIntegerv(GL_DEPTH_FUNC,&v->depthFunc));NFS_SBAR_STATE_GL(glGetIntegerv(GL_SCISSOR_BOX,v->scissor));NFS_SBAR_STATE_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&v->depthMask));NFS_SBAR_STATE_GL(glGetFloatv(GL_DEPTH_CLEAR_VALUE,&v->depthClear));return 1;}
static int nfs_sbar_restore(const NfsControlsSteeringBarSaved*v,const NfsBackgroundSceneStage*t){const NfsFrameSaved*s=&v->common;unsigned i;NFS_SBAR_STATE_GL(glDepthFunc(v->depthFunc));NFS_SBAR_STATE_GL(glDepthMask(v->depthMask));NFS_SBAR_STATE_GL(glClearDepthf(v->depthClear));NFS_SBAR_STATE_GL(glScissor(v->scissor[0],v->scissor[1],v->scissor[2],v->scissor[3]));for(i=0;i<4;++i){const NfsSoftwareTextGpuAttrib*a=s->attributes+i;NFS_SBAR_STATE_GL(glBindBuffer(GL_ARRAY_BUFFER,a->buffer));NFS_SBAR_STATE_GL(glVertexAttribPointer(i,a->size,a->type,a->normalized,a->stride,a->pointer));NFS_SBAR_STATE_GL(glVertexAttrib4fv(i,a->current));if(a->enabled){NFS_SBAR_STATE_GL(glEnableVertexAttribArray(i));}else{NFS_SBAR_STATE_GL(glDisableVertexAttribArray(i));}}NFS_SBAR_STATE_GL(glBindBuffer(GL_ARRAY_BUFFER,s->buffer));NFS_SBAR_STATE_GL(glActiveTexture(GL_TEXTURE0));NFS_SBAR_STATE_GL(glBindTexture(GL_TEXTURE_2D,s->texture));NFS_SBAR_STATE_GL(glActiveTexture(s->active));NFS_SBAR_STATE_GL(glUseProgram(s->program));NFS_SBAR_STATE_GL(glBindFramebuffer(GL_FRAMEBUFFER,s->fbo));NFS_SBAR_STATE_GL(glBindRenderbuffer(GL_RENDERBUFFER,s->rbo));NFS_SBAR_STATE_GL(glViewport(s->viewport[0],s->viewport[1],s->viewport[2],s->viewport[3]));NFS_SBAR_STATE_GL(glPixelStorei(GL_PACK_ALIGNMENT,s->pack));NFS_SBAR_STATE_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,s->unpack));NFS_SBAR_STATE_GL(glColorMask(s->mask[0],s->mask[1],s->mask[2],s->mask[3]));NFS_SBAR_STATE_GL(glBlendEquationSeparate(s->eqr,s->eqa));NFS_SBAR_STATE_GL(glBlendFuncSeparate(s->sr,s->dr,s->sa,s->da));for(i=0;i<8;++i){if(s->caps[i]){NFS_SBAR_STATE_GL(glEnable(nfs_frame_caps[i]));}else{NFS_SBAR_STATE_GL(glDisable(nfs_frame_caps[i]));}}return 1;}

#undef NFS_SBAR_STATE_GL
#define NFS_SBAR_GL(call) do{if(!nfs_sbar_current(t))goto lost;call;if(!nfs_sbar_current(t))goto lost;}while(0)
int nfs_controls_steering_bar_gpu_prepare(NfsControlsSteeringBarGpu*s,const NfsControlsSteeringBarView*v,const NfsBackgroundSceneStage*t){
 GLuint vs=0,fs=0,p=0,tex=0;GLint status=0,matrix=-1,sampler=-1,maxSize=0;GLenum error=GL_NO_ERROR;NfsControlsSteeringBarSaved saved;int ok=0;unsigned char sha[32];size_t i;
 const char*vertex="attribute vec4 vertex;uniform mat4 matrix;varying highp vec2 uv;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;}";
 const char*fragment="precision highp float;uniform sampler2D image;varying highp vec2 uv;void main(){gl_FragColor=texture2D(image,uv);}";
 if(!s||!nfs_sbar_valid(v)||!nfs_sbar_current(t)||(s->generation&&(s->generation!=t->generation||s->contextIdentity!=t->contextIdentity)))return 0;
 if(s->textures[v->level]){GLboolean exists=0;if(s->rgba[v->level]!=v->rgba||!s->program)return 0;NFS_SBAR_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_SBAR_GL(exists=glIsTexture(s->textures[v->level]));return exists!=0;}
 if(!nfs_texture_sha256(v->rgba,v->bytes,sha)||memcmp(sha,nfs_controls_steering_bar_sha256[v->level],32))return 0;
 for(i=0;i<v->bytes;i+=4)if(v->rgba[i]>v->rgba[i+3]||v->rgba[i+1]>v->rgba[i+3]||v->rgba[i+2]>v->rgba[i+3])return 0;
 for(unsigned y=0;y<196;++y)for(unsigned x=0;x<1424;++x)if((x<4||x>=1420||y<4||y>=192)&&v->rgba[((size_t)y*1424+x)*4+3])return 0;
 NFS_SBAR_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_SBAR_GL(error=glGetError());if(maxSize<1424||error!=GL_NO_ERROR)return 0;
 if(!nfs_sbar_save(&saved,t))goto lost;
 if(!s->program){
  NFS_SBAR_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_SBAR_GL(glShaderSource(vs,1,&vertex,NULL));NFS_SBAR_GL(glCompileShader(vs));NFS_SBAR_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
  NFS_SBAR_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_SBAR_GL(glShaderSource(fs,1,&fragment,NULL));NFS_SBAR_GL(glCompileShader(fs));NFS_SBAR_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
  NFS_SBAR_GL(p=glCreateProgram());if(!p)goto done;NFS_SBAR_GL(glAttachShader(p,vs));NFS_SBAR_GL(glAttachShader(p,fs));NFS_SBAR_GL(glBindAttribLocation(p,0,"vertex"));NFS_SBAR_GL(glLinkProgram(p));NFS_SBAR_GL(glGetProgramiv(p,GL_LINK_STATUS,&status));if(!status)goto done;
  NFS_SBAR_GL(matrix=glGetUniformLocation(p,"matrix"));NFS_SBAR_GL(sampler=glGetUniformLocation(p,"image"));if(matrix<0||sampler<0)goto done;
 }
 NFS_SBAR_GL(glActiveTexture(GL_TEXTURE0));NFS_SBAR_GL(glGenTextures(1,&tex));if(!tex)goto done;NFS_SBAR_GL(glBindTexture(GL_TEXTURE_2D,tex));
 NFS_SBAR_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_SBAR_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_SBAR_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_SBAR_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_SBAR_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_SBAR_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1424,196,0,GL_RGBA,GL_UNSIGNED_BYTE,v->rgba));NFS_SBAR_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_SBAR_GL(glDeleteShader(vs));}if(fs){NFS_SBAR_GL(glDeleteShader(fs));}if(!ok){if(p){NFS_SBAR_GL(glDeleteProgram(p));}if(tex){NFS_SBAR_GL(glDeleteTextures(1,&tex));}}
 if(!nfs_sbar_restore(&saved,t))goto lost;if(!ok)return 0;
 if(p){s->program=p;s->matrix=matrix;s->sampler=sampler;}s->textures[v->level]=tex;s->rgba[v->level]=v->rgba;s->generation=t->generation;s->contextIdentity=t->contextIdentity;return 1;
 lost:nfs_controls_steering_bar_gpu_forget(s);return 0;
}
static int nfs_sbar_scissor(const NfsBackgroundSceneStage*t){GLint sc[4];GLboolean enabled=0;const GLfloat*m=t->sampling.projection;const GLint*vp=t->sampling.viewport;double x0,x1,y0,y1;NFS_SBAR_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(!enabled)return 1;x0=vp[0]+(m[0]*280+m[12]+1.0)*vp[2]*.5;x1=vp[0]+(m[0]*636+m[12]+1.0)*vp[2]*.5;y0=vp[1]+(m[5]*329+m[13]+1.0)*vp[3]*.5;y1=vp[1]+(m[5]*280+m[13]+1.0)*vp[3]*.5;NFS_SBAR_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));return sc[2]>=0&&sc[3]>=0&&sc[0]<=floor(x0)&&sc[1]<=floor(y0)&&(double)sc[0]+sc[2]>=ceil(x1)&&(double)sc[1]+sc[3]>=ceil(y1);lost:return 0;}
int nfs_controls_steering_bar_gpu_draw_into_staging(NfsControlsSteeringBarGpu*s,const NfsControlsSteeringBarView*v,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0,viewport[4];GLboolean exists=0;GLenum error;NfsControlsSteeringBarSaved saved;int ok;
 static const GLfloat q[]={280,280,0,0,636,280,1,0,280,329,0,1,280,329,0,1,636,280,1,0,636,329,1,1};
 if(!s||!nfs_sbar_valid(v)||!nfs_sbar_current(t)||!t->framebuffer||s->generation!=t->generation||s->contextIdentity!=t->contextIdentity||!s->program||!s->textures[v->level]||s->rgba[v->level]!=v->rgba||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;
 NFS_SBAR_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_SBAR_GL(glGetIntegerv(GL_VIEWPORT,viewport));if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport))return 0;if(!nfs_sbar_scissor(t)){if(!nfs_sbar_current(t))goto lost;return 0;}
 NFS_SBAR_GL(error=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(error!=GL_FRAMEBUFFER_COMPLETE)return 0;NFS_SBAR_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_SBAR_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name)return 0;for(unsigned i=0;i<4;++i)if((GLuint)name==s->textures[i])return 0;
 NFS_SBAR_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_SBAR_GL(exists=glIsTexture(s->textures[v->level]));if(!exists)return 0;NFS_SBAR_GL(error=glGetError());if(error!=GL_NO_ERROR)return 0;
 if(!nfs_sbar_save(&saved,t))goto lost;
 for(unsigned i=0;i<8;++i){NFS_SBAR_GL(glDisable(nfs_frame_caps[i]));}NFS_SBAR_GL(glColorMask(1,1,1,1));NFS_SBAR_GL(glDepthMask(GL_FALSE));NFS_SBAR_GL(glActiveTexture(GL_TEXTURE0));NFS_SBAR_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_SBAR_GL(glUseProgram(s->program));NFS_SBAR_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection));NFS_SBAR_GL(glUniform1i(s->sampler,0));NFS_SBAR_GL(glBindTexture(GL_TEXTURE_2D,s->textures[v->level]));NFS_SBAR_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_SBAR_GL(glDisableVertexAttribArray(i));}NFS_SBAR_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));NFS_SBAR_GL(glEnable(GL_BLEND));NFS_SBAR_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_SBAR_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));NFS_SBAR_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_SBAR_GL(error=glGetError());ok=error==GL_NO_ERROR;if(!nfs_sbar_restore(&saved,t))goto lost;return ok;
 lost:nfs_controls_steering_bar_gpu_forget(s);return 0;
}
void nfs_controls_steering_bar_gpu_retire(NfsControlsSteeringBarGpu*s,const NfsBackgroundSceneStage*t,int ownerCurrent){if(!s||!t||!t->generation||s->generation!=t->generation||s->contextIdentity!=t->contextIdentity)return;if(ownerCurrent&&nfs_sbar_current(t)){if(s->program){glDeleteProgram(s->program);if(!nfs_sbar_current(t))goto lost;}for(unsigned i=0;i<4;++i)if(s->textures[i]){if(!nfs_sbar_current(t))goto lost;glDeleteTextures(1,s->textures+i);if(!nfs_sbar_current(t))goto lost;}}lost:nfs_controls_steering_bar_gpu_forget(s);}
#undef NFS_SBAR_GL
