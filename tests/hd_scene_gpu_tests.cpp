// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include "hd/HdArtProvider.h"
#include <array>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
static unsigned draws=0,uploads=0;static bool failUpload=false;static GLuint failDrawProgram=0;
static void countedDraw(GLenum mode,GLint first,GLsizei count){++draws;GLint program=0;glGetIntegerv(GL_CURRENT_PROGRAM,&program);glDrawArrays(failDrawProgram&&GLuint(program)==failDrawProgram?GLenum(0xffffffffu):mode,first,count);}
static void countedUpload(GLenum target,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void*p){++uploads;glTexImage2D(failUpload?GL_TEXTURE_CUBE_MAP:target,level,internal,w,h,border,format,type,p);}
#define glDrawArrays countedDraw
#define glTexImage2D countedUpload
#include "hd/BackgroundSceneCompositor.c.inc"
#undef glDrawArrays
#undef glTexImage2D
using namespace nfs_hd_test;
static unsigned checks;
static void ck(bool b,const char*m){require(b,m);++checks;}
struct Assets{
 std::array<std::vector<unsigned char>,5>bytes;std::array<NfsBackgroundSceneTexture,5>tex{};
 std::array<NfsBackgroundSceneWord,12>words{};std::vector<unsigned char>logoBytes;NfsSmallLogoArt logo{};bool badRegion=false;
 Assets(){const unsigned widths[5]={2048,2048,1372,2012,1824},heights[5]={1536,1536,1920,1924,1924};
  for(unsigned i=0;i<5;++i){bytes[i].resize(size_t(widths[i])*heights[i]*4);if(i<3){const unsigned char colors[3][4]={{20,40,60,255},{100,80,40,255},{80,40,0,128}};for(size_t k=0;k<bytes[i].size();k+=4)std::memcpy(bytes[i].data()+k,colors[i],4);}tex[i].pixels=bytes[i].data();tex[i].bytes=bytes[i].size();tex[i].width=widths[i];tex[i].height=heights[i];tex[i].schema=1;tex[i].role=i;tex[i].premultiplied=1;tex[i].opaque=i<2;tex[i].identity=i+1;}
  for(unsigned i=0;i<12;++i){auto&w=words[i];w.schema=1;w.sourceRole=i+1;w.textureRole=i<6?3:4;w.x=2+(i%6)*68;w.y=2;w.width=64;w.height=48;w.scale=4;w.logicalWidth=16;w.logicalHeight=12;
   unsigned char color[4]={static_cast<unsigned char>(i%2?64:0),static_cast<unsigned char>(i%2?0:64),0,128};for(unsigned y=0;y<52;++y)for(unsigned x=0;x<68;++x)std::memcpy(bytes[w.textureRole].data()+((w.y-2+y)*widths[w.textureRole]+w.x-2+x)*4,color,4);}
  for(unsigned i=0;i<5;++i)ck(nfs_texture_sha256(bytes[i].data(),bytes[i].size(),tex[i].sha256),"synthetic texture sha");
  logoBytes.resize(NFS_SMALL_LOGO_ART_BYTES);for(const auto&r:nfs_logo_regions){unsigned char color[4]={static_cast<unsigned char>(r.role==0?30:r.role==2?60:0),static_cast<unsigned char>(r.role==0?30:r.role==1?60:0),static_cast<unsigned char>(r.role==0?30:0),128};for(unsigned y=0;y<r.height;++y)for(unsigned x=0;x<r.width;++x)std::memcpy(logoBytes.data()+((r.y+y)*2048+r.x+x)*4,color,4);}
  logo.pixels=logoBytes.data();logo.bytes=logoBytes.size();logo.width=2048;logo.height=768;logo.schema=1;logo.premultiplied=1;logo.identity=100;ck(nfs_texture_sha256(logoBytes.data(),logoBytes.size(),logo.sha256),"synthetic logo sha");
 }
 static int texture(void*p,uint32_t role,NfsBackgroundSceneTexture*out){if(role>=5)return 0;*out=static_cast<Assets*>(p)->tex[role];return 1;}
 static int word(void*p,uint32_t role,NfsBackgroundSceneWord*out){if(!role||role>12)return 0;auto&a=*static_cast<Assets*>(p);*out=a.words[role-1];if(a.badRegion){out->logicalWidth=0x40000001;out->width=4;}return 1;}
 static int art(void*p,NfsSmallLogoArt*out){*out=static_cast<Assets*>(p)->logo;return 1;}
 static int region(void*,uint32_t role,NfsSmallLogoRegion*out){if(role>=5)return 0;*out=nfs_logo_regions[role];return 1;}
};
struct Target{
 GLuint texture=0,depth=0,fbo=0;unsigned w,h;
 Target(unsigned W,unsigned H):w(W),h(H){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,w,h);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"synthetic target");}
 void seed(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glDepthMask(1);glClearDepthf(1);glClearColor(.8f,.1f,.7f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);}
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);glDeleteRenderbuffers(1,&depth);}
};
static std::vector<unsigned char>read(Target&t){std::vector<unsigned char>b(size_t(t.w)*t.h*4);GLint pack;glGetIntegerv(GL_PACK_ALIGNMENT,&pack);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,t.w,t.h,GL_RGBA,GL_UNSIGNED_BYTE,b.data());glPixelStorei(GL_PACK_ALIGNMENT,pack);ck(glGetError()==GL_NO_ERROR,"fixture-only readback");return b;}
static void native(Target&t,NfsFrameLegacySampling&p){nfs_frame_draw_state();glBindFramebuffer(GL_FRAMEBUFFER,t.fbo);glViewport(0,0,p.viewport[2],p.viewport[3]);glScissor(0,0,p.viewport[2],p.viewport[3]);glEnable(GL_SCISSOR_TEST);glEnable(GL_DEPTH_TEST);glDepthMask(1);glDepthFunc(GL_LEQUAL);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);}
static std::array<uint16_t,307200> original;
static NfsBackgroundSceneFrameView view(uint64_t serial,unsigned map){NfsBackgroundSceneFrameView v{};v.schema=1;v.width=640;v.height=480;v.mapRole=map;v.wordCount=2;v.tailKind=1;v.serial=serial;v.contentRevision=serial;v.planeWords=307200;v.original565=original.data();for(unsigned i=0;i<300;++i)v.rendererAddress[i]=i*2048;v.panelClip[2]=343;v.panelClip[3]=480;for(unsigned i=0;i<2;++i){auto&w=v.words[i];w.sourceRole=i+1;w.scalar256=i?256:128;w.left=400+8*i;w.top=200+4*i;w.clip[2]=640;w.clip[3]=480;}unsigned roles[3]={1,2,0};for(unsigned i=0;i<3;++i){auto&r=nfs_logo_regions[roles[i]];auto&l=v.logos[i];l.role=roles[i];l.scalar256=i==1?127:256;l.left=r.sourceOriginX;l.top=r.sourceOriginY;l.right=l.left+r.logicalWidth;l.bottom=l.top+r.logicalHeight;l.clip[2]=343;l.clip[3]=480;}return v;}
static NfsNativeLogoTargetProof proof(Target&t,NfsFrameLegacySampling&p,uint64_t serial,uint64_t gen=1){NfsNativeLogoTargetProof q{};q.framebuffer=t.fbo;q.width=t.w;q.height=t.h;q.contextGeneration=gen;q.sampling=p;q.depthClearSerial=serial;q.depthClearProven=1;q.depthClearValue=1;return q;}
static std::array<int,3>over(std::array<int,3>d,std::array<int,4>s,unsigned scalar=256){for(unsigned k=0;k<3;++k)d[k]=int(std::lround(s[k]*scalar/256.0+d[k]*(1.0-s[3]*scalar/(256.0*255.0))));return d;}
static void sample(const std::vector<unsigned char>&b,Target&t,float x,float y,std::array<int,3>expected){unsigned px=unsigned((x+.5)*t.h/480.),py=t.h-1-unsigned((y+.5)*t.h/480.);size_t at=(size_t(py)*t.w+px)*4;for(unsigned k=0;k<3;++k)ck(std::abs(int(b[at+k])-expected[k])<=1,"independent PMA order/scalar interior sample");}
int main(){try{Pbuffer egl(640,480);ck(std::strstr((const char*)glGetString(GL_VERSION),"OpenGL ES 2.0"),"real GLES2");static Assets assets;NfsHdArtProvider provider{1,&assets,Assets::texture,Assets::word,Assets::art,Assets::region};ck(nfs_hd_install_art_provider(&provider),"trusted synthetic install");NfsBackgroundSceneCompositor s{};ck(nfs_bg_scene_context_created(&s,1),"context1");assets.badRegion=true;ck(!nfs_bg_scene_prepare(&s),"overflow descriptor rejects before GPU");assets.badRegion=false;ck(nfs_bg_scene_prepare(&s),"prepare synthetic assets");uint64_t serial=0;
 {for(unsigned i=0;i<4;++i){unsigned h=i>=2?1640:480;Target t(h*4/3,h);t.seed();NfsFrameLegacySampling p{};ck(nfs_frame_gpu_legacy_sampling(t.w,h,GL_LINEAR,&p),"exact native projection");native(t,p);auto v=view(++serial,i%2);auto q=proof(t,p,v.serial);auto before=capture();auto untouched=read(t);draws=uploads=0;ck(nfs_bg_scene_begin(&s,&v,&q)&&equal(before,capture()),"begin state");ck(read(t)==untouched,"begin leaves original target");ck(nfs_bg_scene_finish(&s,v.serial,&q)&&equal(before,capture()),"finish state");auto pixels=read(t);std::array<int,3>map=i%2?std::array<int,3>{{100,80,40}}:std::array<int,3>{{20,40,60}};sample(pixels,t,500,300,map);sample(pixels,t,10,200,over(map,{{80,40,0,128}}));sample(pixels,t,410,210,over(over(map,{{0,64,0,128}},128),{{64,0,0,128}}));auto logo=over(over(over(over(map,{{80,40,0,128}}),{{0,60,0,128}}),{{60,0,0,128}},127),{{30,30,30,128}});sample(pixels,t,50,40,logo);
 v.serial=++serial;q.depthClearSerial=v.serial;draws=uploads=0;ck(nfs_bg_scene_begin(&s,&v,&q)&&s.lastBeginKind==2&&!draws&&!uploads,"cache hit zero draw upload");ck(nfs_bg_scene_finish(&s,v.serial,&q)&&read(t)==pixels,"cache publication exact");v.serial=++serial;q.depthClearSerial=v.serial;auto bad=q;bad.contextGeneration++;ck(!nfs_bg_scene_begin(&s,&v,&bad),"wrong owner rejects");
 }}
 // Explicit Location software-caption route shares art/provenance, never MAIN_SKIP.
 {std::vector<unsigned char>coverage(1024u*512u,128);NfsSoftwareTextFontView font{};font.schema=1;font.width=1024;font.height=512;font.identity=17;font.alpha=coverage.data();font.bytes=coverage.size();ck(nfs_texture_sha256(coverage.data(),coverage.size(),font.sha256),"caption coverage pin");
  for(unsigned height:{480u,1640u}){Target t(height*4/3,height);t.seed();NfsFrameLegacySampling sampling{};nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&sampling);native(t,sampling);
   const NfsSoftwareTextVertex a{116,167,.2f,.2f,1,1,1,1},b{116,186,.2f,.8f,1,1,1,1},c{168,167,.8f,.2f,1,1,1,1},d{168,186,.8f,.8f,1,1,1,1};std::array<NfsSoftwareTextVertex,6>vertices{{a,b,c,c,b,d}};
   NfsLocationSceneFrameView location{};location.schema=1;location.scene=view(++serial,0);location.scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;location.caption.logicalWidth=343;location.caption.logicalHeight=480;location.caption.font=font;location.caption.vertices=vertices.data();location.caption.vertexCount=vertices.size();auto q=proof(t,sampling,location.scene.serial);auto before=capture();auto blue=read(t);
   ck(!nfs_bg_scene_begin(&s,&location.scene,&q),"Main entry rejects Location caption kind");
   ck(nfs_bg_scene_begin_location(&s,&location,&q)&&equal(before,capture())&&read(t)==blue,"Location begins unpublished and preserves state");
   ck(!nfs_bg_scene_finish(&s,location.scene.serial,&q)&&read(t)==blue,"Main finish cannot publish pending Location");
   location.scene.serial=++serial;q.depthClearSerial=serial;ck(nfs_bg_scene_begin_location(&s,&location,&q),"Location retry after wrong finish");ck(nfs_bg_scene_finish_location(&s,serial,&q)&&equal(before,capture()),"explicit Location publication");
   auto pixels=read(t);const auto panel=over({{20,40,60}},{{80,40,0,128}});sample(pixels,t,130,175,over(panel,{{128,128,128,128}}));sample(pixels,t,240,175,panel);
   location.scene.serial=++serial;q.depthClearSerial=serial;draws=uploads=0;ck(nfs_bg_scene_begin_location(&s,&location,&q)&&s.lastBeginKind==2&&!draws&&!uploads,"caption full-content cache hit");ck(nfs_bg_scene_finish_location(&s,serial,&q)&&read(t)==pixels,"caption cache publication exact");
   for(auto&v:vertices)v.x+=30;location.scene.serial=++serial;q.depthClearSerial=serial;draws=0;ck(nfs_bg_scene_begin_location(&s,&location,&q)&&s.lastBeginKind==1&&draws,"changed caption geometry cannot hit old image");ck(nfs_bg_scene_finish_location(&s,serial,&q),"changed caption publish");auto moved=read(t);sample(moved,t,130,175,panel);sample(moved,t,175,175,over(panel,{{128,128,128,128}}));
   auto main=location.scene;main.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;main.serial=++serial;q.depthClearSerial=serial;ck(nfs_bg_scene_begin(&s,&main,&q)&&nfs_bg_scene_finish(&s,serial,&q),"Main route remains supported after Location");sample(read(t),t,175,175,panel);
   for(unsigned fault=0;fault<5;++fault){t.seed();native(t,sampling);location.scene.serial=++serial;q.depthClearSerial=serial;auto bad=location;auto malformed=vertices;if(fault==0)bad.schema=2;if(fault==1)bad.scene.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;if(fault==2)bad.caption.logicalWidth=640;if(fault==3)bad.caption.font.sha256[0]^=1;if(fault==4){for(auto&v:malformed)v.y-=150;bad.caption.vertices=malformed.data();}before=capture();blue=read(t);draws=0;ck(!nfs_bg_scene_begin_location(&s,&bad,&q)&&!draws&&equal(before,capture())&&read(t)==blue,"invalid Location admission leaves complete original target");}
   t.seed();native(t,sampling);location.scene.serial=++serial;q.depthClearSerial=serial;for(auto&v:vertices)v.x+=3;before=capture();blue=read(t);unsigned victim=s.nextSlot;failDrawProgram=s.caption.program;ck(!nfs_bg_scene_begin_location(&s,&location,&q),"late caption draw fault rejects whole staging transaction");failDrawProgram=0;ck(equal(before,capture())&&!s.imageReady[victim]&&!s.pendingSerial&&read(t)==blue,"late caption error cannot publish partial image");
  }
 }
 {Target t(1280,960);t.seed();NfsFrameLegacySampling p{};nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&p);native(t,p);auto originalColor=read(t);auto v=view(++serial,1);auto q=proof(t,p,v.serial);unsigned victim=s.nextSlot;failUpload=true;ck(!nfs_bg_scene_begin(&s,&v,&q),"forced resize upload failure");failUpload=false;ck(!s.imageReady[victim]&&!s.imageRevision[victim]&&read(t)==originalColor,"invalidate-before-write fallback");}
 nfs_bg_scene_context_retired(&s,1,0);ck(!s.generation&&!s.imageReady[0]&&!s.imageReady[1],"unbound invalidate");egl.unbind();egl.destroyUnboundContext();egl.createContext();ck(nfs_bg_scene_context_created(&s,2)&&nfs_bg_scene_prepare(&s),"new context recover");nfs_bg_scene_context_retired(&s,1,1);ck(s.generation==2,"stale retire ignored");nfs_bg_scene_context_retired(&s,2,1);ck(glGetError()==GL_NO_ERROR,"cleanup");std::printf("PASS synthetic GLES logical/native checks=%u no game/assets\n",checks);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
