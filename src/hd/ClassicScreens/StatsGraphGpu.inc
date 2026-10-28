// PRIVATE native-resolution vector graph. Draws only into the caller's owned stage.
#pragma once
#include "StatsRuntimeAbi.h"
#include "hd/BackgroundSceneCompositor.h"
#include <math.h>
#include <string.h>
#include <stddef.h>
typedef struct NfsStatsGraphGpu {uint64_t generation;uintptr_t context;GLuint program;GLint matrix,feather;} NfsStatsGraphGpu;
static int nfs_stats_graph_current(const NfsBackgroundSceneStage*t){return t&&t->generation&&t->contextIdentity&&t->ownerCurrent&&t->ownerCurrent(t->ownerUser,t);}
static GLuint nfs_stats_graph_shader(GLenum type,const char*source){GLuint s=glCreateShader(type);GLint ok=0;if(!s)return 0;glShaderSource(s,1,&source,NULL);glCompileShader(s);glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){glDeleteShader(s);return 0;}return s;}
static int nfs_stats_graph_prepare(NfsStatsGraphGpu*s,const NfsBackgroundSceneStage*t){
 static const char vertex[]="attribute vec4 vertex;attribute vec4 shape;attribute vec4 color;attribute vec4 clip;uniform mat4 matrix;varying highp vec2 point;varying highp vec4 ends;varying highp vec2 style;varying highp vec4 tint;varying highp vec4 bounds;void main(){point=vertex.xy;ends=vec4(vertex.zw,shape.xy);style=shape.zw;tint=color;bounds=clip;gl_Position=matrix*vec4(vertex.xy,1.,1.);}";
 static const char fragment[]="precision highp float;uniform float feather;varying highp vec2 point;varying highp vec4 ends;varying highp vec2 style;varying highp vec4 tint;varying highp vec4 bounds;void main(){if(point.x<bounds.x||point.y<bounds.y||point.x>bounds.z||point.y>bounds.w)discard;vec2 a=ends.xy,b=ends.zw;float d;if(style.y<.5){vec2 v=b-a;float h=clamp(dot(point-a,v)/max(dot(v,v),.00001),0.,1.);d=length(point-a-v*h)-style.x;}else if(style.y<1.5){vec2 q=max(min(a,b)-point,point-max(a,b));d=max(q.x,q.y);}else{d=length(point-a)-style.x;}float alpha=tint.a*(1.-smoothstep(-feather,feather,d));gl_FragColor=vec4(tint.rgb*alpha,alpha);}";
 GLuint vs=0,fs=0,program=0;GLint linked=0;NfsFrameSaved saved;int ok=0;
 if(!s||!nfs_stats_graph_current(t))return 0;if(s->program)return s->generation==t->generation&&s->context==t->contextIdentity&&glIsProgram(s->program);
 nfs_frame_save(&saved);vs=nfs_stats_graph_shader(GL_VERTEX_SHADER,vertex);fs=nfs_stats_graph_shader(GL_FRAGMENT_SHADER,fragment);if(!vs||!fs)goto done;program=glCreateProgram();if(!program)goto done;glAttachShader(program,vs);glAttachShader(program,fs);glBindAttribLocation(program,0,"vertex");glBindAttribLocation(program,1,"shape");glBindAttribLocation(program,2,"color");glBindAttribLocation(program,3,"clip");glLinkProgram(program);glGetProgramiv(program,GL_LINK_STATUS,&linked);if(!linked)goto done;s->matrix=glGetUniformLocation(program,"matrix");s->feather=glGetUniformLocation(program,"feather");if(s->matrix<0||s->feather<0||glGetError()!=GL_NO_ERROR)goto done;s->program=program;program=0;s->generation=t->generation;s->context=t->contextIdentity;ok=1;
 done:if(!nfs_stats_graph_current(t)){memset(s,0,sizeof *s);return 0;}if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);if(program)glDeleteProgram(program);nfs_frame_restore(&saved);return ok;
}
static int nfs_stats_graph_draw(NfsStatsGraphGpu*s,const NfsBackgroundSceneStage*t,const NfsStatsGraphVertex*v,size_t count){
 NfsFrameSaved saved;GLint fbo=0;size_t i;int ok=0;
 if(!s||!nfs_stats_graph_current(t)||!t->framebuffer||!t->width||!t->height||!s->program||s->generation!=t->generation||s->context!=t->contextIdentity||!v||!count||count%6||count>6144)return 0;
 for(i=0;i<count;++i){unsigned k;float all[16];memcpy(all,v+i,sizeof all);for(k=0;k<16;++k)if(!isfinite(all[k]))return 0;if(v[i].vertex[0]<0||v[i].vertex[0]>640||v[i].vertex[1]<0||v[i].vertex[1]>480||v[i].shape[2]<0||v[i].shape[2]>64||v[i].shape[3]<0||v[i].shape[3]>2)return 0;for(k=0;k<4;++k)if(v[i].color[k]<0||v[i].color[k]>1)return 0;}
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=t->framebuffer)return 0;
 nfs_frame_save(&saved);nfs_frame_draw_state();glViewport(t->sampling.viewport[0],t->sampling.viewport[1],t->sampling.viewport[2],t->sampling.viewport[3]);glBindBuffer(GL_ARRAY_BUFFER,0);glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection);glUniform1f(s->feather,fmaxf(.08f,.75f*640.f/(float)t->sampling.viewport[2]));
 for(i=0;i<4;++i){glEnableVertexAttribArray((GLuint)i);glVertexAttribPointer((GLuint)i,4,GL_FLOAT,GL_FALSE,sizeof(NfsStatsGraphVertex),(const unsigned char*)v+i*4*sizeof(float));}
 glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,(GLsizei)count);ok=glGetError()==GL_NO_ERROR;
 if(!nfs_stats_graph_current(t)){memset(s,0,sizeof *s);return 0;}nfs_frame_restore(&saved);return ok;
}
static void nfs_stats_graph_retire(NfsStatsGraphGpu*s,const NfsBackgroundSceneStage*t,int owner){if(s&&owner&&nfs_stats_graph_current(t)&&s->generation==t->generation&&s->context==t->contextIdentity&&s->program)glDeleteProgram(s->program);if(s)memset(s,0,sizeof *s);}
