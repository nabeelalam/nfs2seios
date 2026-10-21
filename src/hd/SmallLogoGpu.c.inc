// SPDX-License-Identifier: MIT
/* Additive authored small-logo draw. Provider identity is not source provenance. */
#include "SmallLogoGpu.h"
#ifndef NFS_SMALL_LOGO_EXTERNAL_FRAME_IMPLEMENTATION
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
#include "SceneGpuSupport.inc"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#endif
#include "TextureDigest.h"
#include <math.h>
static const NfsSmallLogoRegion nfs_logo_regions[5]={{0,4,4,868,368,4,217,92,3,14},{1,880,4,628,268,4,157,67,34,22},{2,4,380,628,268,4,157,67,34,22},{3,640,380,628,268,4,157,67,34,22},{4,1276,380,628,268,4,157,67,34,22}};
static void nfs_logo_forget(NfsSmallLogoGpu*s){s->generation=s->ownerGeneration=s->artIdentity=0;s->program=s->texture=0;s->sampler=s->matrix=-1;memset(s->regions,0,sizeof s->regions);memset(s->artSHA256,0,32);}
int nfs_logo_gpu_context_created(NfsSmallLogoGpu*s,uint64_t generation){if(!s||!generation||generation<=s->lastGeneration||s->generation||s->ownerGeneration)return 0;nfs_logo_forget(s);s->generation=generation;s->lastGeneration=generation;return 1;}
void nfs_logo_gpu_context_retired(NfsSmallLogoGpu*s,uint64_t generation,int ownerCurrent){if(!s||!generation||s->generation!=generation)return;if(ownerCurrent&&s->ownerGeneration==generation){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}nfs_logo_forget(s);}
static int nfs_logo_art_valid(const NfsSmallLogoArt*a,const NfsSmallLogoRegion regions[5]){return a&&regions&&a->pixels&&a->bytes==NFS_SMALL_LOGO_ART_BYTES&&a->width==2048&&a->height==768&&a->schema==1&&a->premultiplied==1&&a->opacityBaked==0&&a->identity!=0&&!memcmp(regions,nfs_logo_regions,sizeof nfs_logo_regions);}
static int nfs_logo_ready(const NfsSmallLogoGpu*s){return s&&s->generation&&s->ownerGeneration==s->generation&&s->artIdentity!=0&&s->program&&s->texture&&s->sampler>=0&&s->matrix>=0&&!memcmp(s->regions,nfs_logo_regions,sizeof nfs_logo_regions)&&glIsProgram(s->program)&&glIsTexture(s->texture);}
int nfs_logo_gpu_prepare(NfsSmallLogoGpu*s,const NfsSmallLogoArt*art,const NfsSmallLogoRegion regions[5]){
 unsigned char sha[32];GLuint vs=0,fs=0,program=0,texture=0;GLint linked=0,sampler=-1,matrix=-1;NfsFrameSaved saved;int ok=0;
 static const char vertex[]="attribute vec4 vertex;attribute float coefficient;uniform mat4 matrix;varying highp vec2 uv;varying highp float opacity;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;opacity=coefficient;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp float opacity;void main(){gl_FragColor=texture2D(image,uv)*opacity;}";
 if(!s||!s->generation||!nfs_logo_art_valid(art,regions))return 0;
 if(nfs_logo_ready(s))return s->artIdentity==art->identity&&!memcmp(s->artSHA256,art->sha256,32);
 if(s->ownerGeneration&&s->ownerGeneration!=s->generation)return 0;
 if(!nfs_texture_sha256(art->pixels,art->bytes,sha)||memcmp(sha,art->sha256,32))return 0;
 {size_t k;for(k=0;k<art->bytes;k+=4)if(art->pixels[k]>art->pixels[k+3]||art->pixels[k+1]>art->pixels[k+3]||art->pixels[k+2]>art->pixels[k+3])return 0;}
 if(glGetError()!=GL_NO_ERROR)return 0;
 nfs_frame_save(&saved);
 vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);if(!vs)goto done;fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);if(!fs)goto done;program=glCreateProgram();if(!program)goto done;glAttachShader(program,vs);glAttachShader(program,fs);glBindAttribLocation(program,0,"vertex");glBindAttribLocation(program,1,"coefficient");glLinkProgram(program);glGetProgramiv(program,GL_LINK_STATUS,&linked);if(!linked)goto done;
 sampler=glGetUniformLocation(program,"image");matrix=glGetUniformLocation(program,"matrix");if(sampler<0||matrix<0)goto done;
 glActiveTexture(GL_TEXTURE0);glGenTextures(1,&texture);if(!texture)goto done;glBindTexture(GL_TEXTURE_2D,texture);nfs_frame_texture_params();glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,2048,768,0,GL_RGBA,GL_UNSIGNED_BYTE,art->pixels);ok=glGetError()==GL_NO_ERROR;
 done:if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);if(!ok){if(program)glDeleteProgram(program);if(texture)glDeleteTextures(1,&texture);}nfs_frame_restore(&saved);if(!ok)return 0;
 if(s->ownerGeneration==s->generation){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->ownerGeneration=s->generation;s->artIdentity=art->identity;memcpy(s->artSHA256,sha,32);memcpy(s->regions,regions,sizeof s->regions);return 1;
}
typedef struct NfsLogoVertex {GLfloat x,y,u,v,coefficient;} NfsLogoVertex;
static int nfs_logo_layers(const NfsSmallLogoLayer*p,size_t count,NfsLogoVertex v[18],size_t*used,float bounds[4]){
 size_t out=0;unsigned i;float l=640,t=480,r=0,b=0;if(!p||count!=3||!used||p[0].role<1||p[0].role>4||p[1].role!=p[0].role%4+1||p[2].role!=0||p[0].scalar256!=256||p[2].scalar256!=256)return 0;
 {unsigned n;int known=0;for(n=0;n<80;++n)if(p[1].scalar256==n*255u/80u)known=1;if(!known)return 0;}
 for(i=0;i<3;++i){const NfsSmallLogoLayer*q=p+i;const NfsSmallLogoRegion*z=nfs_logo_regions+q->role;float a,c,d,e,u0,u1,v0,v1,k;unsigned j;NfsLogoVertex quad[6];
  if(!isfinite(q->left)||!isfinite(q->top)||!isfinite(q->right)||!isfinite(q->bottom)||q->left!=z->sourceOriginX||q->top!=z->sourceOriginY||q->right!=q->left+z->logicalWidth||q->bottom!=q->top+z->logicalHeight)return 0;
  for(j=0;j<4;++j)if(!isfinite(q->clip[j]))return 0;
  if(q->clip[0]<0||q->clip[1]<0||q->clip[2]>640||q->clip[3]>480||q->clip[0]>q->clip[2]||q->clip[1]>q->clip[3])return 0;
  a=fmaxf(q->left,q->clip[0]);c=fmaxf(q->top,q->clip[1]);d=fminf(q->right,q->clip[2]);e=fminf(q->bottom,q->clip[3]);if(a>=d||c>=e)continue;
  u0=(z->x+(a-q->left)*z->scale)/2048.0f;u1=(z->x+(d-q->left)*z->scale)/2048.0f;v0=(z->y+(c-q->top)*z->scale)/768.0f;v1=(z->y+(e-q->top)*z->scale)/768.0f;k=q->scalar256/256.0f;
  {const NfsLogoVertex made[6]={{a,c,u0,v0,k},{d,c,u1,v0,k},{a,e,u0,v1,k},{a,e,u0,v1,k},{d,c,u1,v0,k},{d,e,u1,v1,k}};memcpy(quad,made,sizeof quad);}memcpy(v+out,quad,sizeof quad);out+=6;l=fminf(l,a);t=fminf(t,c);r=fmaxf(r,d);b=fmaxf(b,e);
 }*used=out;bounds[0]=l;bounds[1]=t;bounds[2]=r;bounds[3]=b;return 1;
}
static int nfs_logo_clip_contains(const NfsSmallLogoStage*stage,const float bounds[4],size_t used){
 if(!used||!glIsEnabled(GL_SCISSOR_TEST))return 1;
 {GLint sc[4];const GLfloat*m=stage->sampling.projection;const GLint*vp=stage->sampling.viewport;double x0=vp[0]+(m[0]*bounds[0]+m[12]+1.0)*vp[2]*0.5,x1=vp[0]+(m[0]*bounds[2]+m[12]+1.0)*vp[2]*0.5;double y0=vp[1]+(m[5]*bounds[3]+m[13]+1.0)*vp[3]*0.5,y1=vp[1]+(m[5]*bounds[1]+m[13]+1.0)*vp[3]*0.5;glGetIntegerv(GL_SCISSOR_BOX,sc);return sc[2]>=0&&sc[3]>=0&&sc[0]<=floor(x0)&&sc[1]<=floor(y0)&&(double)sc[0]+sc[2]>=ceil(x1)&&(double)sc[1]+sc[3]>=ceil(y1);}}
