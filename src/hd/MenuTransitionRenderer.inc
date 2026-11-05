// SPDX-License-Identifier: MIT
#include "hd/MenuTransitionAbi.h"
typedef struct NfsMenuTransitionGpu {uint64_t context,serial;uintptr_t thread,identity;GLuint image,backup,result,fbo,program;GLint matrix,sampler,nativeDestRgb,nativeDestAlpha;unsigned width,height,selects,triangles;uint32_t addresses[300],ids[300];int active,minimum,maximum,direction,phase,originX,originY,stepX;unsigned drawKind,sourceKind,slot,traceLines;uint64_t uploads,clearEvent;NfsNativeLogoTargetProof target;}NfsMenuTransitionGpu;
static NfsMenuTransitionGpu nfs_mt;
static uintptr_t nfs_mt_thread(void){return (uintptr_t)SDL_GetCurrentThreadID();}
static int nfs_mt_current(void){return nfs_mt.context==nfs_logo_provenance.context&&nfs_mt.identity==(uintptr_t)g_glCtx&&nfs_mt.thread==nfs_mt_thread()&&g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx;}
static void nfs_mt_other(void){nfs_mt.active=0;}
static void nfs_mt_retired(int owner){nfs_mt.active=0;if(owner&&nfs_mt_current()){if(nfs_mt.image)glDeleteTextures(1,&nfs_mt.image);if(nfs_mt.backup)glDeleteTextures(1,&nfs_mt.backup);if(nfs_mt.result)glDeleteTextures(1,&nfs_mt.result);if(nfs_mt.fbo)glDeleteFramebuffers(1,&nfs_mt.fbo);if(nfs_mt.program)glDeleteProgram(nfs_mt.program);}memset(&nfs_mt,0,sizeof nfs_mt);}
static void nfs_mt_created(void){memset(&nfs_mt,0,sizeof nfs_mt);nfs_mt.context=nfs_logo_provenance.context;nfs_mt.identity=(uintptr_t)g_glCtx;nfs_mt.thread=nfs_mt_thread();}
static int nfs_mt_stage_owner(void*user,const NfsBackgroundSceneStage*t){GLint fbo=0,type=0,name=0;if(user!=&nfs_mt||!nfs_mt_current()||!t||t->generation!=nfs_mt.context||t->contextIdentity!=nfs_mt.identity||t->framebuffer!=nfs_mt.fbo||t->width!=nfs_mt.width||t->height!=nfs_mt.height||!nfs_mt.fbo||!nfs_mt.image)return 0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=nfs_mt.fbo)return 0;glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);return type==GL_TEXTURE&&(GLuint)name==nfs_mt.image&&nfs_mt_current();}
static int nfs_mt_prepare(unsigned w,unsigned h){GLuint vs=0,fs=0,p=0;GLint linked=0;NfsBgSaved saved;int ok=0;static const char vertex[]="attribute vec4 vertex;attribute float coefficient;uniform mat4 matrix;varying highp vec2 uv;varying highp float gray;void main(){gl_Position=matrix*vec4(vertex.xy,1.0,1.0);uv=vertex.zw;gray=coefficient;}";static const char fragment[]="precision highp float;uniform sampler2D image;varying highp vec2 uv;varying highp float gray;void main(){vec4 p=texture2D(image,uv);gl_FragColor=vec4(p.rgb*gray,p.a);}";if(!nfs_mt_current()||!nfs_frame_dimensions(w,h))return 0;if(nfs_mt.program&&nfs_mt.width==w&&nfs_mt.height==h&&glIsTexture(nfs_mt.image)&&glIsTexture(nfs_mt.backup)&&glIsTexture(nfs_mt.result)&&glIsFramebuffer(nfs_mt.fbo))return 1;nfs_bg_save(&saved);if(!nfs_mt.program){vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);if(!vs||!fs)goto done;p=glCreateProgram();if(!p)goto done;glAttachShader(p,vs);glAttachShader(p,fs);glBindAttribLocation(p,0,"vertex");glBindAttribLocation(p,1,"coefficient");glLinkProgram(p);glGetProgramiv(p,GL_LINK_STATUS,&linked);if(!linked)goto done;nfs_mt.matrix=glGetUniformLocation(p,"matrix");nfs_mt.sampler=glGetUniformLocation(p,"image");if(nfs_mt.matrix<0||nfs_mt.sampler<0)goto done;nfs_mt.program=p;p=0;}if(!nfs_mt.image)glGenTextures(1,&nfs_mt.image);if(!nfs_mt.backup)glGenTextures(1,&nfs_mt.backup);if(!nfs_mt.result)glGenTextures(1,&nfs_mt.result);if(!nfs_mt.fbo)glGenFramebuffers(1,&nfs_mt.fbo);if(!nfs_mt.image||!nfs_mt.backup||!nfs_mt.result||!nfs_mt.fbo||nfs_mt.image==nfs_mt.backup||nfs_mt.image==nfs_mt.result||nfs_mt.backup==nfs_mt.result||nfs_mt.image==g_framebufferTexture||nfs_mt.backup==g_framebufferTexture||nfs_mt.result==g_framebufferTexture||nfs_mt.fbo==g_framebuffer)goto done;glActiveTexture(GL_TEXTURE0);for(unsigned i=0;i<3;++i){glBindTexture(GL_TEXTURE_2D,i==0?nfs_mt.image:i==1?nfs_mt.backup:nfs_mt.result);nfs_frame_texture_params();glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);}glBindFramebuffer(GL_FRAMEBUFFER,nfs_mt.fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mt.image,0);if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)goto done;nfs_mt.width=w;nfs_mt.height=h;ok=glGetError()==GL_NO_ERROR;done:if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);if(p)glDeleteProgram(p);nfs_bg_restore(&saved);return ok&&nfs_mt_current();}
static int nfs_mt_gray(unsigned column,int right){if(nfs_mt.phase== -1)return 255;int offset=nfs_mt.direction?(right?228:240)-12*(int)column:(right?12:0)+12*(int)column;int v=2*nfs_mt.phase-247+offset;if(v<nfs_mt.minimum)v=nfs_mt.minimum;if(v>nfs_mt.maximum)v=nfs_mt.maximum;return v;}
static int nfs_mt_vertex(const GrVertex*v,unsigned tile,unsigned corner){unsigned col=tile/15,row=tile%15;int right=corner==1||corner==2;float gray=(float)nfs_mt_gray(col,right);return v&&v->x==(float)(nfs_mt.originX+(int)col*nfs_mt.stepX+(right?nfs_mt.stepX:0))+VertexSnap&&v->y==(float)(nfs_mt.originY+row*32+(corner>=2?32:0))+VertexSnap&&v->r==gray&&v->g==gray&&v->b==gray&&v->a==255&&v->oow==1&&v->tmuvtx[0].sow==(right?255:0)&&v->tmuvtx[0].tow==(corner>=2?255:0);}
static void nfs_mt_select(unsigned tmu,uint32_t address,uint32_t parity,const GrTexInfo*info,TextureInfo*ti){if(!nfs_mt.active)return;unsigned i=nfs_mt.selects;if(i>=300||nfs_mt.triangles!=i*2||tmu||parity!=3||!info||!ti||info->format!=GR_TEXFMT_ARGB_1555||info->smallLod!=3||info->largeLod!=3||info->aspectRatio!=3||address!=nfs_mt.addresses[i]||ti->id!=nfs_mt.ids[i]){nfs_mt.active=0;return;}++nfs_mt.selects;}
static void nfs_mt_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){if(!nfs_mt.active)return;unsigned i=nfs_mt.triangles/2,k=nfs_mt.triangles&1;if(i>=300||nfs_mt.selects!=i+1||!g_textureEnabled||g_fogEnabled||!nfs_mt_vertex(a,i,0)||!nfs_mt_vertex(b,i,k?2:1)||!nfs_mt_vertex(c,i,k?3:2)){nfs_mt.active=0;return;}++nfs_mt.triangles;}
static int nfs_mt_native_blend(GLint*dstRgb,GLint*dstAlpha){
 GLint sr=0,sa=0,dr=0,da=0,eqr=0,eqa=0;
 if(!glIsEnabled(GL_BLEND))return 0;
 glGetIntegerv(GL_BLEND_SRC_RGB,&sr);glGetIntegerv(GL_BLEND_SRC_ALPHA,&sa);glGetIntegerv(GL_BLEND_DST_RGB,&dr);glGetIntegerv(GL_BLEND_DST_ALPHA,&da);glGetIntegerv(GL_BLEND_EQUATION_RGB,&eqr);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&eqa);
 if(sr!=GL_SRC_ALPHA||sa!=GL_SRC_ALPHA||eqr!=GL_FUNC_ADD||eqa!=GL_FUNC_ADD||(dr!=GL_ZERO&&dr!=GL_ONE&&dr!=GL_ONE_MINUS_SRC_ALPHA)||(da!=GL_ZERO&&da!=GL_ONE&&da!=GL_ONE_MINUS_SRC_ALPHA))return 0;
 *dstRgb=dr;*dstAlpha=da;return glGetError()==GL_NO_ERROR;
}
int nfs_private_menu_transition_begin(const NfsMenuTransitionView*v){NfsNativeLogoTargetProof target;NfsBgSaved saved;NfsBackgroundSceneStage stage;GLfloat previousClear[4];GLint dstRgb=0,dstAlpha=0;int ok=0;static const int32_t full[4]={0,0,640,480};nfs_mt.active=0;if(!v||!nfs_bg_feature()||!nfs_mt_current()||!v->serial||!v->content||v->slot>1||(v->kind<1||v->kind>4)||(v->drawKind!=1&&v->drawKind!=2)||v->originX< -32768||v->originX>32767||v->originY< -32768||v->originY>32767||v->stepX<0||v->stepX>32||v->mapRole>1||!v->original565||v->minimum<0||v->maximum>255||v->minimum>v->maximum||v->direction<0||v->direction>1||v->phase< -1||v->phase>255)return 0;drawTriangles();if(!nfs_mt_current()||!nfs_logo_target_with_fog_policy(&target,v->serial,1)||!nfs_mt_native_blend(&dstRgb,&dstAlpha))return 0;if((v->kind==1||v->kind==3)&&(!nfs_logo_black_clear.valid||nfs_logo_black_clear.context!=nfs_mt.context||nfs_logo_black_clear.epoch!=nfs_logo_provenance.clearEvent||nfs_logo_black_clear.fbo!=target.framebuffer))return 0;for(unsigned i=0;i<300;++i){uint32_t address=v->addresses[i];if((address&3)||address>TextureMem-2048)return 0;TextureInfo*ti=&g_textures[address>>2];if(!ti->id||ti->size!=32||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+address||!nfs_logo_native_upload_valid(address,ti))return 0;for(unsigned j=0;j<i;++j)if(ti->id==nfs_mt.ids[j]||((uint64_t)address<(uint64_t)v->addresses[j]+2048&&(uint64_t)v->addresses[j]<(uint64_t)address+2048))return 0;for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t raw=v->original565[((i%15)*32+y)*640+(i/15)*32+x];uint16_t a=(uint16_t)(((raw&0xf800)>>1)|((raw&0x7c0)>>1)|(raw&31));a=a?(uint16_t)(a|0x8000):0;uint16_t expected=(uint16_t)((a<<1)|(a>>15)),actual;memcpy(&actual,(const uint8_t*)ti->data+2*(y*32+x),2);if(actual!=expected)return 0;}nfs_mt.addresses[i]=address;nfs_mt.ids[i]=ti->id;}
 /* All original tile payloads have now been verified. Queued draws were
    flushed before admission, so frontend animation can clear inherited fog. */
 if(g_fogEnabled){
  g_fogEnabled=0.0f;
  glUniform1f(g_uFogEnabledLoc,g_fogEnabled);
 }
 if(!nfs_logo_texture_filters(nfs_mt.ids,target.sampling.magFilter)||!nfs_bg_scene_prepare(&nfs_bg_compositor)||!nfs_mt_prepare(target.width,target.height))return 0;nfs_bg_save(&saved);glGetFloatv(GL_COLOR_CLEAR_VALUE,previousClear);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,nfs_mt.backup);glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,target.width,target.height);if(glGetError()!=GL_NO_ERROR)goto done;glBindFramebuffer(GL_FRAMEBUFFER,nfs_mt.fbo);glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]);nfs_frame_draw_state();glDepthMask(GL_FALSE);glClearColor(0,0,0,0);glClear(GL_COLOR_BUFFER_BIT);memset(&stage,0,sizeof stage);stage.generation=nfs_mt.context;stage.contextIdentity=nfs_mt.identity;stage.framebuffer=nfs_mt.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_mt_stage_owner;stage.ownerUser=&nfs_mt;if(v->kind==1){glUseProgram(nfs_bg_compositor.program);glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,target.sampling.projection);ok=nfs_bg_stage_quad(&nfs_bg_compositor,&stage,v->mapRole,0,0,640,480,0,0,nfs_bg_compositor.art[v->mapRole].width,nfs_bg_compositor.art[v->mapRole].height,full,256,0);}else ok=nfs_bg_scene_draw_parts_into_staging(&nfs_bg_compositor,&stage,&v->layers,v->kind==3);done:glClearColor(previousClear[0],previousClear[1],previousClear[2],previousClear[3]);nfs_bg_restore(&saved);if(!ok||!nfs_mt_current())return 0;nfs_mt.nativeDestRgb=dstRgb;nfs_mt.nativeDestAlpha=dstAlpha;nfs_mt.serial=v->serial;nfs_mt.target=target;nfs_mt.minimum=v->minimum;nfs_mt.maximum=v->maximum;nfs_mt.direction=v->direction;nfs_mt.phase=v->phase;nfs_mt.originX=v->originX;nfs_mt.originY=v->originY;nfs_mt.stepX=v->stepX;nfs_mt.drawKind=v->drawKind;nfs_mt.sourceKind=v->kind;nfs_mt.slot=v->slot;nfs_mt.uploads=nfs_logo_provenance.uploads;nfs_mt.clearEvent=nfs_logo_provenance.clearEvent;nfs_mt.selects=nfs_mt.triangles=0;nfs_mt.active=1;return 1;}
