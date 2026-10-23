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
#include "hd/GraphicsSceneLayers.c.inc"
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
struct Resources {NfsBackgroundSceneCompositor bg{};NfsSoftwareTextGpu caption{},font{};std::vector<unsigned char>alpha;NfsSoftwareTextFontView fontView{};Resources(EGLContext,uint64_t generation):alpha(1024*512,128){fontView.schema=1;fontView.width=1024;fontView.height=512;fontView.alpha=alpha.data();fontView.bytes=alpha.size();fontView.identity=67;nfs_texture_sha256(alpha.data(),alpha.size(),fontView.sha256);ck(nfs_bg_scene_context_created(&bg,generation)&&nfs_bg_scene_prepare(&bg),"prepare synthetic background");ck(nfs_soft_gpu_context_created(&caption,generation)&&nfs_soft_gpu_prepare(&caption,&fontView)&&nfs_soft_gpu_context_created(&font,generation)&&nfs_soft_gpu_prepare(&font,&fontView),"prepare separate fonts");}NfsGraphicsSceneGpu view(){return {&bg,&caption,&font};}void retire(){nfs_bg_scene_context_retired(&bg,bg.generation,1);nfs_soft_gpu_context_retired(&caption,caption.generation,1);nfs_soft_gpu_context_retired(&font,font.generation,1);}};
// Actual GLES mechanics and maintained Controls artwork. Source packet/proof is
// explicitly SYNTHETIC; no guest hooks, FIFO/native witness or scene activation.
#include "hd/ControlsDialogWordDisplay.h"
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
static void run(Pbuffer&egl,Resources&r,Target&t,Target&visible,Registered&owner){
 auto e=controls_synthetic::evidence(0);auto packet=controls_synthetic::packet(controls_synthetic::commands(0));auto proof=nfs_controls_text::TailScope::synthetic(packet,e);auto display=nfs_controls_dialog_display::Prepared::create(proof,packet,{e.form,e.authority.archive,e.game,e.serial,e.lifetime});ck(bool(display),"synthetic Controls sealed packet/display");const auto v=display->view();const auto&stream=packet->vertices();NfsSoftwareTextLayerView text{640,480,r.fontView,stream.data(),stream.size()};NfsControlsDialogWordGpu words{};auto s=stage(t,owner);auto guard=s;guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;const auto inherited=capture();
 ck(nfs_controls_dialog_valid(&v,&text),"actual five images and synthetic exact sixteen-command spans accepted");instrumentation();ck(nfs_controls_dialog_prepare(&words,&v,&text,&guard)&&equal(inherited,capture()),"cold Controls upload preserves complete GL state");recording=false;ck(eventCount("glTexImage2D")==5,"cold Controls uploads exactly five word textures");const auto saved=words;
 instrumentation();ck(nfs_controls_dialog_prepare(&words,&v,&text,&guard)&&nfs_controls_dialog_ready(&words,&v,&text,&guard),"warm Controls prepare/ready");recording=false;ck(!eventCount("glTexImage2D")&&!eventCount("glGenTextures")&&!eventCount("glCreateProgram")&&words.program==saved.program&&!std::memcmp(words.textures,saved.textures,sizeof saved.textures)&&equal(inherited,capture()),"warm Controls has no allocations/uploads or state changes");
 auto alternate=v;alternate.placements[1].color565=37904;alternate.placements[3].color565=65535;ck(nfs_controls_dialog_prepare(&words,&alternate,&text,&guard)&&!nfs_controls_dialog_ready(&words,&v,&text,&guard)&&nfs_controls_dialog_prepare(&words,&v,&text,&guard),"selection key changes preserve immutable image resources");
 NfsControlsDialogWordsView empty{};ck(nfs_controls_dialog_empty(&empty)&&nfs_controls_dialog_valid(&empty,nullptr),"exact zero view selects original text fallback");instrumentation();ck(nfs_controls_dialog_prepare(&words,&empty,nullptr,&guard),"empty prepare performs no replacement");recording=false;ck(events.empty(),"empty prepare has no helper GL");
 // A separate visible target is never passed to the mechanics helper.
 visible.seed();const auto published=visible.read();t.seed();s=stage(t,owner);const auto clean=t.read();auto before=capture();
 NfsBgSaved state{};ck(nfs_nested_caption_save(&state,&s),"save fallback state");ck(nfs_controls_dialog_vertices(r.font.program,r.font.texture,r.font.matrix,r.font.sampler,text.vertices,text.vertexCount,&s)==1,"caller-owned zero-view path renders all original synthetic font vertices");ck(nfs_nested_caption_restore(&state,&s)&&equal(before,capture()),"original-font fallback restores state");const auto original=t.read();ck(original!=clean,"original synthetic font fallback is visible");
 // Reference includes only the six original ellipsis spans. Finite label spans
 // are absent, then independently blended below using authored alpha/tints.
 t.seed();s=stage(t,owner);ck(nfs_nested_caption_save(&state,&s),"save ellipsis reference state");for(unsigned first:{2u,6u,10u}){const auto&a=packet->records()[first];const auto&b=packet->records()[first+1];ck(nfs_controls_dialog_vertices(r.font.program,r.font.texture,r.font.matrix,r.font.sampler,text.vertices+a.vertexFirst,a.vertexCount+b.vertexCount,&s)==1,"independent original paired-ellipsis reference");}ck(nfs_nested_caption_restore(&state,&s),"restore ellipsis reference state");const auto base=t.read();
 t.seed();s=stage(t,owner);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glColorMask(0,1,0,1);glEnable(GL_CULL_FACE);glActiveTexture(GL_TEXTURE1);before=capture();instrumentation();ck(nfs_controls_dialog_draw(&words,&v,&r.font,&text,&s)&&equal(before,capture()),"Controls ordered mechanics restores full inherited GL state");recording=false;
 ck(drawCount==13&&programs.size()==13&&controlsDrawCounts.size()==13,"ten word draws plus three original paired-ellipsis draws");unsigned at=0;for(unsigned i=0;i<5;++i){for(unsigned j=0;j<2;++j){ck(programs[at]==words.program&&controlsDrawCounts[at]==6,"exact source shadow then foreground word order");++at;}if(i<3){ck(programs[at]==r.font.program&&controlsDrawCounts[at]==36,"each intervening paired ellipsis remains original font");++at;}}ck(at==13,"complete original order consumed");
 const auto image=t.read();unsigned ink[5]={0},originalPixels=0;for(unsigned y=0;y<t.h;++y)for(unsigned x=0;x<t.w;++x){const double X=(x+.5)*480/t.h,Y=(y+.5)*480/t.h;const size_t offset=(size_t(t.h-1-y)*t.w+x)*4;bool affected=false;for(const auto&p:v.placements)affected|=X>=p.left&&X<p.right&&Y>=p.top&&Y<p.bottom;if(!affected){ck(!std::memcmp(image.data()+offset,base.data()+offset,4),"all pixels outside finite word boxes preserve original ellipses/background");if(std::memcmp(base.data()+offset,clean.data()+offset,4))++originalPixels;continue;}if(ellipsisPixel(stream,*packet,X,Y))continue;double rgb[3]={0,0,255};for(unsigned i=0;i<10;++i){const auto&p=v.placements[i];auto art=nfs_controls_dialog_display::artwork(p.word);const double A=alpha(*art,X,Y,p);unsigned R=p.color565>>11,G=(p.color565>>5)&63,B=p.color565&31;const double color[3]={double((R<<3)|(R>>2)),double((G<<2)|(G>>4)),double((B<<3)|(B>>2))};for(unsigned k=0;k<3;++k)rgb[k]=std::round(color[k]*A+rgb[k]*(1-A));if(A>.5&&i%2)++ink[p.word-1];}for(unsigned k=0;k<3;++k)ck(std::abs(int(image[offset+k])-int(rgb[k]))<=3,"independent actual-art bilinear shadow/foreground tint oracle");}for(unsigned n:ink)ck(n>20,"all five actual Controls images visibly render");ck(originalPixels>0,"preserved original ellipsis pixels visibly remain");ck(visible.read()==published,"isolated Controls mechanics never publishes");depthUnchanged(t);depthUnchanged(visible);
 t.seed();s=stage(t,owner);const auto untouched=t.read();before=capture();for(unsigned n=0;n<25;++n){auto bad=v;auto badText=text;switch(n){case 0:bad.schema=0;break;case 1:bad.count--;break;case 2:bad.images[0].word=2;break;case 3:bad.images[0].width--;break;case 4:bad.images[4].height--;break;case 5:bad.images[0].bytes--;break;case 6:bad.images[0].rgba=nullptr;break;case 7:bad.images[0].sha256[0]^=1;break;case 8:bad.placements[0].word=1;break;case 9:bad.placements[0].color565=65535;break;case 10:bad.placements[1].color565=18952;break;case 11:bad.placements[0].vertexFirst++;break;case 12:bad.placements[0].vertexCount-=6;break;case 13:std::swap(bad.placements[0],bad.placements[2]);break;case 14:bad.placements[9].vertexFirst=696;break;case 15:bad.placements[0].left++;break;case 16:bad.placements[0].right++;break;case 17:bad.placements[0].top=NAN;break;case 18:bad.placements[0].bottom++;break;case 19:bad.placements[9].color565=65535;break;case 20:bad.placements[3].color565=65535;break;case 21:bad.placements[1].color565=37904;break;case 22:badText.vertexCount=690;break;case 23:badText.vertices=nullptr;break;case 24:bad.schema=2;bad.images[4].rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));break;}
 instrumentation();ck(!nfs_controls_dialog_prepare(&words,&bad,&badText,&guard),"invalid Controls image/span/order/tint refuses");recording=false;ck(events.empty()&&!drawCount&&equal(before,capture())&&t.read()==untouched,"malformed preflight has zero helper GL and no state/target writes");}
 auto corrupted=*nfs_controls_dialog_display::artwork(5);corrupted.rgba[20]^=1;auto bad=v;bad.images[4].rgba=corrupted.rgba.data();instrumentation();ck(!nfs_controls_dialog_prepare(&words,&bad,&text,&guard),"changed actual image data refuses digest");recording=false;ck(events.empty(),"bad digest emits no helper GL");
 // Alias preflight is a caller prerequisite and covers all five image slots.
 for(unsigned i=0;i<5;++i){auto alias=words;alias.textures[i]=t.texture;ck(nfs_controls_dialog_alias(&alias,&r.bg,&r.caption,&r.font,nullptr,t.texture,0),"all five word-target aliases refused");}ck(!nfs_controls_dialog_alias(&words,&r.bg,&r.caption,&r.font,nullptr,t.texture,0),"separate Controls resources are not aliases");
 for(unsigned failure:{1u,3u,12u,13u}){t.seed();s=stage(t,owner);before=capture();drawCount=0;failAt=failure;ck(!nfs_controls_dialog_draw(&words,&v,&r.font,&text,&s)&&equal(before,capture()),"representative word/ellipsis late errors refuse whole staging result");failAt=0;ck(visible.read()==published,"late staging failure never publishes partial Controls output");}
 ck(nfs_controls_dialog_retire(&words,&guard,1)&&!words.program&&!words.generation&&!glIsProgram(saved.program),"owned Controls retirement clears program/generation");for(GLuint name:saved.textures)ck(!glIsTexture(name),"owned Controls retirement deletes each image");ck(glIsTexture(r.font.texture),"owned retirement preserves original font");
 owner.generation=2;s=stage(t,owner);guard=s;guard.framebuffer=0;guard.ownerCurrent=contextOnlyCurrent;instrumentation();ck(nfs_controls_dialog_prepare(&words,&v,&text,&guard)&&words.generation==2,"generation-two neutral context lifecycle reuploads fresh words");recording=false;ck(eventCount("glTexImage2D")==5,"new generation uploads all five actual images");auto stale=guard;stale.generation=1;instrumentation();ck(!nfs_controls_dialog_ready(&words,&v,&text,&stale),"stale generation refuses");recording=false;ck(events.empty(),"stale generation issues no helper GL");ck(nfs_controls_dialog_retire(&words,&guard,1),"retire generation-two word resources");owner.generation=1;
}
}
int main(){try{Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic background provider");{Resources resources(egl.context,1);Target target(640,480),published(640,480);Registered owner{egl.context};controls_gpu_test::run(egl,resources,target,published,owner);resources.retire();}ck(glGetError()==GL_NO_ERROR,"final Controls GL error");std::printf("PASS Controls isolated actual GLES mechanics checks=%u source-proof=SYNTHETIC no-native-consumer=1 no-publication=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL Controls isolated GLES %s checks=%u\n",e.what(),checks);return 1;}}