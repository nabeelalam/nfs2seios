// SPDX-License-Identifier: MIT
/* Actual optional Graphics renderer adapter with synthetic art and real GLES.
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
static void drawInjected(GLenum mode,GLint first,GLsizei count){++drawCalls;::glDrawArrays(drawCalls==failDraw?GLenum(0xffffffff):mode,first,count);}
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
#define NFS_GRAPHICS_SCENE_THREAD_TOKEN() testThread()
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
#include "hd/GraphicsSceneRenderer.inc"
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
static std::vector<GLuint>programs;
#include "NestedCaptionWordGpuCases.h"
struct Lifetime{unsigned retained=0,released=0,holds=0;static void retain(void*p){auto&v=*static_cast<Lifetime*>(p);++v.retained;++v.holds;}static void release(void*p){auto&v=*static_cast<Lifetime*>(p);ck(v.holds==1,"one live retained frame owner");++v.released;--v.holds;}};
struct Input{std::shared_ptr<const nfs_compact_caption_display::Artwork>wordArt=nfs_compact_caption_display::artwork(3);std::vector<unsigned char>alpha;std::vector<uint16_t>plane;std::array<NfsSoftwareTextVertex,6>caption,full;NfsSoftwareTextFontView font{};Lifetime lifetime;Input():alpha(1024*512,128),plane(640*480),caption(text(35,30,70,50)),full(text(480,150,520,175)){for(size_t i=0;i<plane.size();++i)plane[i]=uint16_t(i*79u);font.schema=1;font.width=1024;font.height=512;font.alpha=alpha.data();font.bytes=alpha.size();font.identity=67;ck(nfs_texture_sha256(alpha.data(),alpha.size(),font.sha256),"synthetic font SHA");}NfsGraphicsSceneFrameView view(uint64_t serial){NfsGraphicsSceneFrameView v{};v.schema=NFS_GRAPHICS_SCENE_RUNTIME_SCHEMA;v.width=640;v.height=480;v.serial=v.contentRevision=serial;v.planeWords=plane.size();v.original565=plane.data();v.layers.background=layers();v.layers.sidebarWord=nested_caption_word_gpu_cases::word(*wordArt);v.layers.sidebarCaption={343,480,font,caption.data(),caption.size()};v.layers.text={640,480,font,full.data(),full.size()};v.lifetimeOwner=&lifetime;v.retainOwner=Lifetime::retain;v.releaseOwner=Lifetime::release;return v;}};
static GLuint originalProgram(){const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,0,1,1);}";GLuint v=compileShader(GL_VERTEX_SHADER,vs),f=compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glDeleteShader(v);glDeleteShader(f);return p;}
static void originalState(Target&t){t.bind();NfsFrameLegacySampling sampling{};ck(nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&sampling),"original sampling");std::memcpy(g_matrix,sampling.projection,sizeof g_matrix);glViewport(0,0,sampling.viewport[2],sampling.viewport[3]);glUseProgram(g_shaderProgram);glDisable(GL_CULL_FACE);glDisable(GL_STENCIL_TEST);glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_LEQUAL);glEnable(GL_SCISSOR_TEST);glScissor(0,0,t.w,t.h);glColorMask(1,1,1,1);glActiveTexture(GL_TEXTURE1);g_textureEnabled=true;g_fogEnabled=false;g_depthMask=true;g_renderBuffer=GR_BUFFER_BACKBUFFER;}
static void observePass(){auto*ti=g_textures;GrTexInfo info{3,3,3,10};for(unsigned tile=0;tile<300;++tile){nfs_graphics_renderer_upload_begin(ti);std::memcpy(textureBytes,nfs_graphics_renderer.pass.expected[tile],2048);nfs_graphics_renderer_upload_done(ti);nfs_graphics_renderer_select(0,0,3,&info,ti);GrVertex v[4]{};for(unsigned k=0;k<4;++k){v[k].x=float((tile/15)*32+(k==1||k==2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP;v[k].y=float((tile%15)*32+(k>=2?32:0))+NFS_AUDIO_NATIVE_VERTEX_SNAP;v[k].r=v[k].g=v[k].b=v[k].a=255;v[k].oow=1;v[k].tmuvtx[0].sow=k==1||k==2?255:0;v[k].tmuvtx[0].tow=k>=2?255:0;}nfs_graphics_renderer_triangle(v,v+1,v+2);nfs_graphics_renderer_triangle(v,v+2,v+3);}ck(nfs_graphics_renderer.view.serial&&nfs_graphics_renderer.pass.uploads==300&&nfs_graphics_renderer.pass.selects==300&&nfs_graphics_renderer.pass.triangles==600,"actual adapter accepts exact synthetic native observations");}
static void resetTrace(){events.clear();callIndex=loseIndex=foreignTouches=drawCalls=0;recording=true;}
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
 Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic sealed art provider");Target original(640,480);original.seed();g_glCtx=egl.context;g_framebuffer=original.fbo;g_framebufferTexture=original.texture;g_framebufferDepth=original.depth;g_shaderProgram=originalProgram();nfs_logo_provenance.context=1;nfs_logo_provenance.alive=1;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,1),"fresh background generation");nfs_graphics_renderer_created();originalState(original);
 GLuint scratch=0;glGenTextures(1,&scratch);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,scratch);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,32,32,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,textureBytes);g_textures[0]={scratch,10,32,0,textureBytes};originalState(original);Input input;
 // Old ABI schema must refuse before any poisoned new/legacy borrow is used.
 {auto old=input.view(1);old.schema=1;old.original565=reinterpret_cast<const uint16_t*>(uintptr_t(1));old.layers.sidebarWord.rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));old.layers.sidebarCaption.vertices=reinterpret_cast<const NfsSoftwareTextVertex*>(uintptr_t(1));old.layers.text.vertices=reinterpret_cast<const NfsSoftwareTextVertex*>(uintptr_t(1));old.lifetimeOwner=reinterpret_cast<void*>(uintptr_t(1));old.retainOwner=reinterpret_cast<void(*)(void*)>(uintptr_t(1));old.releaseOwner=reinterpret_cast<void(*)(void*)>(uintptr_t(1));const unsigned retained=input.lifetime.retained;const auto before=capture();const auto pixels=original.read();ck(!nfs_private_graphics_scene_begin(&old)&&input.lifetime.retained==retained&&equal(before,capture())&&original.read()==pixels,"schema1 rejects before new and original borrowed data or callbacks");}
 // Actual adapter finish cold-prepares all resources, then exact warm reuse.
 auto v=input.view(1);ck(nfs_private_graphics_scene_begin(&v)&&input.lifetime.holds==1,"begin retains immutable owner once");observePass();const auto state=capture();resetTrace();ck(nfs_private_graphics_scene_finish(1),"cold complete actual adapter finish");recording=false;ck(equal(state,capture())&&input.lifetime.retained==1&&input.lifetime.released==1&&!input.lifetime.holds&&!nfs_graphics_renderer.view.serial,"finish restores inherited state and releases exactly once");ck(nfs_graphics_renderer.pass.uploads==300&&nfs_graphics_renderer.pass.selects==300&&nfs_graphics_renderer.pass.triangles==600,"finish retains full native proof counts");ck(std::find(events.begin(),events.end(),"glCreateShader")!=events.end(),"cold finish allocated shaders");const auto cold=original.read();pixel(original,cold,600,300,{{20,40,60}});nested_caption_word_gpu_cases::image(original,cold,*input.wordArt);depthUnchanged(original);originalState(original);NfsNativeLogoTargetProof proof{};ck(nfs_graphics_read_target(&proof,1)&&!proof.depthClearProven&&!proof.depthClearSerial&&proof.depthClearValue==0,"no invented depth clear witness");
 const GLuint captionProgram=nfs_graphics_renderer.caption.program,captionTexture=nfs_graphics_renderer.caption.texture,color=nfs_graphics_renderer.color,fbo=nfs_graphics_renderer.fbo;v=input.view(2);ck(nfs_private_graphics_scene_begin(&v),"warm begin");observePass();resetTrace();ck(nfs_private_graphics_scene_finish(2),"warm complete actual adapter finish");recording=false;ck(captionProgram==nfs_graphics_renderer.caption.program&&captionTexture==nfs_graphics_renderer.caption.texture&&color==nfs_graphics_renderer.color&&fbo==nfs_graphics_renderer.fbo&&std::find(events.begin(),events.end(),"glCreateShader")==events.end()&&std::find(events.begin(),events.end(),"glGenTextures")==events.end(),"warm finish reuses all exact resources");
 for(unsigned kind=0;kind<4;++kind){originalState(original);v=input.view(3+kind*2);const unsigned before=input.lifetime.released;ck(nfs_private_graphics_scene_begin(&v),"cancel scenario retains");if(kind==0)nfs_graphics_renderer_other();if(kind==1)ck(!nfs_private_graphics_scene_begin(&v),"nested begin refused");if(kind==2)ck(!nfs_private_graphics_scene_finish(v.serial+1),"wrong serial refused");if(kind==3)ck(!nfs_private_graphics_scene_finish(v.serial),"incomplete native pass refused");ck(input.lifetime.released==before+1&&!input.lifetime.holds&&!nfs_graphics_renderer.view.serial,"cancel releases exactly once");nfs_graphics_renderer_other();ck(input.lifetime.released==before+1,"repeated cancel never double releases");}
 // Same-owner failures of new cold preparation restore GL state and leave the
 // original color/depth target untouched. No original GL draw is replaced here.
 const auto warm=nfs_graphics_renderer;originalState(original);NfsSoftwareTextGpu testFont{};ck(nfs_soft_gpu_context_created(&testFont,1),"isolated font generation");auto failureState=capture();badCompile=true;ck(!nfs_graphics_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"shader compile failure restores exact state");badCompile=false;originalState(original);failureState=capture();failUpload=1;uploadIndex=0;ck(!nfs_graphics_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"font upload failure restores exact state");failUpload=0;
 nfs_graphics_renderer=warm;nfs_graphics_renderer.color=nfs_graphics_renderer.fbo=0;nfs_graphics_renderer.width=nfs_graphics_renderer.height=0;originalState(original);failureState=capture();failUpload=1;uploadIndex=0;ck(!nfs_graphics_color_target(640,480)&&equal(failureState,capture()),"color allocation failure restores exact state");failUpload=0;nfs_graphics_renderer=warm;
 for(unsigned fault=0;fault<7;++fault){originalState(original);auto bad=input.view(50+fault);auto pixels=input.wordArt->rgba;if(fault==0)bad.layers.sidebarWord.word=2;if(fault==1)bad.layers.sidebarWord.schema=0;if(fault==2)bad.layers.sidebarWord.left++;if(fault==3)--bad.layers.sidebarWord.bytes;if(fault==4)bad.layers.sidebarWord.rgba=nullptr;if(fault==5)bad.layers.sidebarWord.sha256[0]^=1;if(fault==6){pixels[3]^=1;bad.layers.sidebarWord.rgba=pixels.data();}const unsigned retained=input.lifetime.retained;auto state=capture();auto visible=original.read();ck(!nfs_private_graphics_scene_begin(&bad)&&input.lifetime.retained==retained&&!input.lifetime.holds&&equal(state,capture())&&original.read()==visible,"actual Graphics adapter malformed sidebar word rejected before retain or draw");}
 // Record direct optional helpers, not the original flush stub or 300 tile calls.
 EGLint contextAttrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,contextAttrs);ck(lossContext!=EGL_NO_CONTEXT,"independent foreign context");unsigned points=0;
 const auto emptyFont=testFont;points+=lossFamily(egl,original,"font_cold",[&](){return nfs_graphics_font_prepare(&testFont,&input.font);},[&](){nfs_graphics_renderer=warm;testFont=emptyFont;});
 points+=lossFamily(egl,original,"font_warm",[&](){return nfs_graphics_font_prepare(&nfs_graphics_renderer.caption,&input.font);},[&](){nfs_graphics_renderer=warm;});
 points+=lossFamily(egl,original,"color_cold",[&](){return nfs_graphics_color_target(640,480);},[&](){nfs_graphics_renderer=warm;nfs_graphics_renderer.color=nfs_graphics_renderer.fbo=0;nfs_graphics_renderer.width=nfs_graphics_renderer.height=0;});
 points+=lossFamily(egl,original,"color_warm",[&](){return nfs_graphics_color_target(640,480);},[&](){nfs_graphics_renderer=warm;});
 points+=lossFamily(egl,original,"target_reader",[&](){NfsNativeLogoTargetProof t{};return nfs_graphics_read_target(&t,99);},[&](){nfs_graphics_renderer=warm;});
 NfsNestedCaptionGpu isolatedWord{};const auto emptyWord=isolatedWord;const auto word=nested_caption_word_gpu_cases::word(*input.wordArt);
 points+=lossFamily(egl,original,"nested_word_cold",[&](){auto guard=nfs_graphics_context_stage();return nfs_nested_caption_prepare(&isolatedWord,&word,3,&guard);},[&](){nfs_graphics_renderer=warm;isolatedWord=emptyWord;});
 points+=lossFamily(egl,original,"nested_word_warm",[&](){auto guard=nfs_graphics_context_stage();return nfs_nested_caption_prepare(&nfs_graphics_renderer.sidebarWordGpu,&word,3,&guard);},[&](){nfs_graphics_renderer=warm;});
 NfsNestedCaptionGpu retireWord{};points+=lossFamily(egl,original,"nested_word_retire",[&](){auto guard=nfs_graphics_context_stage();return nfs_nested_caption_retire(&retireWord,&guard,1);},[&](){nfs_graphics_renderer=warm;retireWord={};auto guard=nfs_graphics_context_stage();ck(nfs_nested_caption_prepare(&retireWord,&word,3,&guard),"prepare separately owned retirement-loss resources");});{auto guard=nfs_graphics_context_stage();ck(nfs_nested_caption_retire(&retireWord,&guard,1),"release final retirement-loss resource");}
 originalState(original);depthUnchanged(original);originalState(original);
 // Commit requires a retained frame and registered original target. Reset uses
 // the real retain callback; each trial releases or cancels its one live borrow.
 originalState(original);nfs_graphics_renderer=warm;v=input.view(11);ck(nfs_private_graphics_scene_begin(&v),"commit fixture retained frame");auto commitBase=nfs_graphics_renderer;NfsNativeLogoTargetProof target=commitBase.target;resetTrace();ck(nfs_graphics_copy_color(&target),"direct color-only commit helper");recording=false;const unsigned commitPoints=callIndex;nfs_graphics_cancel("test-commit-release");
 for(unsigned loss=1;loss<=commitPoints;++loss){originalState(original);nfs_graphics_renderer=commitBase;Lifetime::retain(&input.lifetime);ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"commit foreign baseline");glViewport(3,5,73,91);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"commit owner return");originalState(original);resetTrace();loseIndex=loss;trackForeign=true;const int result=nfs_graphics_copy_color(&target);recording=trackForeign=false;loseIndex=0;ck(!result&&foreignTouches==0&&equal(foreign,capture()),"commit stops before any foreign GL operation");nfs_graphics_abandon();ck(!input.lifetime.holds,"commit loss caller abandonment releases borrow");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"commit owner recovery");while(glGetError()!=GL_NO_ERROR){}}
 points+=commitPoints;std::printf("commit_loss_points=%u ",commitPoints);
 nfs_graphics_renderer=warm;originalState(original);v=input.view(12);ck(nfs_private_graphics_scene_begin(&v),"retire retained frame");const unsigned released=input.lifetime.released;nfs_graphics_renderer_retired(1);ck(!nfs_graphics_renderer.generation&&!nfs_graphics_renderer.caption.generation&&!nfs_graphics_renderer.text.generation&&!nfs_graphics_renderer.color&&!nfs_graphics_renderer.fbo&&!nfs_graphics_renderer.sidebarWordGpu.program&&!nfs_graphics_renderer.sidebarWordGpu.texture&&input.lifetime.released==released+1&&!input.lifetime.holds,"owner retirement releases and forgets all owned resources");
 ck(eglDestroyContext(egl.display,lossContext),"destroy foreign test context");lossContext=EGL_NO_CONTEXT;nfs_bg_scene_context_retired(&nfs_bg_compositor,1,1);glUseProgram(0);glDeleteProgram(g_shaderProgram);glDeleteTextures(1,&scratch);ck(glGetError()==GL_NO_ERROR,"final GL error state");ck(originalFlushCalls>=10,"original flush stub invoked but no original GL coverage claimed");std::printf("PASS actual Graphics adapter checks=%u optional_loss_points=%u synthetic_only=1 original_draw_stub=1\n",checks,points);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
