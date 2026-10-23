// SPDX-License-Identifier: MIT
/* Player-only complete-word graphics. Include after guarded Graphics helpers.
   Preparation accepts a context-only guard; draw requires an owned staging guard. */
#include "PlayerCaptionRuntimeAbi.h"
#include "PlayerCaptionWordIdentity.h"
typedef struct NfsPlayerCaptionGpu {uint64_t generation;GLuint program,texture;GLint matrix,sampler;unsigned word,traceLines;} NfsPlayerCaptionGpu;
static int nfs_player_caption_empty(const NfsPlayerCaptionWordView*v){unsigned i;if(!v)return 0;if(v->schema||v->word||v->width||v->height||v->rgba||v->bytes||v->left||v->top||v->right||v->bottom)return 0;for(i=0;i<32;++i)if(v->sha256[i])return 0;return 1;}
static int nfs_player_caption_valid(const NfsPlayerCaptionWordView*v,unsigned player){unsigned char sha[32];if(nfs_player_caption_empty(v))return 1;if(!v||v->schema!=1||v->word!=player||player<1||player>2||v->width!=(player==1?272u:280u)||v->height!=128||v->bytes!=(size_t)v->width*128u*4u||!v->rgba||v->left!=(player==1?100.f:98.f)||v->top!=(player==1?198.f:234.f)||v->right!=168.f||v->bottom!=(player==1?230.f:266.f)||memcmp(v->sha256,nfs_player_caption_word_sha256[player-1],32))return 0;return nfs_texture_sha256(v->rgba,v->bytes,sha)&&!memcmp(sha,nfs_player_caption_word_sha256[player-1],32);}
/* Resource names cannot borrow an original/cache/background/font texture. */
static int nfs_player_caption_alias(GLuint texture,GLuint font,GLuint preview,GLuint color,const uint32_t ids[300]){if(!texture)return 0;if(texture==g_framebufferTexture||texture==font||texture==preview||texture==color||texture==nfs_bg_compositor.logo.texture)return 1;for(unsigned i=0;i<5;++i)if(texture==nfs_bg_compositor.textures[i])return 1;for(unsigned i=0;i<300;++i)if(texture==ids[i])return 1;return 0;}
static void nfs_player_caption_forget(NfsPlayerCaptionGpu*s){memset(s,0,sizeof *s);}
#define NFS_PLAYER_CAPTION_GL(call) do{if(!nfs_graphics_scene_current(t))goto lost;call;if(!nfs_graphics_scene_current(t))goto lost;}while(0)
static int nfs_player_caption_retire(NfsPlayerCaptionGpu*s,const NfsBackgroundSceneStage*t,int owner){if(owner&&s->generation==t->generation&&nfs_graphics_scene_current(t)){if(s->program){NFS_PLAYER_CAPTION_GL(glDeleteProgram(s->program));}if(s->texture){NFS_PLAYER_CAPTION_GL(glDeleteTextures(1,&s->texture));}}nfs_player_caption_forget(s);return 1;lost:nfs_player_caption_forget(s);return 0;}
static int nfs_player_caption_prepare(NfsPlayerCaptionGpu*s,const NfsPlayerCaptionWordView*v,unsigned player,const NfsBackgroundSceneStage*t){
 GLuint vs=0,fs=0,p=0,tex=0;GLint status=0,matrix=-1,sampler=-1,maxSize=0;GLboolean exists=0;GLenum error;NfsBgSaved saved;int ok=0;
 static const char vertex[]="attribute vec4 vertex;uniform mat4 matrix;varying highp vec2 uv;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;void main(){vec4 p=texture2D(image,uv);gl_FragColor=vec4(p.rgb*p.a,p.a);}";
 const char*vp=vertex;const char*fp=fragment;
 if(!s||!nfs_player_caption_valid(v,player)||!nfs_graphics_scene_current(t))return 0;if(nfs_player_caption_empty(v))return 1;
 if(s->generation&&s->generation!=t->generation)return 0;
 if(s->program&&s->texture&&s->word==player){NFS_PLAYER_CAPTION_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_PLAYER_CAPTION_GL(exists=glIsTexture(s->texture));return exists!=0;}
 NFS_PLAYER_CAPTION_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));if(maxSize<(GLint)v->width)return 0;
 if(!nfs_graphics_scene_save(&saved,t))goto lost;
 NFS_PLAYER_CAPTION_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_PLAYER_CAPTION_GL(glShaderSource(vs,1,&vp,NULL));NFS_PLAYER_CAPTION_GL(glCompileShader(vs));NFS_PLAYER_CAPTION_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_CAPTION_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_PLAYER_CAPTION_GL(glShaderSource(fs,1,&fp,NULL));NFS_PLAYER_CAPTION_GL(glCompileShader(fs));NFS_PLAYER_CAPTION_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&status));if(!status)goto done;
 NFS_PLAYER_CAPTION_GL(p=glCreateProgram());if(!p)goto done;NFS_PLAYER_CAPTION_GL(glAttachShader(p,vs));NFS_PLAYER_CAPTION_GL(glAttachShader(p,fs));NFS_PLAYER_CAPTION_GL(glBindAttribLocation(p,0,"vertex"));NFS_PLAYER_CAPTION_GL(glLinkProgram(p));NFS_PLAYER_CAPTION_GL(glGetProgramiv(p,GL_LINK_STATUS,&status));if(!status)goto done;NFS_PLAYER_CAPTION_GL(matrix=glGetUniformLocation(p,"matrix"));NFS_PLAYER_CAPTION_GL(sampler=glGetUniformLocation(p,"image"));if(matrix<0||sampler<0)goto done;
 NFS_PLAYER_CAPTION_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_CAPTION_GL(glGenTextures(1,&tex));if(!tex)goto done;NFS_PLAYER_CAPTION_GL(glBindTexture(GL_TEXTURE_2D,tex));NFS_PLAYER_CAPTION_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_PLAYER_CAPTION_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_PLAYER_CAPTION_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_PLAYER_CAPTION_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_PLAYER_CAPTION_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_PLAYER_CAPTION_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,v->width,v->height,0,GL_RGBA,GL_UNSIGNED_BYTE,v->rgba));NFS_PLAYER_CAPTION_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(vs){NFS_PLAYER_CAPTION_GL(glDeleteShader(vs));}if(fs){NFS_PLAYER_CAPTION_GL(glDeleteShader(fs));}if(!ok){if(p){NFS_PLAYER_CAPTION_GL(glDeleteProgram(p));}if(tex){NFS_PLAYER_CAPTION_GL(glDeleteTextures(1,&tex));}}
 if(!nfs_graphics_scene_restore(&saved,t))goto lost;if(!ok)return 0;
 if(s->program){NFS_PLAYER_CAPTION_GL(glDeleteProgram(s->program));}if(s->texture){NFS_PLAYER_CAPTION_GL(glDeleteTextures(1,&s->texture));}s->program=p;s->texture=tex;s->matrix=matrix;s->sampler=sampler;s->word=player;s->generation=t->generation;return 1;
 lost:nfs_player_caption_forget(s);return 0;
}
static int nfs_player_caption_draw(NfsPlayerCaptionGpu*s,const NfsPlayerCaptionWordView*v,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0,vp[4];GLenum error;GLboolean exists=0;GLfloat q[24];
 if(!s||!v||!nfs_graphics_scene_current(t)||nfs_player_caption_empty(v)||!nfs_player_caption_valid(v,s->word)||s->generation!=t->generation||!s->program||!s->texture||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;
 NFS_PLAYER_CAPTION_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_PLAYER_CAPTION_GL(glGetIntegerv(GL_VIEWPORT,vp));if((GLuint)fbo!=t->framebuffer||memcmp(vp,t->sampling.viewport,sizeof vp))return 0;
 NFS_PLAYER_CAPTION_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_PLAYER_CAPTION_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name||(GLuint)name==s->texture)return 0;
 NFS_PLAYER_CAPTION_GL(exists=glIsProgram(s->program));if(!exists)return 0;NFS_PLAYER_CAPTION_GL(exists=glIsTexture(s->texture));if(!exists)return 0;
 {const GLfloat made[]={v->left,v->top,0,0,v->right,v->top,1,0,v->left,v->bottom,0,1,v->left,v->bottom,0,1,v->right,v->top,1,0,v->right,v->bottom,1,1};memcpy(q,made,sizeof q);}
 for(unsigned i=0;i<8;++i){NFS_PLAYER_CAPTION_GL(glDisable(nfs_frame_caps[i]));}NFS_PLAYER_CAPTION_GL(glColorMask(1,1,1,1));NFS_PLAYER_CAPTION_GL(glDepthMask(GL_FALSE));NFS_PLAYER_CAPTION_GL(glActiveTexture(GL_TEXTURE0));NFS_PLAYER_CAPTION_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_PLAYER_CAPTION_GL(glUseProgram(s->program));NFS_PLAYER_CAPTION_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection));NFS_PLAYER_CAPTION_GL(glUniform1i(s->sampler,0));NFS_PLAYER_CAPTION_GL(glBindTexture(GL_TEXTURE_2D,s->texture));NFS_PLAYER_CAPTION_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_PLAYER_CAPTION_GL(glDisableVertexAttribArray(i));}NFS_PLAYER_CAPTION_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_PLAYER_CAPTION_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_PLAYER_CAPTION_GL(error=glGetError());if(error!=GL_NO_ERROR||!nfs_graphics_scene_current(t))return 0;
 if(nfs_logo_trace_enabled()&&s->traceLines<64){++s->traceLines;fprintf(stderr,"PLAYER CAPTION WORD player=%u generation=%llu fbo=%u\n",s->word,(unsigned long long)s->generation,(unsigned)t->framebuffer);}return 1;
 lost:nfs_player_caption_forget(s);return 0;
}
#undef NFS_PLAYER_CAPTION_GL
