// SPDX-License-Identifier: MIT
// Actual GLES2 executes fragment literals extracted from production source.
// Synthetic pixels only; no game, source archive, artwork or presentation proof.
#include <SDL3/SDL.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
static void ck(bool value,const char*why){if(!value)throw std::runtime_error(why);}
static std::string fragment(const char*path){
 std::ifstream f(path);ck(bool(f),"production file readable");
 const std::string text((std::istreambuf_iterator<char>(f)),{});
 const auto begin=text.find("const char* fragment=");
 const auto end=text.find("if(nfs_info_wide.program)",begin);
 ck(begin!=std::string::npos&&end!=std::string::npos,"production fragment anchors");
 std::string result;
 for(size_t i=begin;i<end;++i)if(text[i]=='"'){
  for(++i;i<end&&text[i]!='"';++i){
   char value=text[i];if(value=='\\'){ck(++i<end,"literal escape");value=text[i];if(value=='n')value='\n';else if(value=='t')value='\t';else ck(value=='\\'||value=='"',"known literal escape");}
   result+=value;
  }
 }
 ck(!result.empty(),"extracted fragment");return result;
}
static GLuint shader(GLenum type,const std::string&source){
 GLuint s=glCreateShader(type);const char*p=source.c_str();glShaderSource(s,1,&p,nullptr);glCompileShader(s);
 GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
 if(!ok){char log[4096]{};glGetShaderInfoLog(s,sizeof log,nullptr,log);std::fprintf(stderr,"%s\n",log);}
 ck(ok,"actual shader compile");return s;
}
static GLuint program(const std::string&source){
 GLuint v=shader(GL_VERTEX_SHADER,"attribute vec4 vertex;varying highp vec2 uv;void main(){gl_Position=vec4(vertex.xy,0.,1.);uv=vertex.zw;}");
 GLuint f=shader(GL_FRAGMENT_SHADER,source),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);
 GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);ck(ok,"actual shader link");glDeleteShader(v);glDeleteShader(f);return p;
}
static GLuint texture(unsigned width,unsigned height,const unsigned char*data){
 GLuint t=0;glGenTextures(1,&t);glBindTexture(GL_TEXTURE_2D,t);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);return t;
}
struct Fixture {
 GLuint fixed=0,baseline=0,image=0,panorama=0,target=0,fbo=0;
 Fixture(const std::string&source){
  fixed=program(source);
  // Baseline is the same production shader with ONLY the scoped correction
  // removed. History should be byte-identical; slideshow must expose old bleed.
  const std::string correction="if(photoBackdrop){if(x<0.)panoramaX=min(panoramaX,(1.-panoramaCenter-panoramaTexel)*.5);else panoramaX=max(panoramaX,(1.+panoramaCenter+panoramaTexel)*.5);}";
  auto old=source;auto at=old.find(correction);ck(at!=std::string::npos,"exact production correction extracted");old.erase(at,correction.size());baseline=program(old);
  const unsigned char blue[]={0,0,255,255};image=texture(1,1,blue);
  std::vector<unsigned char>rgba(1932u*1086u*4u);
  for(unsigned y=0;y<1086;++y)for(unsigned x=0;x<1932;++x){auto*p=rgba.data()+4*(y*1932+x);p[0]=x>=242&&x<1690?255:0;p[1]=x<242||x>=1690?255:0;p[2]=x>=1690?255:0;p[3]=255;}
  panorama=texture(1932,1086,rgba.data());glGenFramebuffers(1,&fbo);
 }
 ~Fixture(){glDeleteProgram(fixed);glDeleteProgram(baseline);GLuint ids[]={image,panorama,target};glDeleteTextures(3,ids);glDeleteFramebuffers(1,&fbo);}
 std::vector<unsigned char>draw(GLuint p,int width,int height,bool photo){
  if(target)glDeleteTextures(1,&target);
  target=texture(width,height,nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,target,0);
  ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"synthetic target complete");
  glViewport(0,0,width,height);glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);glDisable(GL_SCISSOR_TEST);glUseProgram(p);
  glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,image);glUniform1i(glGetUniformLocation(p,"image"),0);
  glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,panorama);glUniform1i(glGetUniformLocation(p,"panoramaImage"),1);
  glUniform1i(glGetUniformLocation(p,"hasPanorama"),1);glUniform1i(glGetUniformLocation(p,"photoBackdrop"),photo);
  glUniform1f(glGetUniformLocation(p,"centerWidth"),float(height)*4.f/(3.f*width));
  glUniform1f(glGetUniformLocation(p,"panoramaCenter"),1448.f/1932.f);
  glUniform1f(glGetUniformLocation(p,"panoramaTexel"),1.f/1932.f);
  glUniform1f(glGetUniformLocation(p,"edgeInset"),0.f);
  glUniform1f(glGetUniformLocation(p,"reflectionDepth"),.045f);
  glUniform3f(glGetUniformLocation(p,"leftTint"),0,0,0);glUniform3f(glGetUniformLocation(p,"rightTint"),0,0,0);
  const GLfloat quad[]={-1,-1,0,0,1,-1,1,0,-1,1,0,1,1,1,1,1};
  glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  std::vector<unsigned char>out(size_t(width)*height*4);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,out.data());ck(glGetError()==GL_NO_ERROR,"synthetic GL no errors");return out;
 }
 void check(int width,int height){
  auto old=draw(baseline,width,height,true),now=draw(fixed,width,height,true);
  const double center=double(height)*4/3,left=(width-center)/2,right=(width+center)/2;
  const int firstLeft=int(std::ceil(left-.5))-1,firstRight=int(std::floor(right-.5))+1;
  unsigned baselineRed=0,fixedRed=0;
  for(int y:{0,height/2,height-1}){
   const auto l=4*(size_t(y)*width+firstLeft),r=4*(size_t(y)*width+firstRight),c=4*(size_t(y)*width+width/2);
   baselineRed=std::max(baselineRed,std::max(unsigned(old[l]),unsigned(old[r])));
   fixedRed=std::max(fixedRed,std::max(unsigned(now[l]),unsigned(now[r])));
   ck(now[l]==0&&now[l+1]==255&&now[l+2]==0,"first left side is pure green");
   ck(now[r]==0&&now[r+1]==255&&now[r+2]==255,"first right side is pure cyan");
   ck(now[c]==0&&now[c+1]==0&&now[c+2]==255,"native center stays blue");
   for(int inside:{firstLeft+1,firstRight-1}){
    const auto at=4*(size_t(y)*width+inside);
    ck(now[at]==0&&now[at+1]==0&&now[at+2]==255,"first protected center pixels stay blue");
   }
  }
  ck(baselineRed>0,"fixture reproduces uncorrected hidden-center bleed");
  auto historyOld=draw(baseline,width,height,false),historyNow=draw(fixed,width,height,false);
  ck(historyOld==historyNow,"History unchanged across every output byte");
  std::printf("%dx%d firstOutside=%d/%d baselineRed=%u fixedRed=%u historyByteExact=1\n",width,height,firstLeft,firstRight,baselineRed,fixedRed);
 }
};
int main(int argc,char**argv){SDL_Window*w=nullptr;SDL_GLContext context=nullptr;try{
 ck(argc==2,"production source argument required");auto source=fragment(argv[1]);
 ck(SDL_Init(SDL_INIT_VIDEO),"SDL video");ck(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0),"ES2 attributes");
 w=SDL_CreateWindow("Synthetic panorama sampler",16,16,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);ck(w,"hidden SDL window");context=SDL_GL_CreateContext(w);ck(context&&SDL_GL_MakeCurrent(w,context),"ES2 context");
 {Fixture f(source);f.check(2360,1640);f.check(2732,2048);}SDL_GL_DestroyContext(context);SDL_DestroyWindow(w);SDL_Quit();return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s SDL=%s\n",e.what(),SDL_GetError());if(context)SDL_GL_DestroyContext(context);if(w)SDL_DestroyWindow(w);SDL_Quit();return 1;}}
