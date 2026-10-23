// SPDX-License-Identifier: MIT
/* Actual Intro adapter with synthetic art and REAL native-array GLES draws.
   The original game dispatcher/SDL backend is simulated; this fixture proves
   GPU pixels/state and presentation selection, not original-game execution. */
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
static void drawInjected(GLenum mode,GLint first,GLsizei count){GLboolean depth=GL_TRUE;::glGetBooleanv(GL_DEPTH_WRITEMASK,&depth);ck(!depth,"every optional stage/mirror draw keeps depth writes disabled");++drawCalls;::glDrawArrays(drawCalls==failDraw?GLenum(0xffffffff):mode,first,count);}
static const unsigned TextureMem=0x200000u;
struct TextureInfo{GLuint id;unsigned fmt,size,hdDigits;void*data;};static TextureInfo g_textures[TextureMem/4]{};static unsigned char textureBytes[TextureMem];static unsigned char*g_textureMem=textureBytes;
struct GrTexInfo{unsigned smallLod,largeLod,aspectRatio,format;};
struct GrVertex{float x,y,r,g,b,a,oow;struct{float sow,tow;}tmuvtx[2];};
static EGLContext g_glCtx=EGL_NO_CONTEXT;static GLuint g_framebuffer=0,g_framebufferTexture=0,g_framebufferDepth=0,g_shaderProgram=0;
static int g_framebufferHeight=480;static const int GR_BUFFER_BACKBUFFER=1;static int g_renderBuffer=GR_BUFFER_BACKBUFFER;static bool g_textureEnabled=true,g_fogEnabled=false,g_depthMask=true,linearFiltering=true;static GLfloat g_matrix[16];
static NfsLogoProvenance nfs_logo_provenance{};static unsigned originalFlushCalls=0;
static const unsigned MaxTriangles=1024;static unsigned g_trianglesCount=0;
struct Vertices{struct{float x,y,z;}vertex[3];};struct TextureCoord{struct{float s,t,r,q;}vertex[3];};struct ColorValues{struct{unsigned char r,g,b,a;}vertex[3];};
static Vertices g_vertices[MaxTriangles];static TextureCoord g_textureCoord[MaxTriangles];static ColorValues g_colorValues[MaxTriangles];
static GLint g_aPositionLoc=0,g_aTexCoordLoc=1,g_aColorLoc=2,nativeMatrix=-1,nativeTone=-1;
static float queuedTone=1;static unsigned actualNativeDraws=0;
static void drawTriangles();
static EGLContext SDL_GL_GetCurrentContext(){return eglGetCurrentContext();}
static uintptr_t testThread(){return uintptr_t(std::hash<std::thread::id>{}(std::this_thread::get_id()))|uintptr_t(1);}
#define NFS_GRAPHICS_SCENE_THREAD_TOKEN() testThread()
#define NFS_OPTIONS_SCENE_THREAD_TOKEN() testThread()
#define NFS_PLAYER_SCENE_THREAD_TOKEN() testThread()
#define NFS_PLAYER_INTRO_THREAD_TOKEN() testThread()
static const unsigned GR_TEXFMT_ARGB_1555=11;
struct NfsLogoUploadWitness{uint64_t context,revision;uint32_t address,id;unsigned valid;unsigned char bytes[2048];};
static NfsLogoUploadWitness nfs_logo_uploads[1024];
static int nfs_logo_native_upload_valid(uint32_t address,const TextureInfo*ti){for(unsigned i=0;i<1024;++i){const auto&w=nfs_logo_uploads[i];if(w.valid&&w.address==address&&w.id==ti->id&&w.context==nfs_logo_provenance.context&&w.revision&&w.revision<=nfs_logo_provenance.uploads&&!std::memcmp(w.bytes,ti->data,2048))return 1;}return 0;}
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
#define glCopyTexSubImage2D(...) probe("glCopyTexSubImage2D",[&](){return ::glCopyTexSubImage2D(__VA_ARGS__);})
#define glFramebufferRenderbuffer(...) probe("glFramebufferRenderbuffer",[&](){return ::glFramebufferRenderbuffer(__VA_ARGS__);})
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
#define glGetTexParameteriv(...) probe("glGetTexParameteriv",[&](){return ::glGetTexParameteriv(__VA_ARGS__);})
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
#include "hd/OptionsSceneRenderer.inc"
#include "hd/PlayerSceneRenderer.inc"
#include "hd/PlayerIntroRenderer.inc"
#undef glCopyTexSubImage2D
#undef glFramebufferRenderbuffer
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
#undef glGetTexParameteriv
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
struct Input{
 std::vector<unsigned char>alpha,mask;std::vector<uint16_t>plane,preview;std::array<NfsSoftwareTextVertex,6>caption;NfsSoftwareTextFontView font{};
 Input():alpha(1024*512,128),mask(300*310,255),plane(640*480),preview(300*310,0xffe0),caption(text(220,325,240,340)){
  for(size_t i=0;i<plane.size();++i)plane[i]=uint16_t(i*79u);for(unsigned y=298;y<303;++y)for(unsigned x=448;x<453;++x)mask[(y-140)*300+x-200]=0;
  for(unsigned y=141;y<163;++y)for(unsigned x=366;x<451;++x)preview[(y-140)*300+x-200]=0xf800;
  font.schema=1;font.width=1024;font.height=512;font.alpha=alpha.data();font.bytes=alpha.size();font.identity=67;ck(nfs_texture_sha256(alpha.data(),alpha.size(),font.sha256),"synthetic font SHA");
 }
 NfsOriginalPreviewLayerView previewView(){NfsOriginalPreviewLayerView v{};v.schema=1;v.left=200;v.top=140;v.width=300;v.height=310;v.pixels565=preview.data();v.pixelCount=preview.size();v.coverage=mask.data();v.coverageCount=mask.size();ck(nfs_texture_sha256((const unsigned char*)preview.data(),preview.size()*2,v.sha256),"synthetic preview565SHA");ck(nfs_texture_sha256(mask.data(),mask.size(),v.coverageSha256),"synthetic previewcoverageSHA");return v;}
 NfsPlayerSceneFrameView view(uint64_t serial,unsigned player=1){NfsPlayerSceneFrameView v{};v.schema=1;v.form=player==1?0x4d3050:0x4d3100;auto&s=v.scene;s.schema=1;s.width=640;s.height=480;s.serial=s.contentRevision=serial;s.tailKind=player==1?5:6;s.planeWords=plane.size();s.original565=plane.data();auto b=layers();s.mapRole=b.mapRole;s.wordCount=b.wordCount;std::memcpy(s.panelClip,b.panelClip,sizeof s.panelClip);std::memcpy(s.words,b.words,sizeof s.words);std::memcpy(s.logos,b.logos,sizeof s.logos);for(unsigned i=0;i<300;++i)s.rendererAddress[i]=i*2048;v.caption={343,480,font,caption.data(),caption.size()};v.preview=previewView();return v;}
};
static std::array<int,3>oracle(Input&i,unsigned x,unsigned y){auto b=layers();std::array<int,3>rgb{{20,40,60}};if(x>=200&&x<500&&y>=140&&y<450){size_t at=(y-140)*300+x-200;if(i.mask[at]){uint16_t p=i.preview[at];rgb={{int(((p>>11)&31)*255/31),int(((p>>5)&63)*255/63),int((p&31)*255/31)}};}}if(x<343)rgb=over(rgb,{{80,40,0,128}});for(unsigned k=0;k<b.wordCount;++k){const auto&w=b.words[k];if(x>=unsigned(w.left)&&x<unsigned(w.left+16)&&y>=unsigned(w.top)&&y<unsigned(w.top+12))rgb=over(rgb,w.sourceRole%2?std::array<int,4>{{0,64,0,128}}:std::array<int,4>{{64,0,0,128}},w.scalar256);}for(const auto&l:b.logos)if(x>=unsigned(l.left)&&x<unsigned(l.right)&&y>=unsigned(l.top)&&y<unsigned(l.bottom)){std::array<int,4>c{{l.role==0?30:l.role==2?60:0,l.role==0?30:l.role==1?60:0,l.role==0?30:0,128}};rgb=over(rgb,c,l.scalar256);}if(x>=220&&x<240&&y>=325&&y<340)rgb=over(rgb,{{128,128,128,128}});return rgb;}
static GLuint originalProgram(){const char*vs="attribute vec2 vertex;void main(){gl_Position=vec4(vertex,0,1);}";const char*fs="precision mediump float;void main(){gl_FragColor=vec4(0,0,1,1);}";GLuint v=compileShader(GL_VERTEX_SHADER,vs),f=compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glBindAttribLocation(p,0,"vertex");glLinkProgram(p);glDeleteShader(v);glDeleteShader(f);return p;}
static void originalState(Target&t){t.bind();NfsFrameLegacySampling sampling{};ck(nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&sampling),"original sampling");std::memcpy(g_matrix,sampling.projection,sizeof g_matrix);glViewport(0,0,sampling.viewport[2],sampling.viewport[3]);glUseProgram(g_shaderProgram);glDisable(GL_CULL_FACE);glDisable(GL_STENCIL_TEST);glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_LEQUAL);glEnable(GL_SCISSOR_TEST);glScissor(0,0,t.w,t.h);glColorMask(1,1,1,1);glActiveTexture(GL_TEXTURE1);g_textureEnabled=true;g_fogEnabled=false;g_depthMask=true;g_renderBuffer=GR_BUFFER_BACKBUFFER;}
static void frameStart(Target&t){t.seed();glClearDepthf(1);glClear(GL_DEPTH_BUFFER_BIT);originalState(t);nfs_logo_p_clear(&nfs_logo_provenance,t.fbo,1,1,65535);}
static void nativeCache(Input&input,std::array<GLuint,300>&ids){glGenTextures(300,ids.data());glActiveTexture(GL_TEXTURE0);for(unsigned tile=0;tile<300;++tile){unsigned address=tile*2048;auto*bytes=textureBytes+address;for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t source=input.plane[((tile%15)*32+y)*640+(tile/15)*32+x],a1555=uint16_t(((source&0xf800)>>1)|((source&0x7c0)>>1)|(source&31));a1555=a1555?uint16_t(a1555|0x8000):0;const uint16_t converted=uint16_t((a1555<<1)|(a1555>>15));std::memcpy(bytes+2*(y*32+x),&converted,2);}g_textures[address>>2]={ids[tile],11,32,0,bytes};auto&w=nfs_logo_uploads[tile];w.context=nfs_logo_provenance.context;w.revision=tile+1;w.address=address;w.id=ids[tile];w.valid=1;std::memcpy(w.bytes,bytes,2048);glBindTexture(GL_TEXTURE_2D,ids[tile]);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_SHORT_5_5_5_1,bytes);}nfs_logo_provenance.uploads=300;ck(glGetError()==GL_NO_ERROR,"300actual synthetic cachedARGB1555 GLtextures");}
/* Synthetic original-call observations feed the actual shared production
   NfsLogoProvenance validators. The original drawTriangles backend is a no-GL
   stub, so this is not original renderer execution/coverage. */
