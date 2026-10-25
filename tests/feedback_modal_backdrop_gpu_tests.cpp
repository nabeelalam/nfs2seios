// SPDX-License-Identifier: MIT
// Isolated real GLES mechanics with synthetic source vertices/stage ownership.
// This test supplies no actual game, source proof, native witness or publication.
#include "HdSceneGlesFixture.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "hd/BackgroundSceneCompositor.c.inc"
#include "hd/NestedCaptionGpu.c.inc"
#include <vector>
static unsigned draws=0,failDraw=0,loseDraw=0;
static EGLDisplay lossDisplay=EGL_NO_DISPLAY;
static EGLSurface lossSurface=EGL_NO_SURFACE;
static EGLContext lossContext=EGL_NO_CONTEXT;
static void wordDraw(GLenum mode,GLint first,GLsizei count){
 ++draws;glDrawArrays(draws==failDraw?GLenum(0xffffffff):mode,first,count);
 if(draws==loseDraw)nfs_hd_test::require(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected actual context loss");
}
#define glDrawArrays(...) wordDraw(__VA_ARGS__)
#include "hd/FeedbackModalBackdropGpu.c.inc"
#undef glDrawArrays
namespace {
using namespace nfs_hd_test;
struct Owner {EGLContext context;uint64_t generation;GLuint fbo;bool strict;
 static int current(void*user,const NfsBackgroundSceneStage*t){auto&o=*static_cast<Owner*>(user);
  if(eglGetCurrentContext()!=o.context||t->contextIdentity!=reinterpret_cast<uintptr_t>(o.context)||t->generation!=o.generation)return 0;
  if(!o.strict)return 1;GLint fbo=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);return GLuint(fbo)==o.fbo&&t->framebuffer==o.fbo;
 }
};
struct Target {GLuint texture=0,fbo=0;
 Target(){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);require(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"isolated target complete");}
 void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);glViewport(0,0,640,480);}
 void clear(){bind();glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(64.f/255,128.f/255,192.f/255,1);glClear(GL_COLOR_BUFFER_BIT);}
 std::vector<unsigned char>read(){bind();std::vector<unsigned char>p(640*480*4);glReadPixels(0,0,640,480,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;}
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);}
};
struct Art {unsigned width,height;std::vector<unsigned char>rgba;};
static NfsFeedbackModalBackdropArtView view(std::array<Art,2>&art){
 NfsFeedbackModalBackdropArtView v{};v.schema=1;v.count=2;
 for(unsigned i=0;i<2;++i){art[i].width=i?468:1376;art[i].height=i?244:188;auto&a=art[i];a.rgba.resize(size_t(a.width)*a.height*4);
  for(unsigned y=0;y<a.height;++y)for(unsigned x=0;x<a.width;++x){size_t at=(size_t(y)*a.width+x)*4;bool edge=x<8||y<8||x>=a.width-8||y>=a.height-8;
   a.rgba[at]=static_cast<unsigned char>(50+(x/4)%150);a.rgba[at+1]=static_cast<unsigned char>(i?80:130);a.rgba[at+2]=static_cast<unsigned char>(i?180:20);
   a.rgba[at+3]=edge?0:static_cast<unsigned char>(i?64+128*((x/16+y/16)%2):128+50*((x/24+y/16)%3));
  }
  auto&out=v.images[i];out.schema=1;out.role=i+1;out.width=a.width;out.height=a.height;out.straightRgba=1;out.rgba=a.rgba.data();out.bytes=a.rgba.size();require(nfs_texture_sha256(out.rgba,out.bytes,out.sha256),"synthetic authored art hash");
 }return v;
}
static std::array<double,4>sample(const Art&a,double x,double y){
 double X=std::max(0.,std::min(double(a.width-1),4*x-.5)),Y=std::max(0.,std::min(double(a.height-1),4*y-.5));unsigned i=unsigned(X),j=unsigned(Y),k=std::min(i+1,a.width-1),l=std::min(j+1,a.height-1);double fx=X-i,fy=Y-j;
 auto at=[&](unsigned x,unsigned y,unsigned c){size_t n=(size_t(y)*a.width+x)*4;unsigned alpha=a.rgba[n+3];return double(c==3?alpha:unsigned(a.rgba[n+c])*alpha/255);};
 std::array<double,4>out{};for(unsigned c=0;c<4;++c)out[c]=(at(i,j,c)*(1-fx)+at(k,j,c)*fx)*(1-fy)+(at(i,l,c)*(1-fx)+at(k,l,c)*fx)*fy;return out;
}
static void run(Pbuffer&egl){
 Target target;Owner owner{egl.context,1,target.fbo,false};NfsBackgroundSceneStage stage{};stage.generation=1;stage.contextIdentity=reinterpret_cast<uintptr_t>(egl.context);stage.ownerUser=&owner;stage.ownerCurrent=Owner::current;stage.width=640;stage.height=480;stage.framebuffer=target.fbo;
 require(nfs_frame_gpu_legacy_sampling(640,480,GL_LINEAR,&stage.sampling),"neutral sampling");std::array<Art,2>art;auto v=view(art);
 GLuint borrowedTexture=0;glGenTextures(1,&borrowedTexture);glBindTexture(GL_TEXTURE_2D,borrowedTexture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
 GLuint borrowedProgram=glCreateProgram();GLuint forbiddenPrograms[]={borrowedProgram};GLuint forbiddenTextures[]={target.texture,borrowedTexture};NfsFeedbackModalBackdropGpuAliases aliases{forbiddenPrograms,1,forbiddenTextures,2};NfsFeedbackModalBackdropGpu gpu{};
 target.bind();auto before=capture();require(nfs_feedback_modal_backdrop_gpu_prepare(&gpu,&v,&stage,&aliases)&&equal(before,capture()),"cold preparation converts PMA and restores state");
 const auto saved=gpu;before=capture();require(nfs_feedback_modal_backdrop_gpu_prepare(&gpu,&v,&stage,&aliases)&&equal(before,capture())&&!std::memcmp(&saved,&gpu,sizeof gpu),"warm immutable dynamic-pinned cache retained");
 owner.strict=true;target.clear();glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glEnable(GL_SCISSOR_TEST);glScissor(3,4,17,19);glActiveTexture(GL_TEXTURE1);before=capture();draws=0;
 require(nfs_feedback_modal_backdrop_gpu_draw(&gpu,&v,&stage,&aliases)&&draws==3&&equal(before,capture()),"body banner emblem ordered draws restore state");auto rendered=target.read();
 unsigned clipped=0,overlap=0,body=0;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){double X=x+.5,Y=y+.5;double rgb[3]={64,128,192};
  if(X>=227&&X<413&&Y>=210&&Y<270){++body;for(unsigned k=0;k<3;++k)rgb[k]=std::round(5+rgb[k]*76./256.);}
  auto blend=[&](unsigned image,double originX,double originY){auto rgba=sample(art[image],X-originX,Y-originY);for(unsigned k=0;k<3;++k)rgb[k]=std::round(rgba[k]+rgb[k]*(1-rgba[3]/255.));};
  if(X>=247&&X<442&&Y>=179&&Y<226)blend(0,98,179);
  if(X>=175&&X<292&&Y>=171&&Y<232){blend(1,175,171);if(X>=247&&Y>=210&&Y<226)++overlap;}
  if(X>=98&&X<247&&Y>=179&&Y<210&&!(X>=175&&X<292&&Y>=171&&Y<232))++clipped;
  size_t at=(size_t(479-y)*640+x)*4;for(unsigned k=0;k<3;++k)require(std::abs(int(rendered[at+k])-int(rgb[k]))<=3,"independent PMA body banner-left-clip emblem-order pixel oracle");
 }
 require(clipped>100&&overlap>100&&body>100,"left clip overlap and body exercised");
 for(unsigned fault=0;fault<12;++fault){auto bad=v;if(fault==0)bad.schema=0;if(fault==1)bad.count=1;if(fault==2)bad.images[0].schema=0;if(fault==3)bad.images[1].role=1;if(fault==4)bad.images[0].width++;if(fault==5)bad.images[1].height--;if(fault==6)bad.images[0].bytes--;if(fault==7)bad.images[1].rgba=nullptr;if(fault==8)bad.images[0].sha256[0]^=1;if(fault==9)bad.images[1].straightRgba=0;if(fault==10)bad.images[0].bytes=SIZE_MAX;if(fault==11)bad.images[0].role=2;target.bind();before=capture();draws=0;require(!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&bad,&stage,&aliases)&&!draws&&equal(before,capture())&&target.read()==rendered,"malformed art metadata rejected before draws");}
 // Dynamic pins are provider-owned. A newly pinned revision must not silently
 // reuse textures from the previous revision; owner explicitly retires first.
 art[0].rgba[0]^=1;auto revision=v;require(nfs_texture_sha256(revision.images[0].rgba,revision.images[0].bytes,revision.images[0].sha256),"revision digest");before=capture();draws=0;require(!nfs_feedback_modal_backdrop_gpu_prepare(&gpu,&revision,&stage,&aliases)&&!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&revision,&stage,&aliases)&&!draws&&equal(before,capture()),"new provider revision cannot use stale GPU pixels");art[0].rgba[0]^=1;
 for(unsigned fault=0;fault<4;++fault){gpu=saved;if(fault==0)gpu.program=borrowedProgram;if(fault==1)gpu.textures[0]=target.texture;if(fault==2)gpu.textures[1]=borrowedTexture;if(fault==3)gpu.textures[1]=gpu.textures[0];target.bind();before=capture();draws=0;require(!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&v,&stage,&aliases)&&!draws&&equal(before,capture()),"independent resource alias refused");}
 gpu=saved;auto badAliases=aliases;badAliases.textureCount=129;draws=0;require(!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&v,&stage,&badAliases)&&!draws,"bounded alias inventory");
 for(unsigned failure:{1u,2u,3u}){target.clear();before=capture();draws=0;failDraw=failure;require(!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&v,&stage,&aliases)&&equal(before,capture()),"late draw error restores state and refuses unpublished stage");failDraw=0;}
 const EGLint attributes[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};auto foreign=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attributes);require(foreign!=EGL_NO_CONTEXT,"foreign context");
 for(unsigned point:{1u,2u,3u}){require(eglMakeCurrent(egl.display,egl.surface,egl.surface,foreign),"foreign current");glViewport(7,9,71,83);glActiveTexture(GL_TEXTURE1);auto foreignState=capture();require(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owned current");target.clear();gpu=saved;lossDisplay=egl.display;lossSurface=egl.surface;lossContext=foreign;draws=0;loseDraw=point;require(!nfs_feedback_modal_backdrop_gpu_draw(&gpu,&v,&stage,&aliases)&&!gpu.program&&!gpu.generation&&equal(foreignState,capture()),"owner loss forgets without foreign state changes");loseDraw=0;}
 require(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"return owner");require(eglDestroyContext(egl.display,foreign),"destroy foreign");gpu=saved;target.bind();owner.strict=false;
 require(nfs_feedback_modal_backdrop_gpu_retire(&gpu,&stage,1,&aliases)&&!gpu.program&&!glIsTexture(saved.textures[0])&&!glIsTexture(saved.textures[1]),"only independent textures retired");
 if(glIsProgram(saved.program)){GLint pending=0,current=0;glGetProgramiv(saved.program,GL_DELETE_STATUS,&pending);glGetIntegerv(GL_CURRENT_PROGRAM,&current);require(pending&&GLuint(current)==saved.program,"current owned program deletion pending");glUseProgram(0);}require(!glIsProgram(saved.program),"owned program deleted after unbind");
 require(glIsTexture(target.texture)&&glIsTexture(borrowedTexture)&&glIsProgram(borrowedProgram),"borrowed resources retained");glDeleteTextures(1,&borrowedTexture);glDeleteProgram(borrowedProgram);require(glGetError()==GL_NO_ERROR,"final GL error clear");
}
}
int main(){try{nfs_hd_test::Pbuffer egl(640,480);run(egl);std::puts("PASS feedback backdrop isolated GPU mechanics; synthetic art; no publication");return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL feedback backdrop: %s\n",e.what());return 1;}}
