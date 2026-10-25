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
#include "WarningWordPixels.inc"
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

struct Fixture{
 GLuint normal,basic,atlas,target,fbo;std::vector<unsigned char> alpha;
 Fixture(){for(size_t i=0;i<sizeof(nfs_warning_word_rle);i+=3){unsigned n=nfs_warning_word_rle[i]|unsigned(nfs_warning_word_rle[i+1])<<8;alpha.insert(alpha.end(),n,nfs_warning_word_rle[i+2]);}
 std::vector<unsigned char> pixels(4194304,173);ck(nfs_warning_word_expand(pixels.data(),pixels.size()),"production decode");normal=program(false);basic=program(true);
 ck(glGetUniformLocation(normal,"uHdCellLayout")>=0,"highp capability active");ck(glGetUniformLocation(basic,"uHdCellLayout")==-1,"basic capability optimized out");
 glGenTextures(1,&atlas);glBindTexture(GL_TEXTURE_2D,atlas);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
 glGenTextures(1,&target);glBindTexture(GL_TEXTURE_2D,target);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,4,4,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,target,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"FBO");glViewport(0,0,4,4);glDisable(GL_BLEND);glBindTexture(GL_TEXTURE_2D,atlas);}
 unsigned expected(float x,float y,bool punct=false){unsigned width=punct?18:618,offset=punct?35226:0;if(x<0||x>=width||y<0||y>=57)return 0;double ax=double(x)-.5,ay=double(y)-.5;int ix=int(std::floor(ax)),iy=int(std::floor(ay));double sum=0;for(int j=0;j<2;++j)for(int i=0;i<2;++i){int xx=ix+i,yy=iy+j;if(xx>=0&&xx<int(width)&&yy>=0&&yy<57)sum+=alpha[offset+yy*width+xx]*(i?ax-ix:1-(ax-ix))*(j?ay-iy:1-(ay-iy));}return unsigned(std::lround(sum));}
 void probe(float x,float y,bool fallback=false,bool inactive=false,bool single=false,bool punct=false){GLuint p=fallback?basic:normal;glUseProgram(p);glUniform1i(glGetUniformLocation(p,"uHdTextureSampler"),0);glUniform1f(glGetUniformLocation(p,"uHdCellLayout"),1);glUniform1f(glGetUniformLocation(p,"uHdActive"),inactive?0:1);glUniform4f(glGetUniformLocation(p,"probe"),((punct?233.f:17.f)+x/(single&&!punct?618.f/213.f:3.f))*(255.f/256)/256,(1+y/3)*(255.f/256)/256,single?77:76,1);glUniform4f(glGetUniformLocation(p,"tint"),1,1,1,1);const GLfloat xy[]={-1,-1,1,-1,1,1,-1,-1,1,1,-1,1};glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,xy);glDrawArrays(GL_TRIANGLES,0,6);unsigned char got[4];glReadPixels(2,2,1,1,GL_RGBA,GL_UNSIGNED_BYTE,got);unsigned want[4]={0,0,0,expected(x,y,punct)};if(fallback||inactive){want[0]=32;want[1]=64;want[2]=96;want[3]=255;}for(int k=0;k<4;++k){if(std::abs(int(got[k])-int(want[k]))>2)std::fprintf(stderr,"x%g y%g k%d got%d want%u\n",x,y,k,got[k],want[k]);ck(std::abs(int(got[k])-int(want[k]))<=2,"actual shader monolithic bilinear oracle");}ck(glGetError()==GL_NO_ERROR,"GL error");}
 void run(){for(float x:{-.1f,618.1f,620.f,650.f,690.f})probe(x,20.25f,false,false,true);for(float x:{.02f,.5f,100.25f,317.75f,318.f,318.25f,443.75f,444.f,444.25f,505.75f,506.f,506.25f,567.75f,568.f,568.25f,617.98f})for(float y:{.02f,.5f,20.25f,56.98f})probe(x,y,false,false,true);for(float x:{-.02f,.02f,.5f,8.5f,17.98f,18.02f})for(float y:{.02f,.5f,20.25f,56.98f}){probe(x,y,false,false,true,true);probe(x,y,true,false,true,true);probe(x,y,false,true,true,true);}for(float x:{-3.f,-.02f,.02f,.5f,17.5f,100.25f,317.75f,318.f,318.25f,443.75f,444.f,444.25f,505.75f,506.f,506.25f,567.75f,568.f,568.25f,617.5f,617.98f,618.02f,630.f})for(float y:{-.02f,.02f,.5f,1.25f,15.5f,30.25f,55.5f,56.98f,57.02f})probe(x,y);for(float x:{.5f,318.f,444.f,506.f,568.f,617.5f})for(float y:{.5f,20.25f,56.5f}){probe(x,y,true);probe(x,y,false,true);}}
 ~Fixture(){glDeleteFramebuffers(1,&fbo);GLuint ids[]={atlas,target};glDeleteTextures(2,ids);glDeleteProgram(normal);glDeleteProgram(basic);}};
int main(){SDL_Window *window=nullptr;SDL_GLContext context=nullptr;try{
 ck(SDL_Init(SDL_INIT_VIDEO),"SDL video");ck(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0),"ES2 attributes");window=SDL_CreateWindow("Warning shader",16,16,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);ck(window,"hidden window");context=SDL_GL_CreateContext(window);ck(context&&SDL_GL_MakeCurrent(window,context),"actual current ES context");{Fixture f;f.run();}SDL_GL_DestroyContext(context);SDL_DestroyWindow(window);SDL_Quit();std::printf("PASS %u actual production warning shader checks; synthetic atlas only\n",checks);return 0;
 }catch(const std::exception &e){std::fprintf(stderr,"FAIL %s SDL=%s checks=%u\n",e.what(),SDL_GetError(),checks);if(context)SDL_GL_DestroyContext(context);if(window)SDL_DestroyWindow(window);SDL_Quit();return 1;}}