static void observeCached(bool incomplete=false,bool badSelection=false){for(unsigned tile=0;tile<300;++tile){const unsigned address=tile*2048;const auto&ti=g_textures[address>>2];nfs_logo_p_select(&nfs_logo_provenance,badSelection&&tile==0?1:0,address,ti.id,ti.fmt,ti.size);nfs_logo_p_triangle(&nfs_logo_provenance,1);if(!incomplete||tile!=299)nfs_logo_p_triangle(&nfs_logo_provenance,1);}}
static void resetTrace(){events.clear();callIndex=loseIndex=foreignTouches=drawCalls=0;recording=true;}
template<class Run,class Reset>static unsigned lossFamily(Pbuffer&egl,Target&target,const char*label,Run run,Reset reset,bool writesColor=false){reset();originalState(target);const auto before=target.read();resetTrace();ck(run(),"record actual optional Player helper");recording=false;const unsigned points=callIndex;ck(points>0,"actual helper emitted GL calls");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"foreign baseline");glViewport(3,5,73,91);glActiveTexture(GL_TEXTURE1);const auto foreign=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"owner baseline return");for(unsigned loss=1;loss<=points;++loss){reset();originalState(target);resetTrace();loseIndex=loss;trackForeign=true;const int result=run();recording=trackForeign=false;loseIndex=0;ck(!result&&foreignTouches==0&&eglGetCurrentContext()==lossContext&&equal(foreign,capture()),"actual optional helper stops all foreign GL operations");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"recover actual owner");while(glGetError()!=GL_NO_ERROR){}if(!writesColor)ck(target.read()==before,"resource/query helper never modifies original color");}reset();std::printf("%s_loss_points=%u ",label,points);return points;}

