// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include <thread>
#include <type_traits>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cstdio>
static unsigned checks=0;static void ck(bool b,const char*w){nfs_hd_test::require(b,w);++checks;}
static bool recording=false,trackForeign=false,badCompile=false;static unsigned callIndex=0,loseIndex=0,foreignTouches=0;static std::vector<std::string>events;static EGLDisplay lossDisplay;static EGLSurface lossSurface;static EGLContext lossContext=EGL_NO_CONTEXT;
static void afterCall(const char*name){if(!recording)return;events.push_back(name);if(++callIndex==loseIndex)ck(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected actual context loss");}
template<class F>static auto probe(const char*name,F f)->decltype(f()) {if(trackForeign&&eglGetCurrentContext()==lossContext)++foreignTouches;if constexpr(std::is_void_v<decltype(f())>){f();afterCall(name);}else{auto v=f();afterCall(name);return v;}}
static unsigned failUpload=0,uploadAttempt=0;
static void uploadCold(GLenum t,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void*data){if(failUpload&&++uploadAttempt==failUpload)w=-1;glTexImage2D(t,level,internal,w,h,border,format,type,data);}
static void compileCold(GLuint s){if(badCompile){const char*invalid="not shader code";glShaderSource(s,1,&invalid,nullptr);}glCompileShader(s);}
#define glActiveTexture(...) probe("glActiveTexture",[&](){return ::glActiveTexture(__VA_ARGS__);})
#define glAttachShader(...) probe("glAttachShader",[&](){return ::glAttachShader(__VA_ARGS__);})
#define glBindAttribLocation(...) probe("glBindAttribLocation",[&](){return ::glBindAttribLocation(__VA_ARGS__);})
#define glBindBuffer(...) probe("glBindBuffer",[&](){return ::glBindBuffer(__VA_ARGS__);})
#define glBindFramebuffer(...) probe("glBindFramebuffer",[&](){return ::glBindFramebuffer(__VA_ARGS__);})
#define glBindRenderbuffer(...) probe("glBindRenderbuffer",[&](){return ::glBindRenderbuffer(__VA_ARGS__);})
#define glBindTexture(...) probe("glBindTexture",[&](){return ::glBindTexture(__VA_ARGS__);})
#define glBlendEquationSeparate(...) probe("glBlendEquationSeparate",[&](){return ::glBlendEquationSeparate(__VA_ARGS__);})
#define glBlendFuncSeparate(...) probe("glBlendFuncSeparate",[&](){return ::glBlendFuncSeparate(__VA_ARGS__);})
#define glClearDepthf(...) probe("glClearDepthf",[&](){return ::glClearDepthf(__VA_ARGS__);})
#define glColorMask(...) probe("glColorMask",[&](){return ::glColorMask(__VA_ARGS__);})
#define glCompileShader(...) probe("glCompileShader",[&](){return compileCold(__VA_ARGS__);})
#define glCreateProgram(...) probe("glCreateProgram",[&](){return ::glCreateProgram(__VA_ARGS__);})
#define glCreateShader(...) probe("glCreateShader",[&](){return ::glCreateShader(__VA_ARGS__);})
#define glDeleteProgram(...) probe("glDeleteProgram",[&](){return ::glDeleteProgram(__VA_ARGS__);})
#define glDeleteShader(...) probe("glDeleteShader",[&](){return ::glDeleteShader(__VA_ARGS__);})
#define glDeleteTextures(...) probe("glDeleteTextures",[&](){return ::glDeleteTextures(__VA_ARGS__);})
#define glDepthFunc(...) probe("glDepthFunc",[&](){return ::glDepthFunc(__VA_ARGS__);})
#define glDepthMask(...) probe("glDepthMask",[&](){return ::glDepthMask(__VA_ARGS__);})
#define glDisable(...) probe("glDisable",[&](){return ::glDisable(__VA_ARGS__);})
#define glDisableVertexAttribArray(...) probe("glDisableVertexAttribArray",[&](){return ::glDisableVertexAttribArray(__VA_ARGS__);})
#define glEnable(...) probe("glEnable",[&](){return ::glEnable(__VA_ARGS__);})
#define glEnableVertexAttribArray(...) probe("glEnableVertexAttribArray",[&](){return ::glEnableVertexAttribArray(__VA_ARGS__);})
#define glGenTextures(...) probe("glGenTextures",[&](){return ::glGenTextures(__VA_ARGS__);})
#define glGetBooleanv(...) probe("glGetBooleanv",[&](){return ::glGetBooleanv(__VA_ARGS__);})
#define glGetError(...) probe("glGetError",[&](){return ::glGetError(__VA_ARGS__);})
#define glGetFloatv(...) probe("glGetFloatv",[&](){return ::glGetFloatv(__VA_ARGS__);})
#define glGetIntegerv(...) probe("glGetIntegerv",[&](){return ::glGetIntegerv(__VA_ARGS__);})
#define glGetProgramiv(...) probe("glGetProgramiv",[&](){return ::glGetProgramiv(__VA_ARGS__);})
#define glGetShaderiv(...) probe("glGetShaderiv",[&](){return ::glGetShaderiv(__VA_ARGS__);})
#define glGetUniformLocation(...) probe("glGetUniformLocation",[&](){return ::glGetUniformLocation(__VA_ARGS__);})
#define glGetVertexAttribPointerv(...) probe("glGetVertexAttribPointerv",[&](){return ::glGetVertexAttribPointerv(__VA_ARGS__);})
#define glGetVertexAttribfv(...) probe("glGetVertexAttribfv",[&](){return ::glGetVertexAttribfv(__VA_ARGS__);})
#define glGetVertexAttribiv(...) probe("glGetVertexAttribiv",[&](){return ::glGetVertexAttribiv(__VA_ARGS__);})
#define glIsEnabled(...) probe("glIsEnabled",[&](){return ::glIsEnabled(__VA_ARGS__);})
#define glIsProgram(...) probe("glIsProgram",[&](){return ::glIsProgram(__VA_ARGS__);})
#define glIsTexture(...) probe("glIsTexture",[&](){return ::glIsTexture(__VA_ARGS__);})
#define glLinkProgram(...) probe("glLinkProgram",[&](){return ::glLinkProgram(__VA_ARGS__);})
#define glPixelStorei(...) probe("glPixelStorei",[&](){return ::glPixelStorei(__VA_ARGS__);})
#define glScissor(...) probe("glScissor",[&](){return ::glScissor(__VA_ARGS__);})
#define glShaderSource(...) probe("glShaderSource",[&](){return ::glShaderSource(__VA_ARGS__);})
#define glTexImage2D(...) probe("glTexImage2D",[&](){return uploadCold(__VA_ARGS__);})
#define glTexParameteri(...) probe("glTexParameteri",[&](){return ::glTexParameteri(__VA_ARGS__);})
#define glUseProgram(...) probe("glUseProgram",[&](){return ::glUseProgram(__VA_ARGS__);})
#define glVertexAttrib4fv(...) probe("glVertexAttrib4fv",[&](){return ::glVertexAttrib4fv(__VA_ARGS__);})
#define glVertexAttribPointer(...) probe("glVertexAttribPointer",[&](){return ::glVertexAttribPointer(__VA_ARGS__);})
#define glViewport(...) probe("glViewport",[&](){return ::glViewport(__VA_ARGS__);})

#include "hd/BackgroundSceneResourcePrepare.c.inc"
#undef glActiveTexture
#undef glAttachShader
#undef glBindAttribLocation
#undef glBindBuffer
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glBindTexture
#undef glBlendEquationSeparate
#undef glBlendFuncSeparate
#undef glClearDepthf
#undef glColorMask
#undef glCompileShader
#undef glCreateProgram
#undef glCreateShader
#undef glDeleteProgram
#undef glDeleteShader
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDisable
#undef glDisableVertexAttribArray
#undef glEnable
#undef glEnableVertexAttribArray
#undef glGenTextures
#undef glGetBooleanv
#undef glGetError
#undef glGetFloatv
#undef glGetIntegerv
#undef glGetProgramiv
#undef glGetShaderiv
#undef glGetUniformLocation
#undef glGetVertexAttribPointerv
#undef glGetVertexAttribfv
#undef glGetVertexAttribiv
#undef glIsEnabled
#undef glIsProgram
#undef glIsTexture
#undef glLinkProgram
#undef glPixelStorei
#undef glScissor
#undef glShaderSource
#undef glTexImage2D
#undef glTexParameteri
#undef glUseProgram
#undef glVertexAttrib4fv
#undef glVertexAttribPointer
#undef glViewport

#include "AudioSceneLayersFixture.h"
using namespace nfs_hd_test;
struct Target{GLuint fbo=0,color=0,depth=0;unsigned w,h;Target(unsigned W=640,unsigned H=480):w(W),h(H){glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"owned original target");glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glDepthMask(1);glClearColor(0,0,1,1);glClearDepthf(.25);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);}std::vector<unsigned char>read(){std::vector<unsigned char>v(size_t(w)*h*4);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,v.data());return v;}~Target(){glDeleteTextures(1,&color);glDeleteFramebuffers(1,&fbo);glDeleteRenderbuffers(1,&depth);}};
struct Owner{EGLContext context;GLuint fbo;uint64_t generation;std::thread::id thread=std::this_thread::get_id();};
static int current(void*p,const NfsBackgroundResourceOwner*o){auto&s=*static_cast<Owner*>(p);if(eglGetCurrentContext()!=s.context||s.thread!=std::this_thread::get_id()||o->generation!=s.generation||o->contextIdentity!=reinterpret_cast<uintptr_t>(s.context))return 0;GLint f=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&f);return GLuint(f)==s.fbo;}
static NfsBackgroundResourceOwner guard(Owner&o){return {o.generation,reinterpret_cast<uintptr_t>(o.context),current,&o};}
static void resetTrace(){recording=true;callIndex=0;events.clear();}
static void depthUnchanged(Target&t){const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,1,0,1);}";GLuint v=compileShader(GL_VERTEX_SHADER,vs),f=compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glUseProgram(p);glBindBuffer(GL_ARRAY_BUFFER,0);glViewport(0,0,t.w,t.h);glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glColorMask(1,1,1,1);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(0);const GLfloat q[]={-1,-1,1,-1,-1,1,1,1};glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,0,0,q);glDrawArrays(GL_TRIANGLE_STRIP,0,4);auto pixels=t.read();ck(pixels[0]==0&&pixels[1]==0&&pixels[2]==255,"original .25 depth rejects .5 probe after cold preparation");glDeleteProgram(p);glDeleteShader(v);glDeleteShader(f);}
int main(){try{Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"immutable synthetic startup provider");std::vector<std::string>fullEvents;unsigned restorePoint=0;
 for(unsigned h:{480u,1640u}){Target target(h*4/3,h);Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1),"fresh generation without Main");auto before=target.read();glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_LEQUAL);glEnable(GL_SCISSOR_TEST);glScissor(0,0,target.w,target.h);glColorMask(0,1,0,1);glActiveTexture(GL_TEXTURE1);auto state=capture();resetTrace();ck(nfs_bg_scene_prepare_owned(&bg,&o),"cold resources without Main");recording=false;ck(equal(state,capture())&&before==target.read(),"cold prepare preserves original target color/state");ck(bg.ownerGeneration==1&&bg.logo.ownerGeneration==1&&!bg.pendingSerial&&!bg.lastSerial&&!bg.imageReady[0]&&!bg.imageReady[1],"resources only no cache pass or image authority");auto programs=std::array<GLuint,3>{bg.program,bg.copyProgram,bg.logo.program};resetTrace();ck(nfs_bg_scene_prepare_owned(&bg,&o),"ready exact resource reuse");recording=false;ck(programs==std::array<GLuint,3>{bg.program,bg.copyProgram,bg.logo.program},"ready reuse allocates nothing");if(h==480){fullEvents.clear();nfs_bg_scene_context_retired(&bg,1,1);ck(nfs_bg_scene_context_created(&bg,2),"retire recreate");owner.generation=2;o=guard(owner);resetTrace();ck(nfs_bg_scene_prepare_owned(&bg,&o),"recreated resources without Main");recording=false;fullEvents=events;for(unsigned i=0;i<fullEvents.size();++i)if(fullEvents[i]=="glDepthFunc")restorePoint=i+1;ck(restorePoint>0,"restore call injection identified");}depthUnchanged(target);nfs_bg_scene_context_retired(&bg,bg.generation,1);}
 // All content/region negatives reject before any resource shader or upload.
 for(unsigned fault=0;fault<8;++fault){Target target;Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1),"negative generation");auto saved=assets.tex[0];auto logo=assets.logo;auto words=assets.words;unsigned char byte=assets.bytes[2][0];if(fault==0)assets.tex[0].sha256[0]^=1;if(fault==1)assets.tex[0].width++;if(fault==2)assets.tex[0].premultiplied=0;if(fault==3)assets.badRegion=true;if(fault==4)assets.logo.sha256[0]^=1;if(fault==5){assets.bytes[2][0]=255;nfs_texture_sha256(assets.bytes[2].data(),assets.bytes[2].size(),assets.tex[2].sha256);}if(fault==6)o.current=nullptr;if(fault==7)o.generation++;resetTrace();ck(!nfs_bg_scene_prepare_owned(&bg,&o),"invalid owner/art/region rejected");recording=false;ck(std::find(events.begin(),events.end(),"glCreateShader")==events.end()&&std::find(events.begin(),events.end(),"glTexImage2D")==events.end(),"invalid inputs do not create GPU resources");assets.tex[0]=saved;assets.logo=logo;assets.words=words;assets.badRegion=false;assets.bytes[2][0]=byte;nfs_texture_sha256(assets.bytes[2].data(),assets.bytes[2].size(),assets.tex[2].sha256);nfs_bg_scene_context_retired(&bg,1,1);}
 // Same-owner actual shader compilation failure cleans temporary names/state.
 {Target target;Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1),"compile failure generation");auto before=target.read();auto state=capture();badCompile=true;ck(!nfs_bg_scene_prepare_owned(&bg,&o),"same owner real compile failure");badCompile=false;ck(equal(state,capture())&&before==target.read()&&!bg.ownerGeneration&&!bg.program&&!bg.logo.program,"failed compilation does not publish resources or change target");ck(nfs_bg_scene_prepare_owned(&bg,&o),"same generation clean retry");nfs_bg_scene_context_retired(&bg,1,1);}
 {Target target;Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1),"upload failure generation");auto before=target.read();auto state=capture();failUpload=3;uploadAttempt=0;ck(!nfs_bg_scene_prepare_owned(&bg,&o),"same owner real upload failure");failUpload=0;ck(equal(state,capture())&&before==target.read()&&!bg.ownerGeneration&&!bg.program&&!bg.logo.program,"upload failure restores owner and publishes nothing");ck(nfs_bg_scene_prepare_owned(&bg,&o),"upload failure clean retry");nfs_bg_scene_context_retired(&bg,1,1);}
 // Each shader/source/compile/link/uniform/upload event plus early state-save,
 // texture-parameter and state-restore failures switch the actual EGL context.
 std::set<unsigned>points;std::set<std::string>once;for(unsigned i=0;i<fullEvents.size();++i){const auto&n=fullEvents[i];if(n=="glCreateShader"||n=="glShaderSource"||n=="glCompileShader"||n=="glGetShaderiv"||n=="glCreateProgram"||n=="glAttachShader"||n=="glBindAttribLocation"||n=="glLinkProgram"||n=="glGetProgramiv"||n=="glGetUniformLocation"||n=="glTexImage2D")points.insert(i+1);else if(once.insert(n).second)points.insert(i+1);}points.insert(restorePoint);unsigned losses=0;
 for(unsigned point:points){if(losses){egl.unbind();egl.destroyUnboundContext();egl.createContext();}Target target;Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1),"loss generation");EGLint attrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attrs);ck(lossContext!=EGL_NO_CONTEXT&&eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"foreign context");glViewport(3,4,79,93);glActiveTexture(GL_TEXTURE1);auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owner before injection");glBindFramebuffer(GL_FRAMEBUFFER,target.fbo);loseIndex=point;foreignTouches=0;trackForeign=true;resetTrace();ck(!nfs_bg_scene_prepare_owned(&bg,&o),"injected loss refuses resources");recording=false;trackForeign=false;loseIndex=0;ck(foreignTouches==0&&equal(foreign,capture())&&!bg.generation&&!bg.ownerGeneration&&!bg.program&&!bg.logo.program,"zero foreign GL cleanup/restore and atomic resource discard");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"restore owner after loss");ck(eglDestroyContext(egl.display,lossContext),"destroy foreign");lossContext=EGL_NO_CONTEXT;++losses;/* Target dies under its actual owner; next iteration tears down leaked temporary GPU objects. */}
 // Already-ready glIsProgram probing must also stop before foreign GL.
 {Target target;Owner owner{egl.context,target.fbo,1};auto o=guard(owner);NfsBackgroundSceneCompositor bg{};ck(nfs_bg_scene_context_created(&bg,1)&&nfs_bg_scene_prepare_owned(&bg,&o),"ready-loss prepared resources");EGLint attrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attrs);ck(lossContext!=EGL_NO_CONTEXT&&eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"ready foreign context");glViewport(3,4,79,93);auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"ready owner return");glBindFramebuffer(GL_FRAMEBUFFER,target.fbo);loseIndex=1;foreignTouches=0;trackForeign=true;resetTrace();ck(!nfs_bg_scene_prepare_owned(&bg,&o),"ready glIsProgram loss refuses");recording=false;trackForeign=false;loseIndex=0;ck(events.size()==1&&events.front()=="glIsProgram"&&!foreignTouches&&equal(foreign,capture())&&!bg.generation,"ready probe performs no foreign continuation");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"ready owner cleanup");ck(eglDestroyContext(egl.display,lossContext),"ready foreign cleanup");lossContext=EGL_NO_CONTEXT;++losses;}
 ck(glGetError()==GL_NO_ERROR,"final errors");std::printf("PASS Audio cold BG checks=%u ownerLossPoints=%u freshAudioOnly=1 originalColorDepthUntouched=1\n",checks,losses);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
