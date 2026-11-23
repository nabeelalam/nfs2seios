// SPDX-License-Identifier: MIT
#include "CreditsWholeGpu.h"
#include "TextureDigest.h"
static int nfs_cwg_context(const NfsCreditsWholeGpu*s){return s&&nfs_graphics_scene_current(&s->context);}
static int nfs_cwg_current(const NfsCreditsWholeGpu*s){return nfs_cwg_context(s)&&s->frame.owner&&s->frame.current&&s->frame.current(s->frame.owner)&&s->frame.art.generation==nfs_credits_art_generation();}
static void nfs_cwg_revoke(NfsCreditsWholeGpu*s){if(!s)return;NfsCreditsWholeView frame=s->frame;memset(&s->frame,0,sizeof s->frame);s->ready=0;if(frame.owner&&frame.release)frame.release(frame.owner);}
#define NFS_CWG_GL(call) do{if(!nfs_cwg_current(s))goto lost;call;if(!nfs_cwg_current(s))goto lost;}while(0)
#define NFS_CWG_CONTEXT_GL(call) do{if(!nfs_cwg_context(s))goto done;call;if(!nfs_cwg_context(s))goto done;}while(0)
static void nfs_cwg_retire(NfsCreditsWholeGpu*s,int owner){
 if(!s)return;if(owner&&nfs_cwg_context(s)){
  if(s->fbo){NFS_CWG_CONTEXT_GL(glDeleteFramebuffers(1,&s->fbo));}
  if(s->color){NFS_CWG_CONTEXT_GL(glDeleteTextures(1,&s->color));}
  if(s->mask){NFS_CWG_CONTEXT_GL(glDeleteTextures(1,&s->mask));}
  for(unsigned i=0;i<NFS_CREDITS_ART_COUNT;++i)if(s->images[i]){NFS_CWG_CONTEXT_GL(glDeleteTextures(1,s->images+i));}
  if(s->program){NFS_CWG_CONTEXT_GL(glDeleteProgram(s->program));}
 }
 done:nfs_cwg_revoke(s);memset(s,0,sizeof *s);
}
static int nfs_cwg_rect(const NfsCreditsWholeRect*r){return r&&isfinite(r->left)&&isfinite(r->top)&&isfinite(r->right)&&isfinite(r->bottom)&&r->left>= -8192&&r->top>= -8192&&r->right<=8192&&r->bottom<=8192&&r->left<r->right&&r->top<r->bottom;}
static int nfs_cwg_valid(const NfsCreditsWholeView*v){
 if(!v||v->schema!=1||!v->request||!v->content||v->locale>=6||v->art.schema!=1||!v->art.generation||!v->owner||!v->retain||!v->release||!v->current||!v->current(v->owner)||v->art.generation!=nfs_credits_art_generation()||v->text.logicalWidth!=640||v->text.logicalHeight!=480||v->text.vertexCount>16384||(v->text.vertexCount&&!v->text.vertices)||v->eventCount>129||(v->eventCount&&!v->events)||v->hasPartner>1||!nfs_cwg_rect(&v->heading))return 0;
 size_t next=0;unsigned partners=0;
 for(size_t i=0;i<v->eventCount;++i){const NfsCreditsWholeEvent*e=v->events+i;if(e->kind==1){if(e->first!=next||e->count%6||e->count>v->text.vertexCount-next)return 0;next+=e->count;}else if(e->kind==2){if(e->first||e->count||++partners>1||!v->hasPartner)return 0;}else return 0;}
 if(next!=v->text.vertexCount||partners!=v->hasPartner)return 0;
 if(v->hasPartner&&(!nfs_cwg_rect(v->partner)||!nfs_cwg_rect(v->partner+1)||!nfs_cwg_rect(&v->partnerClip)||v->partnerClip.left<0||v->partnerClip.top<0||v->partnerClip.right>640||v->partnerClip.bottom>480))return 0;
 for(size_t i=0;i<v->text.vertexCount;++i){const NfsSoftwareTextVertex*p=v->text.vertices+i;const float a[]={p->x,p->y,p->u,p->v,p->r,p->g,p->b,p->a};for(unsigned k=0;k<8;++k)if(!isfinite(a[k]))return 0;if(p->x<0||p->x>640||p->y<0||p->y>480||p->u<0||p->u>1||p->v<0||p->v>1||p->r<0||p->r>1||p->g<0||p->g>1||p->b<0||p->b>1||p->a<0||p->a>1||p->r>p->a||p->g>p->a||p->b>p->a)return 0;}
 return 1;
}
// Cleanup uses actual context ownership even if source content was retired.
static void nfs_cwg_delete_shader(NfsCreditsWholeGpu*s,GLuint*name,int program){if(!name||!*name)return;const GLuint id=*name;*name=0;if(!nfs_cwg_context(s))return;if(program)glDeleteProgram(id);else glDeleteShader(id);(void)nfs_cwg_context(s);}
static GLuint nfs_cwg_shader(NfsCreditsWholeGpu*s,GLenum type,const char*source){GLuint shader=0;GLint compiled=0;
 NFS_CWG_GL(shader=glCreateShader(type));if(!shader)return 0;NFS_CWG_GL(glShaderSource(shader,1,&source,NULL));NFS_CWG_GL(glCompileShader(shader));NFS_CWG_GL(glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled));if(compiled)return shader;
 lost:nfs_cwg_delete_shader(s,&shader,0);return 0;
}
static int nfs_cwg_program(NfsCreditsWholeGpu*s){GLuint vs=0,fs=0,p=0;GLint linked=0;int okay=0;
 static const char vertex[]="attribute vec4 vertex;attribute vec4 color;uniform mat4 matrix;varying highp vec2 uv;varying lowp vec4 tint;void main(){gl_Position=matrix*vec4(vertex.xy,0.,1.);uv=vertex.zw;tint=color;}";
 static const char fragment[]="precision highp float;varying highp vec2 uv;varying lowp vec4 tint;uniform sampler2D image;uniform int kind;void main(){vec4 p=texture2D(image,uv);gl_FragColor=kind==1?vec4(tint.rgb*p.a,tint.a*p.a):p*tint;}";
 if(s->program)return 1;vs=nfs_cwg_shader(s,GL_VERTEX_SHADER,vertex);fs=nfs_cwg_shader(s,GL_FRAGMENT_SHADER,fragment);if(!vs||!fs)goto done;
 NFS_CWG_GL(p=glCreateProgram());if(!p)goto done;NFS_CWG_GL(glAttachShader(p,vs));NFS_CWG_GL(glAttachShader(p,fs));NFS_CWG_GL(glBindAttribLocation(p,0,"vertex"));NFS_CWG_GL(glBindAttribLocation(p,1,"color"));NFS_CWG_GL(glLinkProgram(p));NFS_CWG_GL(glGetProgramiv(p,GL_LINK_STATUS,&linked));if(!linked)goto done;
 NFS_CWG_GL(s->matrix=glGetUniformLocation(p,"matrix"));NFS_CWG_GL(s->sampler=glGetUniformLocation(p,"image"));NFS_CWG_GL(s->kind=glGetUniformLocation(p,"kind"));if(s->matrix<0||s->sampler<0||s->kind<0)goto done;s->program=p;p=0;okay=1;
 done:nfs_cwg_delete_shader(s,&vs,0);nfs_cwg_delete_shader(s,&fs,0);nfs_cwg_delete_shader(s,&p,1);return okay&&nfs_cwg_current(s);
 lost:okay=0;goto done;
}
static int nfs_cwg_texture(NfsCreditsWholeGpu*s,GLuint*name,unsigned w,unsigned h,GLenum format,const void*pixels){GLenum error=0;
 if(!*name){NFS_CWG_GL(glGenTextures(1,name));}if(!*name||*name==s->color)return 0;
 NFS_CWG_GL(glActiveTexture(GL_TEXTURE0));NFS_CWG_GL(glBindTexture(GL_TEXTURE_2D,*name));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CWG_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_CWG_GL(glTexImage2D(GL_TEXTURE_2D,0,format,w,h,0,format,GL_UNSIGNED_BYTE,pixels));NFS_CWG_GL(error=glGetError());return error==GL_NO_ERROR;
 lost:return 0;
}
static int nfs_cwg_images(NfsCreditsWholeGpu*s,GLint maximum){
 if(s->artGeneration==s->frame.art.generation)return 1;s->artGeneration=0;
 for(unsigned i=0;i<NFS_CREDITS_ART_COUNT;++i){const NfsCreditsArtImage*im=s->frame.art.images+i;unsigned char sha[32];if(!im->width||!im->height||im->width>(unsigned)maximum||im->height>(unsigned)maximum||!im->pixels||im->bytes!=(size_t)im->width*im->height*4||!nfs_texture_sha256(im->pixels,im->bytes,sha)||memcmp(sha,im->sha256,32))return 0;for(size_t k=0;k<im->bytes;k+=4){const unsigned a=im->pixels[k+3];if(im->pixels[k]>a||im->pixels[k+1]>a||im->pixels[k+2]>a||(!i&&a!=255))return 0;}if(!nfs_cwg_texture(s,s->images+i,im->width,im->height,GL_RGBA,im->pixels))return 0;}
 s->artGeneration=s->frame.art.generation;return 1;
}
static int nfs_cwg_font(NfsCreditsWholeGpu*s,GLint maximum){const NfsSoftwareTextFontView*f=&s->frame.text.font;unsigned char sha[32];
 if(!s->frame.text.vertexCount)return 1;
 if(f->schema!=1||f->width!=1024||f->height!=512||maximum<1024||!f->identity||!f->alpha||f->bytes!=524288)return 0;
 if(s->mask&&s->fontIdentity==f->identity&&!memcmp(s->fontSha,f->sha256,32))return 1;
 s->fontIdentity=0;if(!nfs_texture_sha256(f->alpha,f->bytes,sha)||memcmp(sha,f->sha256,32)||!nfs_cwg_texture(s,&s->mask,1024,512,GL_ALPHA,f->alpha))return 0;s->fontIdentity=f->identity;memcpy(s->fontSha,sha,32);return 1;
}
static int nfs_cwg_target(NfsCreditsWholeGpu*s,unsigned w,unsigned h){GLenum status=0,error=0;
 if(!s->color){NFS_CWG_GL(glGenTextures(1,&s->color));}if(!s->fbo){NFS_CWG_GL(glGenFramebuffers(1,&s->fbo));}if(!s->color||!s->fbo||s->fbo==s->context.framebuffer)return 0;
 NFS_CWG_GL(glActiveTexture(GL_TEXTURE0));NFS_CWG_GL(glBindTexture(GL_TEXTURE_2D,s->color));
 if(s->width!=w||s->height!=h){s->width=s->height=0;NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_CWG_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_CWG_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));}
 NFS_CWG_GL(glBindFramebuffer(GL_FRAMEBUFFER,s->fbo));NFS_CWG_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,s->color,0));NFS_CWG_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_CWG_GL(error=glGetError());if(status!=GL_FRAMEBUFFER_COMPLETE||error!=GL_NO_ERROR){s->width=s->height=0;return 0;}s->width=w;s->height=h;return 1;
 lost:return 0;
}
static int nfs_cwg_attachment(NfsCreditsWholeGpu*s){GLint bound=0,type=0,name=0;
 NFS_CWG_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&bound));if((GLuint)bound!=s->fbo)return 0;NFS_CWG_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CWG_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==s->color;
 lost:return 0;
}
static void nfs_cwg_matrix(float*m,float logical,int text){memset(m,0,16*sizeof(float));m[0]=2/logical;m[5]=-2.f/480;m[10]=m[15]=1;m[12]=text? -640.f/logical:-1;m[13]=1;}
static int nfs_cwg_quad(NfsCreditsWholeGpu*s,unsigned role,NfsCreditsWholeRect r,const NfsCreditsWholeRect*clip,float shift,float logical,int cover){
 const NfsCreditsArtImage*im=s->frame.art.images+role;float u0=0,v0=0,u1=1,v1=1,m[16];
 if(cover){const float imageAspect=(float)im->width/im->height,aspect=(r.right-r.left)/(r.bottom-r.top);if(imageAspect>aspect){float extent=aspect/imageAspect;u0=(1-extent)/2;u1=1-u0;}else{float extent=imageAspect/aspect;v0=(1-extent)/2;v1=1-v0;}}
 if(clip){const float l=fmaxf(r.left,clip->left),t=fmaxf(r.top,clip->top),rr=fminf(r.right,clip->right),b=fminf(r.bottom,clip->bottom);if(l>=rr||t>=b)return 1;const float w=r.right-r.left,h=r.bottom-r.top,du=u1-u0,dv=v1-v0;u1=u0+(rr-r.left)/w*du;v1=v0+(b-r.top)/h*dv;u0+=(l-r.left)/w*du;v0+=(t-r.top)/h*dv;r.left=l;r.top=t;r.right=rr;r.bottom=b;}
 r.left+=shift;r.right+=shift;const GLfloat q[]={r.left,r.top,u0,v0,r.left,r.bottom,u0,v1,r.right,r.top,u1,v0,r.right,r.top,u1,v0,r.left,r.bottom,u0,v1,r.right,r.bottom,u1,v1};const GLfloat white[]={1,1,1,1};nfs_cwg_matrix(m,logical,0);
 NFS_CWG_GL(glUseProgram(s->program));NFS_CWG_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,m));NFS_CWG_GL(glUniform1i(s->sampler,0));NFS_CWG_GL(glUniform1i(s->kind,0));NFS_CWG_GL(glActiveTexture(GL_TEXTURE0));NFS_CWG_GL(glBindTexture(GL_TEXTURE_2D,s->images[role]));NFS_CWG_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CWG_GL(glEnableVertexAttribArray(0));NFS_CWG_GL(glDisableVertexAttribArray(1));NFS_CWG_GL(glDisableVertexAttribArray(2));NFS_CWG_GL(glDisableVertexAttribArray(3));NFS_CWG_GL(glVertexAttrib4fv(1,white));NFS_CWG_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));NFS_CWG_GL(glDrawArrays(GL_TRIANGLES,0,6));return 1;
 lost:return 0;
}
static int nfs_cwg_text(NfsCreditsWholeGpu*s,const NfsCreditsWholeEvent*e,float logical){float m[16];if(!e->count)return 1;const NfsSoftwareTextVertex*v=s->frame.text.vertices+e->first;nfs_cwg_matrix(m,logical,1);
 NFS_CWG_GL(glUseProgram(s->program));NFS_CWG_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,m));NFS_CWG_GL(glUniform1i(s->sampler,0));NFS_CWG_GL(glUniform1i(s->kind,1));NFS_CWG_GL(glActiveTexture(GL_TEXTURE0));NFS_CWG_GL(glBindTexture(GL_TEXTURE_2D,s->mask));NFS_CWG_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CWG_GL(glEnableVertexAttribArray(0));NFS_CWG_GL(glEnableVertexAttribArray(1));NFS_CWG_GL(glDisableVertexAttribArray(2));NFS_CWG_GL(glDisableVertexAttribArray(3));NFS_CWG_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof *v,v));NFS_CWG_GL(glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof *v,&v->r));NFS_CWG_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)e->count));return 1;
 lost:return 0;
}
static int nfs_cwg_render(NfsCreditsWholeGpu*s,const NfsCreditsWholeView*v,const NfsBackgroundSceneStage*context,unsigned w,unsigned h){
 NfsBgSaved saved;GLint maximum=0,viewport[2]={0,0};GLenum error=0;int savedActive=0,okay=0;float logical=0;
 if(!s)return 0;nfs_cwg_revoke(s);
 if(!context||!nfs_graphics_scene_current(context)||!nfs_cwg_valid(v)||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS||(int64_t)w*3<(int64_t)h*4||(int64_t)w*9>(int64_t)h*16+16)return 0;
 if(s->context.generation&&(s->context.generation!=context->generation||s->context.contextIdentity!=context->contextIdentity||s->context.ownerCurrent!=context->ownerCurrent||s->context.ownerUser!=context->ownerUser))return 0;
 s->context=*context;v->retain(v->owner);s->frame=*v;if(!nfs_graphics_scene_save(&saved,&s->context))goto lost;savedActive=1;
 NFS_CWG_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum));NFS_CWG_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,viewport));if(maximum<=0||w>(unsigned)maximum||h>(unsigned)maximum||w>(unsigned)viewport[0]||h>(unsigned)viewport[1])goto done;
 if(!nfs_cwg_program(s)||!nfs_cwg_images(s,maximum)||!nfs_cwg_font(s,maximum)||!nfs_cwg_target(s,w,h)||!nfs_cwg_attachment(s))goto done;
 for(unsigned i=0;i<8;++i){NFS_CWG_GL(glDisable(nfs_frame_caps[i]));}NFS_CWG_GL(glColorMask(1,1,1,1));NFS_CWG_GL(glDepthMask(GL_FALSE));NFS_CWG_GL(glViewport(0,0,w,h));logical=480.f*w/h;
 {const NfsCreditsWholeRect full={0,0,logical,480},logo={3,14,220,106},watermark={106,84,156,504};if(!nfs_cwg_quad(s,0,full,NULL,0,logical,1))goto done;
  NFS_CWG_GL(glEnable(GL_BLEND));NFS_CWG_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_CWG_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));
  if(!nfs_cwg_quad(s,2,watermark,NULL,0,logical,0)||!nfs_cwg_quad(s,1,logo,NULL,0,logical,0))goto done;
  NfsCreditsWholeRect heading=v->heading;heading.left+=logical-640;heading.right+=logical-640;if(!nfs_cwg_quad(s,3+v->locale,heading,NULL,0,logical,0))goto done;
 }
 for(size_t i=0;i<v->eventCount;++i){const NfsCreditsWholeEvent*e=v->events+i;if(e->kind==1){NfsCreditsWholeEvent batch=*e;while(i+1<v->eventCount&&v->events[i+1].kind==1){++i;batch.count+=v->events[i].count;}if(!nfs_cwg_text(s,&batch,logical))goto done;}else{if(!nfs_cwg_quad(s,9,v->partner[0],&v->partnerClip,(logical-640)/2,logical,0)||!nfs_cwg_quad(s,10,v->partner[1],&v->partnerClip,(logical-640)/2,logical,0))goto done;}}
 NFS_CWG_GL(error=glGetError());okay=error==GL_NO_ERROR&&nfs_cwg_attachment(s);
 done:if(savedActive&&!nfs_graphics_scene_restore(&saved,&s->context))goto lost;savedActive=0;if(!okay||!nfs_cwg_current(s)){nfs_cwg_revoke(s);return 0;}s->ready=1;return 1;
 lost:if(savedActive&&nfs_cwg_context(s))(void)nfs_graphics_scene_restore(&saved,&s->context);nfs_cwg_revoke(s);return 0;
}
static GLuint nfs_cwg_staging_color(const NfsCreditsWholeGpu*s){return s&&s->ready&&nfs_cwg_current(s)?s->color:0;}
#undef NFS_CWG_GL
#undef NFS_CWG_CONTEXT_GL
