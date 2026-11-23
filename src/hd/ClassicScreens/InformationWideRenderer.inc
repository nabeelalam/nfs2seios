// SPDX-License-Identifier: MIT
// Reframe an authenticated, already rendered information page. Its center
// preserves the native art, text, graph and cursor at a uniform scale.
#include "HistoryPanoramaAbi.h"
static struct {
 GLuint program, fbo, color,panoramaTexture;
 NfsHistoryPanoramaView panorama;unsigned char panoramaSha[32];unsigned panoramaWidth,panoramaHeight;
 GLint image, centerWidth, leftTint, rightTint, edgeInset, reflectionDepth, photoBackdrop,panoramaSampler,hasPanorama,panoramaCenter,panoramaTexel;
 unsigned width, height, page, nativeWidth,lastMarkedPage;
 uintptr_t context;
 uint64_t generation, clearEvent;
 GLuint nativeFbo, nativeColor;
} nfs_info_wide;

static void nfs_info_wide_clear_panorama(void){
 void*owner=nfs_info_wide.panorama.owner;void(*release)(void*)=nfs_info_wide.panorama.release;
 memset(&nfs_info_wide.panorama,0,sizeof nfs_info_wide.panorama);if(owner&&release)release(owner);
}
static int nfs_info_wide_current(void){
 return nfs_info_wide.context && nfs_info_wide.context==(uintptr_t)g_glCtx
     && SDL_GL_GetCurrentContext()==g_glCtx && nfs_logo_provenance.alive
     && nfs_logo_provenance.context==nfs_info_wide.generation;
}

static void nfs_info_wide_retire(int owner){
 nfs_info_wide_clear_panorama();
 if(owner&&nfs_info_wide_current()){
  if(nfs_info_wide.program)glDeleteProgram(nfs_info_wide.program);
  if(nfs_info_wide.fbo)glDeleteFramebuffers(1,&nfs_info_wide.fbo);
  if(nfs_info_wide.color)glDeleteTextures(1,&nfs_info_wide.color);
  if(nfs_info_wide.panoramaTexture)glDeleteTextures(1,&nfs_info_wide.panoramaTexture);
 }
 memset(&nfs_info_wide,0,sizeof nfs_info_wide);
}

static void nfs_info_wide_mark(unsigned page,const NfsNativeLogoTargetProof*target){
 nfs_info_wide_clear_panorama();
 if(!target||target->framebuffer!=g_framebuffer||!g_framebufferTexture)return;
 if(nfs_info_wide.context&&!nfs_info_wide_current())nfs_info_wide_retire(0);
 nfs_info_wide.context=(uintptr_t)g_glCtx;
 nfs_info_wide.generation=nfs_logo_provenance.context;
 nfs_info_wide.clearEvent=nfs_logo_provenance.clearEvent;
 nfs_info_wide.nativeFbo=target->framebuffer;
 nfs_info_wide.nativeColor=g_framebufferTexture;
 nfs_info_wide.page=page;nfs_info_wide.lastMarkedPage=page;
 nfs_info_wide.nativeWidth=target->width;
}

