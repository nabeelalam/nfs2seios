// SPDX-License-Identifier: MIT
// Actual production fragment include, synthetic auxiliary bytes only.
#include <SDL3/SDL.h>
#include <GLES2/gl2.h>
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
static unsigned checks;
static void ck(bool v,const char *why){if(!v)throw std::runtime_error(why);++checks;}
static GLuint shader(GLenum kind,const char *source){GLuint s=glCreateShader(kind);glShaderSource(s,1,&source,nullptr);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[4096]{};glGetShaderInfoLog(s,sizeof log,nullptr,log);std::fprintf(stderr,"%s\n",log);}ck(ok,"production shader compile");return s;}
static GLuint program(bool basic){
 const char *v="attribute vec2 position;uniform vec4 probe;varying highp vec4 vTexCoord;void main(){gl_Position=vec4(position,0.,1.);vTexCoord=probe;}";
 const char *body=
 "precision highp float;varying highp vec4 vTexCoord;uniform sampler2D uHdTextureSampler;uniform float uHdCellLayout;uniform float uHdActive;uniform vec4 tint;void main(){vec4 sampled=vec4(.125,.25,.375,1.);if(uHdActive>.5){"
#include "SpeedShader.inc"
 "gl_FragColor=sampled*tint;}";
 std::string f=basic?std::string("#define NFS_FORCE_BASIC_HD 1\n")+body:body;
 GLuint a=shader(GL_VERTEX_SHADER,v),b=shader(GL_FRAGMENT_SHADER,f.c_str()),p=glCreateProgram();glAttachShader(p,a);glAttachShader(p,b);glBindAttribLocation(p,0,"position");glLinkProgram(p);GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);ck(ok,"production shader link");glDeleteShader(a);glDeleteShader(b);return p;
}
struct Fixture {
 GLuint normal=0,basic=0,atlas=0,target=0,fbo=0;
 std::vector<unsigned char> pixels=std::vector<unsigned char>(1024*1024*4);
 Fixture(){
  for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x)set(x,y,255,0,255);
  // Reserved preceding rows catch any accidental allocation shift or vertical bleed.
  rect(0,396,1024,398,0,255,255);
  gradient(240,398,520,112,61);gradient(768,398,152,112,113);
  // Lower neighbour remains original: glyph4 starts at texel centre512.5.
  rect(0,510,1024,512,0,255,255);rect(256,512,320,576,120,70,30);
  // Six existing body-word interiors remain distinct, far from new allocations.
  rect(322,642,627,721,19,37,59);rect(631,642,901,721,31,47,71);
  rect(322,725,759,804,43,61,83);rect(322,814,809,893,53,73,97);
  rect(763,725,902,810,67,89,109);rect(322,897,818,982,79,101,127);
  rect(29,313,35,320,211,97,23); // Representative existing numeric tag2 mapping.
  normal=program(false);basic=program(true);
  glGenTextures(1,&atlas);glBindTexture(GL_TEXTURE_2D,atlas);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
  glGenTextures(1,&target);glBindTexture(GL_TEXTURE_2D,target);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,4,4,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,target,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"shader target complete");glViewport(0,0,4,4);glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);glDisable(GL_SCISSOR_TEST);glBindTexture(GL_TEXTURE_2D,atlas);
 }
 void set(unsigned x,unsigned y,unsigned r,unsigned g,unsigned b){auto *p=pixels.data()+4*(y*1024+x);p[0]=r;p[1]=g;p[2]=b;p[3]=255;}
 void rect(unsigned x0,unsigned y0,unsigned x1,unsigned y1,unsigned r,unsigned g,unsigned b){for(unsigned y=y0;y<y1;++y)for(unsigned x=x0;x<x1;++x)set(x,y,r,g,b);}
 void gradient(unsigned x0,unsigned y0,unsigned w,unsigned h,unsigned blue){for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)set(x0+x,y0+y,unsigned(std::lround(20.+200.*x/(w-1))),unsigned(std::lround(30.+180.*y/(h-1))),blue);}
 void sample(GLuint p,float tag,float u,float v,unsigned r,unsigned g,unsigned b,float q=1,bool active=true,float tint=1,unsigned alpha=255){
  glUseProgram(p);glUniform1i(glGetUniformLocation(p,"uHdTextureSampler"),0);glUniform1f(glGetUniformLocation(p,"uHdCellLayout"),0);glUniform1f(glGetUniformLocation(p,"uHdActive"),active?1:0);glUniform4f(glGetUniformLocation(p,"probe"),u*q,v*q,tag,q);glUniform4f(glGetUniformLocation(p,"tint"),tint,tint,tint,1);
  const GLfloat xy[]={-1,-1,1,-1,1,1,-1,-1,1,1,-1,1};glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,xy);glDrawArrays(GL_TRIANGLES,0,6);unsigned char got[4]{};glReadPixels(2,2,1,1,GL_RGBA,GL_UNSIGNED_BYTE,got);const unsigned expected[]={r,g,b,alpha};for(unsigned k=0;k<4;++k){if(std::abs(int(got[k])-int(expected[k]))>2)std::fprintf(stderr,"tag %.1f uv %.9f %.9f component %u got %u expected %u\n",tag,u,v,k,unsigned(got[k]),expected[k]);ck(std::abs(int(got[k])-int(expected[k]))<=2,"actual shader independent synthetic pixel");}ck(glGetError()==GL_NO_ERROR,"actual shader no GL errors");
 }
 void headers(){
  // Includes exact corners, subpixel border jitter, interior fractions and overshoot.
  const float positions[]={-.01f,-.00001f,0.f,.00001f,.125f,.5f,.875f,.99999f,1.f,1.00001f,1.01f};
  const unsigned source[2][4]={{0,229,130,29},{209,227,38,29}};
  for(unsigned label=0;label<2;++label)for(float y:positions)for(float x:positions){
   const auto &s=source[label];const float u=((s[0]+.5f)+(s[2]-.5f)*x)*(255.f/256)/256;
   const float v=((s[1]+.5f)+(s[3]-.5f)*y)*(255.f/256)/256;
   const float px=std::fmax(0.f,std::fmin(1.f,x)),py=std::fmax(0.f,std::fmin(1.f,y));
   sample(normal,float(73+label),u,v,unsigned(std::lround(20+200*px)),unsigned(std::lround(30+180*py)),label?113:61);
   if(x==.5f&&y==.5f){sample(normal,float(73+label),u,v,120,120,label?113:61,.5f);sample(normal,float(73+label),u,v,60,60,label?57:31,1,true,.5f);sample(normal,float(73+label),u,v,32,64,96,1,false);sample(basic,float(73+label),u,v,32,64,96);}
  }
 }
 void regressions(){
  const float source[6][4]={{.498046875f,.498046875f,154.39453125f,23.90625f},{.498046875f,25.400390625f,154.39453125f,48.80859375f},{.498046875f,73.212890625f,154.39453125f,96.62109375f},{.498046875f,97.119140625f,254.00390625f,120.52734375f},{.498046875f,49.306640625f,154.39453125f,72.71484375f},{.498046875f,198.720703125f,254.00390625f,226.11328125f}};
  const unsigned rgb[6][3]={{19,37,59},{31,47,71},{43,61,83},{53,73,97},{67,89,109},{79,101,127}};
  // Independent catalog originalRasterInkBounds (4x raster) and original atlas rows.
  // Whole source-cell centres need not lie inside the narrower preserved ink crop.
  const unsigned crop[6][4]={{10,9,315,88},{11,9,281,88},{11,9,448,88},{9,9,496,88},{10,9,149,94},{10,17,506,102}};
  const unsigned atlasRow[6]={0,25,73,97,49,199};
  for(unsigned i=0;i<6;++i){
   const float u=(crop[i][0]+crop[i][2])*.125f*(255.f/256)/256;
   const float v=(atlasRow[i]+(crop[i][1]+crop[i][3])*.125f)*(255.f/256)/256;
   sample(normal,float(50+i),u,v,rgb[i][0],rgb[i][1],rgb[i][2]);
   // Original right source-cell edge is outside each recorded ink crop: transparent,
   // even though the synthetic underlying auxiliary pixel is opaque magenta.
   sample(normal,float(50+i),source[i][2]/256,v,0,0,0,1,true,1,0);
  }
  sample(normal,2,.2f,.2f,211,97,23);
  for(unsigned y:{396u,397u})sample(normal,1,.5f,(y+.5f)/1024,0,255,255);
  // Independent bilinear baseline for glyph4 at its original minimum native V.
  // A small accepted raw-UV jitter straddles the untouched row511/512 boundary.
  // Compare the complete authored atlas against a no-header baseline on actual GL.
  for(float jitter:{-.01f,0.f,.01f}){
   const float v=(15.9375f+jitter)/256;
   const double centre=512.5+double(jitter)*(.0658823529411765*4);
   const double lowerWeight=std::fmax(0.,512.5-centre);
   const unsigned red=unsigned(std::lround(120*(1-lowerWeight)));
   const unsigned green=unsigned(std::lround(70*(1-lowerWeight)+255*lowerWeight));
   const unsigned blue=unsigned(std::lround(30*(1-lowerWeight)+255*lowerWeight));
   sample(normal,16,127.5f/256,v,red,green,blue);
   unsigned char authored[4]{};glReadPixels(2,2,1,1,GL_RGBA,GL_UNSIGNED_BYTE,authored);
   auto baseline=pixels;
   for(unsigned y=398;y<510;++y)for(unsigned x=240;x<920;++x){auto *p=baseline.data()+4*(y*1024+x);p[0]=255;p[1]=0;p[2]=255;p[3]=255;}
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,baseline.data());
   sample(normal,16,127.5f/256,v,red,green,blue);
   unsigned char untouched[4]{};glReadPixels(2,2,1,1,GL_RGBA,GL_UNSIGNED_BYTE,untouched);
   for(unsigned k=0;k<4;++k)ck(authored[k]==untouched[k],"small glyph lower-neighbour matches untouched baseline exactly");
   glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
  }
  sample(normal,72,.1f,.3f,255,0,255); // Existing frontend selector is not reinterpreted here.
  sample(normal,0,.5f,.5f,32,64,96);
 }
 ~Fixture(){glDeleteFramebuffers(1,&fbo);GLuint ids[]={atlas,target};glDeleteTextures(2,ids);glDeleteProgram(normal);glDeleteProgram(basic);}
};
int main(){SDL_Window *window=nullptr;SDL_GLContext context=nullptr;try{
 ck(SDL_Init(SDL_INIT_VIDEO),"SDL video");ck(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0),"ES2 attributes");window=SDL_CreateWindow("Pause header shader",16,16,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);ck(window,"hidden window");context=SDL_GL_CreateContext(window);ck(context&&SDL_GL_MakeCurrent(window,context),"actual current ES context");{Fixture f;f.headers();f.regressions();}SDL_GL_DestroyContext(context);SDL_DestroyWindow(window);SDL_Quit();std::printf("PASS %u actual production pause header shader checks; synthetic atlas only\n",checks);return 0;
 }catch(const std::exception &e){std::fprintf(stderr,"FAIL %s SDL=%s checks=%u\n",e.what(),SDL_GetError(),checks);if(context)SDL_GL_DestroyContext(context);if(window)SDL_DestroyWindow(window);SDL_Quit();return 1;}}
