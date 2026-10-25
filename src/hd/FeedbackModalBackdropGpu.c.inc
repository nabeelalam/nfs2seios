// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_BACKDROP_GPU_IMPLEMENTATION
#define NFS_FEEDBACK_MODAL_BACKDROP_GPU_IMPLEMENTATION
#include "FeedbackModalBackdropGpu.h"
#include <stdlib.h>
/* Mechanical backdrop only; no source/native/queue/presentation authority.
   Prerequisites: neutral stage types, digest, and guarded nested-caption
   current/save/restore/blend helpers. Caller retains the immutable authored
   provider, current scene/native witness and exhaustive alias inventory, and
   owns the unpublished target. Every failed transaction must be discarded. */
static int nfs_feedback_modal_backdrop_gpu_empty(const NfsFeedbackModalBackdropArtView*v){
 unsigned i,j;if(!v||v->schema||v->count)return 0;for(i=0;i<2;++i){const NfsFeedbackModalBackdropArtImage*a=v->images+i;if(a->schema||a->role||a->width||a->height||a->straightRgba||a->rgba||a->bytes)return 0;for(j=0;j<32;++j)if(a->sha256[j])return 0;}return 1;
}
static int nfs_feedback_modal_backdrop_gpu_valid(const NfsFeedbackModalBackdropArtView*v){
 static const unsigned widths[2]={1376,468},heights[2]={188,244};unsigned i;unsigned char sha[32];
 if(nfs_feedback_modal_backdrop_gpu_empty(v))return 1;
 if(!v||v->schema!=NFS_FEEDBACK_MODAL_BACKDROP_ART_SCHEMA||v->count!=2)return 0;
 for(i=0;i<2;++i){const NfsFeedbackModalBackdropArtImage*a=v->images+i;
  if(a->schema!=1||a->role!=i+1||a->width!=widths[i]||a->height!=heights[i]||a->straightRgba!=1||a->bytes!=(size_t)widths[i]*heights[i]*4||!a->rgba||!nfs_texture_sha256(a->rgba,a->bytes,sha)||memcmp(sha,a->sha256,32))return 0;
 }return 1;
}
static void nfs_feedback_modal_backdrop_gpu_key(const NfsFeedbackModalBackdropArtView*v,unsigned char out[32]){
 unsigned char b[256];size_t used=0;unsigned i;
#define NFS_FMB_KEY(x) do{memcpy(b+used,&(x),sizeof(x));used+=sizeof(x);}while(0)
 NFS_FMB_KEY(v->schema);NFS_FMB_KEY(v->count);for(i=0;i<2;++i){const NfsFeedbackModalBackdropArtImage*a=v->images+i;NFS_FMB_KEY(a->schema);NFS_FMB_KEY(a->role);NFS_FMB_KEY(a->width);NFS_FMB_KEY(a->height);NFS_FMB_KEY(a->straightRgba);NFS_FMB_KEY(a->bytes);memcpy(b+used,a->sha256,32);used+=32;}nfs_texture_sha256(b,used,out);
#undef NFS_FMB_KEY
}
static int nfs_feedback_modal_backdrop_gpu_name(GLuint name,const GLuint*names,size_t count){size_t i;for(i=0;i<count;++i)if(name&&name==names[i])return 1;return 0;}
static int nfs_feedback_modal_backdrop_gpu_alias(const NfsFeedbackModalBackdropGpu*s,const NfsFeedbackModalBackdropGpuAliases*a){
 unsigned i;if(!s||!a||a->programCount>128||a->textureCount>128||(a->programCount&&!a->programs)||(a->textureCount&&!a->textures))return 1;
 if(nfs_feedback_modal_backdrop_gpu_name(s->program,a->programs,a->programCount))return 1;
 for(i=0;i<2;++i)if(s->textures[i]&&(nfs_feedback_modal_backdrop_gpu_name(s->textures[i],a->textures,a->textureCount)||(i&&s->textures[0]==s->textures[1])))return 1;
 return 0;
}
static void nfs_feedback_modal_backdrop_gpu_forget(NfsFeedbackModalBackdropGpu*s){if(s)memset(s,0,sizeof *s);}
#define NFS_FMB_GL(call) do{if(!nfs_nested_caption_current(t))goto lost;call;if(!nfs_nested_caption_current(t))goto lost;}while(0)
static int nfs_feedback_modal_backdrop_gpu_retire(NfsFeedbackModalBackdropGpu*s,const NfsBackgroundSceneStage*t,int owner,const NfsFeedbackModalBackdropGpuAliases*aliases){unsigned i;if(!s)return 0;if(nfs_feedback_modal_backdrop_gpu_alias(s,aliases)){nfs_feedback_modal_backdrop_gpu_forget(s);return 0;}if(owner&&t&&s->generation==t->generation&&nfs_nested_caption_current(t)){if(s->program){NFS_FMB_GL(glDeleteProgram(s->program));}for(i=0;i<2;++i)if(s->textures[i]){NFS_FMB_GL(glDeleteTextures(1,s->textures+i));}}nfs_feedback_modal_backdrop_gpu_forget(s);return 1;lost:nfs_feedback_modal_backdrop_gpu_forget(s);return 0;}
static int nfs_feedback_modal_backdrop_gpu_ready(NfsFeedbackModalBackdropGpu*s,const NfsFeedbackModalBackdropArtView*v,const NfsBackgroundSceneStage*t,const NfsFeedbackModalBackdropGpuAliases*aliases){unsigned i;GLboolean alive;unsigned char key[32];if(!s||nfs_feedback_modal_backdrop_gpu_alias(s,aliases)||!nfs_nested_caption_current(t)||!nfs_feedback_modal_backdrop_gpu_valid(v)||nfs_feedback_modal_backdrop_gpu_empty(v)||s->generation!=t->generation||!s->program||s->matrix<0||s->sampler<0||s->kind<0)return 0;nfs_feedback_modal_backdrop_gpu_key(v,key);if(memcmp(key,s->key,32))return 0;NFS_FMB_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<2;++i){if(!s->textures[i])return 0;NFS_FMB_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}return 1;lost:nfs_feedback_modal_backdrop_gpu_forget(s);return 0;}
static int nfs_feedback_modal_backdrop_gpu_prepare(NfsFeedbackModalBackdropGpu*s,const NfsFeedbackModalBackdropArtView*v,const NfsBackgroundSceneStage*t,const NfsFeedbackModalBackdropGpuAliases*aliases){GLuint vs=0,fs=0,program=0,textures[2]={0};GLint linked=0,matrix=-1,sampler=-1,kind=-1;unsigned char*pixels[2]={NULL,NULL};unsigned char key[32];GLboolean alive;GLenum error;NfsBgSaved saved;unsigned i;int ok=0;
 static const char vertex[]="attribute vec4 vertex;uniform mat4 matrix;varying highp vec2 uv;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;uniform float kind;varying highp vec2 uv;void main(){gl_FragColor=mix(vec4(5.0/255.0,5.0/255.0,5.0/255.0,180.0/256.0),texture2D(image,uv),kind);}";const char*vp=vertex,*fp=fragment;
 if(!s||nfs_feedback_modal_backdrop_gpu_alias(s,aliases)||!nfs_feedback_modal_backdrop_gpu_valid(v)||!nfs_nested_caption_current(t))return 0;if(nfs_feedback_modal_backdrop_gpu_empty(v))return 1;if((s->generation&&s->generation!=t->generation)||(s->program&&!s->generation)||(!s->program&&s->generation))return 0;
 if(!s->program){for(i=0;i<2;++i)if(s->textures[i])return 0;}
 if(s->program){if(s->matrix<0||s->sampler<0||s->kind<0)return 0;NFS_FMB_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<2;++i){if(!s->textures[i])return 0;NFS_FMB_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}nfs_feedback_modal_backdrop_gpu_key(v,key);return !memcmp(key,s->key,32);}
 for(i=0;i<2;++i){size_t at;pixels[i]=(unsigned char*)malloc(v->images[i].bytes);if(!pixels[i])goto lost;for(at=0;at<v->images[i].bytes;at+=4){unsigned a=v->images[i].rgba[at+3];pixels[i][at]=(unsigned char)((unsigned)v->images[i].rgba[at]*a/255);pixels[i][at+1]=(unsigned char)((unsigned)v->images[i].rgba[at+1]*a/255);pixels[i][at+2]=(unsigned char)((unsigned)v->images[i].rgba[at+2]*a/255);pixels[i][at+3]=(unsigned char)a;}}
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 NFS_FMB_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_FMB_GL(glShaderSource(vs,1,&vp,NULL));NFS_FMB_GL(glCompileShader(vs));NFS_FMB_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_FMB_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_FMB_GL(glShaderSource(fs,1,&fp,NULL));NFS_FMB_GL(glCompileShader(fs));NFS_FMB_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_FMB_GL(program=glCreateProgram());if(!program)goto done;if(nfs_feedback_modal_backdrop_gpu_name(program,aliases->programs,aliases->programCount)){program=0;goto done;}NFS_FMB_GL(glAttachShader(program,vs));NFS_FMB_GL(glAttachShader(program,fs));NFS_FMB_GL(glBindAttribLocation(program,0,"vertex"));NFS_FMB_GL(glLinkProgram(program));NFS_FMB_GL(glGetProgramiv(program,GL_LINK_STATUS,&linked));if(!linked)goto done;NFS_FMB_GL(matrix=glGetUniformLocation(program,"matrix"));NFS_FMB_GL(sampler=glGetUniformLocation(program,"image"));NFS_FMB_GL(kind=glGetUniformLocation(program,"kind"));if(matrix<0||sampler<0||kind<0)goto done;
 NFS_FMB_GL(glActiveTexture(GL_TEXTURE0));NFS_FMB_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));for(i=0;i<2;++i){NFS_FMB_GL(glGenTextures(1,textures+i));if(!textures[i])goto done;if(nfs_feedback_modal_backdrop_gpu_name(textures[i],aliases->textures,aliases->textureCount)||(i&&textures[0]==textures[1])){textures[i]=0;goto done;}NFS_FMB_GL(glBindTexture(GL_TEXTURE_2D,textures[i]));NFS_FMB_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_FMB_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_FMB_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_FMB_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_FMB_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,v->images[i].width,v->images[i].height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels[i]));NFS_FMB_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;}ok=1;
 done:for(i=0;i<2;++i){free(pixels[i]);pixels[i]=NULL;}if(vs){NFS_FMB_GL(glDeleteShader(vs));}if(fs){NFS_FMB_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_FMB_GL(glDeleteProgram(program));}for(i=0;i<2;++i)if(textures[i]){NFS_FMB_GL(glDeleteTextures(1,textures+i));}}if(!nfs_nested_caption_restore(&saved,t))goto lost;if(!ok)return 0;s->program=program;memcpy(s->textures,textures,sizeof textures);s->matrix=matrix;s->sampler=sampler;s->kind=kind;s->generation=t->generation;nfs_feedback_modal_backdrop_gpu_key(v,s->key);return 1;
 lost:for(i=0;i<2;++i)free(pixels[i]);nfs_feedback_modal_backdrop_gpu_forget(s);return 0;
}
static int nfs_feedback_modal_backdrop_gpu_quad(NfsFeedbackModalBackdropGpu*s,unsigned image,float kind,float left,float top,float right,float bottom,float u0,const NfsBackgroundSceneStage*t){
 const GLfloat q[]={left,top,u0,0,left,bottom,u0,1,right,top,1,0,right,top,1,0,left,bottom,u0,1,right,bottom,1,1};unsigned i;GLenum error;
 for(i=0;i<8;++i){NFS_FMB_GL(glDisable(nfs_frame_caps[i]));}NFS_FMB_GL(glColorMask(1,1,1,1));NFS_FMB_GL(glDepthMask(GL_FALSE));NFS_FMB_GL(glActiveTexture(GL_TEXTURE0));NFS_FMB_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_FMB_GL(glUseProgram(s->program));NFS_FMB_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection));NFS_FMB_GL(glUniform1i(s->sampler,0));NFS_FMB_GL(glUniform1f(s->kind,kind));NFS_FMB_GL(glBindTexture(GL_TEXTURE_2D,s->textures[image]));NFS_FMB_GL(glEnableVertexAttribArray(0));for(i=1;i<4;++i){NFS_FMB_GL(glDisableVertexAttribArray(i));}NFS_FMB_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));if(!nfs_nested_caption_blend(t,1))return -1;NFS_FMB_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_FMB_GL(error=glGetError());return error==GL_NO_ERROR;lost:return -1;
}
static int nfs_feedback_modal_backdrop_gpu_draw(NfsFeedbackModalBackdropGpu*s,const NfsFeedbackModalBackdropArtView*v,const NfsBackgroundSceneStage*t,const NfsFeedbackModalBackdropGpuAliases*aliases){
 NfsBgSaved saved;int ok;GLint fbo=0,type=0,name=0,viewport[4];GLenum status;
 if(!nfs_feedback_modal_backdrop_gpu_ready(s,v,t,aliases)||!t->framebuffer||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;
 NFS_FMB_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_FMB_GL(glGetIntegerv(GL_VIEWPORT,viewport));if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport))return 0;
 NFS_FMB_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_FMB_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_FMB_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name||(GLuint)name==s->textures[0]||(GLuint)name==s->textures[1])return 0;
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 /* Body RGB is ALREADY premultiplied: do not multiply 5/255 by its alpha.
    Destination weight is exactly 76/256. Banner clip is screen LEFT x247;
    its authored canvas still spans x98..442, therefore UV starts at149/344. */
 ok=nfs_feedback_modal_backdrop_gpu_quad(s,0,0,227,210,413,270,0,t);if(ok<0)goto lost;
 if(ok){ok=nfs_feedback_modal_backdrop_gpu_quad(s,0,1,247,179,442,226,149.f/344.f,t);if(ok<0)goto lost;}
 if(ok){ok=nfs_feedback_modal_backdrop_gpu_quad(s,1,1,175,171,292,232,0,t);if(ok<0)goto lost;}
 if(!nfs_nested_caption_restore(&saved,t))goto lost;return ok;
 lost:nfs_feedback_modal_backdrop_gpu_forget(s);return 0;
}
#undef NFS_FMB_GL
#endif
