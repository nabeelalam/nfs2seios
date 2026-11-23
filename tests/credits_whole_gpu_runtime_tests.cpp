// SPDX-License-Identifier: MIT
// Real GLES independent staging with synthetic immutable content and artwork.
// Original native cached draws, backend source authority and swap are explicit
// omissions: these pixel checks do not establish application/device acceptance.
#define NFS_GRAPHICS_RENDERER_FIXTURE_ONLY
#include "graphics_renderer_runtime_tests.cpp"
#undef NFS_GRAPHICS_RENDERER_FIXTURE_ONLY
#include "hd/ClassicScreens/CreditsWholeGpu.h"
static bool wholeAlive=true,retireAfterDraw=false;static uint64_t creditsArtGeneration=1;
extern "C" uint64_t nfs_credits_art_generation(){return creditsArtGeneration;}
static int wholeCurrent(const void*){return wholeAlive;}
template<class F>static auto creditsProbe(const char*n,F f)->decltype(f()){
 if constexpr(std::is_void_v<decltype(f())>){probe(n,f);if(retireAfterDraw&&!std::strcmp(n,"glDrawArrays"))wholeAlive=false;}
 else{auto value=probe(n,f);if(retireAfterDraw&&!std::strcmp(n,"glDrawArrays"))wholeAlive=false;return value;}
}
#define glActiveTexture(...) creditsProbe("glActiveTexture",[&](){return ::glActiveTexture(__VA_ARGS__);})
#define glAttachShader(...) creditsProbe("glAttachShader",[&](){return ::glAttachShader(__VA_ARGS__);})
#define glBindAttribLocation(...) creditsProbe("glBindAttribLocation",[&](){return ::glBindAttribLocation(__VA_ARGS__);})
#define glBindBuffer(...) creditsProbe("glBindBuffer",[&](){return ::glBindBuffer(__VA_ARGS__);})
#define glBindFramebuffer(...) creditsProbe("glBindFramebuffer",[&](){return ::glBindFramebuffer(__VA_ARGS__);})
#define glBindRenderbuffer(...) creditsProbe("glBindRenderbuffer",[&](){return ::glBindRenderbuffer(__VA_ARGS__);})
#define glBindTexture(...) creditsProbe("glBindTexture",[&](){return ::glBindTexture(__VA_ARGS__);})
#define glBlendEquationSeparate(...) creditsProbe("glBlendEquationSeparate",[&](){return ::glBlendEquationSeparate(__VA_ARGS__);})
#define glBlendFuncSeparate(...) creditsProbe("glBlendFuncSeparate",[&](){return ::glBlendFuncSeparate(__VA_ARGS__);})
#define glCheckFramebufferStatus(...) creditsProbe("glCheckFramebufferStatus",[&](){return ::glCheckFramebufferStatus(__VA_ARGS__);})
#define glClearDepthf(...) creditsProbe("glClearDepthf",[&](){return ::glClearDepthf(__VA_ARGS__);})
#define glColorMask(...) creditsProbe("glColorMask",[&](){return ::glColorMask(__VA_ARGS__);})
#define glCompileShader(...) creditsProbe("glCompileShader",[&](){return compileInjected(__VA_ARGS__);})
#define glCreateProgram(...) creditsProbe("glCreateProgram",[&](){return ::glCreateProgram(__VA_ARGS__);})
#define glCreateShader(...) creditsProbe("glCreateShader",[&](){return ::glCreateShader(__VA_ARGS__);})
#define glDeleteFramebuffers(...) creditsProbe("glDeleteFramebuffers",[&](){return ::glDeleteFramebuffers(__VA_ARGS__);})
#define glDeleteProgram(...) creditsProbe("glDeleteProgram",[&](){return ::glDeleteProgram(__VA_ARGS__);})
#define glDeleteShader(...) creditsProbe("glDeleteShader",[&](){return ::glDeleteShader(__VA_ARGS__);})
#define glDeleteTextures(...) creditsProbe("glDeleteTextures",[&](){return ::glDeleteTextures(__VA_ARGS__);})
#define glDepthFunc(...) creditsProbe("glDepthFunc",[&](){return ::glDepthFunc(__VA_ARGS__);})
#define glDepthMask(...) creditsProbe("glDepthMask",[&](){return ::glDepthMask(__VA_ARGS__);})
#define glDisable(...) creditsProbe("glDisable",[&](){return ::glDisable(__VA_ARGS__);})
#define glDisableVertexAttribArray(...) creditsProbe("glDisableVertexAttribArray",[&](){return ::glDisableVertexAttribArray(__VA_ARGS__);})
#define glDrawArrays(...) creditsProbe("glDrawArrays",[&](){return drawInjected(__VA_ARGS__);})
#define glEnable(...) creditsProbe("glEnable",[&](){return ::glEnable(__VA_ARGS__);})
#define glEnableVertexAttribArray(...) creditsProbe("glEnableVertexAttribArray",[&](){return ::glEnableVertexAttribArray(__VA_ARGS__);})
#define glFramebufferTexture2D(...) creditsProbe("glFramebufferTexture2D",[&](){return ::glFramebufferTexture2D(__VA_ARGS__);})
#define glGenFramebuffers(...) creditsProbe("glGenFramebuffers",[&](){return ::glGenFramebuffers(__VA_ARGS__);})
#define glGenTextures(...) creditsProbe("glGenTextures",[&](){return ::glGenTextures(__VA_ARGS__);})
#define glGetBooleanv(...) creditsProbe("glGetBooleanv",[&](){return ::glGetBooleanv(__VA_ARGS__);})
#define glGetError(...) creditsProbe("glGetError",[&](){return ::glGetError(__VA_ARGS__);})
#define glGetFloatv(...) creditsProbe("glGetFloatv",[&](){return ::glGetFloatv(__VA_ARGS__);})
#define glGetFramebufferAttachmentParameteriv(...) creditsProbe("glGetFramebufferAttachmentParameteriv",[&](){return ::glGetFramebufferAttachmentParameteriv(__VA_ARGS__);})
#define glGetIntegerv(...) creditsProbe("glGetIntegerv",[&](){return ::glGetIntegerv(__VA_ARGS__);})
#define glGetProgramiv(...) creditsProbe("glGetProgramiv",[&](){return ::glGetProgramiv(__VA_ARGS__);})
#define glGetRenderbufferParameteriv(...) creditsProbe("glGetRenderbufferParameteriv",[&](){return ::glGetRenderbufferParameteriv(__VA_ARGS__);})
#define glGetShaderiv(...) creditsProbe("glGetShaderiv",[&](){return ::glGetShaderiv(__VA_ARGS__);})
#define glGetUniformLocation(...) creditsProbe("glGetUniformLocation",[&](){return ::glGetUniformLocation(__VA_ARGS__);})
#define glGetVertexAttribPointerv(...) creditsProbe("glGetVertexAttribPointerv",[&](){return ::glGetVertexAttribPointerv(__VA_ARGS__);})
#define glGetVertexAttribfv(...) creditsProbe("glGetVertexAttribfv",[&](){return ::glGetVertexAttribfv(__VA_ARGS__);})
#define glGetVertexAttribiv(...) creditsProbe("glGetVertexAttribiv",[&](){return ::glGetVertexAttribiv(__VA_ARGS__);})
#define glIsEnabled(...) creditsProbe("glIsEnabled",[&](){return ::glIsEnabled(__VA_ARGS__);})
#define glIsFramebuffer(...) creditsProbe("glIsFramebuffer",[&](){return ::glIsFramebuffer(__VA_ARGS__);})
#define glIsProgram(...) creditsProbe("glIsProgram",[&](){return ::glIsProgram(__VA_ARGS__);})
#define glIsTexture(...) creditsProbe("glIsTexture",[&](){return ::glIsTexture(__VA_ARGS__);})
#define glLinkProgram(...) creditsProbe("glLinkProgram",[&](){return ::glLinkProgram(__VA_ARGS__);})
#define glPixelStorei(...) creditsProbe("glPixelStorei",[&](){return ::glPixelStorei(__VA_ARGS__);})
#define glScissor(...) creditsProbe("glScissor",[&](){return ::glScissor(__VA_ARGS__);})
#define glShaderSource(...) creditsProbe("glShaderSource",[&](){return ::glShaderSource(__VA_ARGS__);})
#define glTexImage2D(...) creditsProbe("glTexImage2D",[&](){return uploadInjected(__VA_ARGS__);})
#define glTexParameteri(...) creditsProbe("glTexParameteri",[&](){return ::glTexParameteri(__VA_ARGS__);})
#define glUniform1f(...) creditsProbe("glUniform1f",[&](){return ::glUniform1f(__VA_ARGS__);})
#define glUniform1i(...) creditsProbe("glUniform1i",[&](){return ::glUniform1i(__VA_ARGS__);})
#define glUniformMatrix4fv(...) creditsProbe("glUniformMatrix4fv",[&](){return ::glUniformMatrix4fv(__VA_ARGS__);})
#define glUseProgram(...) creditsProbe("glUseProgram",[&](){return ::glUseProgram(__VA_ARGS__);})
#define glVertexAttrib4fv(...) creditsProbe("glVertexAttrib4fv",[&](){return ::glVertexAttrib4fv(__VA_ARGS__);})
#define glVertexAttribPointer(...) creditsProbe("glVertexAttribPointer",[&](){return ::glVertexAttribPointer(__VA_ARGS__);})
#define glViewport(...) creditsProbe("glViewport",[&](){return ::glViewport(__VA_ARGS__);})
#define glDeleteFramebuffers(...) creditsProbe("glDeleteFramebuffers",[&](){return ::glDeleteFramebuffers(__VA_ARGS__);})

