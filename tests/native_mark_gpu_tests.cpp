// SPDX-License-Identifier: MIT
#include <SDL3/SDL.h>
#include <GLES2/gl2.h>
#include "hd/NativeMarkProvider.h"
#include "TextureDigest.h"
#include <array>
#include <vector>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cmath>
static unsigned deleted=0;
static void countDelete(GLsizei n,const GLuint*p){deleted+=unsigned(n);glDeleteTextures(n,p);}
#define glDeleteTextures countDelete
#include "hd/NativeMarkGpu.c.inc"
#undef glDeleteTextures
static unsigned checks=0;
static void ck(bool x,const char*why){if(!x)throw std::runtime_error(why);++checks;}
struct Synthetic {
 std::array<unsigned char,32768>prepared{};
 std::vector<unsigned char>pixels;NfsNativeMarkArt art{};std::array<uint8_t,32>key{};
 Synthetic():pixels(1048576){prepared.fill(0x35);ck(nfs_texture_sha256(prepared.data(),prepared.size(),key.data()),"prepared sha");
  for(unsigned y=0;y<512;++y)for(unsigned x=0;x<512;++x){auto*p=pixels.data()+4*(y*512+x);p[0]=x<256?64:0;p[1]=x<256?0:64;p[2]=0;p[3]=128;}
  art.pixels=pixels.data();art.bytes=pixels.size();art.width=art.height=512;art.logicalWidth=art.logicalHeight=128;art.schema=art.premultiplied=1;art.identity=1001;ck(nfs_texture_sha256(pixels.data(),pixels.size(),art.sha256),"art sha");
 }
 static int lookup(void*u,uint32_t encoding,uint32_t edge,const uint8_t digest[32],NfsNativeMarkArt*out){auto&s=*static_cast<Synthetic*>(u);if(encoding!=1||edge!=128||!digest||std::memcmp(digest,s.key.data(),32)||!out)return 0;*out=s.art;return 1;}
};
static GLuint shader(GLenum kind,const char*source){GLuint s=glCreateShader(kind);glShaderSource(s,1,&source,nullptr);glCompileShader(s);GLint okay=0;glGetShaderiv(s,GL_COMPILE_STATUS,&okay);ck(okay!=0,"tiny shader compile");return s;}
static GLuint program(){
 const char*v="attribute vec4 vertex;varying vec2 uv;void main(){gl_Position=vec4(vertex.xy,0.,1.);uv=vertex.zw;}";
 const char*f="precision highp float;uniform sampler2D originalSampler;uniform sampler2D uMarkTextureSampler;uniform float uMarkActive;uniform float uHdActive;uniform vec4 tint;varying vec2 uv;void main(){vec4 c=texture2D(originalSampler,uv);if(uHdActive>.5)c=vec4(0.,0.,1.,1.);if(uMarkActive>.5){vec4 p=texture2D(uMarkTextureSampler,uv);c=vec4(p.a>0.?p.rgb/p.a:vec3(0.),p.a);}gl_FragColor=c*tint;}";
 GLuint a=shader(GL_VERTEX_SHADER,v),b=shader(GL_FRAGMENT_SHADER,f),p=glCreateProgram();glAttachShader(p,a);glAttachShader(p,b);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);GLint okay=0;glGetProgramiv(p,GL_LINK_STATUS,&okay);ck(okay!=0,"tiny program link");glDeleteShader(a);glDeleteShader(b);return p;
}
static GLuint solid(unsigned char r,unsigned char g,unsigned char b){const unsigned char p[4]={r,g,b,255};GLuint id=0;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,p);return id;}
struct Surface {
 GLuint p=0,native=0,sentinel=0,color=0,fbo=0;GLint hd=-1;int hdCache=0;
 Surface(){p=program();glUseProgram(p);glUniform1i(glGetUniformLocation(p,"originalSampler"),0);hd=glGetUniformLocation(p,"uHdActive");ck(hd>=0,"HD location");glActiveTexture(GL_TEXTURE0);native=solid(0,255,0);glActiveTexture(GL_TEXTURE2);sentinel=solid(0,0,255);glActiveTexture(GL_TEXTURE1);glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,8,8,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"tiny target");glViewport(0,0,8,8);glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);glDisable(GL_SCISSOR_TEST);glUniform4f(glGetUniformLocation(p,"tint"),1,1,1,1);glPixelStorei(GL_UNPACK_ALIGNMENT,8);}
 void unitState(){GLint active=0,id=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);ck(active==GL_TEXTURE1,"active unit preserved");glActiveTexture(GL_TEXTURE2);glGetIntegerv(GL_TEXTURE_BINDING_2D,&id);ck(GLuint(id)==sentinel,"unit2 binding preserved");glActiveTexture(GL_TEXTURE1);}
 void draw(){const GLfloat vertices[24]={-1,-1,0,0,1,-1,255.f/256,0,1,1,255.f/256,255.f/256,-1,-1,0,0,1,1,255.f/256,255.f/256,-1,1,0,255.f/256};auto copy=std::array<GLfloat,24>{};std::memcpy(copy.data(),vertices,sizeof vertices);glBindBuffer(GL_ARRAY_BUFFER,0);glEnableVertexAttribArray(0);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,vertices);glDrawArrays(GL_TRIANGLES,0,6);ck(!std::memcmp(copy.data(),vertices,sizeof vertices),"original vertices unchanged");}
 void pixel(unsigned x,unsigned y,unsigned r,unsigned g,unsigned b,unsigned a){unsigned char got[4]{};glReadPixels(GLint(x),GLint(y),1,1,GL_RGBA,GL_UNSIGNED_BYTE,got);const unsigned expected[4]={r,g,b,a};for(unsigned k=0;k<4;++k)ck(std::abs(int(got[k])-int(expected[k]))<=1,"independent UV/tint pixel");}
 void cleanup(){glDeleteFramebuffers(1,&fbo);GLuint ids[3]={native,sentinel,color};glDeleteTextures(3,ids);glDeleteProgram(p);}
};
static void select(Surface&s){glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,s.native);glActiveTexture(GL_TEXTURE1);nfs_native_mark_gpu_selected(4096,s.native,128,1,0);}
static void sync(Surface&s,int triangles=1,float textured=1,int hdSelected=0){nfs_native_mark_gpu_sync(triangles,textured,hdSelected,reinterpret_cast<uintptr_t>(SDL_GL_GetCurrentContext()),s.p,s.hd,&s.hdCache,480);}
int main(){SDL_Window*window=nullptr;SDL_GLContext context=nullptr;try{
 ck(SDL_Init(SDL_INIT_VIDEO),"SDL video init");ck(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2)&&SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0),"SDL ES2 attributes");window=SDL_CreateWindow("Native mark GPU test",32,32,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);ck(window!=nullptr,"SDL hidden window");context=SDL_GL_CreateContext(window);ck(context&&SDL_GL_MakeCurrent(window,context),"SDL actual current context");Synthetic source;Surface s;nfs_native_mark_gpu_created(reinterpret_cast<uintptr_t>(context),s.p);
 nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,source.prepared.data());ck(!nfs_native_marks.uniformsReady&&!nfs_native_marks.textures[0].texture,"missing provider original fallback");s.unitState();NfsNativeMarkProvider provider={1,&source,Synthetic::lookup};ck(nfs_hd_install_native_mark_provider(&provider),"install synthetic immutable provider");
 auto unknown=source.prepared;unknown[0]^=1;nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,unknown.data());ck(!nfs_native_marks.textures[0].texture,"unknown content original fallback");select(s);sync(s);s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,0,255,0,255);s.unitState();
 nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,source.prepared.data());GLuint owned=nfs_native_marks.textures[0].texture;ck(owned&&glIsTexture(owned),"known actual GL auxiliary");const auto revision=nfs_native_marks.revision;GLint unpack=0;glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack);ck(unpack==8,"unpack restored");s.unitState();
 select(s);glUniform1f(s.hd,1);s.hdCache=1;glUniform4f(glGetUniformLocation(s.p,"tint"),.5f,1,1,1);sync(s);GLfloat value=1;glGetUniformfv(s.p,s.hd,&value);ck(value==0&&s.hdCache==0,"mark disables only active HD mode");GLint active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);ck(active==GL_TEXTURE1,"draw setup active unit");s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,64,0,0,128);s.pixel(6,4,0,128,0,128);s.unitState();glGetUniformfv(s.p,nfs_native_marks.enable,&value);ck(value==0,"mark sampler disabled after draw");
 nfs_native_mark_gpu_invalidate(4096,32768);nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,source.prepared.data());ck(nfs_native_marks.textures[0].texture==owned&&nfs_native_marks.revision>revision,"same authored resource cached across source reupload");
 // Caller original draws complete before invalidation; subsequent overlapping source is unsupported.
 select(s);sync(s);s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,64,0,0,128);nfs_native_mark_gpu_invalidate(4097,1);select(s);sync(s);s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,0,255,0,255);ck(glIsTexture(owned),"invalidation retains reusable art only");
 nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,unknown.data());select(s);sync(s);s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,0,255,0,255);
 nfs_native_mark_gpu_uploaded(4096,s.native,128,1,0,source.prepared.data());select(s);glUniform1f(s.hd,1);s.hdCache=1;sync(s,1,1,1);s.draw();nfs_native_mark_gpu_after_draw();s.pixel(1,4,0,0,255,255);glGetUniformfv(s.p,s.hd,&value);ck(value==1&&s.hdCache==1,"other HD state retained");
 glUniform1f(s.hd,0);s.hdCache=0;select(s);sync(s,0);glGetUniformfv(s.p,nfs_native_marks.enable,&value);ck(value==0,"line path original fallback");sync(s,1,0);glGetUniformfv(s.p,nfs_native_marks.enable,&value);ck(value==0,"untextured path fallback");nfs_native_mark_gpu_selected(4096,s.native+1,128,1,0);sync(s);glGetUniformfv(s.p,nfs_native_marks.enable,&value);ck(value==0,"wrong native identity fallback");
 const auto generation=nfs_native_marks.generation;unsigned count=deleted;nfs_native_mark_gpu_retired(1);ck(deleted==count+1&&!glIsTexture(owned)&&glIsTexture(s.native),"bound retire deletes only owned auxiliary");s.cleanup();SDL_GL_DestroyContext(context);context=SDL_GL_CreateContext(window);ck(context&&SDL_GL_MakeCurrent(window,context),"recreated current SDL context");Surface next;nfs_native_mark_gpu_created(reinterpret_cast<uintptr_t>(context),next.p);ck(nfs_native_marks.generation>generation,"generation advanced across actual contexts");select(next);sync(next);next.draw();nfs_native_mark_gpu_after_draw();next.pixel(1,4,0,255,0,255);nfs_native_mark_gpu_uploaded(4096,next.native,128,1,0,source.prepared.data());ck(nfs_native_marks.textures[0].texture&&glIsTexture(nfs_native_marks.textures[0].texture),"recreated auxiliary from immutable source");ck(glGetError()==GL_NO_ERROR,"bound operations no GL errors");
 ck(SDL_GL_MakeCurrent(window,nullptr),"unbind SDL context");count=deleted;nfs_native_mark_gpu_retired(0);ck(deleted==count&&!nfs_native_marks.context&&!nfs_native_marks.textures[0].texture&&SDL_GL_GetCurrentContext()==nullptr,"unbound abandon no delete call or context");SDL_GL_DestroyContext(context);context=nullptr;SDL_DestroyWindow(window);window=nullptr;SDL_Quit();std::printf("PASS %u actual SDL/GLES native mark pixels/cache/state/lifecycle checks; synthetic bytes only\n",checks);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s SDL=%s checks=%u\n",e.what(),SDL_GetError(),checks);if(context)SDL_GL_DestroyContext(context);if(window)SDL_DestroyWindow(window);SDL_Quit();return 1;}}