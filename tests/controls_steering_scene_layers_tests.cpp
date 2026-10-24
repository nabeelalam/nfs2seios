// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include "TextureDigest.h"
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
// The unchanged shared OPTIONS helper validates its pixels on draw. Track that
// inherited work separately; new Steering words/bar must never rehash pixels.
static unsigned stageHashes=0,captionHashes=0;static const void*captionPixels=nullptr;
static int stageDigest(const void*p,size_t n,unsigned char*out){if(n>=84u*128u*4u){if(p==captionPixels)++captionHashes;else ++stageHashes;}return nfs_texture_sha256(p,n,out);}
#define nfs_texture_sha256 stageDigest
#include "hd/ControlsSteeringSceneLayers.c.inc"
#undef nfs_texture_sha256

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
struct Resources {NfsBackgroundSceneCompositor bg{};NfsSoftwareTextGpu caption{},font{};std::vector<unsigned char>alpha;NfsSoftwareTextFontView fontView{};Resources(EGLContext,uint64_t generation):alpha(1024*512,128){fontView.schema=1;fontView.width=1024;fontView.height=512;fontView.alpha=alpha.data();fontView.bytes=alpha.size();fontView.identity=67;nfs_texture_sha256(alpha.data(),alpha.size(),fontView.sha256);ck(nfs_bg_scene_context_created(&bg,generation)&&nfs_bg_scene_prepare(&bg),"prepare synthetic background");ck(nfs_soft_gpu_context_created(&caption,generation)&&nfs_soft_gpu_prepare(&caption,&fontView)&&nfs_soft_gpu_context_created(&font,generation)&&nfs_soft_gpu_prepare(&font,&fontView),"prepare separate fonts");}void retire(){nfs_bg_scene_context_retired(&bg,bg.generation,1);nfs_soft_gpu_context_retired(&caption,caption.generation,1);nfs_soft_gpu_context_retired(&font,font.generation,1);}};


