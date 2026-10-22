// SPDX-License-Identifier: MIT
#include "AudioBarGpu.h"
#ifndef NFS_AUDIO_BAR_EXTERNAL_SCENE_IMPLEMENTATION
#include "SceneGpuSupport.inc"
#endif
#include "TextureDigest.h"
#include <math.h>
#include <string.h>
static int nfs_audio_bar_current(const NfsAudioBarGpu*s){return s&&s->generation&&s->contextIdentity&&s->ownerCurrent&&s->ownerCurrent(s->ownerUser,s->generation,s->contextIdentity);}
static void nfs_audio_bar_forget(NfsAudioBarGpu*s){s->generation=s->resourceGeneration=s->artIdentity=0;s->contextIdentity=0;s->ownerCurrent=NULL;s->ownerUser=NULL;s->program=s->texture=0;s->sampler=s->matrix=-1;memset(s->artSha256,0,32);}
int nfs_audio_bar_context_created(NfsAudioBarGpu*s,uint64_t g,uintptr_t c,NfsAudioBarOwnerCurrent check,void*user){
 if(!s||!g||g<=s->lastGeneration||s->generation||s->resourceGeneration||!c||!check||!check(user,g,c))return 0;
 nfs_audio_bar_forget(s);s->generation=s->lastGeneration=g;s->contextIdentity=c;s->ownerCurrent=check;s->ownerUser=user;return 1;
}
void nfs_audio_bar_context_retired(NfsAudioBarGpu*s,uint64_t g,int owner){
 if(!s||!g||g!=s->generation)return;
 if(owner&&nfs_audio_bar_current(s)&&s->resourceGeneration==g){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}
 nfs_audio_bar_forget(s);
}
static int nfs_audio_bar_ready(const NfsAudioBarGpu*s){return nfs_audio_bar_current(s)&&s->resourceGeneration==s->generation&&s->artIdentity&&s->program&&s->texture&&s->sampler>=0&&s->matrix>=0&&glIsProgram(s->program)&&glIsTexture(s->texture);}
int nfs_audio_bar_prepare(NfsAudioBarGpu*s,const NfsAudioArt*a){
 unsigned char digest[32];size_t i;GLuint vs=0,fs=0,program=0,texture=0;GLint linked=0,sampler=-1,matrix=-1,maxSize=0;NfsFrameSaved saved;int ok=0;
 static const char vertex[]="attribute vec4 vertex;uniform mat4 matrix;varying highp vec2 uv;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;}";
 static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;void main(){gl_FragColor=texture2D(image,uv);}";
 if(!nfs_audio_bar_current(s)||!a||!a->pixels||a->bytes!=NFS_AUDIO_ART_BYTES||a->width!=NFS_AUDIO_ART_WIDTH||a->height!=NFS_AUDIO_ART_HEIGHT||a->schema!=NFS_AUDIO_ART_SCHEMA||a->premultiplied!=1||a->opacityBaked!=1||!a->identity)return 0;
 /* Sealed immutable art is fully checked once per actual context, not per frame. */
 if(nfs_audio_bar_ready(s))return s->artIdentity==a->identity&&!memcmp(s->artSha256,a->sha256,32);
 if(s->resourceGeneration&&s->resourceGeneration!=s->generation)return 0;
 if(!nfs_texture_sha256(a->pixels,a->bytes,digest)||memcmp(digest,a->sha256,32))return 0;
 for(i=0;i<a->bytes;i+=4)if(a->pixels[i]>a->pixels[i+3]||a->pixels[i+1]>a->pixels[i+3]||a->pixels[i+2]>a->pixels[i+3])return 0;
 glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);if(maxSize<2048||glGetError()!=GL_NO_ERROR)return 0;
 nfs_frame_save(&saved);
 vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);if(!vs)goto done;fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);if(!fs)goto done;
 program=glCreateProgram();if(!program)goto done;glAttachShader(program,vs);glAttachShader(program,fs);glBindAttribLocation(program,0,"vertex");glLinkProgram(program);glGetProgramiv(program,GL_LINK_STATUS,&linked);if(!linked)goto done;
 sampler=glGetUniformLocation(program,"image");matrix=glGetUniformLocation(program,"matrix");if(sampler<0||matrix<0)goto done;
 glActiveTexture(GL_TEXTURE0);glGenTextures(1,&texture);if(!texture)goto done;glBindTexture(GL_TEXTURE_2D,texture);nfs_frame_texture_params();glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,NFS_AUDIO_ART_WIDTH,NFS_AUDIO_ART_HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,a->pixels);if(!nfs_audio_bar_current(s))goto done;ok=glGetError()==GL_NO_ERROR;
 done:if(!nfs_audio_bar_current(s)){nfs_audio_bar_forget(s);return 0;}if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);if(!ok){if(program)glDeleteProgram(program);if(texture)glDeleteTextures(1,&texture);}nfs_frame_restore(&saved);if(!nfs_audio_bar_current(s)){nfs_audio_bar_forget(s);return 0;}if(!ok)return 0;
 if(s->resourceGeneration==s->generation){if(s->program)glDeleteProgram(s->program);if(s->texture)glDeleteTextures(1,&s->texture);}
 s->program=program;s->texture=texture;s->sampler=sampler;s->matrix=matrix;s->resourceGeneration=s->generation;s->artIdentity=a->identity;memcpy(s->artSha256,digest,32);return 1;
}
static const unsigned nfs_audio_bar_regions[12][4]={{4,4,1098,186},{1106,4,936,60},{1106,72,246,156},{1360,72,228,138},{1596,72,228,138},{1106,236,228,144},{4,388,1098,186},{1106,388,936,60},{1106,456,246,156},{1360,456,228,138},{1596,456,228,138},{1106,620,228,144}};
static int nfs_audio_bar_quad(const NfsAudioBarVertex*q,unsigned asset,unsigned dim,float l,float t,float r,float b,float ol,float ot){
 const unsigned*z=nfs_audio_bar_regions[asset+dim*6];const float u0=(z[0]+(l-ol)*6.0f)/2048.0f,u1=(z[0]+(r-ol)*6.0f)/2048.0f,v0=(z[1]+(t-ot)*6.0f)/768.0f,v1=(z[1]+(b-ot)*6.0f)/768.0f;
 const NfsAudioBarVertex expected[6]={{l,t,u0,v0},{r,t,u1,v0},{l,b,u0,v1},{l,b,u0,v1},{r,t,u1,v0},{r,b,u1,v1}};
 for(unsigned i=0;i<6;++i)if(q[i].x!=expected[i].x||q[i].y!=expected[i].y||q[i].u!=expected[i].u||q[i].v!=expected[i].v)return 0;return 1;
}
static int nfs_audio_bar_vertices(const NfsAudioBarVertex*v,size_t n){
 unsigned dim[3];int selected=-1;if(!v||(n!=72&&n!=78))return 0;
 for(size_t i=0;i<n;++i)if(!isfinite(v[i].x)||!isfinite(v[i].y)||!isfinite(v[i].u)||!isfinite(v[i].v)||v[i].x<0||v[i].x>640||v[i].y<0||v[i].y>480||v[i].u<0||v[i].u>1||v[i].v<0||v[i].v>1)return 0;
 for(unsigned row=0;row<3;++row){const float t=147.0f+27.0f*row;const NfsAudioBarVertex*q=v+row*6;
 if(nfs_audio_bar_quad(q,0,0,409,t,592,t+31,409,t)){if(selected!=-1)return 0;selected=(int)row;dim[row]=0;}else if(nfs_audio_bar_quad(q,0,1,409,t,592,t+31,409,t))dim[row]=1;else return 0;
 }
 for(unsigned row=0;row<3;++row){const NfsAudioBarVertex*q=v+(3+row)*6;const float value=q[1].x-425.0f,t=153.0f+27.0f*row,l=269.0f+value,visible=l<392?392:l;
 if(value<0||value>140||floorf(value)!=value||!nfs_audio_bar_quad(q,1,dim[row],visible,t,425+value,t+10,l,t))return 0;
 q=v+(6+row*2)*6;const float y=147.0f+27.0f*row;if(!nfs_audio_bar_quad(q,2,0,388,y,429,y+26,388,y)||!nfs_audio_bar_quad(q+6,3,dim[row],554,y,592,y+23,554,y))return 0;
 }
 if(n==78){if(selected<0)return 0;const float y=147.0f+27.0f*(unsigned)selected;const NfsAudioBarVertex*q=v+72;if(!nfs_audio_bar_quad(q,4,0,388,y,426,y+23,388,y)&&!nfs_audio_bar_quad(q,5,0,554,y,592,y+24,554,y))return 0;}return 1;
}
static int nfs_audio_bar_scissor(const NfsAudioBarStage*t){
 GLint sc[4];const GLfloat*m=t->sampling.projection;const GLint*vp=t->sampling.viewport;double x0,x1,y0,y1;if(!glIsEnabled(GL_SCISSOR_TEST))return 1;
 x0=vp[0]+(m[0]*388+m[12]+1.0)*vp[2]*0.5;x1=vp[0]+(m[0]*592+m[12]+1.0)*vp[2]*0.5;
 y0=vp[1]+(m[5]*232+m[13]+1.0)*vp[3]*0.5;y1=vp[1]+(m[5]*147+m[13]+1.0)*vp[3]*0.5;
 glGetIntegerv(GL_SCISSOR_BOX,sc);return sc[2]>=0&&sc[3]>=0&&sc[0]<=floor(x0)&&sc[1]<=floor(y0)&&(double)sc[0]+sc[2]>=ceil(x1)&&(double)sc[1]+sc[3]>=ceil(y1);
}
int nfs_audio_bar_draw_into_staging(NfsAudioBarGpu*s,const NfsAudioBarStage*t,const NfsAudioBarVertex*v,size_t n){
 GLint fbo=0,viewport[4],attachmentType=0,attachmentName=0;NfsFrameSaved saved;GLboolean depthMask=GL_TRUE;int ok;
 if(!nfs_audio_bar_ready(s)||!t||t->generation!=s->generation||!t->framebuffer||!nfs_frame_dimensions(t->width,t->height)||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling)||!nfs_audio_bar_vertices(v,n))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_VIEWPORT,viewport);if((GLuint)fbo!=t->framebuffer||memcmp(viewport,t->sampling.viewport,sizeof viewport)||!nfs_audio_bar_scissor(t)||glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||glGetError()!=GL_NO_ERROR)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&attachmentType);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&attachmentName);if((attachmentType==GL_TEXTURE&&(GLuint)attachmentName==s->texture)||glGetError()!=GL_NO_ERROR)return 0;
 nfs_frame_save(&saved);glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);nfs_frame_draw_state();glDepthMask(GL_FALSE);
 glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection);glUniform1i(s->sampler,0);glBindTexture(GL_TEXTURE_2D,s->texture);glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsAudioBarVertex),v);
 glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,(GLsizei)n);if(!nfs_audio_bar_current(s)){nfs_audio_bar_forget(s);return 0;}ok=glGetError()==GL_NO_ERROR;if(!nfs_audio_bar_current(s)){nfs_audio_bar_forget(s);return 0;}glDepthMask(depthMask);nfs_frame_restore(&saved);if(!nfs_audio_bar_current(s)){nfs_audio_bar_forget(s);return 0;}return ok;
}