/* Native flush draws the same real CPU arrays inspected by the production
   adapter. Simulated mark-uniform reset deliberately occurs AFTER mirroring. */
static void drawTriangles(){if(!g_trianglesCount)return;++originalFlushCalls;glUniform1f(nativeTone,queuedTone);::glDrawArrays(GL_TRIANGLES,0,GLsizei(g_trianglesCount*3));++actualNativeDraws;nfs_player_intro_renderer_tail_batch(g_trianglesCount);glUniform1f(nativeTone,1);g_trianglesCount=0;queuedTone=1;}
static GLuint nativeProgram(){const char*vs="attribute vec3 aPosition;attribute vec4 aTexCoord;attribute vec4 aColor;uniform mat4 matrix;varying vec2 uv;varying vec4 color;void main(){uv=aTexCoord.xy;color=aColor;gl_Position=matrix*vec4(aPosition,1);}";const char*fs="precision highp float;uniform sampler2D image;uniform float tone;varying vec2 uv;varying vec4 color;void main(){vec4 p=texture2D(image,uv);if(p.a<0.5)discard;gl_FragColor=vec4(p.rgb*color.rgb*tone,p.a*color.a);}";GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs),program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glBindAttribLocation(program,0,"aPosition");glBindAttribLocation(program,1,"aTexCoord");glBindAttribLocation(program,2,"aColor");glLinkProgram(program);GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);ck(ok,"native fixture shader links");glDeleteShader(a);glDeleteShader(b);nativeMatrix=glGetUniformLocation(program,"matrix");nativeTone=glGetUniformLocation(program,"tone");glUseProgram(program);glUniform1i(glGetUniformLocation(program,"image"),0);glUniform1f(nativeTone,1);return program;}
static void nativeState(Target&t,GLuint depth){g_framebuffer=t.fbo;g_framebufferTexture=t.texture;g_framebufferDepth=depth;g_framebufferHeight=int(t.h);originalState(t);glDisable(GL_POLYGON_OFFSET_FILL);glDisable(GL_DITHER);glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glActiveTexture(GL_TEXTURE0);glBindBuffer(GL_ARRAY_BUFFER,0);glUseProgram(g_shaderProgram);glUniformMatrix4fv(nativeMatrix,1,GL_FALSE,g_matrix);glUniform1f(nativeTone,1);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,g_vertices);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,g_textureCoord);glVertexAttribPointer(2,4,GL_UNSIGNED_BYTE,GL_TRUE,0,g_colorValues);for(unsigned i=0;i<3;++i)glEnableVertexAttribArray(i);glDisableVertexAttribArray(3);queuedTone=1;ck(glGetError()==GL_NO_ERROR,"native array state");}
static void queueVertex(const GrVertex&v,unsigned tri,unsigned corner){g_vertices[tri].vertex[corner]={v.x-786432.f,v.y-786432.f,v.oow};g_textureCoord[tri].vertex[corner]={v.tmuvtx[0].sow/256.f,v.tmuvtx[0].tow/256.f,0,v.oow};g_colorValues[tri].vertex[corner]={static_cast<unsigned char>(v.r),static_cast<unsigned char>(v.g),static_cast<unsigned char>(v.b),static_cast<unsigned char>(v.a)};}
static void nativeTriangle(const GrVertex&a,const GrVertex&b,const GrVertex&c,bool observe){if(observe)nfs_player_intro_renderer_triangle(&a,&b,&c);ck(g_trianglesCount<MaxTriangles,"bounded native batch");queueVertex(a,g_trianglesCount,0);queueVertex(b,g_trianglesCount,1);queueVertex(c,g_trianglesCount,2);++g_trianglesCount;}
static GrVertex corner(unsigned tile,unsigned which){GrVertex v{};v.x=float(tile/15*32+((which==1||which==2)?32:0))+786432.f;v.y=float(tile%15*32+(which>=2?32:0))+786432.f;v.r=v.g=v.b=v.a=255;v.oow=1;v.tmuvtx[0].sow=(which==1||which==2)?255:0;v.tmuvtx[0].tow=which>=2?255:0;return v;}
static void nativeSlot0(bool observe,unsigned tiles=300){GrTexInfo info{3,3,3,11};for(unsigned tile=0;tile<tiles;++tile){drawTriangles();unsigned address=tile*2048;auto*ti=&g_textures[address>>2];glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,ti->id);if(observe)nfs_player_intro_renderer_select(0,address,3,&info,ti);auto a=corner(tile,0),b=corner(tile,1),c=corner(tile,2),d=corner(tile,3);nativeTriangle(a,b,c,observe);nativeTriangle(a,c,d,observe);}}
static void nativeTail(GLuint white,bool observe,bool bad=false){drawTriangles();glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,white);GrVertex v[4]{};for(unsigned i=0;i<4;++i){v[i].x=786432.f+((i==1||i==2)?240:220);v[i].y=786432.f+(i>=2?340:320);v[i].g=255;v[i].a=128;v[i].oow=bad?.5f:1.f;}nativeTriangle(v[0],v[1],v[2],observe);nativeTriangle(v[0],v[2],v[3],observe);queuedTone=.75f;}
static NfsPlayerIntroFrameView introView(Input&i,uint64_t serial,unsigned player){auto inherited=i.view(17);NfsPlayerIntroFrameView v{};v.schema=1;v.form=player==1?0x4d3050:0x4d3100;v.requestSerial=serial;v.uploadEvent=9;v.sourceOwner=0x1800;v.sourceBitmap=0x40000;v.inheritedMain=inherited.scene;v.inheritedMain.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;v.caption=inherited.caption;v.paddedOriginal565=i.plane.data();v.wordsCount=i.plane.size();std::copy(std::begin(inherited.scene.rendererAddress),std::end(inherited.scene.rendererAddress),v.keys);return v;}
static std::vector<unsigned char>stageImage(){std::vector<unsigned char>v(size_t(nfs_player_intro_renderer.width)*nfs_player_intro_renderer.height*4);glBindFramebuffer(GL_FRAMEBUFFER,nfs_player_intro_renderer.fbo);glReadPixels(0,0,nfs_player_intro_renderer.width,nfs_player_intro_renderer.height,GL_RGBA,GL_UNSIGNED_BYTE,v.data());ck(glGetError()==GL_NO_ERROR,"read actual display-only stage");return v;}
// Nonwriting depth witness: a separate COLOR attachment uses the native depth
// renderbuffer. A green .5 probe exposes its reject/pass mask without touching
// native COLOR or DEPTH. This is stronger than inspecting clear-depth state.
static std::vector<unsigned char>depthImage(Target&t,GLuint depth){Target probeTarget(t.w,t.h);probeTarget.bind();glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(1,0,0,1);glClear(GL_COLOR_BUFFER_BIT);const char*vs="attribute vec2 position;void main(){gl_Position=vec4(position,0,1);}";const char*fs="precision highp float;void main(){gl_FragColor=vec4(0,1,0,1);}";GLuint a=compileShader(GL_VERTEX_SHADER,vs),b=compileShader(GL_FRAGMENT_SHADER,fs),p=glCreateProgram();glAttachShader(p,a);glAttachShader(p,b);glBindAttribLocation(p,0,"position");glLinkProgram(p);glUseProgram(p);glBindBuffer(GL_ARRAY_BUFFER,0);const GLfloat verts[]={-1,-1,1,-1,-1,1,1,1};glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,verts);glEnableVertexAttribArray(0);glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(GL_FALSE);glViewport(0,0,t.w,t.h);glDrawArrays(GL_TRIANGLE_STRIP,0,4);auto result=probeTarget.read();glUseProgram(0);glDeleteProgram(p);glDeleteShader(a);glDeleteShader(b);return result;}
static std::array<int,3>prePixel(Target&t,const std::vector<unsigned char>&pixels,unsigned x,unsigned y){size_t at=(size_t(t.h-1-y)*t.w+x)*4;return {{pixels[at],pixels[at+1],pixels[at+2]}};}
static std::array<int,3>introOracle(std::array<int,3>rgb,unsigned x,unsigned y){auto b=layers();if(x<343)rgb=over(rgb,{{80,40,0,128}});for(unsigned k=0;k<b.wordCount;++k){const auto&w=b.words[k];if(x>=unsigned(w.left)&&x<unsigned(w.left+16)&&y>=unsigned(w.top)&&y<unsigned(w.top+12))rgb=over(rgb,w.sourceRole%2?std::array<int,4>{{0,64,0,128}}:std::array<int,4>{{64,0,0,128}},w.scalar256);}for(const auto&l:b.logos)if(x>=unsigned(l.left)&&x<unsigned(l.right)&&y>=unsigned(l.top)&&y<unsigned(l.bottom)){std::array<int,4>c{{l.role==0?30:l.role==2?60:0,l.role==0?30:l.role==1?60:0,l.role==0?30:0,128}};rgb=over(rgb,c,l.scalar256);}if(x>=220&&x<240&&y>=325&&y<340)rgb=over(rgb,{{128,128,128,128}});if(x>=220&&x<240&&y>=320&&y<340)rgb=over(rgb,{{0,96,0,128}});return rgb;}
static void displayTexture(Target&display,GLuint texture){display.bind();glViewport(0,0,display.w,display.h);glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);glDisable(GL_BLEND);glDisable(GL_SCISSOR_TEST);glUseProgram(nfs_bg_compositor.copyProgram);glUniform1i(nfs_bg_compositor.copySampler,0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);glBindBuffer(GL_ARRAY_BUFFER,0);static const GLfloat q[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};glEnableVertexAttribArray(0);for(unsigned i=1;i<4;++i)glDisableVertexAttribArray(i);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q);glDrawArrays(GL_TRIANGLES,0,6);}
int main(){try{
 Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic authored art");Target first(640,480),second(640,480),control(640,480),display(640,480);second.bind();glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,first.depth);g_glCtx=egl.context;g_shaderProgram=nativeProgram();nfs_logo_provenance.context=1;nfs_logo_provenance.alive=1;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,1),"shared compositor owner");nfs_player_intro_renderer_created();Input input;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)input.plane[y*640+x]=x<343?0xf800:0;std::array<GLuint,300>ids{};nativeCache(input,ids);GLuint white=0;glGenTextures(1,&white);glBindTexture(GL_TEXTURE_2D,white);const unsigned char texel[]={255,255,255,255};glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,texel);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);uint64_t serial=1;
 for(unsigned frame=0;frame<4;++frame){Target&t=frame%2?second:first;Target&back=frame%2?first:second;nativeState(t,first.depth);if(frame<2){frameStart(t);nativeState(t,first.depth);}nfs_logo_provenance.clearValid=0;const auto before=t.read();
  // Independent real GLES baseline executes the native draws without observer.
  nativeState(control,control.depth);control.seed();glClearDepthf(1);glClear(GL_DEPTH_BUFFER_BIT);glBindTexture(GL_TEXTURE_2D,control.texture);glTexSubImage2D(GL_TEXTURE_2D,0,0,0,640,480,GL_RGBA,GL_UNSIGNED_BYTE,before.data());nativeState(control,control.depth);nativeSlot0(false);drawTriangles();nativeTail(white,false);drawTriangles();const auto nativeExpected=control.read();const auto depthExpected=depthImage(control,control.depth);
  nativeState(t,first.depth);const auto depthBefore=depthImage(t,first.depth);nativeState(t,first.depth);auto view=introView(input,serial++,frame%2+1);const auto inherited=capture();ck(nfs_private_player_intro_begin(&view),"real Intro begin without clear authority");ck(t.read()==before&&equal(inherited,capture()),"begin never changes native color/state");ck(depthImage(t,first.depth)==depthBefore,"authored stage never changes shared native depth");nativeState(t,first.depth);
  nativeSlot0(true);ck(nfs_private_player_intro_slot0_finish(view.requestSerial),"actual 300 native selects/600 triangles complete");ck(nfs_player_intro_renderer.pass.selects==300&&nfs_player_intro_renderer.pass.triangles==600,"complete independent original pass");nativeTail(white,true);ck(nfs_private_player_intro_tail_finish(view.requestSerial),"actual tail drawn and mirrored before uniform reset");GLfloat tone=0;glGetUniformfv(g_shaderProgram,nativeTone,&tone);ck(tone==1,"native after-draw uniform reset preserved");ck(t.read()==nativeExpected,"HD does not change native color versus independently drawn baseline");ck(depthImage(t,first.depth)==depthExpected,"shared native depth equals original draw baseline");const auto hd=stageImage();ck(hd!=nativeExpected,"HD stage is separate from original raster");for(auto pt:std::array<std::array<unsigned,2>,4>{{{{100,100}},{{405,205}},{{225,330}},{{600,300}}}})pixel(t,hd,pt[0],pt[1],introOracle(prePixel(t,before,pt[0],pt[1]),pt[0],pt[1]));
  // Simulate the actual physical swap: original current FBO becomes FRONT.
  g_framebuffer=back.fbo;g_framebufferTexture=back.texture;g_framebufferDepth=first.depth;glBindFramebuffer(GL_FRAMEBUFFER,0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,t.texture);const auto presentState=capture();GLuint selected=nfs_player_intro_renderer_presentation_texture(t.fbo,t.texture);ck(selected==nfs_player_intro_renderer.color&&selected!=t.texture&&equal(presentState,capture()),"matching physical FRONT selects stage without native writes or state drift");displayTexture(display,selected);ck(display.read()==hd,"real display quad presents the HD stage");ck(t.read()==nativeExpected,"presentation leaves original native surface pristine");nfs_player_intro_renderer_after_present();ck(!nfs_player_intro_renderer.serial&&nfs_player_intro_renderer_presentation_texture(t.fbo,t.texture)==t.texture,"presentation consumes stage once");
 }
 // Wrong key, incomplete pass, unsupported depth and exact cancellation all
 // keep the original presentation path; no cached background guard is widened.
 nativeState(first,first.depth);auto wrong=introView(input,serial++,1);wrong.keys[0]++;const auto before=first.read();ck(!nfs_private_player_intro_begin(&wrong)&&first.read()==before,"misaligned source key refuses before native writes");
 nativeState(first,first.depth);auto incomplete=introView(input,serial++,1);ck(nfs_private_player_intro_begin(&incomplete),"incomplete fixture begin");nativeSlot0(true,299);ck(!nfs_private_player_intro_slot0_finish(incomplete.requestSerial)&&!nfs_player_intro_renderer.serial,"missing original tile refuses stage");
 nativeState(first,first.depth);auto bad=introView(input,serial++,2);ck(nfs_private_player_intro_begin(&bad),"unsupported-tail fixture begin");nativeSlot0(true);ck(nfs_private_player_intro_slot0_finish(bad.requestSerial),"unsupported-tail original pass completes");nativeTail(white,true,true);drawTriangles();ck(!nfs_private_player_intro_tail_finish(bad.requestSerial)&&nfs_player_intro_renderer_presentation_texture(first.fbo,first.texture)==first.texture,"nonflat tail falls back to original");
 nativeState(first,first.depth);auto cancel=introView(input,serial++,1);ck(nfs_private_player_intro_begin(&cancel),"cancel fixture begin");ck(!nfs_private_player_intro_cancel(cancel.requestSerial+1)&&nfs_player_intro_renderer.serial==cancel.requestSerial,"unrelated serial cannot cancel");ck(nfs_private_player_intro_cancel(cancel.requestSerial)&&!nfs_player_intro_renderer.serial,"exact cancellation clears only optional stage");
 const auto nativeAtRetire=first.read();nfs_player_intro_renderer_retired(1);ck(!nfs_player_intro_renderer.color&&!nfs_player_intro_renderer.fbo&&!nfs_player_intro_renderer.generation&&first.read()==nativeAtRetire,"owned resource retirement never modifies native color");glDeleteTextures(1,&white);glDeleteTextures(300,ids.data());glDeleteProgram(g_shaderProgram);std::printf("PASS Intro actual GLES: checks=%u real_native_draws=%u optional_draws=%u; native COLOR/depth baseline, two retained physical buffers, real tail mirror/uniform timing, display-only selection, cancellation/retirement; no original-game or device claim\n",checks,actualNativeDraws,drawCalls);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
