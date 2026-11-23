// SPDX-License-Identifier: MIT
// Actual information-wide shader/state code on EGL; synthetic frame provenance.
#include "HdSceneGlesFixture.h"
#include "hd/SceneGpuSupport.inc"
#include "hd/BackgroundSceneState.h"
#include "hd/NativeSceneTarget.h"
#include "hd/NativeSceneProvenance.h"
#include "TextureDigest.h"
#include <cstdio>
#include <cerrno>
#include <array>
static EGLContext g_glCtx=EGL_NO_CONTEXT;
static EGLContext SDL_GL_GetCurrentContext(){return eglGetCurrentContext();}
static GLuint g_framebuffer=0,g_framebufferTexture=0;
static NfsLogoProvenance nfs_logo_provenance{};
static int nfs_logo_trace_enabled(){return 0;}
static bool foreignGuard=false;static unsigned foreignDeletes=0;
static void monitoredDelete(GLsizei count,const GLuint*names){if(foreignGuard)foreignDeletes+=unsigned(count);glDeleteTextures(count,names);}
#define glDeleteTextures monitoredDelete
#include "hd/ClassicScreens/InformationWideRenderer.inc"
#undef glDeleteTextures
static unsigned checks=0;
static void check(bool ok,const char*why){nfs_hd_test::require(ok,why);++checks;}
struct Owner {unsigned refs=1;bool valid=true;static void retain(void*p){++static_cast<Owner*>(p)->refs;}static void release(void*p){auto&o=*static_cast<Owner*>(p);check(o.refs>1,"balanced retained presentation owner");--o.refs;}static int current(void*p){return static_cast<Owner*>(p)->valid;}};
static std::array<unsigned char,4>pixel(unsigned x,unsigned y){std::array<unsigned char,4>p{};glReadPixels(x,y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;}
int main(){try{
 nfs_hd_test::Pbuffer context(800,500);g_glCtx=context.context;nfs_logo_provenance.alive=1;nfs_logo_provenance.context=1;nfs_logo_provenance.clearEvent=1;
 std::vector<unsigned char>green(640*480*4,0);for(size_t i=0;i<green.size();i+=4){green[i]=static_cast<unsigned char>((i/4)%640%61);green[i+1]=255;green[i+2]=static_cast<unsigned char>((i/4)/640%59);green[i+3]=255;}
 glGenTextures(1,&g_framebufferTexture);glBindTexture(GL_TEXTURE_2D,g_framebufferTexture);nfs_frame_texture_params();glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,green.data());glGenFramebuffers(1,&g_framebuffer);glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,g_framebufferTexture,0);check(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"native target");
 NfsNativeLogoTargetProof target{};target.framebuffer=g_framebuffer;target.width=640;target.height=480;
 std::vector<unsigned char>art(16*9*4,255);for(unsigned y=0;y<9;++y)for(unsigned x=0;x<16;++x){auto*p=art.data()+4*(y*16+x);p[0]=x<14?255:0;p[1]=(x<2&&y<4)?255:0;p[2]=x>=2?255:0;}
 Owner owner;NfsHistoryPanoramaView view{};view.width=16;view.height=9;view.sourceIndex=0;view.baseGeneration=7;view.bytes=art.size();view.pixels=art.data();view.owner=&owner;view.retain=Owner::retain;view.release=Owner::release;view.current=Owner::current;nfs_texture_sha256(art.data(),art.size(),view.sha);
 GLuint sentinel=0;glGenTextures(1,&sentinel);glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,sentinel);glActiveTexture(GL_TEXTURE0);glPixelStorei(GL_UNPACK_ALIGNMENT,8);
 for(auto dimensions:{std::array<int,2>{800,450},std::array<int,2>{708,492}}){
  const int w=dimensions[0],h=dimensions[1];glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);glViewport(3,5,600,420);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);auto before=nfs_hd_test::capture();
  nfs_info_wide_mark(3,&target);check(nfs_info_wide_present(g_framebuffer,g_framebufferTexture,w,h)!=0,"reference center presentation");glBindFramebuffer(GL_FRAMEBUFFER,nfs_info_wide.fbo);std::vector<unsigned char>reference(size_t(w)*h*4);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,reference.data());glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);nfs_info_wide_after_present();
  nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);check(owner.refs==2,"presentation retains owner");auto result=nfs_info_wide_present(g_framebuffer,g_framebufferTexture,w,h);check(result!=0,"panorama presentation succeeds");check(nfs_hd_test::equal(before,nfs_hd_test::capture()),"production state restored");
  GLint bound=0;glActiveTexture(GL_TEXTURE1);glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);check(GLuint(bound)==sentinel,"texture unit 1 restored");glActiveTexture(GL_TEXTURE0);
  glBindFramebuffer(GL_FRAMEBUFFER,nfs_info_wide.fbo);auto center=pixel(w/2,h/2),left=pixel(2,8),top=pixel(2,h-8),right=pixel(w-3,h/2);check(center[0]<80&&center[1]>250&&center[2]<80,"original center pattern retained, never panorama magenta");
  std::vector<unsigned char>output(size_t(w)*h*4);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,output.data());bool centerExact=true;const int inset=(w-h*4/3)/2;
  for(int y=1;y<h-1;++y)for(int x=inset+2;x<w-inset-2;++x){const auto*p=output.data()+4*(y*w+x);if(std::memcmp(p,reference.data()+4*(y*w+x),4))centerExact=false;}check(centerExact,"complete interior central region preserved pixel-exactly");
