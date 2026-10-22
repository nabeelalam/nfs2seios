// SPDX-License-Identifier: MIT
#include "SoftwareTextGpu.h"
#ifndef NFS_SOFTWARE_TEXT_EXTERNAL_FRAME_IMPLEMENTATION
#include "SceneGpuSupport.inc"
#endif
#include "TextureDigest.h"
#include <math.h>
#include <string.h>
static void nfs_soft_gpu_forget(NfsSoftwareTextGpu*s){s->generation=s->ownerGeneration=s->fontIdentity=0;s->program=s->texture=0;s->sampler=s->matrix=-1;memset(s->fontSha256,0,32);}
int nfs_soft_gpu_context_created(NfsSoftwareTextGpu*s,uint64_t g){if(!s||!g||g<=s->lastGeneration||s->generation||s->ownerGeneration)return 0;nfs_soft_gpu_forget(s);s->generation=s->lastGeneration=g;return 1;}
void nfs_soft_gpu_context_retired(NfsSoftwareTextGpu*s,uint64_t g,int owner){if(!s||!g||g!=s->generation)return;if(owner&&s->ownerGeneration==g){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}nfs_soft_gpu_forget(s);}
static int nfs_soft_gpu_ready(const NfsSoftwareTextGpu*s){return s&&s->generation&&s->ownerGeneration==s->generation&&s->fontIdentity&&s->program&&s->texture&&s->sampler>=0&&s->matrix>=0&&glIsProgram(s->program)&&glIsTexture(s->texture);}
int nfs_soft_gpu_prepare(NfsSoftwareTextGpu*s,const NfsSoftwareTextFontView*f){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint linked=0,sampler=-1,matrix=-1,maxSize=0;NfsFrameSaved saved;int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp vec4 tint;void main(){gl_FragColor=tint*texture2D(image,uv).a;}";
 if(!s||!s->generation||!f||f->schema!=1||f->width!=1024||f->height!=512||f->bytes!=1024u*512u||!f->alpha||!f->identity)return 0;
 /* The provider is sealed and immutable; compare its full validated content pin,
    not just its numeric identity, before reusing the owned context resource. */
 if(nfs_soft_gpu_ready(s))return s->fontIdentity==f->identity&&!memcmp(s->fontSha256,f->sha256,32);
 if(s->ownerGeneration&&s->ownerGeneration!=s->generation)return 0;
 if(!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32))return 0;
 glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);if(maxSize<1024||glGetError()!=GL_NO_ERROR)return 0;
 nfs_frame_save(&saved);
 vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);if(!vs)goto done;
 fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);if(!fs)goto done;
 program=glCreateProgram();if(!program)goto done;
 glAttachShader(program,vs);glAttachShader(program,fs);glBindAttribLocation(program,0,"vertex");glBindAttribLocation(program,1,"color");glLinkProgram(program);glGetProgramiv(program,GL_LINK_STATUS,&linked);if(!linked)goto done;
 sampler=glGetUniformLocation(program,"image");matrix=glGetUniformLocation(program,"matrix");if(sampler<0||matrix<0)goto done;
 glActiveTexture(GL_TEXTURE0);glGenTextures(1,&texture);if(!texture)goto done;
 glBindTexture(GL_TEXTURE_2D,texture);nfs_frame_texture_params();glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,1024,512,0,GL_ALPHA,GL_UNSIGNED_BYTE,f->alpha);ok=glGetError()==GL_NO_ERROR;
 done:if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);if(!ok){if(program)glDeleteProgram(program);if(texture)glDeleteTextures(1,&texture);}nfs_frame_restore(&saved);if(!ok)return 0;
 if(s->ownerGeneration==s->generation){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}
 s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->fontIdentity=f->identity;s->ownerGeneration=s->generation;memcpy(s->fontSha256,sha,32);return 1;
}
static int nfs_soft_gpu_vertices(const NfsSoftwareTextStage*t,const NfsSoftwareTextVertex*v,size_t n,float bounds[4]){
 size_t i;float l=640,top=480,r=0,b=0;
 if(!t||!v||!n||n>12288||n%6||(t->logicalWidth!=343&&t->logicalWidth!=640)||t->logicalHeight!=480)return 0;
 for(i=0;i<n;++i){const NfsSoftwareTextVertex*p=v+i;
  if(!isfinite(p->x)||!isfinite(p->y)||!isfinite(p->u)||!isfinite(p->v)||!isfinite(p->r)||!isfinite(p->g)||!isfinite(p->b)||!isfinite(p->a)||p->x<0||p->x>t->logicalWidth||p->y<0||p->y>480||p->u<0||p->u>1||p->v<0||p->v>1||p->a<0||p->a>1||p->r<0||p->g<0||p->b<0||p->r>p->a||p->g>p->a||p->b>p->a)return 0;
  l=fminf(l,p->x);top=fminf(top,p->y);r=fmaxf(r,p->x);b=fmaxf(b,p->y);
 }
 bounds[0]=l;bounds[1]=top;bounds[2]=r;bounds[3]=b;return l<r&&top<b;
}
static int nfs_soft_gpu_clip_contains(const NfsSoftwareTextStage*t,const float*b){
 GLint sc[4];const GLfloat*m=t->sampling.projection;const GLint*vp=t->sampling.viewport;double x0,x1,y0,y1;
 if(!glIsEnabled(GL_SCISSOR_TEST))return 1;
 x0=vp[0]+(m[0]*b[0]+m[12]+1.0)*vp[2]*0.5;x1=vp[0]+(m[0]*b[2]+m[12]+1.0)*vp[2]*0.5;
 y0=vp[1]+(m[5]*b[3]+m[13]+1.0)*vp[3]*0.5;y1=vp[1]+(m[5]*b[1]+m[13]+1.0)*vp[3]*0.5;
 glGetIntegerv(GL_SCISSOR_BOX,sc);return sc[2]>=0&&sc[3]>=0&&sc[0]<=floor(x0)&&sc[1]<=floor(y0)&&(double)sc[0]+sc[2]>=ceil(x1)&&(double)sc[1]+sc[3]>=ceil(y1);
}
int nfs_soft_gpu_draw_into_staging(NfsSoftwareTextGpu*s,const NfsSoftwareTextStage*t,const NfsSoftwareTextVertex*v,size_t n){
 GLint fbo=0,viewport[4];NfsFrameSaved saved;float bounds[4];int ok;
 if(!nfs_soft_gpu_ready(s)||!t||!t->framebuffer||!nfs_frame_dimensions(t->width,t->height)||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling)||!nfs_soft_gpu_vertices(t,v,n,bounds))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_VIEWPORT,viewport);
 if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport)||!nfs_soft_gpu_clip_contains(t,bounds)||glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||glGetError()!=GL_NO_ERROR)return 0;
 nfs_frame_save(&saved);nfs_frame_draw_state();glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection);glUniform1i(s->sampler,0);
 glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,s->texture);glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);
 glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),v);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&v[0].r);
 glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,(GLsizei)n);ok=glGetError()==GL_NO_ERROR;nfs_frame_restore(&saved);return ok;
}