#include "hd/ControlsSteeringWordArtwork.h"
#include "hd/ControlsSteeringBarArtwork.h"
#include "hd/CompactCaptionWordArtwork.h"
static NfsControlsSteeringWordsView words(){NfsControlsSteeringWordsView v{};v.schema=1;v.count=2;for(unsigned i=0;i<8;++i){auto a=nfs_controls_steering_word_display::artwork(i+1);ck(bool(a),"word art");auto&im=v.images[i];im.word=i+1;im.width=a->width;im.height=a->height;im.rgba=a->rgba.data();im.bytes=a->rgba.size();std::copy(a->sha.begin(),a->sha.end(),im.sha256);}v.placements[0]={1,65535,0,6,369,144,435,176};v.placements[1]={5,63077,12,6,454,144,475,176};return v;}
static NfsCompactCaptionWordView options(){auto art=nfs_compact_caption_display::artwork(3);ck(bool(art),"OPTIONS art");NfsCompactCaptionWordView v{};v.schema=1;v.word=3;v.width=192;v.height=128;v.rgba=art->rgba.data();v.bytes=art->rgba.size();std::copy(art->sha.begin(),art->sha.end(),v.sha256);v.left=120;v.top=364;v.right=168;v.bottom=396;return v;}
static void resetRecording(){drawCount=callIndex=stageHashes=captionHashes=0;programs.clear();controlsDrawCounts.clear();events.clear();failAt=loseIndex=0;}
// Independent bilinear premultiplied sample at an actual native pixel center.
static std::array<int,3>barPixel(const NfsControlsSteeringBarView&v,const Target&t,unsigned x,unsigned y){
 unsigned px=unsigned((x+.5)*t.h/480.),py=t.h-1-unsigned((y+.5)*t.h/480.);double X=(px+.5)*480./t.h,Y=(t.h-py-.5)*480./t.h;
 double u=std::max(0.,std::min(1423.,(X-280)*4-.5)),w=std::max(0.,std::min(195.,(Y-280)*4-.5));unsigned a=unsigned(u),b=unsigned(w),c=std::min(a+1,1423u),d=std::min(b+1,195u);double fx=u-a,fy=w-b;std::array<double,4>sample{};
 for(unsigned k=0;k<4;++k){auto at=[&](unsigned i,unsigned j){return double(v.rgba[(size_t(j)*1424+i)*4+k]);};sample[k]=(at(a,b)*(1-fx)+at(c,b)*fx)*(1-fy)+(at(a,d)*(1-fx)+at(c,d)*fx)*fy;}
 std::array<int,3>rgb{};const int map[]={20,40,60};for(unsigned k=0;k<3;++k)rgb[k]=int(std::lround(sample[k]+map[k]*(1.-sample[3]/255.)));return rgb;
}
static void run(Pbuffer&egl,unsigned height){
 Resources resources(egl.context,1);Target target(height*4/3,height),published(640,480);published.seed();const auto original=published.read();Registered owner{egl.context};auto st=stage(target,owner),guard=st;guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;
 NfsNestedCaptionGpu caption{};NfsControlsSteeringWordGpu child{};NfsControlsSteeringBarGpu bar{};auto captionView=options();captionPixels=captionView.rgba;auto wordView=words();
 auto captionVertices=text(120,364,168,396);std::vector<NfsSoftwareTextVertex>vertices;for(auto box:{std::array<float,4>{369,144,435,176},std::array<float,4>{443,144,447,176},std::array<float,4>{454,144,475,176}}){auto q=text(box[0],box[1],box[2],box[3]);vertices.insert(vertices.end(),q.begin(),q.end());}
 NfsControlsSteeringSceneLayers view{};view.background=layers();view.sidebarCaption={343,480,resources.fontView,captionVertices.data(),captionVertices.size()};view.text={640,480,resources.fontView,vertices.data(),vertices.size()};view.sidebarWord=captionView;view.childWords=wordView;
 NfsControlsSteeringSceneGpu gpu{&resources.bg,&resources.caption,&resources.font,&caption,&child,&bar};
 ck(nfs_nested_caption_prepare(&caption,&captionView,3,&guard),"prepare mandatory OPTIONS");ck(nfs_controls_steering_word_prepare(&child,&wordView,&view.text,&guard),"prepare finite words");
 for(unsigned level=0;level<4;++level){auto art=nfs_controls_steering_bar::artwork(level);ck(bool(art),"bar art");auto v=art->view();ck(nfs_controls_steering_bar_gpu_prepare(&bar,&v,&guard),"prepare all four independent bar levels");}
 view.chart=nfs_controls_steering_bar::artwork(0)->view();
 // A fresh corrupt payload refuses before publishing a resource or changing GL.
 {auto bad=view.chart;std::vector<unsigned char>corrupt(bad.rgba,bad.rgba+bad.bytes);corrupt[corrupt.size()/2]^=1;bad.rgba=corrupt.data();NfsControlsSteeringBarGpu fresh{};auto before=capture();ck(!nfs_controls_steering_bar_gpu_prepare(&fresh,&bad,&guard)&&!fresh.program&&!fresh.generation&&equal(before,capture()),"cold corrupt bar never admitted");}
 for(unsigned level=0;level<4;++level){view.chart=nfs_controls_steering_bar::artwork(level)->view();target.seed();st=stage(target,owner);glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_GREATER);glEnable(GL_SCISSOR_TEST);glScissor(0,0,target.w,target.h);glColorMask(0,1,0,1);glActiveTexture(GL_TEXTURE1);auto before=capture();resetRecording();recording=true;
  ck(nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view),"complete prepared steering composition");recording=false;ck(equal(before,capture()),"all inherited GL state restored");ck(stageHashes==0,"staging never hashes new Steering artwork pixels");ck(captionHashes==3,"inherited OPTIONS validation remains explicitly accounted for");ck(drawCount==10,"map panel two words logos OPTIONS authored source words colon and bar");
  ck(programs[5]==caption.program&&programs[6]==child.program&&programs[7]==resources.font.program&&programs[8]==child.program&&programs[9]==bar.program,"OPTIONS then authored shadow/foreground-order text with original colon then bar LAST");
  for(const auto&e:events)ck(e!="glTexImage2D"&&e!="glGenTextures"&&e!="glCreateProgram"&&e!="glGenFramebuffers","staging does not allocate or upload");
  const auto actual=target.read();pixel(target,actual,600,100,{20,40,60});pixel(target,actual,430,305,barPixel(view.chart,target,430,305));pixel(target,actual,444,150,over({20,40,60},{128,128,128,128}));depthUnchanged(target);ck(published.read()==original,"complete staging never modifies original publication target");
 }
 view.chart=nfs_controls_steering_bar::artwork(0)->view();
 // Every other sampled resource is forbidden as a bar texture, including all
 // nonselected bar levels, both fonts and every finite word texture.
 std::vector<GLuint>aliases={resources.bg.logo.texture,resources.caption.texture,resources.font.texture,caption.texture,target.texture};for(auto name:resources.bg.textures)aliases.push_back(name);for(auto name:child.textures)aliases.push_back(name);
 for(GLuint alias:aliases){auto saved=bar;bar.textures[3]=alias;target.seed();st=stage(target,owner);auto before=capture();resetRecording();ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view)&&!drawCount&&equal(before,capture()),"bar texture cross-alias refuses before draw");bar=saved;}
 for(GLuint alias:{resources.bg.program,resources.bg.copyProgram,resources.bg.logo.program,resources.caption.program,resources.font.program,caption.program,child.program}){auto saved=bar;bar.program=alias;target.seed();st=stage(target,owner);resetRecording();ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view)&&!drawCount,"bar program cross-alias refuses");bar=saved;}
 for(unsigned mode=0;mode<5;++mode){auto bad=view;auto barSaved=bar;if(mode==0)bad.sidebarWord={};if(mode==1)bad.chart.level=4;if(mode==2)bar.generation++;if(mode==3)bar.contextIdentity++;if(mode==4)bad.text.font.identity++;target.seed();st=stage(target,owner);resetRecording();ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&bad)&&!drawCount,"missing caption invalid chart and stale font/bar metadata refuse");bar=barSaved;}
 target.seed();st=stage(target,owner);glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);auto before=capture();resetRecording();ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view)&&!drawCount&&equal(before,capture()),"clipping scissor refuses");glDisable(GL_SCISSOR_TEST);
 for(unsigned failure:{1u,7u,10u}){target.seed();st=stage(target,owner);before=capture();resetRecording();failAt=failure;ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view)&&equal(before,capture()),"late draw error restores GL and discards complete staged frame");failAt=0;ck(published.read()==original,"late refusal preserves original publication");}
 // Actual context loss is injected after the first and last staged draw. No
 // restore/delete/query may subsequently touch the foreign owner's context.
 target.seed();st=stage(target,owner);resetRecording();recording=true;ck(nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view),"record guarded GL event positions");recording=false;std::vector<unsigned>drawPoints;for(unsigned i=0;i<events.size();++i)if(events[i]=="glDrawArrays")drawPoints.push_back(i+1);ck(drawPoints.size()==10,"ten guarded draw events");
 const auto bgSaved=resources.bg;const auto fontSaved=resources.font,captionFontSaved=resources.caption;const auto captionSaved=caption;const auto childSaved=child;const auto barSaved=bar;
 for(unsigned point:{drawPoints.front(),drawPoints.back()}){EGLint attrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attrs);ck(lossContext!=EGL_NO_CONTEXT&&eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"create actual unshared foreign context");glViewport(3,4,70,90);glActiveTexture(GL_TEXTURE1);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"restore owner");resources.bg=bgSaved;resources.font=fontSaved;resources.caption=captionFontSaved;caption=captionSaved;child=childSaved;bar=barSaved;target.seed();st=stage(target,owner);resetRecording();foreignTouches=0;loseIndex=point;recording=trackForeign=true;ck(!nfs_controls_steering_scene_draw_layers_into_staging(&gpu,&st,&view),"owner loss refuses composition");recording=trackForeign=false;loseIndex=0;ck(!foreignTouches&&equal(foreign,capture())&&!resources.bg.generation&&!resources.font.generation&&!resources.caption.generation&&!caption.generation&&!child.generation&&!bar.generation,"owner loss forgets all metadata without foreign GL");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"restore owner after loss");ck(eglDestroyContext(egl.display,lossContext),"destroy foreign context");lossContext=EGL_NO_CONTEXT;}
 resources.bg=bgSaved;resources.font=fontSaved;resources.caption=captionFontSaved;caption=captionSaved;child=childSaved;bar=barSaved;glUseProgram(0);guard=stage(target,owner);guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;nfs_controls_steering_bar_gpu_retire(&bar,&guard,1);ck(nfs_controls_steering_word_retire(&child,&guard,1)&&nfs_nested_caption_retire(&caption,&guard,1),"retire distinct authored resources");resources.retire();ck(glGetError()==GL_NO_ERROR,"composition leaves no GL errors");
}
int main(){try{Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic neutral art provider");for(unsigned height:{480u,1640u})run(egl,height);std::printf("PASS steering scene layers GLES checks=%u source=SYNTHETIC native-authority=none no-publication=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL steering scene layers: %s checks=%u\n",e.what(),checks);return 1;}}
