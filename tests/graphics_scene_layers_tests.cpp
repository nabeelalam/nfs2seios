// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include <array>
#include <thread>
#include <type_traits>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>
static unsigned graphicsDialogLegacyFailIndex=0;static const char*graphicsDialogCheckpoint="legacy";static unsigned checks=0;static void ck(bool b,const char*w){nfs_hd_test::require(b,w);++checks;}
static unsigned drawCount=0,failAt=0,foreignTouches=0,callIndex=0,loseIndex=0;static EGLDisplay lossDisplay=EGL_NO_DISPLAY;static EGLSurface lossSurface=EGL_NO_SURFACE;static EGLContext lossContext=EGL_NO_CONTEXT;static bool recording=false,trackForeign=false;static std::vector<GLuint>programs;static std::vector<std::string>events;
static void afterCall(const char*name){if(!recording)return;events.push_back(name);if(++callIndex==loseIndex)ck(eglMakeCurrent(lossDisplay,lossSurface,lossSurface,lossContext),"injected actual owner loss");}
template<class F>static auto probe(const char*name,F f)->decltype(f()){if(trackForeign&&eglGetCurrentContext()==lossContext)++foreignTouches;if constexpr(std::is_void_v<decltype(f())>){f();afterCall(name);}else{auto value=f();afterCall(name);return value;}}
static void draw(GLenum mode,GLint first,GLsizei count){++drawCount;GLint p=0;glGetIntegerv(GL_CURRENT_PROGRAM,&p);programs.push_back(GLuint(p));GLboolean mask=GL_TRUE;glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);ck(!mask&&!glIsEnabled(GL_DEPTH_TEST),"every staged layer depth isolated");glDrawArrays(drawCount==failAt?GLenum(0xffffffff):mode,first,count);}
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
#include "hd/GraphicsSceneLayers.c.inc"
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
struct Resources {NfsBackgroundSceneCompositor bg{};NfsSoftwareTextGpu caption{},font{};std::vector<unsigned char>alpha;NfsSoftwareTextFontView fontView{};Resources(EGLContext,uint64_t generation):alpha(1024*512,128){fontView.schema=1;fontView.width=1024;fontView.height=512;fontView.alpha=alpha.data();fontView.bytes=alpha.size();fontView.identity=67;nfs_texture_sha256(alpha.data(),alpha.size(),fontView.sha256);ck(nfs_bg_scene_context_created(&bg,generation)&&nfs_bg_scene_prepare(&bg),"prepare synthetic background");ck(nfs_soft_gpu_context_created(&caption,generation)&&nfs_soft_gpu_prepare(&caption,&fontView)&&nfs_soft_gpu_context_created(&font,generation)&&nfs_soft_gpu_prepare(&font,&fontView),"prepare separate fonts");}NfsGraphicsSceneGpu view(){return {&bg,&caption,&font};}void retire(){nfs_bg_scene_context_retired(&bg,bg.generation,1);nfs_soft_gpu_context_retired(&caption,caption.generation,1);nfs_soft_gpu_context_retired(&font,font.generation,1);}};
#include "NestedCaptionWordGpuCases.h"
#include "GraphicsDialogWordGpuCases.h"
int main(){try{Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic provider");
 auto cap=text(35,30,70,50),full=text(480,150,520,175);
 {Resources r(egl.context,1);auto gpu=r.view();Registered owner{egl.context};NfsGraphicsSceneLayers v{};v.background=layers();v.sidebarCaption={343,480,r.fontView,cap.data(),cap.size()};v.text={640,480,r.fontView,full.data(),full.size()};
 for(unsigned h:{480u,1640u}){Target t(h*4/3,h),untouched(640,480);untouched.seed();const auto visible=untouched.read();t.seed();auto s=stage(t,owner);glEnable(GL_SCISSOR_TEST);glScissor(0,0,s.sampling.viewport[2],s.sampling.viewport[3]);glEnable(GL_DEPTH_TEST);glDepthMask(1);glEnable(GL_CULL_FACE);glEnable(GL_STENCIL_TEST);glColorMask(0,1,0,1);glActiveTexture(GL_TEXTURE1);auto state=capture();drawCount=0;programs.clear();std::array<NfsBackgroundSceneImageKey,2>keysBefore{};std::memcpy(keysBefore.data(),r.bg.keys,sizeof r.bg.keys);const auto lastSerial=r.bg.lastSerial;ck(nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v)&&equal(state,capture()),"complete scene restores inherited state");ck(drawCount==7&&programs[0]==r.bg.program&&programs[1]==r.bg.program&&programs[2]==r.bg.program&&programs[3]==r.bg.program&&programs[4]==r.bg.logo.program&&programs[5]==r.caption.program&&programs[6]==r.font.program,"exact BG words/logo caption dialog order");ck(r.bg.lastSerial==lastSerial&&!std::memcmp(keysBefore.data(),r.bg.keys,sizeof r.bg.keys),"existing Main Location cache and serial unchanged");auto image=t.read();pixel(t,image,600,300,{{20,40,60}});pixel(t,image,10,200,over({{20,40,60}},{{80,40,0,128}}));auto logo=over(over(over(over({{20,40,60}},{{80,40,0,128}}),{{0,60,0,128}}),{{60,0,0,128}},127),{{30,30,30,128}});pixel(t,image,50,40,over(logo,{{128,128,128,128}}));pixel(t,image,500,170,over({{20,40,60}},{{128,128,128,128}}));ck(untouched.read()==visible,"stage helper never publishes");depthUnchanged(t);depthUnchanged(untouched);
 nested_caption_word_gpu_cases::layers(gpu,v,nfs_graphics_scene_draw_layers_into_staging,t,s,image,{});
 graphics_dialog_word_gpu_cases::run(gpu,v,t,s);
 graphicsDialogCheckpoint="legacy:invalid-scene-preflight";t.seed();s=stage(t,owner);const auto clean=t.read();state=capture();for(unsigned f=0;f<21;++f){auto bad=v;auto badStage=s;auto invalidFull=full;if(f==0)bad.background.mapRole=2;if(f==1)bad.background.wordCount=5;if(f==2)bad.background.words[2].sourceRole=1;if(f==3)bad.background.logos[0].role=9;if(f==4)bad.sidebarCaption.logicalWidth=640;if(f==5)bad.text.logicalWidth=343;if(f==6)bad.text.font.identity++;if(f==7)bad.text.font.sha256[0]^=1;if(f==8)badStage.generation++;if(f==9)badStage.contextIdentity++;if(f==10)badStage.ownerCurrent=nullptr;if(f==11)bad.sidebarCaption.vertexCount=0;if(f==12)badStage.sampling.projection[0]*=2;if(f==13)badStage.width++;if(f==14)bad.sidebarCaption.font.bytes--;if(f==15)bad.text.vertexCount=5;if(f==16)bad.background.panelClip[2]=641;if(f==17){invalidFull[0].x=641;bad.text.vertices=invalidFull.data();}if(f==18){invalidFull[0].u=NAN;bad.text.vertices=invalidFull.data();}if(f==19){invalidFull[0].r=2;bad.text.vertices=invalidFull.data();}if(f==20)badStage.sampling.viewport[0]=1;drawCount=0;ck(!nfs_graphics_scene_draw_layers_into_staging(&gpu,&badStage,&bad)&&!drawCount&&equal(state,capture())&&t.read()==clean,"invalid scene rejects pre-draw");}
 const GLuint savedFontTexture=r.font.texture;r.font.texture=t.texture;drawCount=0;ck(!nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v)&&!drawCount&&t.read()==clean,"font texture aliases staging attachment rejected");r.font.texture=savedFontTexture;
 untouched.bind();auto unowned=s;unowned.framebuffer=untouched.fbo;state=capture();drawCount=0;ck(!nfs_graphics_scene_draw_layers_into_staging(&gpu,&unowned,&v)&&!drawCount&&equal(state,capture())&&untouched.read()==visible,"complete unregistered FBO rejected");s=stage(t,owner);glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);state=capture();drawCount=0;ck(!nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v)&&!drawCount&&equal(state,capture()),"clipping scissor rejects");glDisable(GL_SCISSOR_TEST);
 graphicsDialogCheckpoint="legacy:draw-error-original-target-read";for(unsigned n=1;n<=7;++n){graphicsDialogLegacyFailIndex=n;t.seed();s=stage(t,owner);state=capture();drawCount=0;failAt=n;ck(!nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v)&&equal(state,capture()),"each draw failure discards stage restores owner state");failAt=0;ck(untouched.read()==visible,"draw failure never publishes");}
 }graphicsDialogLegacyFailIndex=0;r.retire();}
 // Context-only resource preparation can temporarily bind another owned FBO.
 {Target original(640,480),temporary(640,480);original.seed();Registered owner{egl.context};auto t=stage(original,owner);t.ownerCurrent=contextOnlyCurrent;glActiveTexture(GL_TEXTURE1);const auto state=capture();const auto before=original.read();NfsBgSaved saved{};ck(nfs_graphics_scene_save(&saved,&t),"context-only snapshot");temporary.bind();glViewport(7,9,100,120);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,temporary.texture);glDepthMask(0);glEnable(GL_BLEND);ck(nfs_graphics_scene_restore(&saved,&t)&&equal(state,capture())&&original.read()==before,"context-only restore across changed FBO preserves original target");}
 // Every observed individual GL call is a real context-switch injection point,
 // including readiness, attachment queries, save, every layer and restoration.
 {Resources r(egl.context,1);auto gpu=r.view();Registered owner{egl.context};NfsGraphicsSceneLayers v{};v.background=layers();v.sidebarCaption={343,480,r.fontView,cap.data(),cap.size()};v.text={640,480,r.fontView,full.data(),full.size()};
 Target t(640,480),untouched(640,480);untouched.seed();const auto visible=untouched.read();t.seed();auto s=stage(t,owner);
 auto wordArt=nfs_compact_caption_display::artwork(3);v.sidebarWord=nested_caption_word_gpu_cases::word(*wordArt);NfsNestedCaptionGpu nested{};gpu.sidebarWord=&nested;nested_caption_word_gpu_cases::PrepareOwner nestedOwner{egl.context,1};auto nestedGuard=s;nestedGuard.framebuffer=0;nestedGuard.ownerCurrent=nested_caption_word_gpu_cases::PrepareOwner::current;nestedGuard.ownerUser=&nestedOwner;ck(nfs_nested_caption_prepare(&nested,&v.sidebarWord,3,&nestedGuard),"prepare adopted word for every-GL context-loss fixture");
 recording=true;callIndex=0;events.clear();drawCount=0;ck(nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v),"record complete guarded GL path");recording=false;const unsigned lossPoints=callIndex;
 EGLint attrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attrs);ck(lossContext!=EGL_NO_CONTEXT,"foreign context");
 const auto bg=r.bg;const auto caption=r.caption;const auto font=r.font;const auto nestedSaved=nested;
 for(unsigned loss=1;loss<=lossPoints;++loss){ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"foreign return");glViewport(3,4,79,93);glDepthMask(1);glActiveTexture(GL_TEXTURE1);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owner return");r.bg=bg;r.caption=caption;r.font=font;nested=nestedSaved;t.seed();s=stage(t,owner);drawCount=callIndex=foreignTouches=0;loseIndex=loss;recording=trackForeign=true;const int result=nfs_graphics_scene_draw_layers_into_staging(&gpu,&s,&v);recording=trackForeign=false;loseIndex=0;ck(!result&&foreignTouches==0&&equal(foreign,capture()),"loss causes zero foreign GL calls and no foreign state changes");ck(!r.bg.generation&&!r.caption.generation&&!r.font.generation&&!nested.generation&&!nested.program&&!nested.texture,"loss retires metadata without foreign deletes");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"return after loss");ck(untouched.read()==visible,"loss never publishes original target");}
 r.bg=bg;r.caption=caption;r.font=font;nested=nestedSaved;ck(nfs_nested_caption_retire(&nested,&nestedGuard,1),"retire exactly owned nested loss-fixture resources");ck(eglDestroyContext(egl.display,lossContext),"destroy foreign");lossContext=EGL_NO_CONTEXT;r.retire();std::printf("guarded_loss_points=%u ",lossPoints);
 }
 ck(glGetError()==GL_NO_ERROR,"final error state");std::printf("PASS Graphics ordered layers actual GLES checks=%u synthetic-only=1 no-publication=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
