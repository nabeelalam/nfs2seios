// SPDX-License-Identifier: MIT
/* Controls-owned actual renderer adapter, maintained finite artwork and real GLES.
   Source proof, neutral background assets and native callbacks are SYNTHETIC.
   drawTriangles is a no-GL counter stub: this fixture does not claim original
   renderer execution, platform SDL dispatch, or physical iPad coverage. */
#include "HdSceneGlesFixture.h"
#include "hd/NativeSceneProvenance.h"
#include <array>
#include <thread>
#include <functional>
#include <type_traits>
#include <string>
#include <set>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>
static unsigned checks=0;static void ck(bool b,const char*w){nfs_hd_test::require(b,w);++checks;}
static bool recording=false,trackForeign=false,badCompile=false;static unsigned callIndex=0,loseIndex=0,foreignTouches=0,drawCalls=0,failDraw=0,failUpload=0,uploadIndex=0;
static EGLDisplay lossDisplay=EGL_NO_DISPLAY;static EGLSurface lossSurface=EGL_NO_SURFACE;static EGLContext lossContext=EGL_NO_CONTEXT;static std::vector<std::string>events;
static void afterCall(const char*name){if(!recording)return;events.emplace_back(name);if(++callIndex==loseIndex)ck(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected real foreign EGL context");}
template<class F>static auto probe(const char*name,F f)->decltype(f()){if(trackForeign&&eglGetCurrentContext()==lossContext)++foreignTouches;if constexpr(std::is_void_v<decltype(f())>){f();afterCall(name);}else{auto value=f();afterCall(name);return value;}}
static void compileInjected(GLuint id){if(badCompile){const char*bad="not GLSL";::glShaderSource(id,1,&bad,nullptr);}::glCompileShader(id);}
static void uploadInjected(GLenum target,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void*data){if(failUpload&&++uploadIndex==failUpload)w=-1;::glTexImage2D(target,level,internal,w,h,border,format,type,data);}
static std::vector<GLuint>programs;static std::vector<GLsizei>drawCounts;
static void drawInjected(GLenum mode,GLint first,GLsizei count){GLint p=0;::glGetIntegerv(GL_CURRENT_PROGRAM,&p);programs.push_back(GLuint(p));drawCounts.push_back(count);++drawCalls;::glDrawArrays(drawCalls==failDraw?GLenum(0xffffffff):mode,first,count);}
static const unsigned TextureMem=0x200000u;
struct TextureInfo{GLuint id;unsigned fmt,size,hdDigits;void*data;};static TextureInfo g_textures[16]{};static unsigned char textureBytes[TextureMem];static unsigned char*g_textureMem=textureBytes;
struct GrTexInfo{unsigned smallLod,largeLod,aspectRatio,format;};
struct GrVertex{float x,y,r,g,b,a,oow;struct{float sow,tow;}tmuvtx[2];};
static EGLContext g_glCtx=EGL_NO_CONTEXT;static GLuint g_framebuffer=0,g_framebufferTexture=0,g_framebufferDepth=0,g_shaderProgram=0;
static int g_framebufferHeight=480;static const int GR_BUFFER_BACKBUFFER=1;static int g_renderBuffer=GR_BUFFER_BACKBUFFER;static bool g_textureEnabled=true,g_fogEnabled=false,g_depthMask=true,linearFiltering=true;static GLfloat g_matrix[16];
static NfsLogoProvenance nfs_logo_provenance{};static unsigned originalFlushCalls=0;
static void drawTriangles(){++originalFlushCalls;}
static EGLContext SDL_GL_GetCurrentContext(){return eglGetCurrentContext();}
static uintptr_t testThread(){return uintptr_t(std::hash<std::thread::id>{}(std::this_thread::get_id()))|uintptr_t(1);}
#define NFS_CONTROLS_SCENE_THREAD_TOKEN() testThread()
static int nfs_bg_feature(){return 1;}static int nfs_logo_trace_enabled(){return 0;}
static int nfs_logo_fullbox(const GLint*b,unsigned w,unsigned h){return b[0]==0&&b[1]==0&&b[2]>0&&b[3]>0&&(uint64_t)(unsigned)b[2]>=w&&(uint64_t)(unsigned)b[3]>=h;}
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
#define glCompileShader(...) probe("glCompileShader",[&](){return compileInjected(__VA_ARGS__);})
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
#define glDrawArrays(...) probe("glDrawArrays",[&](){return drawInjected(__VA_ARGS__);})
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
#define glGetRenderbufferParameteriv(...) probe("glGetRenderbufferParameteriv",[&](){return ::glGetRenderbufferParameteriv(__VA_ARGS__);})
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
#define glTexImage2D(...) probe("glTexImage2D",[&](){return uploadInjected(__VA_ARGS__);})
#define glTexParameteri(...) probe("glTexParameteri",[&](){return ::glTexParameteri(__VA_ARGS__);})
#define glUniform1f(...) probe("glUniform1f",[&](){return ::glUniform1f(__VA_ARGS__);})
#define glUniform1i(...) probe("glUniform1i",[&](){return ::glUniform1i(__VA_ARGS__);})
#define glUniformMatrix4fv(...) probe("glUniformMatrix4fv",[&](){return ::glUniformMatrix4fv(__VA_ARGS__);})
#define glUseProgram(...) probe("glUseProgram",[&](){return ::glUseProgram(__VA_ARGS__);})
#define glVertexAttrib4fv(...) probe("glVertexAttrib4fv",[&](){return ::glVertexAttrib4fv(__VA_ARGS__);})
#define glVertexAttribPointer(...) probe("glVertexAttribPointer",[&](){return ::glVertexAttribPointer(__VA_ARGS__);})
#define glViewport(...) probe("glViewport",[&](){return ::glViewport(__VA_ARGS__);})
#include "hd/BackgroundSceneResourcePrepare.c.inc"
static NfsBackgroundSceneCompositor nfs_bg_compositor;
#include "hd/ControlsSceneRenderer.inc"
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
#undef glGetRenderbufferParameteriv
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
struct Target {GLuint texture=0,fbo=0,depth=0;unsigned w,h;Target(unsigned W,unsigned H):w(W),h(H){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"fixture FBO");}void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);}void seed(){bind();glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glDepthMask(1);glClearColor(0,0,1,1);glClearDepthf(.25);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);}std::vector<unsigned char>read(){bind();std::vector<unsigned char>b(size_t(w)*h*4);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,b.data());ck(glGetError()==GL_NO_ERROR,"read");return b;}~Target(){glDeleteFramebuffers(1,&fbo);glDeleteRenderbuffers(1,&depth);glDeleteTextures(1,&texture);}};
static void depthUnchanged(Target&t){t.bind();const auto before=t.read();const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,1,0,1);}";GLuint v=nfs_hd_test::compileShader(GL_VERTEX_SHADER,vs),f=nfs_hd_test::compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glUseProgram(p);glBindBuffer(GL_ARRAY_BUFFER,0);glViewport(0,0,t.w,t.h);glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glDisable(GL_STENCIL_TEST);glDisable(GL_CULL_FACE);glColorMask(1,1,1,1);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(0);const GLfloat q[]={-1,-1,1,-1,-1,1,1,1};glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,0,0,q);glDrawArrays(GL_TRIANGLE_STRIP,0,4);ck(t.read()==before,"seeded .25 depth rejects .5 probe without color change");glUseProgram(0);glDeleteProgram(p);glDeleteShader(v);glDeleteShader(f);}
static std::array<NfsSoftwareTextVertex,6>text(float l,float t,float r,float b){return {{{l,t,.2,.2,1,1,1,1},{r,t,.8,.2,1,1,1,1},{l,b,.2,.8,1,1,1,1},{l,b,.2,.8,1,1,1,1},{r,t,.8,.2,1,1,1,1},{r,b,.8,.8,1,1,1,1}}};}
static NfsBackgroundSceneLayers layers(){NfsBackgroundSceneLayers v{};v.panelClip[2]=343;v.panelClip[3]=480;v.wordCount=2;for(unsigned i=0;i<2;++i){auto&w=v.words[i];w.sourceRole=i+1;w.scalar256=i?256:128;w.left=400+8*i;w.top=200+4*i;w.clip[2]=640;w.clip[3]=480;}unsigned roles[3]={1,2,0};for(unsigned i=0;i<3;++i){auto&r=nfs_logo_regions[roles[i]];auto&l=v.logos[i];l.role=roles[i];l.scalar256=i==1?127:256;l.left=r.sourceOriginX;l.top=r.sourceOriginY;l.right=l.left+r.logicalWidth;l.bottom=l.top+r.logicalHeight;l.clip[2]=343;l.clip[3]=480;}return v;}
static std::array<int,3>over(std::array<int,3>d,std::array<int,4>s,unsigned scalar=256){for(unsigned k=0;k<3;++k)d[k]=int(std::lround(s[k]*scalar/256.+d[k]*(1.-s[3]*scalar/(256.*255.))));return d;}
static void pixel(const Target&t,const std::vector<unsigned char>&b,unsigned x,unsigned y,std::array<int,3>rgb){unsigned px=unsigned((x+.5)*t.h/480.),py=t.h-1-unsigned((y+.5)*t.h/480.);for(unsigned k=0;k<3;++k)ck(std::abs(int(b[(size_t(py)*t.w+px)*4+k])-rgb[k])<=2,"independent layer order pixel oracle");}
static unsigned &drawCount=drawCalls;
#include "NestedCaptionWordGpuCases.h"
#include "hd/ControlsDialogWordDisplay.h"
namespace controls_synthetic {
#define main controlsPureProofUnusedMain
#include "controls_dialog_word_proof_tests.cpp"
#undef main
}
#undef CHECK
namespace nfs_controls_text {struct TailScope {static std::shared_ptr<const nfs_controls_dialog_word::Proof>synthetic(controls_synthetic::Packet p,const nfs_controls_dialog_word::Evidence&e){return std::shared_ptr<const nfs_controls_dialog_word::Proof>(new nfs_controls_dialog_word::Proof(std::move(p),e));}};}
struct Lifetime{unsigned retained=0,released=0,holds=0;static void retain(void*p){auto&v=*static_cast<Lifetime*>(p);++v.retained;++v.holds;}static void release(void*p){auto&v=*static_cast<Lifetime*>(p);ck(v.holds==1,"one live retained frame owner");++v.released;--v.holds;}};
struct Input{
 std::shared_ptr<const nfs_compact_caption_display::Artwork>wordArt=nfs_compact_caption_display::artwork(3);
 controls_synthetic::Packet packet;
 std::shared_ptr<const nfs_controls_dialog_display::Prepared>dialog;
 std::vector<unsigned char>alpha;std::vector<uint16_t>plane;
 std::array<NfsSoftwareTextVertex,6>caption;NfsSoftwareTextFontView font{};Lifetime lifetime;
 Input():alpha(1024*512,128),plane(640*480),caption(text(35,30,70,50)){
  for(size_t i=0;i<plane.size();++i)plane[i]=uint16_t(i*79u);
  font.schema=1;font.width=1024;font.height=512;font.alpha=alpha.data();font.bytes=alpha.size();font.identity=67;ck(nfs_texture_sha256(alpha.data(),alpha.size(),font.sha256),"synthetic font SHA");
  packet=controls_synthetic::packet(controls_synthetic::commands(0));const auto e=controls_synthetic::evidence(0);
  auto proof=nfs_controls_text::TailScope::synthetic(packet,e);dialog=nfs_controls_dialog_display::Prepared::create(proof,packet,{e.form,e.authority.archive,e.game,e.serial,e.lifetime});
  ck(dialog&&wordArt&&packet->vertices().size()==696,"real five-word artwork with explicitly synthetic16-command source packet");
 }
 NfsControlsSceneFrameView view(uint64_t serial){NfsControlsSceneFrameView v{};v.schema=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA;v.width=640;v.height=480;v.serial=v.contentRevision=serial;v.planeWords=plane.size();v.original565=plane.data();v.layers.background=layers();v.layers.sidebarWord=nested_caption_word_gpu_cases::word(*wordArt);v.layers.sidebarCaption={343,480,font,caption.data(),caption.size()};v.layers.text={640,480,font,packet->vertices().data(),packet->vertices().size()};v.layers.dialogWords=dialog->view();v.lifetimeOwner=&lifetime;v.retainOwner=Lifetime::retain;v.releaseOwner=Lifetime::release;return v;}
};
static GLuint originalProgram(){const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,0,1,1);}";GLuint v=compileShader(GL_VERTEX_SHADER,vs),f=compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glDeleteShader(v);glDeleteShader(f);return p;}
static void originalState(Target&t){t.bind();NfsFrameLegacySampling sampling{};ck(nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&sampling),"original sampling");std::memcpy(g_matrix,sampling.projection,sizeof g_matrix);glViewport(0,0,sampling.viewport[2],sampling.viewport[3]);glUseProgram(g_shaderProgram);glDisable(GL_CULL_FACE);glDisable(GL_STENCIL_TEST);glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_LEQUAL);glEnable(GL_SCISSOR_TEST);glScissor(0,0,t.w,t.h);glColorMask(1,1,1,1);glActiveTexture(GL_TEXTURE1);g_textureEnabled=true;g_fogEnabled=false;g_depthMask=true;g_renderBuffer=GR_BUFFER_BACKBUFFER;}
static void observePass(const Input&input){auto*ti=g_textures;GrTexInfo info{3,3,3,10};for(unsigned tile=0;tile<300;++tile){nfs_controls_renderer_upload_begin(ti);for(unsigned row=0;row<32;++row)for(unsigned col=0;col<32;++col){const uint16_t value=input.plane[((tile%15)*32+row)*640+(tile/15)*32+col];const size_t at=(row*32+col)*2;textureBytes[at]=uint8_t(value);textureBytes[at+1]=uint8_t(value>>8);}unsigned char observedSha[32],expectedSha[32];ck(nfs_texture_sha256(textureBytes,2048,observedSha)&&nfs_texture_sha256(nfs_controls_renderer.pass.expected[tile],2048,expectedSha)&&!std::memcmp(observedSha,expectedSha,32),"independent original565 tile bytes and immutable native witness hashes agree");nfs_controls_renderer_upload_done(ti);nfs_controls_renderer_select(0,0,3,&info,ti);GrVertex v[4]{};for(unsigned k=0;k<4;++k){v[k].x=float((tile/15)*32+(k==1||k==2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP;v[k].y=float((tile%15)*32+(k>=2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP;v[k].r=v[k].g=v[k].b=v[k].a=255;v[k].oow=1;v[k].tmuvtx[0].sow=k==1||k==2?255:0;v[k].tmuvtx[0].tow=k>=2?255:0;}nfs_controls_renderer_triangle(v,v+1,v+2);nfs_controls_renderer_triangle(v,v+2,v+3);}ck(nfs_controls_renderer.view.serial&&nfs_controls_renderer.pass.uploads==300&&nfs_controls_renderer.pass.selects==300&&nfs_controls_renderer.pass.triangles==600,"actual adapter accepts exact synthetic native observations");}
static void resetTrace(){events.clear();programs.clear();drawCounts.clear();callIndex=loseIndex=foreignTouches=drawCalls=0;recording=true;}
/* Record and exhaustively lose the real context after every GL operation in
   one optional adapter helper. A preserved CPU snapshot reuses existing valid
   owner resources; lost transient GL objects are reclaimed with the EGL context.
   There is deliberately no attempt to delete them in the foreign context. */
template<class Run,class Reset>static unsigned lossFamily(Pbuffer&egl,Target&target,const char*label,Run run,Reset reset){
 reset();originalState(target);const auto originalPixels=target.read();resetTrace();ck(run(),"record optional adapter helper");recording=false;const unsigned points=callIndex;ck(points>0,"helper emitted real GL calls");
 ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"foreign baseline context");glViewport(3,5,73,91);glActiveTexture(GL_TEXTURE1);glDepthMask(1);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owner baseline return");
 for(unsigned loss=1;loss<=points;++loss){reset();originalState(target);resetTrace();loseIndex=loss;trackForeign=true;const int result=run();recording=trackForeign=false;loseIndex=0;ck(!result&&foreignTouches==0&&eglGetCurrentContext()==lossContext&&equal(foreign,capture()),"optional helper stops all foreign GL operations");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"return after injected loss");while(glGetError()!=GL_NO_ERROR){}ck(target.read()==originalPixels,"resource/query loss never changes original color"); }
 reset();std::printf("%s_loss_points=%u ",label,points);return points;
}
int main(){try{
 Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic sealed art provider");Target original(640,480);original.seed();g_glCtx=egl.context;g_framebuffer=original.fbo;g_framebufferTexture=original.texture;g_framebufferDepth=original.depth;g_shaderProgram=originalProgram();nfs_logo_provenance.context=1;nfs_logo_provenance.alive=1;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,1),"fresh background generation");nfs_controls_renderer_created();originalState(original);
 GLuint scratch=0;glGenTextures(1,&scratch);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,scratch);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,32,32,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,textureBytes);g_textures[0]={scratch,10,32,0,textureBytes};originalState(original);Input input;
 // Actual callback thread and current EGL context are independent of packet metadata.
 {auto rejected=input.view(1);const auto retained=input.lifetime.retained;bool accepted=true;std::thread foreignThread([&](){accepted=nfs_private_controls_scene_begin(&rejected)!=0;});foreignThread.join();ck(!accepted&&input.lifetime.retained==retained,"wrong actual thread/current context rejects before retaining borrowed view");}
 // Old ABI schema must refuse before any poisoned new/legacy borrow is used.
 {auto old=input.view(1);old.schema=0;old.original565=reinterpret_cast<const uint16_t*>(uintptr_t(1));old.layers.dialogWords.images[0].rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));old.layers.sidebarWord.rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));old.layers.sidebarCaption.vertices=reinterpret_cast<const NfsSoftwareTextVertex*>(uintptr_t(1));old.layers.text.vertices=reinterpret_cast<const NfsSoftwareTextVertex*>(uintptr_t(1));old.lifetimeOwner=reinterpret_cast<void*>(uintptr_t(1));old.retainOwner=reinterpret_cast<void(*)(void*)>(uintptr_t(1));old.releaseOwner=reinterpret_cast<void(*)(void*)>(uintptr_t(1));const unsigned retained=input.lifetime.retained;const auto before=capture();const auto pixels=original.read();ck(!nfs_private_controls_scene_begin(&old)&&input.lifetime.retained==retained&&equal(before,capture())&&original.read()==pixels,"schema0 rejects before new and original borrowed data or callbacks");}
 for(unsigned missing=0;missing<3;++missing){auto bad=input.view(1);if(!missing)bad.lifetimeOwner=nullptr;else if(missing==1)bad.retainOwner=nullptr;else bad.releaseOwner=nullptr;const auto before=capture();const auto retained=input.lifetime.retained;ck(!nfs_private_controls_scene_begin(&bad)&&input.lifetime.retained==retained&&equal(before,capture()),"missing paired lifetime callback/owner refuses without borrowing");}
 // Actual adapter finish cold-prepares all resources, then exact warm reuse.
 auto v=input.view(1);ck(nfs_private_controls_scene_begin(&v)&&input.lifetime.holds==1,"begin retains immutable owner once");observePass(input);const auto state=capture();resetTrace();ck(nfs_private_controls_scene_finish(1),"cold complete actual adapter finish");recording=false;ck(equal(state,capture())&&input.lifetime.retained==1&&input.lifetime.released==1&&!input.lifetime.holds&&!nfs_controls_renderer.view.serial,"finish restores inherited state and releases exactly once");ck(nfs_controls_renderer.pass.uploads==300&&nfs_controls_renderer.pass.selects==300&&nfs_controls_renderer.pass.triangles==600,"finish retains full native proof counts");ck(std::find(events.begin(),events.end(),"glCreateShader")!=events.end(),"cold finish allocated shaders");const auto cold=original.read();pixel(original,cold,600,300,{{20,40,60}});nested_caption_word_gpu_cases::image(original,cold,*input.wordArt);// Exact complete-stage to published-color equality proves the renderer commits
 // every authored label and intervening original ellipsis, not a partial overlay.
 {glBindFramebuffer(GL_FRAMEBUFFER,nfs_controls_renderer.fbo);std::vector<unsigned char>staged(cold.size());glReadPixels(0,0,original.w,original.h,GL_RGBA,GL_UNSIGNED_BYTE,staged.data());ck(glGetError()==GL_NO_ERROR&&staged==cold,"complete Controls staging equals every published color pixel");original.bind();}
 ck(programs.size()==20&&drawCounts.size()==20,"whole Controls publish has19 ordered scene draws then one color commit");
 for(unsigned j=0;j<4;++j)ck(programs[j]==nfs_bg_compositor.program,"neutral background/panel/word source order");
 ck(programs[4]==nfs_bg_compositor.logo.program&&programs[5]==nfs_controls_renderer.sidebarWordGpu.program,"original logo then exact nested Options word");
 {unsigned at=6;for(unsigned word=0;word<5;++word){for(unsigned shadow=0;shadow<2;++shadow)ck(programs[at]==nfs_controls_renderer.dialogWordsGpu.program&&drawCounts[at++]==6,"Controls canonical2/3/4/5/1 finite shadow/foreground draw order");if(word<3)ck(programs[at]==nfs_controls_renderer.text.program&&drawCounts[at++]==36,"each original dynamic ellipsis pair stays between finite word pairs");}ck(at==19&&programs[19]==nfs_bg_compositor.copyProgram,"only final scene color copied to original target");}
 depthUnchanged(original);originalState(original);NfsNativeLogoTargetProof proof{};ck(nfs_controls_read_target(&proof,1)&&!proof.depthClearProven&&!proof.depthClearSerial&&proof.depthClearValue==0,"no invented depth clear witness");
 const GLuint captionProgram=nfs_controls_renderer.caption.program,captionTexture=nfs_controls_renderer.caption.texture,color=nfs_controls_renderer.color,fbo=nfs_controls_renderer.fbo;v=input.view(2);ck(nfs_private_controls_scene_begin(&v),"warm begin");observePass(input);resetTrace();ck(nfs_private_controls_scene_finish(2),"warm complete actual adapter finish");recording=false;ck(captionProgram==nfs_controls_renderer.caption.program&&captionTexture==nfs_controls_renderer.caption.texture&&color==nfs_controls_renderer.color&&fbo==nfs_controls_renderer.fbo&&std::find(events.begin(),events.end(),"glCreateShader")==events.end()&&std::find(events.begin(),events.end(),"glGenTextures")==events.end(),"warm finish reuses all exact resources");
 for(unsigned kind=0;kind<4;++kind){originalState(original);v=input.view(3+kind*2);const unsigned before=input.lifetime.released;ck(nfs_private_controls_scene_begin(&v),"cancel scenario retains");if(kind==0)nfs_controls_renderer_other();if(kind==1)ck(!nfs_private_controls_scene_begin(&v),"nested begin refused");if(kind==2)ck(!nfs_private_controls_scene_finish(v.serial+1),"wrong serial refused");if(kind==3)ck(!nfs_private_controls_scene_finish(v.serial),"incomplete native pass refused");ck(input.lifetime.released==before+1&&!input.lifetime.holds&&!nfs_controls_renderer.view.serial,"cancel releases exactly once");nfs_controls_renderer_other();ck(input.lifetime.released==before+1,"repeated cancel never double releases");}
 // Same-owner failures of new cold preparation restore GL state and leave the
 // original color/depth target untouched. No original GL draw is replaced here.
 const auto warm=nfs_controls_renderer;originalState(original);NfsSoftwareTextGpu testFont{};ck(nfs_soft_gpu_context_created(&testFont,1),"isolated font generation");auto failureState=capture();badCompile=true;ck(!nfs_controls_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"shader compile failure restores exact state");badCompile=false;originalState(original);failureState=capture();failUpload=1;uploadIndex=0;ck(!nfs_controls_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"font upload failure restores exact state");failUpload=0;
 nfs_controls_renderer=warm;nfs_controls_renderer.color=nfs_controls_renderer.fbo=0;nfs_controls_renderer.width=nfs_controls_renderer.height=0;originalState(original);failureState=capture();failUpload=1;uploadIndex=0;ck(!nfs_controls_color_target(640,480)&&equal(failureState,capture()),"color allocation failure restores exact state");failUpload=0;nfs_controls_renderer=warm;
 for(unsigned fault=0;fault<7;++fault){originalState(original);auto bad=input.view(50+fault);auto pixels=input.wordArt->rgba;if(fault==0)bad.layers.sidebarWord.word=2;if(fault==1)bad.layers.sidebarWord.schema=0;if(fault==2)bad.layers.sidebarWord.left++;if(fault==3)--bad.layers.sidebarWord.bytes;if(fault==4)bad.layers.sidebarWord.rgba=nullptr;if(fault==5)bad.layers.sidebarWord.sha256[0]^=1;if(fault==6){pixels[3]^=1;bad.layers.sidebarWord.rgba=pixels.data();}const unsigned retained=input.lifetime.retained;auto state=capture();auto visible=original.read();ck(!nfs_private_controls_scene_begin(&bad)&&input.lifetime.retained==retained&&!input.lifetime.holds&&equal(state,capture())&&original.read()==visible,"actual Controls adapter malformed sidebar word rejected before retain or draw");}
 // Malformed nonempty five-word views cannot silently downgrade or retain.
 for(unsigned fault=0;fault<12;++fault){originalState(original);auto bad=input.view(70+fault);auto pixels=nfs_controls_dialog_display::artwork(1)->rgba;
  if(fault==0)bad.layers.dialogWords={};if(fault==1)bad.layers.dialogWords.count=8;if(fault==2)bad.layers.dialogWords.images[0].word=5;if(fault==3)bad.layers.dialogWords.images[0].sha256[0]^=1;if(fault==4){pixels[3]^=1;bad.layers.dialogWords.images[0].rgba=pixels.data();}if(fault==5)bad.layers.dialogWords.placements[2].vertexFirst++;if(fault==6)bad.layers.dialogWords.placements[9].color565=65535;if(fault==7)bad.layers.dialogWords.placements[0].word=1;if(fault==8)bad.layers.text.vertexCount=690;if(fault==9){bad.schema=2;bad.layers.dialogWords.images[0].rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));bad.retainOwner=reinterpret_cast<void(*)(void*)>(uintptr_t(1));}if(fault==10)bad.layers.dialogWords.placements[4].left++;if(fault==11)bad.layers.sidebarWord={};
  const auto retained=input.lifetime.retained;const auto state=capture();const auto visible=original.read();ck(!nfs_private_controls_scene_begin(&bad)&&input.lifetime.retained==retained&&!input.lifetime.holds&&equal(state,capture())&&original.read()==visible,"bad Controls word geometry/source schema/pin declines before retain or publication");}
 // Actual adapter observations reject wrong native payload, owner target,
 // upload pairing and selection, while preserving the untouched original fallback.
 for(unsigned fault=0;fault<4;++fault){originalState(original);const auto before=original.read();auto bad=input.view(90+fault);ck(nfs_private_controls_scene_begin(&bad),"native observation negative begins owned view");
  if(fault==0){nfs_controls_renderer_upload_begin(g_textures);std::memcpy(textureBytes,nfs_controls_renderer.pass.expected[0],2048);textureBytes[37]^=1;nfs_controls_renderer_upload_done(g_textures);}
  if(fault==1){glBindFramebuffer(GL_FRAMEBUFFER,0);nfs_controls_renderer_upload_begin(g_textures);original.bind();}
  if(fault==2)nfs_controls_renderer_upload_done(g_textures);
  if(fault==3){GrTexInfo wrong{3,3,3,10};nfs_controls_renderer_select(0,0,3,&wrong,g_textures);}
  ck(!nfs_controls_renderer.view.serial&&!input.lifetime.holds&&original.read()==before,"wrong actual native observations cancel/release without replacement draw");}
 // Same-owner first/last staging draw failures discard the complete staging
 // transaction; the synthetic original/native fallback color and depth survive.
 for(unsigned failed:{1u,19u,20u}){original.seed();originalState(original);const auto visible=original.read();auto failedView=input.view(100+failed);ck(nfs_private_controls_scene_begin(&failedView),"late error retains Controls view");observePass(input);const auto state=capture();resetTrace();failDraw=failed;ck(!nfs_private_controls_scene_finish(failedView.serial),"injected first/last stage or final commit draw refuses");recording=false;failDraw=0;ck(original.read()==visible&&equal(state,capture())&&!input.lifetime.holds&&!nfs_controls_renderer.view.serial,"late failure preserves original color and restores/releases owner");depthUnchanged(original);}
 originalState(original);
 for(unsigned alias=0;alias<2;++alias){originalState(original);const auto visible=original.read();auto bad=input.view(130+alias);ck(nfs_private_controls_scene_begin(&bad),"resource alias scenario retains");observePass(input);const auto resource=nfs_controls_renderer.dialogWordsGpu;if(alias==0)nfs_controls_renderer.dialogWordsGpu.textures[0]=g_framebufferTexture;else nfs_controls_renderer.dialogWordsGpu.program=nfs_controls_renderer.text.program;const auto state=capture();drawCalls=0;ck(!nfs_private_controls_scene_finish(bad.serial)&&!drawCalls&&original.read()==visible&&equal(state,capture())&&!input.lifetime.holds,"foreign original-color/font-program alias refuses before staged draw and preserves borrowed resources");nfs_controls_renderer.dialogWordsGpu=resource;}
 // Record direct optional helpers, not the original flush stub or 300 tile calls.
 EGLint contextAttrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,contextAttrs);ck(lossContext!=EGL_NO_CONTEXT,"independent foreign context");unsigned points=0;
 const auto emptyFont=testFont;points+=lossFamily(egl,original,"font_cold",[&](){return nfs_controls_font_prepare(&testFont,&input.font);},[&](){nfs_controls_renderer=warm;testFont=emptyFont;});
 points+=lossFamily(egl,original,"font_warm",[&](){return nfs_controls_font_prepare(&nfs_controls_renderer.caption,&input.font);},[&](){nfs_controls_renderer=warm;});
 points+=lossFamily(egl,original,"color_cold",[&](){return nfs_controls_color_target(640,480);},[&](){nfs_controls_renderer=warm;nfs_controls_renderer.color=nfs_controls_renderer.fbo=0;nfs_controls_renderer.width=nfs_controls_renderer.height=0;});
 points+=lossFamily(egl,original,"color_warm",[&](){return nfs_controls_color_target(640,480);},[&](){nfs_controls_renderer=warm;});
 points+=lossFamily(egl,original,"target_reader",[&](){NfsNativeLogoTargetProof t{};return nfs_controls_read_target(&t,99);},[&](){nfs_controls_renderer=warm;});
 NfsControlsDialogWordGpu isolatedDialog{};const auto emptyDialog=isolatedDialog;auto dialogView=input.dialog->view();auto textView=input.view(202).layers.text;
 points+=lossFamily(egl,original,"Controls_dialog_cold",[&](){auto guard=nfs_controls_context_stage();return nfs_controls_dialog_prepare(&isolatedDialog,&dialogView,&textView,&guard);},[&](){nfs_controls_renderer=warm;isolatedDialog=emptyDialog;});
 points+=lossFamily(egl,original,"Controls_dialog_warm",[&](){auto guard=nfs_controls_context_stage();return nfs_controls_dialog_prepare(&nfs_controls_renderer.dialogWordsGpu,&dialogView,&textView,&guard);},[&](){nfs_controls_renderer=warm;});
 NfsNestedCaptionGpu isolatedWord{};const auto emptyWord=isolatedWord;const auto word=nested_caption_word_gpu_cases::word(*input.wordArt);
 points+=lossFamily(egl,original,"nested_word_cold",[&](){auto guard=nfs_controls_context_stage();return nfs_nested_caption_prepare(&isolatedWord,&word,3,&guard);},[&](){nfs_controls_renderer=warm;isolatedWord=emptyWord;});
 points+=lossFamily(egl,original,"nested_word_warm",[&](){auto guard=nfs_controls_context_stage();return nfs_nested_caption_prepare(&nfs_controls_renderer.sidebarWordGpu,&word,3,&guard);},[&](){nfs_controls_renderer=warm;});
 NfsNestedCaptionGpu retireWord{};points+=lossFamily(egl,original,"nested_word_retire",[&](){auto guard=nfs_controls_context_stage();return nfs_nested_caption_retire(&retireWord,&guard,1);},[&](){nfs_controls_renderer=warm;retireWord={};auto guard=nfs_controls_context_stage();ck(nfs_nested_caption_prepare(&retireWord,&word,3,&guard),"prepare separately owned retirement-loss resources");});{auto guard=nfs_controls_context_stage();ck(nfs_nested_caption_retire(&retireWord,&guard,1),"release final retirement-loss resource");}
 originalState(original);depthUnchanged(original);originalState(original);
 // Commit requires a retained frame and registered original target. Reset uses
 // the real retain callback; each trial releases or cancels its one live borrow.
 originalState(original);nfs_controls_renderer=warm;v=input.view(200);ck(nfs_private_controls_scene_begin(&v),"commit fixture retained frame");auto commitBase=nfs_controls_renderer;NfsNativeLogoTargetProof target=commitBase.target;resetTrace();ck(nfs_controls_copy_color(&target),"direct color-only commit helper");recording=false;const unsigned commitPoints=callIndex;nfs_controls_cancel("test-commit-release");
 for(unsigned loss=1;loss<=commitPoints;++loss){originalState(original);nfs_controls_renderer=commitBase;Lifetime::retain(&input.lifetime);ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"commit foreign baseline");glViewport(3,5,73,91);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"commit owner return");originalState(original);resetTrace();loseIndex=loss;trackForeign=true;const int result=nfs_controls_copy_color(&target);recording=trackForeign=false;loseIndex=0;ck(!result&&foreignTouches==0&&equal(foreign,capture()),"commit stops before any foreign GL operation");nfs_controls_abandon();ck(!input.lifetime.holds,"commit loss caller abandonment releases borrow");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"commit owner recovery");while(glGetError()!=GL_NO_ERROR){}}
 points+=commitPoints;std::printf("commit_loss_points=%u ",commitPoints);
 nfs_controls_renderer=warm;originalState(original);v=input.view(201);ck(nfs_private_controls_scene_begin(&v),"retire retained frame");const unsigned released=input.lifetime.released;nfs_controls_renderer_retired(1);ck(!nfs_controls_renderer.generation&&!nfs_controls_renderer.caption.generation&&!nfs_controls_renderer.text.generation&&!nfs_controls_renderer.color&&!nfs_controls_renderer.fbo&&!nfs_controls_renderer.sidebarWordGpu.program&&!nfs_controls_renderer.sidebarWordGpu.texture&&!nfs_controls_renderer.dialogWordsGpu.program&&!nfs_controls_renderer.dialogWordsGpu.textures[0]&&input.lifetime.released==released+1&&!input.lifetime.holds,"owner retirement releases and forgets all owned resources");
 // Same real EGL context, new owned renderer generation: no previous GPU
 // names or borrowed lifetime callbacks survive reset; complete scene recreates.
 glUseProgram(0);nfs_bg_scene_context_retired(&nfs_bg_compositor,1,1);nfs_logo_provenance.context=2;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,2),"reset neutral context generation");nfs_controls_renderer_created();original.seed();originalState(original);v=input.view(300);ck(nfs_private_controls_scene_begin(&v),"new generation Controls begin");observePass(input);ck(nfs_private_controls_scene_finish(300)&&!input.lifetime.holds&&nfs_controls_renderer.dialogWordsGpu.generation==2,"new generation complete publication recreates independent5word resource set");originalState(original);nfs_controls_renderer_retired(1);glUseProgram(0);nfs_bg_scene_context_retired(&nfs_bg_compositor,2,1);
 // The same source packet also publishes through an actual high-resolution
 // original target; logical source authority does not depend on target pixels.
 {Target native(1280,960);g_framebuffer=native.fbo;g_framebufferTexture=native.texture;g_framebufferDepth=native.depth;g_framebufferHeight=960;nfs_logo_provenance.context=3;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,3),"native-resolution background generation");nfs_controls_renderer_created();native.seed();originalState(native);auto nativeView=input.view(400);ck(nfs_private_controls_scene_begin(&nativeView),"native-resolution Controls owned begin");observePass(input);ck(nfs_private_controls_scene_finish(400)&&!input.lifetime.holds,"native-resolution complete Controls publication");const auto published=native.read();nested_caption_word_gpu_cases::image(native,published,*input.wordArt);glBindFramebuffer(GL_FRAMEBUFFER,nfs_controls_renderer.fbo);std::vector<unsigned char>stage(published.size());glReadPixels(0,0,native.w,native.h,GL_RGBA,GL_UNSIGNED_BYTE,stage.data());ck(glGetError()==GL_NO_ERROR&&stage==published,"native-resolution entire staging color copied exactly");depthUnchanged(native);originalState(native);nfs_controls_renderer_retired(1);glUseProgram(0);nfs_bg_scene_context_retired(&nfs_bg_compositor,3,1);}
 ck(eglDestroyContext(egl.display,lossContext),"destroy foreign test context");lossContext=EGL_NO_CONTEXT;glUseProgram(0);glDeleteProgram(g_shaderProgram);glDeleteTextures(1,&scratch);ck(glGetError()==GL_NO_ERROR,"final GL error state");ck(originalFlushCalls>=10,"original flush stub invoked but no original GL coverage claimed");std::printf("PASS actual Controls adapter checks=%u optional_loss_points=%u synthetic_source=1 synthetic_native_callbacks=1 real_finite_art=1 original_draw_stub=1\n",checks,points);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
