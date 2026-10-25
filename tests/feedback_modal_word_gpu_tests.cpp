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
#include "hd/FeedbackModalWordArtwork.h"
static unsigned draws=0,failDraw=0,loseDraw=0;
static EGLDisplay lossDisplay=EGL_NO_DISPLAY;
static EGLSurface lossSurface=EGL_NO_SURFACE;
static EGLContext lossContext=EGL_NO_CONTEXT;
static void wordDraw(GLenum mode,GLint first,GLsizei count){
 ++draws;glDrawArrays(draws==failDraw?GLenum(0xffffffff):mode,first,count);
 if(draws==loseDraw)nfs_hd_test::require(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected actual context loss");
}
#define glDrawArrays(...) wordDraw(__VA_ARGS__)
#include "hd/FeedbackModalWordGpu.c.inc"
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
static NfsFeedbackModalWordsView view(std::array<std::shared_ptr<const nfs_feedback_modal_word::Artwork>,2>&art){
 NfsFeedbackModalWordsView v{};v.schema=1;v.count=2;
 for(unsigned i=0;i<2;++i){art[i]=nfs_feedback_modal_word::artwork(i+1);require(bool(art[i]),"finite art decoded");auto&a=v.images[i];a.word=i+1;a.width=art[i]->width();a.height=128;a.rgba=art[i]->rgba().data();a.bytes=art[i]->rgba().size();std::copy(art[i]->sha().begin(),art[i]->sha().end(),a.sha256);}
 v.placements[0]={1,1,0,78,302,185,398,217};v.placements[1]={2,65535,78,84,266,225,374,257};return v;
}
static double alpha(const nfs_feedback_modal_word::Artwork&a,double x,double y,const NfsFeedbackModalWordPlacement&p){
 if(x<p.left||x>=p.right||y<p.top||y>=p.bottom)return 0;
 double X=std::max(0.,std::min(double(a.width()-1),(x-p.left)*4-.5));
 double Y=std::max(0.,std::min(127.,(y-p.top)*4-.5));unsigned i=unsigned(X),j=unsigned(Y),k=std::min(i+1,a.width()-1),l=std::min(j+1,127u);double fx=X-i,fy=Y-j;
 auto at=[&](unsigned x,unsigned y){return double(a.rgba()[(size_t(y)*a.width()+x)*4+3]);};
 return ((at(i,j)*(1-fx)+at(k,j)*fx)*(1-fy)+(at(i,l)*(1-fx)+at(k,l)*fx)*fy)/255.;
}
static void run(Pbuffer&egl){
 Target target;Owner owner{egl.context,1,target.fbo,false};NfsBackgroundSceneStage stage{};
 stage.generation=1;stage.contextIdentity=reinterpret_cast<uintptr_t>(egl.context);stage.ownerUser=&owner;stage.ownerCurrent=Owner::current;stage.width=640;stage.height=480;stage.framebuffer=target.fbo;
 require(nfs_frame_gpu_legacy_sampling(640,480,GL_LINEAR,&stage.sampling),"neutral sampling");
 std::array<std::shared_ptr<const nfs_feedback_modal_word::Artwork>,2>art;auto v=view(art);
 std::vector<NfsSoftwareTextVertex>vertices(162);NfsSoftwareTextLayerView text{};text.vertices=vertices.data();text.vertexCount=vertices.size();
 GLuint borrowedTexture=0;glGenTextures(1,&borrowedTexture);glBindTexture(GL_TEXTURE_2D,borrowedTexture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
 GLuint borrowedProgram=glCreateProgram();GLuint forbiddenPrograms[]={borrowedProgram};GLuint forbiddenTextures[]={target.texture,borrowedTexture};
 NfsFeedbackModalWordGpuAliases aliases{forbiddenPrograms,1,forbiddenTextures,2};NfsFeedbackModalWordGpu gpu{};
 target.bind();auto before=capture();require(nfs_feedback_modal_word_prepare(&gpu,&v,&text,&stage,&aliases)&&equal(before,capture()),"cold preparation state restored");
 const auto saved=gpu;before=capture();require(nfs_feedback_modal_word_prepare(&gpu,&v,&text,&stage,&aliases)&&equal(before,capture())&&!std::memcmp(&saved,&gpu,sizeof gpu),"warm resource cache retained");
 owner.strict=true;target.clear();glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glEnable(GL_SCISSOR_TEST);glScissor(3,4,17,19);glActiveTexture(GL_TEXTURE1);before=capture();draws=0;
 require(nfs_feedback_modal_word_draw(&gpu,&v,&text,&stage,&aliases)&&draws==2&&equal(before,capture()),"two unshadowed words restore state");
 auto rendered=target.read();unsigned changed[2]={0,0};
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){double rgb[3]={64,128,192};
  for(unsigned i=0;i<2;++i){const auto&p=v.placements[i];double a=alpha(*art[i],x+.5,y+.5,p);if(a>.5)++changed[i];const double tint[3]={i?255.:0.,i?255.:0.,i?255.:8.};for(unsigned k=0;k<3;++k)rgb[k]=rgb[k]*(1-a)+tint[k]*a;}
  size_t at=(size_t(479-y)*640+x)*4;for(unsigned k=0;k<3;++k)require(std::abs(int(rendered[at+k])-int(std::lround(rgb[k])))<=3,"independent original565 tint alpha and placement oracle");
 }
 require(changed[0]>30&&changed[1]>30,"both words visible");
 for(unsigned fault=0;fault<17;++fault){auto bad=v;if(fault==0)bad.schema=0;if(fault==1)bad.count=1;if(fault==2)bad.images[0].word=2;if(fault==3)bad.images[1].width++;if(fault==4)bad.images[0].height--;if(fault==5)bad.images[1].bytes--;if(fault==6)bad.images[0].sha256[0]^=1;if(fault==7)bad.images[1].rgba=nullptr;if(fault==8)bad.placements[0].color565=65535;if(fault==9)bad.placements[1].color565=1;if(fault==10)bad.placements[0].left++;if(fault==11)bad.placements[1].bottom++;if(fault==12)bad.placements[0].vertexFirst=1;if(fault==13)bad.placements[1].vertexFirst=0;if(fault==14)bad.placements[1].vertexCount=SIZE_MAX;if(fault==15)bad.placements[1].top=NAN;if(fault==16)bad.placements[0].vertexCount=0;target.bind();before=capture();draws=0;require(!nfs_feedback_modal_word_draw(&gpu,&bad,&text,&stage,&aliases)&&!draws&&equal(before,capture())&&target.read()==rendered,"malformed finite view rejected before drawing");}
 for(unsigned fault=0;fault<4;++fault){gpu=saved;if(fault==0)gpu.program=borrowedProgram;if(fault==1)gpu.textures[0]=target.texture;if(fault==2)gpu.textures[1]=borrowedTexture;if(fault==3)gpu.textures[1]=gpu.textures[0];target.bind();before=capture();draws=0;require(!nfs_feedback_modal_word_draw(&gpu,&v,&text,&stage,&aliases)&&!draws&&equal(before,capture()),"independent resource alias refused");}
 gpu=saved;auto badAliases=aliases;badAliases.textureCount=129;draws=0;require(!nfs_feedback_modal_word_draw(&gpu,&v,&text,&stage,&badAliases)&&!draws,"bounded alias inventory");
 for(unsigned failure:{1u,2u}){target.clear();before=capture();draws=0;failDraw=failure;require(!nfs_feedback_modal_word_draw(&gpu,&v,&text,&stage,&aliases)&&equal(before,capture()),"late draw error restores state and refuses unpublished stage");failDraw=0;}
 // Change the actual EGL context immediately after either authored-word draw.
 // The guard must stop without restoring/deleting/reading errors in that owner.
 const EGLint attributes[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};auto foreign=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attributes);require(foreign!=EGL_NO_CONTEXT,"foreign context");
 for(unsigned point:{1u,2u}){require(eglMakeCurrent(egl.display,egl.surface,egl.surface,foreign),"foreign current");glViewport(7,9,71,83);glActiveTexture(GL_TEXTURE1);auto foreignState=capture();require(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owned current");target.clear();gpu=saved;lossDisplay=egl.display;lossSurface=egl.surface;lossContext=foreign;draws=0;loseDraw=point;require(!nfs_feedback_modal_word_draw(&gpu,&v,&text,&stage,&aliases)&&!gpu.program&&!gpu.generation&&equal(foreignState,capture()),"owner loss forgets without foreign state changes");loseDraw=0;}
 require(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"return owner");require(eglDestroyContext(egl.display,foreign),"destroy foreign");gpu=saved;target.bind();owner.strict=false;
 require(nfs_feedback_modal_word_retire(&gpu,&stage,1,&aliases)&&!gpu.program&&!glIsTexture(saved.textures[0])&&!glIsTexture(saved.textures[1]),"independent textures retired");
 // Context-loss injection left the owned program current. GL defers deletion
 // of a current program until it is unbound; retirement must not rewrite that
 // binding merely to make glIsProgram immediately false.
 if(glIsProgram(saved.program)){GLint pending=0,current=0;glGetProgramiv(saved.program,GL_DELETE_STATUS,&pending);glGetIntegerv(GL_CURRENT_PROGRAM,&current);require(pending&&GLuint(current)==saved.program,"current owned program deletion pending");glUseProgram(0);}
 require(!glIsProgram(saved.program),"owned program deleted after unbind");
 require(glIsTexture(target.texture)&&glIsTexture(borrowedTexture)&&glIsProgram(borrowedProgram),"borrowed resources retained");glDeleteTextures(1,&borrowedTexture);glDeleteProgram(borrowedProgram);require(glGetError()==GL_NO_ERROR,"final GL error clear");
}
}
int main(){try{nfs_hd_test::Pbuffer egl(640,480);run(egl);std::puts("PASS feedback two-word isolated GPU mechanics; synthetic source; no publication");return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL feedback words: %s\n",e.what());return 1;}}