static int nfs_mt_attached(GLuint texture){
 GLint fbo=0,type=0,name=0;
 if(!nfs_mt_current()||!texture||texture==g_framebufferTexture)return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=nfs_mt.fbo)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 return type==GL_TEXTURE&&(GLuint)name==texture&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE&&nfs_mt_current();
}
static int nfs_mt_same_target(uint64_t serial){NfsNativeLogoTargetProof target;GLint dr=0,da=0;return nfs_mt_native_blend(&dr,&da)&&dr==nfs_mt.nativeDestRgb&&da==nfs_mt.nativeDestAlpha&& nfs_mt_current()&&nfs_logo_provenance.uploads==nfs_mt.uploads&&nfs_logo_provenance.clearEvent==nfs_mt.clearEvent&&nfs_logo_target(&target,serial)&&!memcmp(&target,&nfs_mt.target,sizeof target);}
static void nfs_mt_trace(const char*event){
 if(!nfs_logo_trace_enabled()||nfs_mt.traceLines>=2048)return;++nfs_mt.traceLines;
 fprintf(stderr,"MENU TRANSITION %s serial=%llu kind=%u slot=%u draw=%u phase=%d origin=%d,%d step=%d selections=%u triangles=%u\n",event,(unsigned long long)nfs_mt.serial,nfs_mt.sourceKind,nfs_mt.slot,nfs_mt.drawKind,nfs_mt.phase,nfs_mt.originX,nfs_mt.originY,nfs_mt.stepX,nfs_mt.selects,nfs_mt.triangles);
}
int nfs_private_menu_transition_finish(uint64_t serial){
 NfsBgSaved saved;int ok=0;
 static const GLfloat copy[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};
 drawTriangles();
 if(!nfs_mt.active||nfs_mt.serial!=serial||nfs_mt.selects!=300||nfs_mt.triangles!=600||!nfs_mt_same_target(serial)){nfs_mt.active=0;return 0;}
 nfs_mt.active=0;nfs_bg_save(&saved);
 // Complete replacement is first composed privately. Any failure up to commit
 // leaves all already-rendered original color and depth untouched.
 glBindFramebuffer(GL_FRAMEBUFFER,nfs_mt.fbo);
 glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mt.result,0);
 if(!nfs_mt_attached(nfs_mt.result))goto restore;
 nfs_frame_draw_state();glDepthMask(GL_FALSE);glViewport(0,0,nfs_mt.width,nfs_mt.height);
 glUseProgram(nfs_bg_compositor.copyProgram);glUniform1i(nfs_bg_compositor.copySampler,0);
 glBindTexture(GL_TEXTURE_2D,nfs_mt.backup);glEnableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);
 glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy);glDrawArrays(GL_TRIANGLES,0,6);
 glUseProgram(nfs_mt.program);glUniformMatrix4fv(nfs_mt.matrix,1,GL_FALSE,nfs_mt.target.sampling.projection);glUniform1i(nfs_mt.sampler,0);
 glBindTexture(GL_TEXTURE_2D,nfs_mt.image);glEnableVertexAttribArray(1);glEnable(GL_BLEND);
 glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,(GLenum)nfs_mt.nativeDestRgb,GL_ONE,(GLenum)nfs_mt.nativeDestAlpha);
 for(unsigned col=0;col<20;++col){
  GLfloat l=(GLfloat)(nfs_mt.originX+(int)col*nfs_mt.stepX),r=l+nfs_mt.stepX;
  GLfloat t=(GLfloat)nfs_mt.originY,b=t+480,u0=col/20.f,u1=(col+1)/20.f,c0=nfs_mt_gray(col,0)/255.f,c1=nfs_mt_gray(col,1)/255.f;
  const GLfloat vertices[]={l,t,u0,1,c0,r,t,u1,1,c1,r,b,u1,0,c1,l,t,u0,1,c0,r,b,u1,0,c1,l,b,u0,0,c0};
  glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),vertices);glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),vertices+4);glDrawArrays(GL_TRIANGLES,0,6);
 }
 ok=nfs_mt_attached(nfs_mt.result)&&glGetError()==GL_NO_ERROR;
 restore:
 if(!nfs_mt_current())return 0;
 glBindFramebuffer(GL_FRAMEBUFFER,nfs_mt.fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mt.image,0);
 nfs_bg_restore(&saved);
 if(!ok||!nfs_mt_same_target(serial))return 0;
 nfs_bg_save(&saved);nfs_frame_draw_state();glDepthMask(GL_FALSE);glViewport(0,0,nfs_mt.width,nfs_mt.height);
 glUseProgram(nfs_bg_compositor.copyProgram);glUniform1i(nfs_bg_compositor.copySampler,0);glBindTexture(GL_TEXTURE_2D,nfs_mt.result);
 glEnableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy);glDrawArrays(GL_TRIANGLES,0,6);
 ok=glGetError()==GL_NO_ERROR;if(nfs_mt_current())nfs_bg_restore(&saved);else return 0;
 nfs_mt_trace(ok?"finish-published":"commit-error");return ok;
}
