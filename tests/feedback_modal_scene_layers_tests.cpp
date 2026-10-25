// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include <array>
#include <algorithm>
#include <cstdlib>
#include <thread>
#include <type_traits>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>
static unsigned graphicsDialogLegacyFailIndex=0;static const char*graphicsDialogCheckpoint="legacy";static unsigned checks=0;static void ck(bool b,const char*w){nfs_hd_test::require(b,w);++checks;}
static std::vector<GLsizei>controlsDrawCounts;static unsigned drawCount=0,failAt=0,foreignTouches=0,callIndex=0,loseIndex=0;static EGLDisplay lossDisplay=EGL_NO_DISPLAY;static EGLSurface lossSurface=EGL_NO_SURFACE;static EGLContext lossContext=EGL_NO_CONTEXT;static bool recording=false,trackForeign=false;static std::vector<GLuint>programs;static std::vector<std::string>events;
static void afterCall(const char*name){if(!recording)return;events.push_back(name);if(++callIndex==loseIndex)ck(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected actual owner loss");}
template<class F>static auto probe(const char*name,F f)->decltype(f()){if(trackForeign&&eglGetCurrentContext()==lossContext)++foreignTouches;if constexpr(std::is_void_v<decltype(f())>){f();afterCall(name);}else{auto value=f();afterCall(name);return value;}}
static void draw(GLenum mode,GLint first,GLsizei count){++drawCount;controlsDrawCounts.push_back(count);GLint p=0;glGetIntegerv(GL_CURRENT_PROGRAM,&p);programs.push_back(GLuint(p));GLboolean mask=GL_TRUE;glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);ck(!mask&&!glIsEnabled(GL_DEPTH_TEST),"every staged layer depth isolated");glDrawArrays(drawCount==failAt?GLenum(0xffffffff):mode,first,count);}
#define glActiveTexture(...) probe("glActiveTexture",[&](){return ::glActiveTexture(__VA_ARGS__);})
#define glAttachShader(...) probe("glAttachShader",[&](){return ::glAttachShader(__VA_ARGS__);})
#define glBindAttribLocation(...) probe("glBindAttribLocation",[&](){return ::glBindAttribLocation(__VA_ARGS__);})
#define glBindBuffer(...) probe("glBindBuffer",[&](){return ::glBindBuffer(__VA_ARGS__);})
#define glBindFramebuffer(...) probe("glBindFramebuffer",[&](){return ::glBindFramebuffer(__VA_ARGS__);})
#define glBindRenderbuffer(...) probe("glBindRenderbuffer",[&](){return ::glBindRenderbuffer(__VA_ARGS__);})
#define glBindTexture(...) probe("glBindTexture",[&](){return ::glBindTexture(__VA_ARGS__);})
#define glBlendEquationSeparate(...) probe("glBlendEquationSeparate",[&](){return ::glBlendEquationSeparate(__VA_ARGS__);})
#define glBlendFuncSeparate(...) probe("glBlendFuncSeparate",[&](){return ::glBlendFuncSeparate(__VA_ARGS__);})
#define glCheckFramebufferStatus(...) probe("glCheckFramebufferStatus",[&](){return ::glCheckFramebufferStatus(__VA_ARGS__);})
#define glClearDepthf(...) probe("glClearDepthf",[&](){return ::glClearDepthf(__VA_ARGS__);})
#define glColorMask(...) probe("glColorMask",[&](){return ::glColorMask(__VA_ARGS__);})
#define glCompileShader(...) probe("glCompileShader",[&](){return ::glCompileShader(__VA_ARGS__);})
#define glCreateProgram(...) probe("glCreateProgram",[&](){return ::glCreateProgram(__VA_ARGS__);})
#define glCreateShader(...) probe("glCreateShader",[&](){return ::glCreateShader(__VA_ARGS__);})
#define glDeleteFramebuffers(...) probe("glDeleteFramebuffers",[&](){return ::glDeleteFramebuffers(__VA_ARGS__);})
#define glDeleteProgram(...) probe("glDeleteProgram",[&](){return ::glDeleteProgram(__VA_ARGS__);})
#define glDeleteShader(...) probe("glDeleteShader",[&](){return ::glDeleteShader(__VA_ARGS__);})
#define glDeleteTextures(...) probe("glDeleteTextures",[&](){return ::glDeleteTextures(__VA_ARGS__);})
#define glDepthFunc(...) probe("glDepthFunc",[&](){return ::glDepthFunc(__VA_ARGS__);})
#define glDepthMask(...) probe("glDepthMask",[&](){return ::glDepthMask(__VA_ARGS__);})
#define glDisable(...) probe("glDisable",[&](){return ::glDisable(__VA_ARGS__);})
#define glDisableVertexAttribArray(...) probe("glDisableVertexAttribArray",[&](){return ::glDisableVertexAttribArray(__VA_ARGS__);})
#define glDrawArrays(...) probe("glDrawArrays",[&](){return draw(__VA_ARGS__);})
#define glEnable(...) probe("glEnable",[&](){return ::glEnable(__VA_ARGS__);})
#define glEnableVertexAttribArray(...) probe("glEnableVertexAttribArray",[&](){return ::glEnableVertexAttribArray(__VA_ARGS__);})
#define glFramebufferTexture2D(...) probe("glFramebufferTexture2D",[&](){return ::glFramebufferTexture2D(__VA_ARGS__);})
#define glGenFramebuffers(...) probe("glGenFramebuffers",[&](){return ::glGenFramebuffers(__VA_ARGS__);})
#define glGenTextures(...) probe("glGenTextures",[&](){return ::glGenTextures(__VA_ARGS__);})
#define glGetBooleanv(...) probe("glGetBooleanv",[&](){return ::glGetBooleanv(__VA_ARGS__);})
#define glGetError(...) probe("glGetError",[&](){return ::glGetError(__VA_ARGS__);})
#define glGetFloatv(...) probe("glGetFloatv",[&](){return ::glGetFloatv(__VA_ARGS__);})
#define glGetFramebufferAttachmentParameteriv(...) probe("glGetFramebufferAttachmentParameteriv",[&](){return ::glGetFramebufferAttachmentParameteriv(__VA_ARGS__);})
#define glGetIntegerv(...) probe("glGetIntegerv",[&](){return ::glGetIntegerv(__VA_ARGS__);})
#define glGetProgramiv(...) probe("glGetProgramiv",[&](){return ::glGetProgramiv(__VA_ARGS__);})
#define glGetShaderiv(...) probe("glGetShaderiv",[&](){return ::glGetShaderiv(__VA_ARGS__);})
#define glGetUniformLocation(...) probe("glGetUniformLocation",[&](){return ::glGetUniformLocation(__VA_ARGS__);})
#define glGetVertexAttribPointerv(...) probe("glGetVertexAttribPointerv",[&](){return ::glGetVertexAttribPointerv(__VA_ARGS__);})
#define glGetVertexAttribfv(...) probe("glGetVertexAttribfv",[&](){return ::glGetVertexAttribfv(__VA_ARGS__);})
#define glGetVertexAttribiv(...) probe("glGetVertexAttribiv",[&](){return ::glGetVertexAttribiv(__VA_ARGS__);})
#define glIsEnabled(...) probe("glIsEnabled",[&](){return ::glIsEnabled(__VA_ARGS__);})
#define glIsFramebuffer(...) probe("glIsFramebuffer",[&](){return ::glIsFramebuffer(__VA_ARGS__);})
#define glIsProgram(...) probe("glIsProgram",[&](){return ::glIsProgram(__VA_ARGS__);})
#define glIsTexture(...) probe("glIsTexture",[&](){return ::glIsTexture(__VA_ARGS__);})
#define glLinkProgram(...) probe("glLinkProgram",[&](){return ::glLinkProgram(__VA_ARGS__);})
#define glPixelStorei(...) probe("glPixelStorei",[&](){return ::glPixelStorei(__VA_ARGS__);})
#define glScissor(...) probe("glScissor",[&](){return ::glScissor(__VA_ARGS__);})
#define glShaderSource(...) probe("glShaderSource",[&](){return ::glShaderSource(__VA_ARGS__);})
#define glTexImage2D(...) probe("glTexImage2D",[&](){return ::glTexImage2D(__VA_ARGS__);})
#define glTexParameteri(...) probe("glTexParameteri",[&](){return ::glTexParameteri(__VA_ARGS__);})
#define glUniform1f(...) probe("glUniform1f",[&](){return ::glUniform1f(__VA_ARGS__);})
#define glUniform1i(...) probe("glUniform1i",[&](){return ::glUniform1i(__VA_ARGS__);})
#define glUniformMatrix4fv(...) probe("glUniformMatrix4fv",[&](){return ::glUniformMatrix4fv(__VA_ARGS__);})
#define glUseProgram(...) probe("glUseProgram",[&](){return ::glUseProgram(__VA_ARGS__);})
#define glVertexAttrib4fv(...) probe("glVertexAttrib4fv",[&](){return ::glVertexAttrib4fv(__VA_ARGS__);})
#define glVertexAttribPointer(...) probe("glVertexAttribPointer",[&](){return ::glVertexAttribPointer(__VA_ARGS__);})
#define glViewport(...) probe("glViewport",[&](){return ::glViewport(__VA_ARGS__);})
#include "hd/ControlsSceneLayers.c.inc"
#include "hd/ControlsDialogWordGpu.c.inc"
#undef glActiveTexture
#undef glAttachShader
#undef glBindAttribLocation
#undef glBindBuffer
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glBindTexture
#undef glBlendEquationSeparate
#undef glBlendFuncSeparate
#undef glCheckFramebufferStatus
#undef glClearDepthf
#undef glColorMask
#undef glCompileShader
#undef glCreateProgram
#undef glCreateShader
#undef glDeleteFramebuffers
#undef glDeleteProgram
#undef glDeleteShader
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDisable
#undef glDisableVertexAttribArray
#undef glDrawArrays
#undef glEnable
#undef glEnableVertexAttribArray
#undef glFramebufferTexture2D
#undef glGenFramebuffers
#undef glGenTextures
#undef glGetBooleanv
#undef glGetError
#undef glGetFloatv
#undef glGetFramebufferAttachmentParameteriv
#undef glGetIntegerv
#undef glGetProgramiv
#undef glGetShaderiv
#undef glGetUniformLocation
#undef glGetVertexAttribPointerv
#undef glGetVertexAttribfv
#undef glGetVertexAttribiv
#undef glIsEnabled
#undef glIsFramebuffer
#undef glIsProgram
#undef glIsTexture
#undef glLinkProgram
#undef glPixelStorei
#undef glScissor
#undef glShaderSource
#undef glTexImage2D
#undef glTexParameteri
#undef glUniform1f
#undef glUniform1i
#undef glUniformMatrix4fv
#undef glUseProgram
#undef glVertexAttrib4fv
#undef glVertexAttribPointer
#undef glViewport
#include "AudioSceneLayersFixture.h"
using namespace nfs_hd_test;
struct Target {GLuint texture=0,fbo=0,depth=0;unsigned w,h;Target(unsigned W,unsigned H):w(W),h(H){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"fixture FBO");}void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);}void seed(){bind();glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glDepthMask(1);glClearColor(0,0,1,1);glClearDepthf(.25);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);}std::vector<unsigned char>read(){bind();std::vector<unsigned char>b(size_t(w)*h*4);GLenum prior=glGetError();if(prior!=GL_NO_ERROR)std::fprintf(stderr,"GRAPHICS FIXTURE checkpoint=%s size=%ux%u beforeReadError=0x%x checks=%u legacyFailAt=%u\n",graphicsDialogCheckpoint,w,h,unsigned(prior),checks,graphicsDialogLegacyFailIndex);ck(prior==GL_NO_ERROR,"zero GL error before read");glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,b.data());GLenum after=glGetError();if(after!=GL_NO_ERROR)std::fprintf(stderr,"GRAPHICS FIXTURE checkpoint=%s size=%ux%u readError=0x%x checks=%u legacyFailAt=%u\n",graphicsDialogCheckpoint,w,h,unsigned(after),checks,graphicsDialogLegacyFailIndex);ck(after==GL_NO_ERROR,"read");return b;}~Target(){glDeleteFramebuffers(1,&fbo);glDeleteRenderbuffers(1,&depth);glDeleteTextures(1,&texture);}};
static void depthUnchanged(Target&t){t.bind();const auto before=t.read();const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,1,0,1);}";GLuint v=nfs_hd_test::compileShader(GL_VERTEX_SHADER,vs),f=nfs_hd_test::compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glUseProgram(p);glBindBuffer(GL_ARRAY_BUFFER,0);glViewport(0,0,t.w,t.h);glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glDisable(GL_STENCIL_TEST);glDisable(GL_CULL_FACE);glColorMask(1,1,1,1);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(0);const GLfloat q[]={-1,-1,1,-1,-1,1,1,1};glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,0,0,q);glDrawArrays(GL_TRIANGLE_STRIP,0,4);ck(t.read()==before,"seeded .25 depth rejects .5 probe without color change");glUseProgram(0);glDeleteProgram(p);glDeleteShader(v);glDeleteShader(f);}
struct Registered {EGLContext context;uint64_t generation=1;GLuint fbo=0;unsigned w=0,h=0;std::thread::id thread=std::this_thread::get_id();};
static int stageCurrent(void*user,const NfsBackgroundSceneStage*s){auto&o=*static_cast<Registered*>(user);if(eglGetCurrentContext()!=o.context||o.thread!=std::this_thread::get_id()||s->contextIdentity!=reinterpret_cast<uintptr_t>(o.context)||s->generation!=o.generation||s->framebuffer!=o.fbo||s->width!=o.w||s->height!=o.h)return 0;GLint fbo=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);return GLuint(fbo)==o.fbo;}
static int contextOnlyCurrent(void*user,const NfsBackgroundSceneStage*s){auto&o=*static_cast<Registered*>(user);return eglGetCurrentContext()==o.context&&o.thread==std::this_thread::get_id()&&s->contextIdentity==reinterpret_cast<uintptr_t>(o.context)&&s->generation==o.generation;}
static NfsBackgroundSceneStage stage(Target&t,Registered&o){o.fbo=t.fbo;o.w=t.w;o.h=t.h;NfsBackgroundSceneStage s{};s.generation=o.generation;s.contextIdentity=reinterpret_cast<uintptr_t>(o.context);s.framebuffer=t.fbo;s.width=t.w;s.height=t.h;s.ownerCurrent=stageCurrent;s.ownerUser=&o;ck(nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&s.sampling),"native sampling");t.bind();glViewport(0,0,s.sampling.viewport[2],s.sampling.viewport[3]);return s;}
static std::array<NfsSoftwareTextVertex,6>text(float l,float t,float r,float b){return {{{l,t,.2,.2,1,1,1,1},{r,t,.8,.2,1,1,1,1},{l,b,.2,.8,1,1,1,1},{l,b,.2,.8,1,1,1,1},{r,t,.8,.2,1,1,1,1},{r,b,.8,.8,1,1,1,1}}};}
static NfsBackgroundSceneLayers layers(){NfsBackgroundSceneLayers v{};v.panelClip[2]=343;v.panelClip[3]=480;v.wordCount=2;for(unsigned i=0;i<2;++i){auto&w=v.words[i];w.sourceRole=i+1;w.scalar256=i?256:128;w.left=400+8*i;w.top=200+4*i;w.clip[2]=640;w.clip[3]=480;}unsigned roles[3]={1,2,0};for(unsigned i=0;i<3;++i){auto&r=nfs_logo_regions[roles[i]];auto&l=v.logos[i];l.role=roles[i];l.scalar256=i==1?127:256;l.left=r.sourceOriginX;l.top=r.sourceOriginY;l.right=l.left+r.logicalWidth;l.bottom=l.top+r.logicalHeight;l.clip[2]=343;l.clip[3]=480;}return v;}
static std::array<int,3>over(std::array<int,3>d,std::array<int,4>s,unsigned scalar=256){for(unsigned k=0;k<3;++k)d[k]=int(std::lround(s[k]*scalar/256.+d[k]*(1.-s[3]*scalar/(256.*255.))));return d;}
static void pixel(const Target&t,const std::vector<unsigned char>&b,unsigned x,unsigned y,std::array<int,3>rgb){unsigned px=unsigned((x+.5)*t.h/480.),py=t.h-1-unsigned((y+.5)*t.h/480.);for(unsigned k=0;k<3;++k)ck(std::abs(int(b[(size_t(py)*t.w+px)*4+k])-rgb[k])<=2,"independent layer order pixel oracle");}
struct Resources {NfsBackgroundSceneCompositor bg{};NfsSoftwareTextGpu caption{},font{};std::vector<unsigned char>alpha;NfsSoftwareTextFontView fontView{};Resources(EGLContext,uint64_t generation):alpha(1024*512,128){fontView.schema=1;fontView.width=1024;fontView.height=512;fontView.alpha=alpha.data();fontView.bytes=alpha.size();fontView.identity=67;nfs_texture_sha256(alpha.data(),alpha.size(),fontView.sha256);ck(nfs_bg_scene_context_created(&bg,generation)&&nfs_bg_scene_prepare(&bg),"prepare synthetic background");ck(nfs_soft_gpu_context_created(&caption,generation)&&nfs_soft_gpu_prepare(&caption,&fontView)&&nfs_soft_gpu_context_created(&font,generation)&&nfs_soft_gpu_prepare(&font,&fontView),"prepare separate fonts");}NfsControlsSceneGpu view(){return {&bg,&caption,&font};}void retire(){nfs_bg_scene_context_retired(&bg,bg.generation,1);nfs_soft_gpu_context_retired(&caption,caption.generation,1);nfs_soft_gpu_context_retired(&font,font.generation,1);}};
// Actual GLES mechanics and maintained Controls artwork. Source packet/proof is
// explicitly SYNTHETIC; no guest hooks, FIFO/native witness or scene activation.
#include "hd/ControlsDialogWordDisplay.h"
#include "hd/CompactCaptionWordArtwork.h"
#include <functional>
namespace controls_synthetic {
#define main controlsPureProofFixtureUnusedMain
#include "controls_dialog_word_proof_tests.cpp"
#undef main
}
namespace nfs_controls_text {
struct TailScope {static std::shared_ptr<const nfs_controls_dialog_word::Proof>synthetic(controls_synthetic::Packet p,const nfs_controls_dialog_word::Evidence&e){return std::shared_ptr<const nfs_controls_dialog_word::Proof>(new nfs_controls_dialog_word::Proof(std::move(p),e));}};
}
namespace controls_gpu_test {
static double alpha(const nfs_controls_dialog_display::Artwork&a,double x,double y,const NfsControlsDialogWordPlacement&p){if(x<p.left||x>=p.right||y<p.top||y>=p.bottom)return 0;double X=std::max(0.,std::min(double(a.width-1),(x-p.left)*4-.5)),Y=std::max(0.,std::min(127.,(y-p.top)*4-.5));unsigned i=unsigned(X),j=unsigned(Y),k=std::min(i+1,a.width-1),l=std::min(j+1,127u);double fx=X-i,fy=Y-j;auto at=[&](unsigned x,unsigned y){return double(a.rgba[(size_t(y)*a.width+x)*4+3]);};return ((at(i,j)*(1-fx)+at(k,j)*fx)*(1-fy)+(at(i,l)*(1-fx)+at(k,l)*fx)*fy)/255.;}
static void instrumentation(){events.clear();programs.clear();controlsDrawCounts.clear();drawCount=callIndex=0;recording=true;}
static unsigned eventCount(const char*name){return unsigned(std::count(events.begin(),events.end(),std::string(name)));}
static bool ellipsisPixel(const std::vector<NfsSoftwareTextVertex>&vertices,const nfs_soft::FramePacket&p,double X,double Y){for(unsigned index:{2u,3u,6u,7u,10u,11u}){const auto&r=p.records()[index];for(size_t k=r.vertexFirst;k<r.vertexFirst+r.vertexCount;k+=6){float l=vertices[k].x,t=vertices[k].y,right=vertices[k+2].x,bottom=vertices[k+1].y;if(X>=l&&X<right&&Y>=t&&Y<bottom)return true;}}return false;}
static NfsCompactCaptionWordView sidebar(const nfs_compact_caption_display::Artwork&a){NfsCompactCaptionWordView v{};v.schema=1;v.word=3;v.width=192;v.height=128;v.rgba=a.rgba.data();v.bytes=a.rgba.size();std::copy(a.sha.begin(),a.sha.end(),v.sha256);v.left=120;v.top=364;v.right=168;v.bottom=396;return v;}
static double sideAlpha(const nfs_compact_caption_display::Artwork&a,double x,double y){if(x<120||x>=168||y<364||y>=396)return 0;const double X=std::max(0.,std::min(191.,(x-120)*4-.5)),Y=std::max(0.,std::min(127.,(y-364)*4-.5));const unsigned i=unsigned(X),j=unsigned(Y),k=std::min(i+1,191u),l=std::min(j+1,127u);const double fx=X-i,fy=Y-j;auto at=[&](unsigned x,unsigned y){return double(a.rgba[(size_t(y)*192+x)*4+3]);};return ((at(i,j)*(1-fx)+at(k,j)*fx)*(1-fy)+(at(i,l)*(1-fx)+at(k,l)*fx)*fy)/255.;}
} // namespace controls_gpu_test
#define glActiveTexture(...) probe("glActiveTexture",[&](){return ::glActiveTexture(__VA_ARGS__);})
#define glAttachShader(...) probe("glAttachShader",[&](){return ::glAttachShader(__VA_ARGS__);})
#define glBindAttribLocation(...) probe("glBindAttribLocation",[&](){return ::glBindAttribLocation(__VA_ARGS__);})
#define glBindBuffer(...) probe("glBindBuffer",[&](){return ::glBindBuffer(__VA_ARGS__);})
#define glBindFramebuffer(...) probe("glBindFramebuffer",[&](){return ::glBindFramebuffer(__VA_ARGS__);})
#define glBindRenderbuffer(...) probe("glBindRenderbuffer",[&](){return ::glBindRenderbuffer(__VA_ARGS__);})
#define glBindTexture(...) probe("glBindTexture",[&](){return ::glBindTexture(__VA_ARGS__);})
#define glBlendEquationSeparate(...) probe("glBlendEquationSeparate",[&](){return ::glBlendEquationSeparate(__VA_ARGS__);})
#define glBlendFuncSeparate(...) probe("glBlendFuncSeparate",[&](){return ::glBlendFuncSeparate(__VA_ARGS__);})
#define glCheckFramebufferStatus(...) probe("glCheckFramebufferStatus",[&](){return ::glCheckFramebufferStatus(__VA_ARGS__);})
#define glClearDepthf(...) probe("glClearDepthf",[&](){return ::glClearDepthf(__VA_ARGS__);})
#define glColorMask(...) probe("glColorMask",[&](){return ::glColorMask(__VA_ARGS__);})
#define glCompileShader(...) probe("glCompileShader",[&](){return ::glCompileShader(__VA_ARGS__);})
#define glCreateProgram(...) probe("glCreateProgram",[&](){return ::glCreateProgram(__VA_ARGS__);})
#define glCreateShader(...) probe("glCreateShader",[&](){return ::glCreateShader(__VA_ARGS__);})
#define glDeleteFramebuffers(...) probe("glDeleteFramebuffers",[&](){return ::glDeleteFramebuffers(__VA_ARGS__);})
#define glDeleteProgram(...) probe("glDeleteProgram",[&](){return ::glDeleteProgram(__VA_ARGS__);})
#define glDeleteShader(...) probe("glDeleteShader",[&](){return ::glDeleteShader(__VA_ARGS__);})
#define glDeleteTextures(...) probe("glDeleteTextures",[&](){return ::glDeleteTextures(__VA_ARGS__);})
#define glDepthFunc(...) probe("glDepthFunc",[&](){return ::glDepthFunc(__VA_ARGS__);})
#define glDepthMask(...) probe("glDepthMask",[&](){return ::glDepthMask(__VA_ARGS__);})
#define glDisable(...) probe("glDisable",[&](){return ::glDisable(__VA_ARGS__);})
#define glDisableVertexAttribArray(...) probe("glDisableVertexAttribArray",[&](){return ::glDisableVertexAttribArray(__VA_ARGS__);})
#define glDrawArrays(...) probe("glDrawArrays",[&](){return draw(__VA_ARGS__);})
#define glEnable(...) probe("glEnable",[&](){return ::glEnable(__VA_ARGS__);})
#define glEnableVertexAttribArray(...) probe("glEnableVertexAttribArray",[&](){return ::glEnableVertexAttribArray(__VA_ARGS__);})
#define glFramebufferTexture2D(...) probe("glFramebufferTexture2D",[&](){return ::glFramebufferTexture2D(__VA_ARGS__);})
#define glGenFramebuffers(...) probe("glGenFramebuffers",[&](){return ::glGenFramebuffers(__VA_ARGS__);})
#define glGenTextures(...) probe("glGenTextures",[&](){return ::glGenTextures(__VA_ARGS__);})
#define glGetBooleanv(...) probe("glGetBooleanv",[&](){return ::glGetBooleanv(__VA_ARGS__);})
#define glGetError(...) probe("glGetError",[&](){return ::glGetError(__VA_ARGS__);})
#define glGetFloatv(...) probe("glGetFloatv",[&](){return ::glGetFloatv(__VA_ARGS__);})
#define glGetFramebufferAttachmentParameteriv(...) probe("glGetFramebufferAttachmentParameteriv",[&](){return ::glGetFramebufferAttachmentParameteriv(__VA_ARGS__);})
#define glGetIntegerv(...) probe("glGetIntegerv",[&](){return ::glGetIntegerv(__VA_ARGS__);})
#define glGetProgramiv(...) probe("glGetProgramiv",[&](){return ::glGetProgramiv(__VA_ARGS__);})
#define glGetShaderiv(...) probe("glGetShaderiv",[&](){return ::glGetShaderiv(__VA_ARGS__);})
#define glGetUniformLocation(...) probe("glGetUniformLocation",[&](){return ::glGetUniformLocation(__VA_ARGS__);})
#define glGetVertexAttribPointerv(...) probe("glGetVertexAttribPointerv",[&](){return ::glGetVertexAttribPointerv(__VA_ARGS__);})
#define glGetVertexAttribfv(...) probe("glGetVertexAttribfv",[&](){return ::glGetVertexAttribfv(__VA_ARGS__);})
#define glGetVertexAttribiv(...) probe("glGetVertexAttribiv",[&](){return ::glGetVertexAttribiv(__VA_ARGS__);})
#define glIsEnabled(...) probe("glIsEnabled",[&](){return ::glIsEnabled(__VA_ARGS__);})
#define glIsFramebuffer(...) probe("glIsFramebuffer",[&](){return ::glIsFramebuffer(__VA_ARGS__);})
#define glIsProgram(...) probe("glIsProgram",[&](){return ::glIsProgram(__VA_ARGS__);})
#define glIsTexture(...) probe("glIsTexture",[&](){return ::glIsTexture(__VA_ARGS__);})
#define glLinkProgram(...) probe("glLinkProgram",[&](){return ::glLinkProgram(__VA_ARGS__);})
#define glPixelStorei(...) probe("glPixelStorei",[&](){return ::glPixelStorei(__VA_ARGS__);})
#define glScissor(...) probe("glScissor",[&](){return ::glScissor(__VA_ARGS__);})
#define glShaderSource(...) probe("glShaderSource",[&](){return ::glShaderSource(__VA_ARGS__);})
#define glTexImage2D(...) probe("glTexImage2D",[&](){return ::glTexImage2D(__VA_ARGS__);})
#define glTexParameteri(...) probe("glTexParameteri",[&](){return ::glTexParameteri(__VA_ARGS__);})
#define glUniform1f(...) probe("glUniform1f",[&](){return ::glUniform1f(__VA_ARGS__);})
#define glUniform1i(...) probe("glUniform1i",[&](){return ::glUniform1i(__VA_ARGS__);})
#define glUniformMatrix4fv(...) probe("glUniformMatrix4fv",[&](){return ::glUniformMatrix4fv(__VA_ARGS__);})
#define glUseProgram(...) probe("glUseProgram",[&](){return ::glUseProgram(__VA_ARGS__);})
#define glVertexAttrib4fv(...) probe("glVertexAttrib4fv",[&](){return ::glVertexAttrib4fv(__VA_ARGS__);})
#define glVertexAttribPointer(...) probe("glVertexAttribPointer",[&](){return ::glVertexAttribPointer(__VA_ARGS__);})
#define glViewport(...) probe("glViewport",[&](){return ::glViewport(__VA_ARGS__);})
#include "hd/FeedbackModalSceneLayers.c.inc"
#undef glActiveTexture
#undef glAttachShader
#undef glBindAttribLocation
#undef glBindBuffer
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glBindTexture
#undef glBlendEquationSeparate
#undef glBlendFuncSeparate
#undef glCheckFramebufferStatus
#undef glClearDepthf
#undef glColorMask
#undef glCompileShader
#undef glCreateProgram
#undef glCreateShader
#undef glDeleteFramebuffers
#undef glDeleteProgram
#undef glDeleteShader
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDisable
#undef glDisableVertexAttribArray
#undef glDrawArrays
#undef glEnable
#undef glEnableVertexAttribArray
#undef glFramebufferTexture2D
#undef glGenFramebuffers
#undef glGenTextures
#undef glGetBooleanv
#undef glGetError
#undef glGetFloatv
#undef glGetFramebufferAttachmentParameteriv
#undef glGetIntegerv
#undef glGetProgramiv
#undef glGetShaderiv
#undef glGetUniformLocation
#undef glGetVertexAttribPointerv
#undef glGetVertexAttribfv
#undef glGetVertexAttribiv
#undef glIsEnabled
#undef glIsFramebuffer
#undef glIsProgram
#undef glIsTexture
#undef glLinkProgram
#undef glPixelStorei
#undef glScissor
#undef glShaderSource
#undef glTexImage2D
#undef glTexParameteri
#undef glUniform1f
#undef glUniform1i
#undef glUniformMatrix4fv
#undef glUseProgram
#undef glVertexAttrib4fv
#undef glVertexAttribPointer
#undef glViewport
#include "hd/FeedbackModalWordArtwork.h"
namespace feedback_scene_test {
struct SyntheticArt {std::vector<unsigned char>banner,emblem;NfsFeedbackModalBackdropArtView view{};
 SyntheticArt():banner(size_t(1376)*188*4),emblem(size_t(468)*244*4){view.schema=1;view.count=2;for(unsigned i=0;i<2;++i){auto&pixels=i?emblem:banner;for(size_t at=0;at<pixels.size();at+=4){pixels[at]=i?20:180;pixels[at+1]=i?160:90;pixels[at+2]=i?230:20;pixels[at+3]=i?96:128;}auto&a=view.images[i];a.schema=1;a.role=i+1;a.width=i?468:1376;a.height=i?244:188;a.straightRgba=1;a.rgba=pixels.data();a.bytes=pixels.size();ck(nfs_texture_sha256(a.rgba,a.bytes,a.sha256),"synthetic popup artwork digest");}}
};
static NfsFeedbackModalWordsView words(){NfsFeedbackModalWordsView v{};v.schema=1;v.count=2;for(unsigned i=0;i<2;++i){auto art=nfs_feedback_modal_word::artwork(i+1);ck(bool(art),"finite authored modal words");auto&a=v.images[i];a.word=i+1;a.width=art->width();a.height=art->height();a.rgba=art->rgba().data();a.bytes=art->rgba().size();std::copy(art->sha().begin(),art->sha().end(),a.sha256);}v.placements[0]={1,1,0,78,302,185,398,217};v.placements[1]={2,65535,78,84,266,225,374,257};return v;}
static double alpha(unsigned index,double x,double y,const NfsFeedbackModalWordPlacement&p){if(x<p.left||x>=p.right||y<p.top||y>=p.bottom)return 0;const auto a=nfs_feedback_modal_word::artwork(index+1);double X=std::max(0.,std::min(double(a->width()-1),(x-p.left)*4-.5)),Y=std::max(0.,std::min(127.,(y-p.top)*4-.5));unsigned i=unsigned(X),j=unsigned(Y),k=std::min(i+1,a->width()-1),l=std::min(j+1,127u);double fx=X-i,fy=Y-j;auto at=[&](unsigned x,unsigned y){return double(a->rgba()[(size_t(y)*a->width()+x)*4+3]);};return ((at(i,j)*(1-fx)+at(k,j)*fx)*(1-fy)+(at(i,l)*(1-fx)+at(k,l)*fx)*fy)/255.;}
static void run(Pbuffer&egl,Resources&r,Target&t,Target&visible,Registered&owner){
 // Parent proof/vertices are explicitly synthetic test data, never actual game
 // source authority. Popup text is independently synthetic as well.
 auto evidence=controls_synthetic::evidence(0);auto packet=controls_synthetic::packet(controls_synthetic::commands(0));auto proof=nfs_controls_text::TailScope::synthetic(packet,evidence);auto display=nfs_controls_dialog_display::Prepared::create(proof,packet,{evidence.form,evidence.authority.archive,evidence.game,evidence.serial,evidence.lifetime});auto side=nfs_compact_caption_display::artwork(3);ck(display&&side,"synthetic parent with finite Controls artwork");
 const auto&stream=packet->vertices();auto cap=text(35,30,70,50);NfsFeedbackModalSceneLayers v{};v.schema=1;v.parent.background=layers();v.parent.sidebarCaption={343,480,r.fontView,cap.data(),cap.size()};v.parent.text={640,480,r.fontView,stream.data(),stream.size()};v.parent.sidebarWord=controls_gpu_test::sidebar(*side);v.parent.dialogWords=display->view();v.words=words();SyntheticArt art;v.backdrop=art.view;
 std::vector<NfsSoftwareTextVertex>popup;auto q=text(302,185,398,217);for(unsigned n=0;n<27;++n)popup.insert(popup.end(),q.begin(),q.end());v.text={640,480,r.fontView,popup.data(),popup.size()};
 NfsNestedCaptionGpu sideGpu{};NfsControlsDialogWordGpu dialogGpu{};auto parent=r.view();parent.sidebarWord=&sideGpu;parent.dialogWords=&dialogGpu;NfsFeedbackModalBackdropGpu backdrop{};NfsFeedbackModalWordGpu wordGpu{};NfsFeedbackModalSceneGpu gpu{&parent,&backdrop,&wordGpu};
 auto stageView=stage(t,owner),guard=stageView;guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;GLuint externalTextures[]={visible.texture,t.texture};NfsFeedbackModalSceneAliases aliases{nullptr,0,externalTextures,2};NfsFeedbackModalBackdropGpuAliases bdAliases{nullptr,0,externalTextures,2};NfsFeedbackModalWordGpuAliases wordAliases{nullptr,0,externalTextures,2};
 ck(nfs_nested_caption_prepare(&sideGpu,&v.parent.sidebarWord,3,&guard)&&nfs_controls_dialog_prepare(&dialogGpu,&v.parent.dialogWords,&v.parent.text,&guard)&&nfs_feedback_modal_backdrop_gpu_prepare(&backdrop,&v.backdrop,&guard,&bdAliases)&&nfs_feedback_modal_word_prepare(&wordGpu,&v.words,&v.text,&guard,&wordAliases),"all prepared parent and popup resources");
 const auto sideSaved=sideGpu;const auto dialogSaved=dialogGpu;const auto bdSaved=backdrop;const auto wordSaved=wordGpu;
 visible.seed();const auto original=visible.read();t.seed();stageView=stage(t,owner);ck(nfs_controls_scene_draw_layers_into_staging(&parent,&stageView,&v.parent),"independent complete HD parent reference");auto reference=t.read();
 t.seed();stageView=stage(t,owner);auto state=capture();drawCount=0;ck(nfs_feedback_modal_scene_preflight(&gpu,&stageView,&v,&aliases)&&!drawCount&&equal(state,capture()),"whole scene preflight draws nothing");controls_gpu_test::instrumentation();ck(nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stageView,&v,&aliases)&&equal(state,capture()),"whole popup scene restores inherited state");recording=false;ck(drawCount==24&&programs.size()==24,"nineteen parent then three backdrop then two word draws");for(unsigned i=19;i<22;++i)ck(programs[i]==backdrop.program,"body banner emblem after full HD parent");for(unsigned i=22;i<24;++i)ck(programs[i]==wordGpu.program,"two popup words drawn last");
 auto image=t.read();for(unsigned y=0;y<t.h;++y)for(unsigned x=0;x<t.w;++x){double X=x+.5,Y=y+.5;size_t at=(size_t(t.h-1-y)*t.w+x)*4;double rgb[3]={double(reference[at]),double(reference[at+1]),double(reference[at+2])};if(X>=227&&X<413&&Y>=210&&Y<270)for(unsigned k=0;k<3;++k)rgb[k]=std::round(5+rgb[k]*76./256.);if(X>=247&&X<442&&Y>=179&&Y<226){const double pma[3]={90,45,10};for(unsigned k=0;k<3;++k)rgb[k]=std::round(pma[k]+rgb[k]*127./255.);}if(X>=175&&X<292&&Y>=171&&Y<232){const double pma[3]={7,60,86};for(unsigned k=0;k<3;++k)rgb[k]=std::round(pma[k]+rgb[k]*159./255.);}for(unsigned i=0;i<2;++i){double A=alpha(i,X,Y,v.words.placements[i]);const double color[3]={i?255.:0.,i?255.:0.,i?255.:8.};for(unsigned k=0;k<3;++k)rgb[k]=std::round(color[k]*A+rgb[k]*(1-A));}for(unsigned k=0;k<3;++k)ck(std::abs(int(image[at+k])-int(rgb[k]))<=4,"whole parent body clipped banner emblem words independent pixel oracle");}ck(visible.read()==original,"complete popup never publishes or writes original");
 for(unsigned fault=0;fault<12;++fault){t.seed();stageView=stage(t,owner);auto clean=t.read();state=capture();auto bad=v;auto badStage=stageView;auto badGpu=gpu;auto badAliases=aliases;
  if(fault==0)bad.schema=0;if(fault==1)bad.parent.dialogWords={};if(fault==2)bad.words={};if(fault==3)bad.backdrop={};if(fault==4)bad.words.placements[1].vertexFirst=0;if(fault==5)bad.text.font.identity++;if(fault==6)badGpu.words=nullptr;if(fault==7)badStage.framebuffer=visible.fbo;if(fault==8)badStage.sampling.viewport[0]=1;if(fault==9)badAliases.textureCount=65;if(fault==10)bad.backdrop.images[0].sha256[0]^=1;if(fault==11)bad.parent.text.logicalWidth=343;
  drawCount=0;ck(!nfs_feedback_modal_scene_draw_layers_into_staging(&badGpu,&badStage,&bad,&badAliases)&&!drawCount&&equal(state,capture())&&t.read()==clean,"invalid whole popup rejects before parent writes");
 }
 for(unsigned fault=0;fault<7;++fault){t.seed();stageView=stage(t,owner);state=capture();auto originalColor=r.bg.color[0];if(fault==0)wordGpu.program=r.font.program;if(fault==1)backdrop.textures[0]=r.bg.textures[0];if(fault==2)dialogGpu.textures[0]=wordGpu.textures[0];if(fault==3)backdrop.textures[1]=wordGpu.textures[1];if(fault==4)r.bg.color[0]=wordGpu.textures[0];if(fault==5)wordGpu.program=r.bg.copyProgram;if(fault==6)wordGpu.textures[0]=visible.texture;drawCount=0;ck(!nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stageView,&v,&aliases)&&!drawCount&&equal(state,capture()),"cross parent popup inactive-cache and original-resource aliases reject before all draws");r.bg.color[0]=originalColor;wordGpu=wordSaved;backdrop=bdSaved;dialogGpu=dialogSaved;}
 for(unsigned failure:{1u,19u,20u,21u,22u,23u,24u}){t.seed();stageView=stage(t,owner);state=capture();drawCount=0;failAt=failure;ck(!nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stageView,&v,&aliases)&&equal(state,capture()),"parent or popup late error restores owner and refuses whole stage");failAt=0;ck(visible.read()==original,"late popup failure never publishes");}
 t.seed();stageView=stage(t,owner);controls_gpu_test::instrumentation();ck(nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stageView,&v,&aliases),"record full popup guard sequence");recording=false;std::vector<unsigned>drawPoints;for(unsigned i=0;i<events.size();++i)if(events[i]=="glDrawArrays")drawPoints.push_back(i+1);ck(drawPoints.size()==24,"twenty-four owner-loss draw boundaries");const auto bgSaved=r.bg,unusedBg=r.bg;(void)unusedBg;const auto captionSaved=r.caption,fontSaved=r.font;
 for(unsigned index:{0u,18u,19u,21u,22u,23u}){EGLint attr[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};EGLContext foreign=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attr);ck(foreign!=EGL_NO_CONTEXT&&eglMakeCurrent(egl.display,egl.surface,egl.surface,foreign),"actual foreign context");glViewport(3,4,71,83);glActiveTexture(GL_TEXTURE1);auto foreignState=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"return popup owner");r.bg=bgSaved;r.caption=captionSaved;r.font=fontSaved;sideGpu=sideSaved;dialogGpu=dialogSaved;backdrop=bdSaved;wordGpu=wordSaved;t.seed();stageView=stage(t,owner);lossDisplay=egl.display;lossSurface=egl.surface;lossContext=foreign;foreignTouches=callIndex=drawCount=0;loseIndex=drawPoints[index];recording=trackForeign=true;ck(!nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stageView,&v,&aliases),"whole stage stops after real owner loss");recording=trackForeign=false;loseIndex=0;ck(foreignTouches==0&&equal(foreignState,capture())&&!backdrop.generation&&!wordGpu.generation,"popup metadata forgotten without foreign GL");if(index>=19)ck(r.bg.generation==bgSaved.generation&&r.caption.generation==captionSaved.generation&&r.font.generation==fontSaved.generation,"popup-phase loss leaves borrowed parent metadata to owner");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"restore fixture owner");ck(eglDestroyContext(egl.display,foreign),"destroy foreign");lossContext=EGL_NO_CONTEXT;ck(visible.read()==original,"foreign loss retains original target");}
 // Fixture-only snapshots restore still-live GL names for deterministic cleanup.
 r.bg=bgSaved;r.caption=captionSaved;r.font=fontSaved;sideGpu=sideSaved;dialogGpu=dialogSaved;backdrop=bdSaved;wordGpu=wordSaved;guard=stage(t,owner);guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;glUseProgram(0);ck(nfs_feedback_modal_word_retire(&wordGpu,&guard,1,&wordAliases)&&nfs_feedback_modal_backdrop_gpu_retire(&backdrop,&guard,1,&bdAliases)&&nfs_controls_dialog_retire(&dialogGpu,&guard,1)&&nfs_nested_caption_retire(&sideGpu,&guard,1),"retire own popup and fixture finite parent resources");
}
}
int main(){try{Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic neutral art provider");{Resources r(egl.context,1);Target target(640,480),original(640,480);Registered owner{egl.context};feedback_scene_test::run(egl,r,target,original,owner);glUseProgram(0);r.retire();}ck(glGetError()==GL_NO_ERROR,"final popup scene GLES error");std::puts("PASS whole feedback stage; synthetic source/art; no native authority; no publication");return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL whole feedback stage: %s\n",e.what());return 1;}}
