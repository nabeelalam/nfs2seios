// SPDX-License-Identifier: MIT
/* Actual optional Player cached renderer adapter with synthetic art and real GLES.
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
static void drawInjected(GLenum mode,GLint first,GLsizei count){GLboolean depth=GL_TRUE;::glGetBooleanv(GL_DEPTH_WRITEMASK,&depth);ck(!depth&&!::glIsEnabled(GL_DEPTH_TEST),"optional stage/copy cannot write original depth");++drawCalls;::glDrawArrays(drawCalls==failDraw?GLenum(0xffffffff):mode,first,count);}
static const unsigned TextureMem=0x200000u;
struct TextureInfo{GLuint id;unsigned fmt,size,hdDigits;void*data;};static TextureInfo g_textures[TextureMem/4]{};static unsigned char textureBytes[TextureMem];static unsigned char*g_textureMem=textureBytes;
struct GrTexInfo{unsigned smallLod,largeLod,aspectRatio,format;};
struct GrVertex{float x,y,r,g,b,a,oow;struct{float sow,tow;}tmuvtx[2];};
static EGLContext g_glCtx=EGL_NO_CONTEXT;static GLuint g_framebuffer=0,g_framebufferTexture=0,g_framebufferDepth=0,g_shaderProgram=0;
static int g_framebufferHeight=480;static const int GR_BUFFER_BACKBUFFER=1;static int g_renderBuffer=GR_BUFFER_BACKBUFFER;static bool g_textureEnabled=true,g_fogEnabled=false,g_depthMask=true,linearFiltering=true;static GLfloat g_matrix[16];
static NfsLogoProvenance nfs_logo_provenance{};static unsigned originalFlushCalls=0;
static void drawTriangles(){++originalFlushCalls;}
static EGLContext SDL_GL_GetCurrentContext(){return eglGetCurrentContext();}
static uintptr_t testThread(){return uintptr_t(std::hash<std::thread::id>{}(std::this_thread::get_id()))|uintptr_t(1);}
#define NFS_GRAPHICS_SCENE_THREAD_TOKEN() testThread()
#define NFS_OPTIONS_SCENE_THREAD_TOKEN() testThread()
#define NFS_PLAYER_SCENE_THREAD_TOKEN() testThread()
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
int main(){try{
 Pbuffer egl(640,480);static AudioSceneAssets assets;NfsHdArtProvider provider{1,&assets,AudioSceneAssets::texture,AudioSceneAssets::word,AudioSceneAssets::art,AudioSceneAssets::region};ck(nfs_hd_install_art_provider(&provider),"synthetic art provider");Target original(640,480),other(640,480);g_glCtx=egl.context;g_framebuffer=original.fbo;g_framebufferTexture=original.texture;g_framebufferDepth=original.depth;g_shaderProgram=originalProgram();nfs_logo_provenance.context=1;nfs_logo_provenance.alive=1;ck(nfs_bg_scene_context_created(&nfs_bg_compositor,1),"fresh shared background generation");nfs_player_renderer_created();Input input;std::array<GLuint,300>ids{};nativeCache(input,ids);uint64_t serial=1;
 for(unsigned iteration=0;iteration<2;++iteration){frameStart(original);auto v=input.view(serial++,iteration+1);const auto colorBefore=original.read();const auto state=capture();const uint64_t uploads=nfs_logo_provenance.uploads;resetTrace();ck(nfs_private_player_scene_begin(&v),"actual Player cold/warm cached begin");recording=false;ck(original.read()==colorBefore&&equal(state,capture()),"begin stages only and preserves original color/state");ck(nfs_player_renderer.nativeArmed&&nfs_logo_provenance.active,"native cached pass armed independently");observeCached();ck(nfs_logo_provenance.selects==300&&nfs_logo_provenance.triangles==600&&nfs_logo_provenance.uploads==uploads,"cached pass needs300selects600triangles and no perframeuploads");ck(nfs_private_player_scene_finish(v.scene.serial)&&equal(state,capture()),"complete actual Player colorcommit restores state");ck(!nfs_player_renderer.serial&&!nfs_player_renderer.nativeArmed,"finish clears pending scalar owner");auto image=original.read();pixel(original,image,600,300,{{20,40,60}});for(auto point:std::array<std::array<unsigned,2>,6>{{{{460,300}},{{450,300}},{{250,300}},{{405,205}},{{225,330}},{{400,150}}}})pixel(original,image,point[0],point[1],oracle(input,point[0],point[1]));if(iteration)ck(std::find(events.begin(),events.end(),"glCreateShader")==events.end()&&std::find(events.begin(),events.end(),"glGenTextures")==events.end(),"warm begin reuses exact caption/background/stage resources");}
 // A successful publication must survive changing the original render size
 // and changing back, without deleting/reuploading the logical preview cache.
 {Target resized(960,720);const GLuint retainedPreview=nfs_player_renderer.preview;
  ck(retainedPreview&&glIsTexture(retainedPreview)&&nfs_player_renderer.previewReady,"published preview is live before resize");
  for(unsigned step=0;step<2;++step){Target&t=step?original:resized;
   g_framebuffer=t.fbo;g_framebufferTexture=t.texture;g_framebufferDepth=t.depth;g_framebufferHeight=int(t.h);
   frameStart(t);auto v=input.view(serial++,step?1:2);const auto before=t.read();const auto state=capture();const auto uploads=nfs_logo_provenance.uploads;
   resetTrace();ck(nfs_private_player_scene_begin(&v),"Player publication admitted after framebuffer resize");recording=false;
   ck(t.read()==before&&equal(state,capture()),"resized begin leaves original color and inherited state untouched");
   ck(nfs_player_renderer.preview==retainedPreview&&nfs_player_renderer.previewReady&&glIsTexture(retainedPreview),"resize preserves the exact live verified preview texture");
   ck(std::count(events.begin(),events.end(),"glGenTextures")==1&&std::count(events.begin(),events.end(),"glTexImage2D")==1,"resize allocates only the color stage and never reuploads preview");
   observeCached();ck(nfs_logo_provenance.selects==300&&nfs_logo_provenance.triangles==600&&nfs_logo_provenance.uploads==uploads,"resized publication still observes the complete cached original pass");
   ck(nfs_private_player_scene_finish(v.scene.serial)&&equal(state,capture()),"resized publication commits and restores inherited state");
   const auto image=t.read();for(auto point:std::array<std::array<unsigned,2>,3>{{{{460,300}},{{250,300}},{{400,150}}}})pixel(t,image,point[0],point[1],oracle(input,point[0],point[1]));
  }
 }
 // Only exact schema/form/tail Player pairs are admitted.
 for(unsigned bad=0;bad<12;++bad){frameStart(original);auto v=input.view(serial++);if(bad==0)v.schema++;if(bad==1)v.form=0x4d2a20;if(bad==2)v.scene.tailKind=2;if(bad==3)v.scene.tailKind=6;if(bad==4)v.preview.schema++;if(bad==5)v.preview.width=0;if(bad==6)v.preview.pixels565=nullptr;if(bad==7)v.preview.sha256[0]^=1;if(bad==8)v.preview.coverageSha256[0]^=1;if(bad==9)v.preview.coverageCount--;if(bad==10)v.preview.left=-1;if(bad==11)v.preview.top=480;const auto before=original.read();const auto inherited=capture();ck(!nfs_private_player_scene_begin(&v)&&original.read()==before&&equal(inherited,capture()),"exact Player form/tail/preview guards preserve original");}
 {frameStart(original);auto v=input.view(serial++);ck(nfs_private_player_scene_begin(&v),"nested first Player begin");auto second=input.view(serial++,2);ck(!nfs_private_player_scene_begin(&second)&&!nfs_player_renderer.serial&&!nfs_logo_provenance.active,"qualified crossPlayer begin cancels incomplete stage");}
 {frameStart(original);auto v=input.view(serial++);ck(nfs_private_player_scene_begin(&v),"other-scene fixture begin");nfs_player_renderer_other();ck(!nfs_private_player_scene_finish(v.scene.serial)&&!nfs_player_renderer.serial,"other original operation invalidates pendingPlayer");}
 // Cancellation is CPU-only, exact pending serial, and cannot steal a
 // newer scene's replacement global native arm.
 {frameStart(original);auto v=input.view(serial++);const auto before=original.read();ck(nfs_private_player_scene_begin(&v),"cancel fixture begin");resetTrace();ck(!nfs_private_player_scene_cancel(0)&&!nfs_private_player_scene_cancel(v.scene.serial+1)&&nfs_player_renderer.serial==v.scene.serial&&nfs_logo_provenance.active,"unrelated cancelserial preserves Playerpending");ck(nfs_private_player_scene_cancel(v.scene.serial)&&!nfs_player_renderer.serial&&!nfs_logo_provenance.active&&callIndex==0,"exact cancel emits noGLcalls");recording=false;ck(original.read()==before&&!nfs_private_player_scene_finish(v.scene.serial),"canceledPlayer cannotcommit");}
 {frameStart(original);auto v=input.view(serial++);ck(nfs_private_player_scene_begin(&v),"replacement arm fixture begin");const uint64_t otherSerial=serial++;ck(nfs_logo_p_begin(&nfs_logo_provenance,otherSerial,original.fbo,v.scene.rendererAddress,ids.data()),"simulated unrelatedscene replaces globalarm");resetTrace();ck(nfs_private_player_scene_cancel(v.scene.serial)&&nfs_logo_provenance.active&&nfs_logo_provenance.pendingSerial==otherSerial&&callIndex==0,"Playercancel preserves another scene's replacementnativearm");recording=false;nfs_logo_p_cancel(&nfs_logo_provenance);}
 // All raw original pixels stay outside new public source and are synthetic here.
 for(unsigned bad=0;bad<20;++bad){frameStart(original);auto v=input.view(serial++);const auto before=original.read();const auto savedTexture=g_textures[0],savedSecond=g_textures[512];const auto savedWitness=nfs_logo_uploads[0],savedSecondWitness=nfs_logo_uploads[1];const unsigned char savedByte=textureBytes[0];const auto savedUploads=nfs_logo_provenance.uploads;const GLuint savedColor=g_framebufferTexture,savedPreview=nfs_player_renderer.preview;std::vector<uint16_t>wrong;
  if(bad==0)v.schema++;if(bad==1)v.scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;if(bad==2)v.caption.logicalWidth=640;if(bad==3)v.scene.rendererAddress[0]++;if(bad==4){wrong=input.plane;wrong[0]^=1;v.scene.original565=wrong.data();}if(bad==5)g_textures[0].id=ids[1];if(bad==6)g_textures[0].fmt=10;if(bad==7)textureBytes[0]^=1;if(bad==8)nfs_logo_uploads[0].context++;if(bad==9)nfs_logo_uploads[0].valid=0;if(bad==10)nfs_logo_uploads[0].bytes[0]^=1;if(bad==11)g_textures[0].hdDigits=1;if(bad==12){other.bind();}if(bad==13)glViewport(1,0,640,480);if(bad==14){glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,ids[1]);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);}if(bad==15){g_textures[512].id=ids[0];nfs_logo_uploads[1].id=ids[0];}if(bad==16){g_textures[0].id=0xffffff;nfs_logo_uploads[0].id=0xffffff;}if(bad==17)nfs_logo_provenance.uploads=0;if(bad==18)g_framebufferTexture=other.texture;if(bad==19)nfs_player_renderer.preview=ids[0];
  const auto inherited=capture();ck(!nfs_private_player_scene_begin(&v)&&equal(inherited,capture()),"wrongcachedkey/id/payload/context/target/filter refuses with inherited state intact");g_textures[0]=savedTexture;g_textures[512]=savedSecond;nfs_logo_uploads[0]=savedWitness;nfs_logo_uploads[1]=savedSecondWitness;textureBytes[0]=savedByte;nfs_logo_provenance.uploads=savedUploads;g_framebufferTexture=savedColor;nfs_player_renderer.preview=savedPreview;if(bad==14){glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,ids[1]);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);}ck(original.read()==before,"refused begin preserves original pixels");}
 for(unsigned bad=0;bad<3;++bad){frameStart(original);auto v=input.view(serial++);const auto before=original.read();ck(nfs_private_player_scene_begin(&v),"negative stream cached begin");observeCached(bad==0,bad==1);ck(!nfs_private_player_scene_finish(v.scene.serial+(bad==2?1:0))&&original.read()==before&&!nfs_player_renderer.serial,"incomplete/wrongselection/wrongserial never publishes");}
 // Binary coverage is checked even when the submitted digest is coherent.
 {frameStart(original);auto v=input.view(serial++);input.mask[0]=128;v.preview=input.previewView();const auto before=original.read();const auto inherited=capture();ck(!nfs_private_player_scene_begin(&v)&&original.read()==before&&equal(inherited,capture()),"nonbinary previewcoverage rejects despite matchingSHA");input.mask[0]=255;}
 // Independently sample the optional helpers at every observed GL operation.
 frameStart(original);auto v=input.view(serial++);ck(nfs_private_player_scene_begin(&v),"helper fixture pending Player");const auto warm=nfs_player_renderer;const auto provenance=nfs_logo_provenance;NfsNativeLogoTargetProof target=warm.target;NfsSoftwareTextGpu testFont{};ck(nfs_soft_gpu_context_created(&testFont,1),"isolated cold font");const auto emptyFont=testFont;
 EGLint attrs[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};lossDisplay=egl.display;lossSurface=egl.surface;lossContext=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attrs);ck(lossContext!=EGL_NO_CONTEXT,"foreign EGL context");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,lossContext),"foreign cancel fixture");const auto foreignCancelState=capture();resetTrace();ck(!nfs_private_player_scene_cancel(v.scene.serial)&&callIndex==0&&equal(foreignCancelState,capture()),"foreigncontext cancel rejects withoutGLcalls");recording=false;ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"returncancel owner");unsigned points=0;auto reset=[&](){nfs_player_renderer=warm;nfs_logo_provenance=provenance;};
 points+=lossFamily(egl,original,"font_cold",[&](){return nfs_player_font_prepare(&testFont,&input.font);},[&](){reset();testFont=emptyFont;});
 points+=lossFamily(egl,original,"font_warm",[&](){return nfs_player_font_prepare(&nfs_player_renderer.caption,&input.font);},reset);
 points+=lossFamily(egl,original,"color_cold",[&](){return nfs_player_color_target(640,480);},[&](){reset();nfs_player_renderer.color=nfs_player_renderer.fbo=0;nfs_player_renderer.width=nfs_player_renderer.height=0;});
 points+=lossFamily(egl,original,"color_warm",[&](){return nfs_player_color_target(640,480);},reset);
 points+=lossFamily(egl,original,"target_reader",[&](){NfsNativeLogoTargetProof t{};return nfs_player_read_target(&t,99);},reset);
 points+=lossFamily(egl,original,"cached_tiles",[&](){uint32_t names[300];GLenum filter=0;return nfs_player_cached_tiles(&v.scene,names,&filter);},reset);
 auto previewView=input.previewView();
 points+=lossFamily(egl,original,"preview_cold",[&](){return nfs_player_preview_prepare(&previewView);},[&](){reset();nfs_player_renderer.preview=nfs_player_renderer.previewReady=0;});
 points+=lossFamily(egl,original,"preview_warm",[&](){return nfs_player_preview_prepare(&previewView);},reset);
 points+=lossFamily(egl,original,"preview_changed",[&](){return nfs_player_preview_prepare(&previewView);},[&](){reset();nfs_player_renderer.previewReady=0;});
 NfsBackgroundSceneStage stage{};stage.generation=warm.generation;stage.contextIdentity=warm.context;stage.framebuffer=warm.fbo;stage.width=640;stage.height=480;stage.sampling=target.sampling;stage.ownerCurrent=nfs_player_stage_owner;
 points+=lossFamily(egl,original,"preview_draw",[&](){glBindFramebuffer(GL_FRAMEBUFFER,warm.fbo);glViewport(0,0,640,480);glDisable(GL_DEPTH_TEST);glDepthMask(0);const int result=nfs_player_preview_draw(&stage,&previewView);if(eglGetCurrentContext()==egl.context)originalState(original);return result;},reset);
 points+=lossFamily(egl,original,"color_commit",[&](){return nfs_player_copy_color(&target);},reset,true);
 // Failure path exercises are direct helpers, after the native original stream
 // checks above; original drawTriangles remains explicitly a no-GL stub.
 reset();originalState(original);testFont=emptyFont;const auto failureState=capture();badCompile=true;ck(!nfs_player_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"cold shader failure restores state");badCompile=false;failUpload=1;uploadIndex=0;ck(!nfs_player_font_prepare(&testFont,&input.font)&&equal(failureState,capture()),"cold font upload failure restores state");failUpload=0;
 // Independent real depth check for the COLOR-only commit helper. This
 // resource-helper check uses .25 depth directly; no nativeclear is fabricated.
 reset();original.seed();originalState(original);{NfsBgSaved saved{};auto guard=nfs_player_context_stage();ck(nfs_graphics_scene_save(&saved,&guard),"depthcheck save");ck(nfs_player_copy_color(&target),"depthcheck COLORcommit");ck(nfs_graphics_scene_restore(&saved,&guard),"depthcheck restore");depthUnchanged(original);}
 reset();nfs_player_cancel("fixture-end");nfs_player_renderer_retired(1);ck(!nfs_player_renderer.generation&&!nfs_player_renderer.caption.generation&&!nfs_player_renderer.color&&!nfs_player_renderer.fbo,"retire forgets own caption and staging resources");ck(eglDestroyContext(egl.display,lossContext),"destroy foreign context");lossContext=EGL_NO_CONTEXT;nfs_bg_scene_context_retired(&nfs_bg_compositor,1,1);glUseProgram(0);glDeleteProgram(g_shaderProgram);glDeleteTextures(300,ids.data());ck(glGetError()==GL_NO_ERROR,"final GL state");ck(originalFlushCalls>0,"explicit no-GL original flush stub observed");std::printf("PASS actual Player adapter checks=%u optional_loss_points=%u cached_format=11 perframe_uploads=0 synthetic_only=1 original_draw_stub=1\n",checks,points);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