int nfs_logo_gpu_draw_into_staging(NfsSmallLogoGpu*s,const NfsSmallLogoStage*stage,const NfsSmallLogoLayer*input,size_t count){
 NfsSmallLogoLayer layers[3];NfsLogoVertex vertices[18];size_t used=0;float bounds[4];GLint fbo=0,viewport[4];NfsFrameSaved saved;int ok=0;
 if(!nfs_logo_ready(s)||!stage||!stage->framebuffer||!input||count!=3||!nfs_frame_sampling_valid(stage->width,stage->height,&stage->sampling))return 0;
 memcpy(layers,input,sizeof layers);if(!nfs_logo_layers(layers,count,vertices,&used,bounds))return 0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_VIEWPORT,viewport);if((GLuint)fbo!=stage->framebuffer||memcmp(viewport,stage->sampling.viewport,sizeof viewport)||!nfs_logo_clip_contains(stage,bounds,used)||glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||glGetError()!=GL_NO_ERROR)return 0;
 if(!used)return 1;
 nfs_frame_save(&saved);nfs_frame_draw_state();glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,stage->sampling.projection);glUniform1i(s->sampler,0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,s->texture);glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),vertices);glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&vertices[0].coefficient);glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,(GLsizei)used);ok=glGetError()==GL_NO_ERROR;nfs_frame_restore(&saved);return ok;
}