static void nfs_info_wide_mark_history(const NfsHistoryPanoramaView*p){
 if(nfs_info_wide.page!=3||!p||!p->owner||!p->retain||!p->release||!p->current||!p->current(p->owner)||!p->baseGeneration||!p->pixels||!p->width||!p->height||p->width>4096||p->height>4096||p->bytes>32u*1024u*1024u||p->bytes!=(size_t)p->width*p->height*4)return;
 p->retain(p->owner);nfs_info_wide.panorama=*p;
}
// Slideshow uses the same retained RGBA transport only after its own native pass.
static void nfs_info_wide_mark_slideshow(const NfsHistoryPanoramaView*p,unsigned centerWidth,unsigned centerHeight){
 if(nfs_info_wide.page!=5||!p||!p->owner||!p->retain||!p->release||!p->current||!p->current(p->owner)||!p->baseGeneration||!p->pixels||!centerWidth||!centerHeight||centerWidth>4096||centerHeight>4096||centerWidth*3!=centerHeight*4||p->height!=centerHeight||p->width<=centerWidth||p->width>4096||p->height>4096||((p->width-centerWidth)&1u)||p->bytes>64u*1024u*1024u||p->bytes!=(size_t)p->width*p->height*4)return;
 p->retain(p->owner);nfs_info_wide_clear_panorama();nfs_info_wide.panorama=*p;
}
// Called with texture unit 1 selected, under the presentation state guard.
static int nfs_info_wide_prepare_panorama(void){
 const NfsHistoryPanoramaView*p=&nfs_info_wide.panorama;GLint maximum=0;
 if(!p->owner||!p->current||!p->current(p->owner))return 0;
 glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum);if(p->width>(unsigned)maximum||p->height>(unsigned)maximum)return 0;
 if(nfs_info_wide.panoramaTexture&&nfs_info_wide.panoramaWidth==p->width&&nfs_info_wide.panoramaHeight==p->height&&!memcmp(nfs_info_wide.panoramaSha,p->sha,32)){glBindTexture(GL_TEXTURE_2D,nfs_info_wide.panoramaTexture);return 1;}
 unsigned char sha[32];if(!nfs_texture_sha256(p->pixels,p->bytes,sha)||memcmp(sha,p->sha,32))return 0;
 if(!nfs_info_wide.panoramaTexture)glGenTextures(1,&nfs_info_wide.panoramaTexture);if(!nfs_info_wide.panoramaTexture)return 0;
 nfs_info_wide.panoramaWidth=nfs_info_wide.panoramaHeight=0;
 glBindTexture(GL_TEXTURE_2D,nfs_info_wide.panoramaTexture);nfs_frame_texture_params();glPixelStorei(GL_UNPACK_ALIGNMENT,4);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,p->width,p->height,0,GL_RGBA,GL_UNSIGNED_BYTE,p->pixels);
 if(glGetError()!=GL_NO_ERROR||!p->current(p->owner))return 0;
 nfs_info_wide.panoramaWidth=p->width;nfs_info_wide.panoramaHeight=p->height;memcpy(nfs_info_wide.panoramaSha,p->sha,32);return 1;
}
static int nfs_info_wide_prepare_program(void){
 const char* vertex="attribute vec4 vertex;varying highp vec2 uv;"
  "void main(){gl_Position=vec4(vertex.xy,0.,1.);uv=vertex.zw;}";
 const char* fragment="precision highp float;varying highp vec2 uv;"
  "uniform sampler2D image;uniform float centerWidth,edgeInset,reflectionDepth;uniform vec3 leftTint,rightTint;uniform bool photoBackdrop;"
  "uniform sampler2D panoramaImage;uniform bool hasPanorama;uniform float panoramaCenter,panoramaTexel;"
  "void main(){float x=(uv.x-.5)/centerWidth+.5;"
  "if(x>=0.&&x<=1.){gl_FragColor=texture2D(image,vec2(clamp(x,edgeInset,1.-edgeInset),uv.y));return;}"
  "if(hasPanorama){float panoramaX=(x-.5)*panoramaCenter+.5;"
  // Slideshow's hidden panorama center is not display artwork. Keep bilinear
  // side taps outside it; History's continuous panorama sampling is unchanged.
  "if(photoBackdrop){if(x<0.)panoramaX=min(panoramaX,(1.-panoramaCenter-panoramaTexel)*.5);else panoramaX=max(panoramaX,(1.+panoramaCenter+panoramaTexel)*.5);}"
  "gl_FragColor=texture2D(panoramaImage,vec2(clamp(panoramaX,0.,1.),1.-uv.y));return;}"
  "float edge=x<0.?0.:1.;vec3 extensionTint=mix(leftTint,rightTint,edge);"
  "float distanceFromEdge=abs(x-edge);"
  "if(photoBackdrop){"
  "float sampleX=mix(edgeInset,1.-edgeInset,edge);"
  "vec3 original=texture2D(image,vec2(sampleX,uv.y)).rgb;"
  "vec3 soft=original*.30;"
  "soft+=.23*texture2D(image,vec2(sampleX,clamp(uv.y-.025,0.,1.))).rgb;"
  "soft+=.23*texture2D(image,vec2(sampleX,clamp(uv.y+.025,0.,1.))).rgb;"
  "soft+=.12*texture2D(image,vec2(sampleX,clamp(uv.y-.065,0.,1.))).rgb;"
  "soft+=.12*texture2D(image,vec2(sampleX,clamp(uv.y+.065,0.,1.))).rgb;"
  "float fade=smoothstep(0.,.09,distanceFromEdge);"
  "gl_FragColor=vec4(mix(original,soft*.58,fade),1.);return;}"
  "float keep=1.-clamp(distanceFromEdge/.045,0.,1.);"
  "if(keep==0.){gl_FragColor=vec4(extensionTint,1.);return;}"
  "float borderX=mix(edgeInset,1.-edgeInset,edge+clamp(edge-x,-reflectionDepth,reflectionDepth));"
  "vec3 border=texture2D(image,vec2(borderX,uv.y)).rgb;"
  "gl_FragColor=vec4(mix(extensionTint,border,keep*keep),1.);}";
 if(nfs_info_wide.program)return 1;
 GLuint vs=nfs_text_gpu_shader(GL_VERTEX_SHADER,vertex);
 GLuint fs=nfs_text_gpu_shader(GL_FRAGMENT_SHADER,fragment);
 GLuint program=0;
 GLint linked=0;
 if(vs&&fs){
  program=glCreateProgram();
  glAttachShader(program,vs);glAttachShader(program,fs);
  glBindAttribLocation(program,0,"vertex");glLinkProgram(program);
  glGetProgramiv(program,GL_LINK_STATUS,&linked);
 }
 if(vs)glDeleteShader(vs);
 if(fs)glDeleteShader(fs);
 if(!linked){if(program)glDeleteProgram(program);return 0;}
 nfs_info_wide.image=glGetUniformLocation(program,"image");
 nfs_info_wide.panoramaSampler=glGetUniformLocation(program,"panoramaImage");
 nfs_info_wide.hasPanorama=glGetUniformLocation(program,"hasPanorama");
 nfs_info_wide.panoramaCenter=glGetUniformLocation(program,"panoramaCenter");
 nfs_info_wide.panoramaTexel=glGetUniformLocation(program,"panoramaTexel");
 nfs_info_wide.centerWidth=glGetUniformLocation(program,"centerWidth");
 nfs_info_wide.leftTint=glGetUniformLocation(program,"leftTint");
 nfs_info_wide.rightTint=glGetUniformLocation(program,"rightTint");
 nfs_info_wide.edgeInset=glGetUniformLocation(program,"edgeInset");
 nfs_info_wide.reflectionDepth=glGetUniformLocation(program,"reflectionDepth");
 nfs_info_wide.photoBackdrop=glGetUniformLocation(program,"photoBackdrop");
 if(nfs_info_wide.panoramaSampler<0||nfs_info_wide.hasPanorama<0||nfs_info_wide.panoramaCenter<0||nfs_info_wide.panoramaTexel<0||nfs_info_wide.photoBackdrop<0||nfs_info_wide.reflectionDepth<0||nfs_info_wide.edgeInset<0||nfs_info_wide.image<0||nfs_info_wide.centerWidth<0||nfs_info_wide.leftTint<0||nfs_info_wide.rightTint<0){
  glDeleteProgram(program);
  return 0;
 }
 nfs_info_wide.program=program;
 return 1;
}

