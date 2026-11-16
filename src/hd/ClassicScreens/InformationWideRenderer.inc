// SPDX-License-Identifier: MIT
// Reframe an authenticated, already rendered information page. Its center
// preserves the native art, text, graph and cursor at a uniform scale.
static struct {
 GLuint program, fbo, color;
 GLint image, centerWidth, extensionTint, edgeInset;
 unsigned width, height, page, nativeWidth;
 uintptr_t context;
 uint64_t generation, clearEvent;
 GLuint nativeFbo, nativeColor;
} nfs_info_wide;

static int nfs_info_wide_current(void){
 return nfs_info_wide.context && nfs_info_wide.context==(uintptr_t)g_glCtx
     && SDL_GL_GetCurrentContext()==g_glCtx && nfs_logo_provenance.alive
     && nfs_logo_provenance.context==nfs_info_wide.generation;
}

static void nfs_info_wide_retire(int owner){
 if(owner&&nfs_info_wide_current()){
  if(nfs_info_wide.program)glDeleteProgram(nfs_info_wide.program);
  if(nfs_info_wide.fbo)glDeleteFramebuffers(1,&nfs_info_wide.fbo);
  if(nfs_info_wide.color)glDeleteTextures(1,&nfs_info_wide.color);
 }
 memset(&nfs_info_wide,0,sizeof nfs_info_wide);
}

static void nfs_info_wide_mark(unsigned page,const NfsNativeLogoTargetProof*target){
 if(!target||target->framebuffer!=g_framebuffer||!g_framebufferTexture)return;
 if(nfs_info_wide.context&&!nfs_info_wide_current())nfs_info_wide_retire(0);
 nfs_info_wide.context=(uintptr_t)g_glCtx;
 nfs_info_wide.generation=nfs_logo_provenance.context;
 nfs_info_wide.clearEvent=nfs_logo_provenance.clearEvent;
 nfs_info_wide.nativeFbo=target->framebuffer;
 nfs_info_wide.nativeColor=g_framebufferTexture;
 nfs_info_wide.page=page;
 nfs_info_wide.nativeWidth=target->width;
}

static int nfs_info_wide_prepare_program(void){
 const char* vertex="attribute vec4 vertex;varying highp vec2 uv;"
  "void main(){gl_Position=vec4(vertex.xy,0.,1.);uv=vertex.zw;}";
 const char* fragment="precision highp float;varying highp vec2 uv;"
  "uniform sampler2D image;uniform float centerWidth,edgeInset;uniform vec3 extensionTint;"
  "void main(){float x=(uv.x-.5)/centerWidth+.5;"
  "if(x>=0.&&x<=1.){gl_FragColor=texture2D(image,vec2(clamp(x,edgeInset,1.-edgeInset),uv.y));return;}"
  "float edge=x<0.?0.:1.;"
  "float distanceFromEdge=abs(x-edge);"
  "float keep=1.-clamp(distanceFromEdge/.045,0.,1.);"
  "if(keep==0.){gl_FragColor=vec4(extensionTint,1.);return;}"
  "float borderX=mix(edgeInset,1.-edgeInset,2.*edge-x);"
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
 nfs_info_wide.centerWidth=glGetUniformLocation(program,"centerWidth");
 nfs_info_wide.extensionTint=glGetUniformLocation(program,"extensionTint");
 nfs_info_wide.edgeInset=glGetUniformLocation(program,"edgeInset");
 if(nfs_info_wide.edgeInset<0||nfs_info_wide.image<0||nfs_info_wide.centerWidth<0||nfs_info_wide.extensionTint<0){
  glDeleteProgram(program);
  return 0;
 }
 nfs_info_wide.program=program;
 return 1;
}

static GLuint nfs_info_wide_present(GLuint front,GLuint source,int width,int height){
 const unsigned page=nfs_info_wide.page;
 nfs_info_wide.page=0;
 if(!page||nfs_info_wide.nativeWidth<128||!nfs_info_wide_current()||!nfs_frame_dimensions(width,height)
     ||(int64_t)width*3<=(int64_t)height*4
     ||front!=nfs_info_wide.nativeFbo||source!=nfs_info_wide.nativeColor
     ||nfs_logo_provenance.clearEvent!=nfs_info_wide.clearEvent)return 0;
 NfsBgSaved saved;
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
  glUniform1f(nfs_info_wide.centerWidth,(4.f*height)/(3.f*width));
  // Skip the narrow antialiased edge of the original viewport before
  // extending it into a side panel. Keep all interior sampling in place.
  glUniform1f(nfs_info_wide.edgeInset,5.f/nfs_info_wide.nativeWidth);
  if(page==3)glUniform3f(nfs_info_wide.extensionTint,.077f,.055f,.026f);
  else glUniform3f(nfs_info_wide.extensionTint,.026f,.064f,.043f);
  glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);
  for(unsigned i=1;i<4;++i)glDisableVertexAttribArray(i);
  glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  if(glGetError()==GL_NO_ERROR)result=nfs_info_wide.color;
 }
 done:
 if(!nfs_info_wide_current()){nfs_info_wide_retire(0);return 0;}
 nfs_bg_restore(&saved);
 return result;
}

static void nfs_info_wide_after_present(void){nfs_info_wide.page=0;}