check(left[0]>230&&left[1]<10,"left extension uses panorama bottom");check(top[0]>230&&top[1]>150,"top-down artwork orientation preserved");check(right[2]>230&&right[0]<20,"right extension uses panorama");
  glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);nfs_info_wide_after_present();check(owner.refs==1,"presentation releases owner");
 }
 nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);owner.valid=false;check(nfs_info_wide_present(g_framebuffer,g_framebufferTexture,800,450)!=0,"revoked optional art retains original center fallback");glBindFramebuffer(GL_FRAMEBUFFER,nfs_info_wide.fbo);check(pixel(2,200)[0]<100,"revoked panorama not drawn");check(pixel(400,200)[1]>250,"revoked panorama preserves native center");glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);nfs_info_wide_after_present();check(owner.refs==1,"revoked owner released");owner.valid=true;
 nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);++nfs_logo_provenance.clearEvent;check(!nfs_info_wide_present(g_framebuffer,g_framebufferTexture,800,450),"changed native clear rejects complete frame");nfs_info_wide_after_present();check(owner.refs==1,"declined frame releases owner");
 nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);nfs_info_wide_retire(1);check(owner.refs==1,"context retirement releases owner");check(!nfs_info_wide.panoramaTexture,"context-owned panorama texture retired");check(glGetError()==GL_NO_ERROR,"no GL errors");
 nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);check(nfs_info_wide_present(g_framebuffer,g_framebufferTexture,800,450)!=0,"recreated presentation");nfs_info_wide_after_present();nfs_info_wide_mark(3,&target);nfs_info_wide_mark_history(&view);
 const EGLint attributes[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};EGLContext foreign=eglCreateContext(context.display,context.config,EGL_NO_CONTEXT,attributes);check(foreign!=EGL_NO_CONTEXT,"foreign context created");check(eglMakeCurrent(context.display,context.surface,context.surface,foreign),"foreign context bound");foreignGuard=true;nfs_info_wide_retire(1);foreignGuard=false;check(foreignDeletes==0,"no texture deletion on foreign context");check(owner.refs==1,"foreign-context retirement releases CPU owner");check(eglMakeCurrent(context.display,context.surface,context.surface,context.context),"original context rebound");eglDestroyContext(context.display,foreign);

 glDeleteTextures(1,&sentinel);glDeleteTextures(1,&g_framebufferTexture);glDeleteFramebuffers(1,&g_framebuffer);std::printf("History panorama GPU checks passed: %u\n",checks);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"History panorama GPU failure: %s\n",e.what());return 1;}}