// Bounded diagnostics for the intermittent information-page presentation gap.
// The mask observes existing validation only; it grants no reuse authority.
static void nfs_info_wide_decline(unsigned page,unsigned mask,GLuint front,GLuint source,int width,int height){
 static unsigned lines;const int saved=errno;
 if(nfs_info_wide.lastMarkedPage==4&&nfs_logo_trace_enabled()&&lines++<48)
  fprintf(stderr,"INFO WIDE decline page=%u mask=%u front=%u expectedFront=%u source=%u expectedSource=%u clear=%llu expectedClear=%llu output=%dx%d\n",page,mask,front,nfs_info_wide.nativeFbo,source,nfs_info_wide.nativeColor,(unsigned long long)nfs_logo_provenance.clearEvent,(unsigned long long)nfs_info_wide.clearEvent,width,height);
 errno=saved;
}
static GLuint nfs_info_wide_present(GLuint front,GLuint source,int width,int height){
 const unsigned page=nfs_info_wide.page;
 nfs_info_wide.page=0;
 const unsigned invalid=(!page?1u:0u)|(nfs_info_wide.nativeWidth<128?2u:0u)|(!nfs_info_wide_current()?4u:0u)|(!nfs_frame_dimensions(width,height)?8u:0u)|((int64_t)width*3<=(int64_t)height*4?16u:0u)|(front!=nfs_info_wide.nativeFbo?32u:0u)|(source!=nfs_info_wide.nativeColor?64u:0u)|(nfs_logo_provenance.clearEvent!=nfs_info_wide.clearEvent?128u:0u);
 if(invalid){nfs_info_wide_decline(page,invalid,front,source,width,height);return 0;}
 NfsBgSaved saved;GLint priorTexture1=0,priorUnpack=0;int texture1Touched=0,panoramaReady=0;
 nfs_bg_save(&saved);
 GLuint result=0;
 GLint type=0,name=0;
 glBindFramebuffer(GL_FRAMEBUFFER,front);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 if(type!=GL_TEXTURE||(GLuint)name!=source||!glIsTexture(source)||!nfs_info_wide_prepare_program())goto done;
 if(!nfs_info_wide.color)glGenTextures(1,&nfs_info_wide.color);
 if(!nfs_info_wide.fbo)glGenFramebuffers(1,&nfs_info_wide.fbo);
 if(!nfs_info_wide.color||!nfs_info_wide.fbo||nfs_info_wide.color==source||nfs_info_wide.fbo==front)goto done;
 glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,nfs_info_wide.color);
 if(nfs_info_wide.width!=(unsigned)width||nfs_info_wide.height!=(unsigned)height){
  nfs_frame_texture_params();
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  if(glGetError()!=GL_NO_ERROR)goto done;
  nfs_info_wide.width=(unsigned)width;nfs_info_wide.height=(unsigned)height;
 }
 glBindFramebuffer(GL_FRAMEBUFFER,nfs_info_wide.fbo);
 glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_info_wide.color,0);
 if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)goto done;
 {
  static const GLfloat quad[]={-1,-1,0,0,1,-1,1,0,-1,1,0,1,1,1,1,1};
  nfs_frame_draw_state();glDepthMask(GL_FALSE);glViewport(0,0,width,height);
  glUseProgram(nfs_info_wide.program);glBindTexture(GL_TEXTURE_2D,source);
  glUniform1i(nfs_info_wide.image,0);
  if((page==3||page==5)&&nfs_info_wide.panorama.owner){
   glGetIntegerv(GL_UNPACK_ALIGNMENT,&priorUnpack);glActiveTexture(GL_TEXTURE1);glGetIntegerv(GL_TEXTURE_BINDING_2D,&priorTexture1);texture1Touched=1;
   panoramaReady=nfs_info_wide_prepare_panorama();glActiveTexture(GL_TEXTURE0);
  }
  glUniform1i(nfs_info_wide.panoramaSampler,1);glUniform1i(nfs_info_wide.hasPanorama,panoramaReady);
  glUniform1f(nfs_info_wide.panoramaCenter,panoramaReady?(4.f*nfs_info_wide.panorama.height)/(3.f*nfs_info_wide.panorama.width):.75f);
  glUniform1f(nfs_info_wide.panoramaTexel,panoramaReady?1.f/nfs_info_wide.panorama.width:0.f);

  glUniform1i(nfs_info_wide.photoBackdrop,page==5);
  glUniform1f(nfs_info_wide.centerWidth,(4.f*height)/(3.f*width));
  // Authenticated slideshow composition fills the source right to its edges.
  // Preserve those photo pixels; other pages retain their viewport-edge guard.
  glUniform1f(nfs_info_wide.edgeInset,page==5?0.f:5.f/nfs_info_wide.nativeWidth);
  // Keep Showcase lettering outside the strip used to extend its background.
  glUniform1f(nfs_info_wide.reflectionDepth,page==4?.018f:.045f);
  if(page==4){
   glUniform3f(nfs_info_wide.leftTint,.19f,.135f,.071f);
   glUniform3f(nfs_info_wide.rightTint,.040f,.063f,.115f);
  }else{
   const GLfloat warm[]={.077f,.055f,.026f};
   const GLfloat green[]={.026f,.064f,.043f};
   const GLfloat* tint=page==3?warm:green;
   glUniform3fv(nfs_info_wide.leftTint,1,tint);
   glUniform3fv(nfs_info_wide.rightTint,1,tint);
  }
  glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);
  for(unsigned i=1;i<4;++i)glDisableVertexAttribArray(i);
  glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  if(glGetError()==GL_NO_ERROR&&(!panoramaReady||nfs_info_wide.panorama.current(nfs_info_wide.panorama.owner)))result=nfs_info_wide.color;
  if(result&&page==3&&panoramaReady&&nfs_logo_trace_enabled()){
   static unsigned char presented[7]={0};const uint32_t source=nfs_info_wide.panorama.sourceIndex;
   if(source<=42&&source%7==0&&presented[source/7]<2){
    ++presented[source/7];const int savedError=errno;
    fprintf(stderr,"HISTORY PANORAMA presented source=%u base=%llu output=%dx%d image=%ux%u\n",source,(unsigned long long)nfs_info_wide.panorama.baseGeneration,width,height,nfs_info_wide.panorama.width,nfs_info_wide.panorama.height);errno=savedError;
   }
  }
 }
 done:
 if(texture1Touched&&nfs_info_wide_current()){glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,(GLuint)priorTexture1);glPixelStorei(GL_UNPACK_ALIGNMENT,priorUnpack);glActiveTexture(GL_TEXTURE0);}
 if(!nfs_info_wide_current()){nfs_info_wide_retire(0);return 0;}
 nfs_bg_restore(&saved);
 if(!result)nfs_info_wide_decline(page,256u,front,source,width,height);
 return result;
}

static void nfs_info_wide_after_present(void){nfs_info_wide.page=0;nfs_info_wide_clear_panorama();}
