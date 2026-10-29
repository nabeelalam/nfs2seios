// SPDX-License-Identifier: MIT
#pragma once
#include "LoadingWordData.h"
#include "LoadingWordSourceCatalog.h"
#include <stdlib.h>
#include <string.h>
static int nfs_loading_words_owner_current(void);
typedef struct NfsLoadingWordGpu {GLuint program,textures[48];GLint matrix,sampler,pixel;}NfsLoadingWordGpu;
static void nfs_loading_words_retire(NfsLoadingWordGpu*s,int owner){if(owner&&nfs_loading_words_owner_current()){if(s->program)glDeleteProgram(s->program);glDeleteTextures(48,s->textures);}memset(s,0,sizeof *s);}
static int nfs_loading_words_prepare(NfsLoadingWordGpu*s,const NfsLoadingWordView*words,size_t count){
 NfsBgSaved saved;GLuint vs=0,fs=0;GLint linked=0,maxSize=0;int ok=0;unsigned char*alpha=NULL;if(!nfs_loading_words_owner_current())return 0;if(!count)return 1;if(!words||count>1)return 0;
 static const char vertex[]="attribute vec4 vertex;uniform mat4 matrix;varying highp vec2 uv;void main(){gl_Position=matrix*vec4(vertex.xy,0.,1.);uv=vertex.zw;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;uniform vec2 pixel;varying highp vec2 uv;void main(){float a=texture2D(image,uv).a;float sh=texture2D(image,uv-pixel*vec2(4.,4.)).a*(210./255.);gl_FragColor=vec4(vec3(a),a+sh*(1.-a));}";
 nfs_bg_save(&saved);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);
 if(!s->program){vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);if(!vs||!fs)goto done;s->program=glCreateProgram();if(!s->program)goto done;glAttachShader(s->program,vs);glAttachShader(s->program,fs);glBindAttribLocation(s->program,0,"vertex");glLinkProgram(s->program);glGetProgramiv(s->program,GL_LINK_STATUS,&linked);if(!linked)goto done;s->matrix=glGetUniformLocation(s->program,"matrix");s->sampler=glGetUniformLocation(s->program,"image");s->pixel=glGetUniformLocation(s->program,"pixel");if(s->matrix<0||s->sampler<0||s->pixel<0)goto done;}
 for(size_t j=0;j<count;++j){unsigned index=words[j].index;if(index>=48)goto done;const NfsLoadingWordSource*a=nfs_loading_word_sources+index;unsigned W=a->width*4,H=a->height*4;if(a->artIndex>=48)goto done;if(words[j].width!=a->width||words[j].height!=a->height||W>(unsigned)maxSize||H>(unsigned)maxSize)goto done;if(s->textures[index])continue;size_t pixels=(size_t)W*H,at=0;const NfsLoadingWordArt*c=nfs_loading_word_art+a->artIndex;alpha=(unsigned char*)malloc(pixels);if(!alpha||c->count%2||c->width!=W||c->height!=H)goto done;for(size_t k=0;k<c->count;k+=2){size_t n=c->runs[k];unsigned value=c->runs[k+1];if(!n||value>255||n>pixels-at)goto done;memset(alpha+at,value,n);at+=n;}if(at!=pixels)goto done;glGenTextures(1,s->textures+index);if(!s->textures[index])goto done;glBindTexture(GL_TEXTURE_2D,s->textures[index]);nfs_frame_texture_params();glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,W,H,0,GL_ALPHA,GL_UNSIGNED_BYTE,alpha);free(alpha);alpha=NULL;}
 ok=nfs_loading_words_owner_current()&&glGetError()==GL_NO_ERROR;
 done:free(alpha);if(!nfs_loading_words_owner_current()){memset(s,0,sizeof *s);return 0;}if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);nfs_bg_restore(&saved);if(!ok)nfs_loading_words_retire(s,1);return ok;
}
static int nfs_loading_words_draw(NfsLoadingWordGpu*s,const NfsLoadingWordView*words,size_t count,const NfsFrameLegacySampling*sampling){
 if(!nfs_loading_words_owner_current())return 0;if(!count)return 1;if(!s->program||!words||count>1||!sampling)return 0;nfs_frame_draw_state();glDepthMask(GL_FALSE);glViewport(sampling->viewport[0],sampling->viewport[1],sampling->viewport[2],sampling->viewport[3]);glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,sampling->projection);glUniform1i(s->sampler,0);
 for(size_t j=0;j<count;++j){const NfsLoadingWordView*w=words+j;if(w->index>=48||!s->textures[w->index])return 0;GLfloat l=(GLfloat)w->left,t=(GLfloat)w->top,r=l+w->width,b=t+w->height;GLfloat q[]={l,t,0,0,r,t,1,0,l,b,0,1,l,b,0,1,r,t,1,0,r,b,1,1};glBindTexture(GL_TEXTURE_2D,s->textures[w->index]);glUniform2f(s->pixel,1.f/(w->width*4.f),1.f/(w->height*4.f));glEnableVertexAttribArray(0);for(unsigned k=1;k<4;++k)glDisableVertexAttribArray(k);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q);glDrawArrays(GL_TRIANGLES,0,6);}return nfs_loading_words_owner_current()&&glGetError()==GL_NO_ERROR;
}