#include "hd/ClassicScreens/CreditsWholeGpu.inc"
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
#undef glDeleteFramebuffers
static std::thread::id renderThread;
static int contextCurrent(void*,const NfsBackgroundSceneStage*t){return t&&t->generation==1&&t->contextIdentity==(uintptr_t)g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&std::this_thread::get_id()==renderThread;}
struct WholeFixture {
 Lifetime owner;NfsCreditsWholeView view{};std::vector<unsigned char>images[11],alpha;std::array<NfsSoftwareTextVertex,12>vertices{};std::array<NfsCreditsWholeEvent,3>ordered{};
 WholeFixture(){view.schema=view.art.schema=1;view.request=view.content=view.art.generation=1;view.text.logicalWidth=640;view.text.logicalHeight=480;view.heading={456,17,612,81};view.owner=&owner;view.retain=Lifetime::retain;view.release=Lifetime::release;view.current=wholeCurrent;
  for(unsigned i=0;i<11;++i){auto&im=view.art.images[i];im.width=i?2:16;im.height=i?2:9;images[i].resize(im.width*im.height*4);im.bytes=images[i].size();im.pixels=images[i].data();}
  fill(0,21,52,93,255);for(unsigned i=1;i<11;++i)fill(i,0,0,0,0);fill(4,255,255,0,255);fill(8,255,0,255,255);fill(9,0,0,255,255);fill(10,255,128,0,255);
 }
 void fill(unsigned role,unsigned r,unsigned g,unsigned b,unsigned a){auto&v=images[role];for(size_t k=0;k<v.size();k+=4){v[k]=r;v[k+1]=g;v[k+2]=b;v[k+3]=a;}ck(nfs_texture_sha256(v.data(),v.size(),view.art.images[role].sha256),"fixture image hash");}
 void mixed(){alpha.assign(524288,255);auto&f=view.text.font;f.schema=1;f.width=1024;f.height=512;f.identity=3;f.alpha=alpha.data();f.bytes=alpha.size();ck(nfs_texture_sha256(alpha.data(),alpha.size(),f.sha256),"fixture mask hash");
  auto quad=[&](unsigned at,float l,float t,float r,float b,float red,float green){const float positions[6][2]={{l,t},{l,b},{r,t},{r,t},{l,b},{r,b}};for(unsigned i=0;i<6;++i)vertices[at+i]={positions[i][0],positions[i][1],0,0,red,green,0,1};};quad(0,200,200,300,300,1,0);quad(6,275,225,325,275,0,1);
  view.text.vertices=vertices.data();view.text.vertexCount=vertices.size();ordered={{{1,0,6},{2,0,0},{1,6,6}}};view.events=ordered.data();view.eventCount=3;view.hasPartner=1;view.partner[0]={250,200,350,300};view.partner[1]={450,200,500,300};view.partnerClip={260,210,480,290};
 }
};
static std::array<unsigned char,4>pixel(const NfsCreditsWholeGpu&s,float x,float y){GLint old=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&old);glBindFramebuffer(GL_FRAMEBUFFER,s.fbo);std::array<unsigned char,4>p{};const float logical=480.f*s.width/s.height;int xx=int(x*s.width/logical),yy=int(s.height-y*s.height/480.f)-1;glReadPixels(xx,yy,1,1,GL_RGBA,GL_UNSIGNED_BYTE,p.data());glBindFramebuffer(GL_FRAMEBUFFER,old);return p;}
static bool near(std::array<unsigned char,4>a,std::array<unsigned char,4>b){for(unsigned i=0;i<4;++i)if(std::abs(int(a[i])-int(b[i]))>1)return false;return true;}
int main(){try{
 Pbuffer egl(800,500);Target native(640,480);native.seed();g_glCtx=egl.context;renderThread=std::this_thread::get_id();g_framebuffer=native.fbo;g_framebufferTexture=native.texture;g_framebufferDepth=native.depth;g_shaderProgram=originalProgram();nfs_logo_provenance.context=1;nfs_logo_provenance.alive=1;
 NfsBackgroundSceneStage context{};context.generation=1;context.contextIdentity=(uintptr_t)egl.context;context.framebuffer=native.fbo;context.ownerCurrent=contextCurrent;
 WholeFixture f;NfsCreditsWholeGpu gpu{};const auto originalPixels=native.read();
 for(auto shape:std::vector<std::pair<unsigned,unsigned>>{{640,480},{708,492},{800,450}}){originalState(native);auto state=capture();ck(nfs_cwg_render(&gpu,&f.view,&context,shape.first,shape.second),"empty complete frame renders");ck(equal(state,capture())&&native.read()==originalPixels,"original color and GL state untouched");ck(gpu.width==shape.first&&gpu.height==shape.second&&nfs_cwg_staging_color(&gpu)==gpu.color&&f.owner.holds==1,"owned stage and retained frame");
  GLint old=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&old);glBindFramebuffer(GL_FRAMEBUFFER,gpu.fbo);std::vector<unsigned char>rgba(size_t(shape.first)*shape.second*4);glReadPixels(0,0,shape.first,shape.second,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());glBindFramebuffer(GL_FRAMEBUFFER,old);bool filled=true;for(size_t i=0;i<rgba.size();i+=4)if(rgba[i]!=21||rgba[i+1]!=52||rgba[i+2]!=93||rgba[i+3]!=255){filled=false;break;}ck(filled,"opaque background covers complete drawable at all ratios");
 }
 for(unsigned locale:{1u,5u}){f.view.locale=locale;originalState(native);ck(nfs_cwg_render(&gpu,&f.view,&context,800,450),"selected locale heading renders");const float x=480.f*800/450-100;ck(near(pixel(gpu,x,30),locale==1?std::array<unsigned char,4>{255,255,0,255}:std::array<unsigned char,4>{255,0,255,255}),"six-role locale indexing and right anchor");}f.view.locale=0;
 f.mixed();originalState(native);auto mixedState=capture();ck(nfs_cwg_render(&gpu,&f.view,&context,800,450),"ordered mixed frame renders");ck(equal(mixedState,capture())&&native.read()==originalPixels,"mixed composition preserves original state/color");const float shift=(480.f*800/450-640)/2;
 ck(near(pixel(gpu,220+shift,250),{255,0,0,255}),"text before partners");ck(near(pixel(gpu,255+shift,250),{255,0,0,255}),"partner left clip");ck(near(pixel(gpu,270+shift,250),{0,0,255,255}),"partner overlays prior text");ck(near(pixel(gpu,290+shift,250),{0,255,0,255}),"later text overlays partner");ck(near(pixel(gpu,475+shift,250),{255,128,0,255}),"second partner drawn");ck(near(pixel(gpu,490+shift,250),{21,52,93,255}),"partner right clip");
 std::array<NfsCreditsWholeEvent,2>adjacent={{{1,0,6},{1,6,6}}};auto joined=f.view;joined.events=adjacent.data();joined.eventCount=2;joined.hasPartner=0;originalState(native);resetTrace();ck(nfs_cwg_render(&gpu,&joined,&context,800,450),"adjacent text commands render");recording=false;ck(std::count(events.begin(),events.end(),"glDrawArrays")==5,"four static layers plus one combined glyph draw");ck(near(pixel(gpu,290+shift,250),{0,255,0,255}),"combined glyph draw retains primitive order");
 originalState(native);resetTrace();ck(nfs_cwg_render(&gpu,&f.view,&context,800,450),"cached repeated frame");recording=false;ck(std::count(events.begin(),events.end(),"glTexImage2D")==0,"no repeated artwork/font/output texture uploads at same size");
 auto malformed=f.view;malformed.eventCount=0;originalState(native);auto beforeBad=capture();resetTrace();ck(!nfs_cwg_render(&gpu,&malformed,&context,800,450),"incomplete event coverage rejected");recording=false;ck(events.empty()&&equal(beforeBad,capture())&&!f.owner.holds,"invalid frame has no GL work or retained lease");
 originalState(native);auto beforeLoss=capture();retireAfterDraw=true;ck(!nfs_cwg_render(&gpu,&f.view,&context,800,450),"source retired during private draw");retireAfterDraw=false;ck(equal(beforeLoss,capture())&&native.read()==originalPixels&&!f.owner.holds&&!nfs_cwg_staging_color(&gpu),"same-context source loss restores original state");wholeAlive=true;
 originalState(native);ck(nfs_cwg_render(&gpu,&f.view,&context,800,450),"art-generation source");creditsArtGeneration=2;ck(!nfs_cwg_staging_color(&gpu),"retired artwork cannot expose staging color");creditsArtGeneration=1;nfs_cwg_retire(&gpu,1);ck(!f.owner.holds,"normal retire balances frame");
 // Failed shader or texture creation cannot cache a half-created generation.
 for(unsigned failure:{1u,12u,13u}){NfsCreditsWholeGpu failed{};originalState(native);auto state=capture();failUpload=failure;uploadIndex=0;ck(!nfs_cwg_render(&failed,&f.view,&context,800,450),"injected image/output upload failure rejected");failUpload=0;ck(equal(state,capture())&&!f.owner.holds,"upload failure restores state and frame");originalState(native);ck(nfs_cwg_render(&failed,&f.view,&context,800,450),"failed upload retries without stale dimensions");nfs_cwg_retire(&failed,1);}
 NfsCreditsWholeGpu failedShader{};originalState(native);badCompile=true;ck(!nfs_cwg_render(&failedShader,&f.view,&context,800,450),"bad shader refuses");badCompile=false;originalState(native);ck(nfs_cwg_render(&failedShader,&f.view,&context,800,450),"shader retry succeeds");nfs_cwg_retire(&failedShader,1);
 const EGLint attributes[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};EGLContext foreign=eglCreateContext(egl.display,egl.config,EGL_NO_CONTEXT,attributes);ck(foreign!=EGL_NO_CONTEXT,"foreign context fixture");lossDisplay=egl.display;lossSurface=egl.surface;lossContext=foreign;
 WholeFixture empty;NfsCreditsWholeGpu baseline{};originalState(native);resetTrace();ck(nfs_cwg_render(&baseline,&empty.view,&context,800,450),"baseline operation trace");recording=false;auto operations=events;nfs_cwg_retire(&baseline,1);std::set<unsigned>points{1u,unsigned(operations.size())};for(const char*name:{"glCreateShader","glTexImage2D","glDrawArrays"}){auto at=std::find(operations.begin(),operations.end(),name);ck(at!=operations.end(),"loss phase present");points.insert(unsigned(at-operations.begin()+1));}
 for(unsigned point:points){NfsCreditsWholeGpu lost{};ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,foreign),"foreign snapshot");glViewport(3,4,81,79);auto snapshot=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"restore owner before loss");originalState(native);resetTrace();loseIndex=point;trackForeign=true;ck(!nfs_cwg_render(&lost,&empty.view,&context,800,450),"selected in-call context loss refuses");recording=trackForeign=false;loseIndex=0;ck(!foreignTouches&&eglGetCurrentContext()==foreign&&equal(snapshot,capture())&&!empty.owner.holds,"no foreign GL writes or escaped frame");nfs_cwg_retire(&lost,0);ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"recover owning context");}
 NfsCreditsWholeGpu retired{};originalState(native);ck(nfs_cwg_render(&retired,&empty.view,&context,800,450),"retirement-loss frame");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,foreign),"retirement foreign snapshot");auto retirementState=capture();ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"retirement owner");resetTrace();loseIndex=1;trackForeign=true;nfs_cwg_retire(&retired,1);recording=trackForeign=false;loseIndex=0;ck(!foreignTouches&&eglGetCurrentContext()==foreign&&equal(retirementState,capture())&&!empty.owner.holds,"resource retirement stops after context loss");ck(eglMakeCurrent(egl.display,egl.surface,egl.surface,egl.context),"final owner cleanup");eglDestroyContext(egl.display,foreign);
 ck(f.owner.retained==f.owner.released&&empty.owner.retained==empty.owner.released,"all C frame references balanced");depthUnchanged(native);glDeleteProgram(g_shaderProgram);std::printf("PASS Credits independent GLES staging: %u checks; full drawable, ordered text/partners, clip/locale, state/color/depth, cache/retry and selected source/context loss. Native cached pass and application swap not exercised.\n",checks);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
