// SPDX-License-Identifier: MIT
#ifndef NFS_FEEDBACK_MODAL_WORD_GPU_IMPLEMENTATION
#define NFS_FEEDBACK_MODAL_WORD_GPU_IMPLEMENTATION
#include "FeedbackModalWordGpu.h"
#include "FeedbackModalWordIdentity.h"
/* Mechanical two-word rendering only. Prerequisites: neutral stage/text types,
   digest implementation and guarded nested-caption current/save/restore/blend
   helpers. Caller must retain actual Joined/current source and native witness,
   validate original font vertices, provide exhaustive independent alias names,
   and supply a caller-owned UNPUBLISHED stage. This helper never binds a new
   target or publishes. Any refusal/late error requires discarding the complete
   stage transaction; no partial replacements may reach the screen. */
static int nfs_feedback_modal_word_empty(const NfsFeedbackModalWordsView*v){
 unsigned i,j;if(!v||v->schema||v->count)return 0;
 for(i=0;i<2;++i){const NfsFeedbackModalWordImageView*a=v->images+i;const NfsFeedbackModalWordPlacement*p=v->placements+i;
  if(a->word||a->width||a->height||a->rgba||a->bytes||p->word||p->color565||p->vertexFirst||p->vertexCount||p->left||p->top||p->right||p->bottom)return 0;
  for(j=0;j<32;++j)if(a->sha256[j])return 0;
 }return 1;
}
static int nfs_feedback_modal_word_valid(const NfsFeedbackModalWordsView*v,const NfsSoftwareTextLayerView*text){
 static const unsigned widths[2]={96,108},lefts[2]={302,266},tops[2]={185,225},colors[2]={1,65535};
 unsigned i;size_t end=0;unsigned char sha[32];
 if(nfs_feedback_modal_word_empty(v))return 1;
 /* Reject schema/count before touching borrowed image or text data. Original
    glyph counts are taken from the checked source packet, never inferred. */
 if(!v||v->schema!=NFS_FEEDBACK_MODAL_WORD_SCHEMA||v->count!=2||!text||!text->vertices||!text->vertexCount)return 0;
 for(i=0;i<2;++i){const NfsFeedbackModalWordImageView*a=v->images+i;const NfsFeedbackModalWordPlacement*p=v->placements+i;
  if(a->word!=i+1||a->width!=widths[i]*4||a->height!=128||a->bytes!=(size_t)widths[i]*4*128*4||!a->rgba||memcmp(a->sha256,nfs_feedback_modal_word_sha256[i],32)||!nfs_texture_sha256(a->rgba,a->bytes,sha)||memcmp(sha,a->sha256,32))return 0;
  if(p->word!=i+1||p->color565!=colors[i]||p->left!=(float)lefts[i]||p->top!=(float)tops[i]||p->right!=(float)(lefts[i]+widths[i])||p->bottom!=(float)(tops[i]+32)||p->vertexFirst!=end||!p->vertexCount||p->vertexCount%6||end>text->vertexCount||p->vertexCount>text->vertexCount-end)return 0;
  end+=p->vertexCount;
 }return end==text->vertexCount;
}
static void nfs_feedback_modal_word_key(const NfsFeedbackModalWordsView*v,const NfsSoftwareTextLayerView*text,unsigned char out[32]){
 unsigned char b[512];size_t used=0;unsigned i;
#define NFS_FMW_KEY(x) do{memcpy(b+used,&(x),sizeof(x));used+=sizeof(x);}while(0)
 NFS_FMW_KEY(text->vertexCount);for(i=0;i<2;++i){const NfsFeedbackModalWordImageView*a=v->images+i;const NfsFeedbackModalWordPlacement*p=v->placements+i;NFS_FMW_KEY(a->word);NFS_FMW_KEY(a->width);NFS_FMW_KEY(a->height);memcpy(b+used,a->sha256,32);used+=32;NFS_FMW_KEY(p->word);NFS_FMW_KEY(p->color565);NFS_FMW_KEY(p->vertexFirst);NFS_FMW_KEY(p->vertexCount);NFS_FMW_KEY(p->left);NFS_FMW_KEY(p->top);NFS_FMW_KEY(p->right);NFS_FMW_KEY(p->bottom);}nfs_texture_sha256(b,used,out);
#undef NFS_FMW_KEY
}
static int nfs_feedback_modal_word_name(GLuint name,const GLuint*names,size_t count){size_t i;for(i=0;i<count;++i)if(name&&name==names[i])return 1;return 0;}
static int nfs_feedback_modal_word_alias(const NfsFeedbackModalWordGpu*s,const NfsFeedbackModalWordGpuAliases*a){
 unsigned i;if(!s||!a||a->programCount>128||a->textureCount>128||(a->programCount&&!a->programs)||(a->textureCount&&!a->textures))return 1;
 if(nfs_feedback_modal_word_name(s->program,a->programs,a->programCount))return 1;
 for(i=0;i<2;++i)if(s->textures[i]&&(nfs_feedback_modal_word_name(s->textures[i],a->textures,a->textureCount)||(i&&s->textures[0]==s->textures[1])))return 1;
 return 0;
}
static void nfs_feedback_modal_word_forget(NfsFeedbackModalWordGpu*s){if(s)memset(s,0,sizeof *s);}
#define NFS_FMW_GL(call) do{if(!nfs_nested_caption_current(t))goto lost;call;if(!nfs_nested_caption_current(t))goto lost;}while(0)
static int nfs_feedback_modal_word_retire(NfsFeedbackModalWordGpu*s,const NfsBackgroundSceneStage*t,int owner,const NfsFeedbackModalWordGpuAliases*aliases){unsigned i;if(!s)return 0;if(nfs_feedback_modal_word_alias(s,aliases)){nfs_feedback_modal_word_forget(s);return 0;}if(owner&&t&&s->generation==t->generation&&nfs_nested_caption_current(t)){if(s->program){NFS_FMW_GL(glDeleteProgram(s->program));}for(i=0;i<2;++i)if(s->textures[i]){NFS_FMW_GL(glDeleteTextures(1,s->textures+i));}}nfs_feedback_modal_word_forget(s);return 1;lost:nfs_feedback_modal_word_forget(s);return 0;}
static int nfs_feedback_modal_word_ready(NfsFeedbackModalWordGpu*s,const NfsFeedbackModalWordsView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t,const NfsFeedbackModalWordGpuAliases*aliases){unsigned i;GLboolean alive;unsigned char key[32];if(!s||nfs_feedback_modal_word_alias(s,aliases)||!nfs_nested_caption_current(t)||!nfs_feedback_modal_word_valid(v,text)||nfs_feedback_modal_word_empty(v)||s->generation!=t->generation||!s->program||s->matrix<0||s->sampler<0)return 0;nfs_feedback_modal_word_key(v,text,key);if(memcmp(key,s->key,32))return 0;NFS_FMW_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<2;++i){if(!s->textures[i])return 0;NFS_FMW_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}return 1;lost:nfs_feedback_modal_word_forget(s);return 0;}
static int nfs_feedback_modal_word_prepare(NfsFeedbackModalWordGpu*s,const NfsFeedbackModalWordsView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t,const NfsFeedbackModalWordGpuAliases*aliases){GLuint vs=0,fs=0,program=0,textures[2]={0};GLint linked=0,matrix=-1,sampler=-1;GLboolean alive;GLenum error;NfsBgSaved saved;unsigned i;int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";const char*vp=vertex,*fp=fragment;
 if(!s||nfs_feedback_modal_word_alias(s,aliases)||!nfs_feedback_modal_word_valid(v,text)||!nfs_nested_caption_current(t))return 0;if(nfs_feedback_modal_word_empty(v))return 1;if((s->generation&&s->generation!=t->generation)||(s->program&&!s->generation)||(!s->program&&s->generation))return 0;
 if(!s->program){for(i=0;i<2;++i)if(s->textures[i])return 0;}
 if(s->program){if(s->matrix<0||s->sampler<0)return 0;NFS_FMW_GL(alive=glIsProgram(s->program));if(!alive)return 0;for(i=0;i<2;++i){if(!s->textures[i])return 0;NFS_FMW_GL(alive=glIsTexture(s->textures[i]));if(!alive)return 0;}nfs_feedback_modal_word_key(v,text,s->key);return 1;}
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 NFS_FMW_GL(vs=glCreateShader(GL_VERTEX_SHADER));if(!vs)goto done;NFS_FMW_GL(glShaderSource(vs,1,&vp,NULL));NFS_FMW_GL(glCompileShader(vs));NFS_FMW_GL(glGetShaderiv(vs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_FMW_GL(fs=glCreateShader(GL_FRAGMENT_SHADER));if(!fs)goto done;NFS_FMW_GL(glShaderSource(fs,1,&fp,NULL));NFS_FMW_GL(glCompileShader(fs));NFS_FMW_GL(glGetShaderiv(fs,GL_COMPILE_STATUS,&linked));if(!linked)goto done;
 NFS_FMW_GL(program=glCreateProgram());if(!program)goto done;if(nfs_feedback_modal_word_name(program,aliases->programs,aliases->programCount)){program=0;goto done;}NFS_FMW_GL(glAttachShader(program,vs));NFS_FMW_GL(glAttachShader(program,fs));NFS_FMW_GL(glBindAttribLocation(program,0,"vertex"));NFS_FMW_GL(glBindAttribLocation(program,1,"color"));NFS_FMW_GL(glLinkProgram(program));NFS_FMW_GL(glGetProgramiv(program,GL_LINK_STATUS,&linked));if(!linked)goto done;NFS_FMW_GL(matrix=glGetUniformLocation(program,"matrix"));NFS_FMW_GL(sampler=glGetUniformLocation(program,"image"));if(matrix<0||sampler<0)goto done;
 NFS_FMW_GL(glActiveTexture(GL_TEXTURE0));NFS_FMW_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));for(i=0;i<2;++i){NFS_FMW_GL(glGenTextures(1,textures+i));if(!textures[i])goto done;if(nfs_feedback_modal_word_name(textures[i],aliases->textures,aliases->textureCount)||(i&&textures[0]==textures[1])){textures[i]=0;goto done;}NFS_FMW_GL(glBindTexture(GL_TEXTURE_2D,textures[i]));NFS_FMW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_FMW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_FMW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_FMW_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_FMW_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,v->images[i].width,128,0,GL_RGBA,GL_UNSIGNED_BYTE,v->images[i].rgba));NFS_FMW_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;}ok=1;
 done:if(vs){NFS_FMW_GL(glDeleteShader(vs));}if(fs){NFS_FMW_GL(glDeleteShader(fs));}if(!ok){if(program){NFS_FMW_GL(glDeleteProgram(program));}for(i=0;i<2;++i)if(textures[i]){NFS_FMW_GL(glDeleteTextures(1,textures+i));}}if(!nfs_nested_caption_restore(&saved,t))goto lost;if(!ok)return 0;s->program=program;memcpy(s->textures,textures,sizeof textures);s->matrix=matrix;s->sampler=sampler;s->generation=t->generation;nfs_feedback_modal_word_key(v,text,s->key);return 1;
 lost:nfs_feedback_modal_word_forget(s);return 0;
}
/* The two complete words use source RGB565 tint and premultiplied alpha. */
static int nfs_feedback_modal_word_vertices(GLuint program,GLuint texture,GLint matrix,GLint sampler,const NfsSoftwareTextVertex*vertices,size_t count,const NfsBackgroundSceneStage*t){unsigned i;GLenum error;if(!count)return 1;for(i=0;i<8;++i){NFS_FMW_GL(glDisable(nfs_frame_caps[i]));}NFS_FMW_GL(glColorMask(1,1,1,1));NFS_FMW_GL(glDepthMask(GL_FALSE));NFS_FMW_GL(glActiveTexture(GL_TEXTURE0));NFS_FMW_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_FMW_GL(glUseProgram(program));NFS_FMW_GL(glUniformMatrix4fv(matrix,1,GL_FALSE,t->sampling.projection));NFS_FMW_GL(glUniform1i(sampler,0));NFS_FMW_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_FMW_GL(glEnableVertexAttribArray(0));NFS_FMW_GL(glEnableVertexAttribArray(1));NFS_FMW_GL(glDisableVertexAttribArray(2));NFS_FMW_GL(glDisableVertexAttribArray(3));NFS_FMW_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),vertices));NFS_FMW_GL(glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&vertices[0].r));if(!nfs_nested_caption_blend(t,1))return -1;NFS_FMW_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_FMW_GL(error=glGetError());return error==GL_NO_ERROR;lost:return -1;}
static int nfs_feedback_modal_word_draw(NfsFeedbackModalWordGpu*s,const NfsFeedbackModalWordsView*v,const NfsSoftwareTextLayerView*text,const NfsBackgroundSceneStage*t,const NfsFeedbackModalWordGpuAliases*aliases){
 NfsBgSaved saved;unsigned i;int ok=1;GLint fbo=0,type=0,name=0,viewport[4];GLenum status;
 if(!nfs_feedback_modal_word_ready(s,v,text,t,aliases)||!t->framebuffer||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling))return 0;
 NFS_FMW_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_FMW_GL(glGetIntegerv(GL_VIEWPORT,viewport));
 if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport))return 0;
 NFS_FMW_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_FMW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));
 NFS_FMW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||!name||(GLuint)name==s->textures[0]||(GLuint)name==s->textures[1])return 0;
 if(!nfs_nested_caption_save(&saved,t))goto lost;
 for(i=0;i<2&&ok;++i){const NfsFeedbackModalWordPlacement*p=v->placements+i;NfsSoftwareTextVertex q[6];
  unsigned r=p->color565>>11,g=(p->color565>>5)&63,b=p->color565&31;
  float red=(float)((r<<3)|(r>>2))/255.f,green=(float)((g<<2)|(g>>4))/255.f,blue=(float)((b<<3)|(b>>2))/255.f;
  const NfsSoftwareTextVertex a={p->left,p->top,0,0,red,green,blue,1},b0={p->left,p->bottom,0,1,red,green,blue,1},c={p->right,p->top,1,0,red,green,blue,1},d={p->right,p->bottom,1,1,red,green,blue,1};
  q[0]=a;q[1]=b0;q[2]=c;q[3]=c;q[4]=b0;q[5]=d;
  ok=nfs_feedback_modal_word_vertices(s->program,s->textures[p->word-1],s->matrix,s->sampler,q,6,t);if(ok<0)goto lost;
 }
 if(!nfs_nested_caption_restore(&saved,t))goto lost;return ok;
 lost:nfs_feedback_modal_word_forget(s);return 0;
}
#undef NFS_FMW_GL
#endif
