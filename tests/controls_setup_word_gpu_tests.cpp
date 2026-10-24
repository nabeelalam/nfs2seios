// SPDX-License-Identifier: MIT
// Actual GLES mechanics with synthetic text; no source/FIFO/native authority.
#include "HdSceneGlesFixture.h"
#include "hd/BackgroundSceneCompositor.c.inc"
#include "hd/NestedCaptionGpu.c.inc"
#include "hd/ControlsSetupWordArtwork.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <thread>
#include <vector>
using namespace nfs_hd_test;
static unsigned checks=0;
static void ck(bool value,const char*why){require(value,why);++checks;}
namespace instrument {
static unsigned pixelHashes=0,uploads=0,allocations=0,draws=0;
static std::array<const void*,28> pixels{};
static std::vector<GLuint> programs;
static std::vector<GLsizei> counts;
static int digest(const void*p,size_t n,unsigned char*out){
 if(n>=100u*128u*4u)++pixelHashes;
 return nfs_texture_sha256(p,n,out);
}
static void upload(GLenum t,GLint l,GLint i,GLsizei w,GLsizei h,GLint b,GLenum f,GLenum ty,const void*p){++uploads;::glTexImage2D(t,l,i,w,h,b,f,ty,p);}
static GLuint program(){++allocations;return ::glCreateProgram();}
static void textures(GLsizei n,GLuint*p){allocations+=unsigned(n);::glGenTextures(n,p);}
static void draw(GLenum mode,GLint first,GLsizei count){GLint p=0;::glGetIntegerv(GL_CURRENT_PROGRAM,&p);programs.push_back(GLuint(p));counts.push_back(count);++draws;::glDrawArrays(mode,first,count);}
static void reset(){pixelHashes=uploads=allocations=draws=0;programs.clear();counts.clear();}
}
#define nfs_texture_sha256 instrument::digest
#define glTexImage2D instrument::upload
#define glCreateProgram instrument::program
#define glGenTextures instrument::textures
#define glDrawArrays instrument::draw
#include "hd/ControlsSetupWordGpu.c.inc"
#undef nfs_texture_sha256
#undef glTexImage2D
#undef glCreateProgram
#undef glGenTextures
#undef glDrawArrays

struct Target {
 GLuint texture=0,fbo=0;
 Target(){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);bind();glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"owned staging target");}
 void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);}
 void seed(){bind();glViewport(0,0,640,480);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);}
 std::vector<unsigned char>read(){bind();std::vector<unsigned char>p(640*480*4);glReadPixels(0,0,640,480,GL_RGBA,GL_UNSIGNED_BYTE,p.data());ck(glGetError()==GL_NO_ERROR,"read actual target pixels");return p;}
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);}
};
struct Owner {EGLContext context;uint64_t generation=1;std::thread::id thread=std::this_thread::get_id();};
static int current(void*user,const NfsBackgroundSceneStage*s){auto&o=*static_cast<Owner*>(user);return s&&eglGetCurrentContext()==o.context&&o.thread==std::this_thread::get_id()&&s->contextIdentity==reinterpret_cast<uintptr_t>(o.context)&&s->generation==o.generation;}
static NfsBackgroundSceneStage stage(Owner&o,Target&t){NfsBackgroundSceneStage s{};s.generation=o.generation;s.contextIdentity=reinterpret_cast<uintptr_t>(o.context);s.framebuffer=t.fbo;s.width=640;s.height=480;s.ownerCurrent=current;s.ownerUser=&o;ck(nfs_frame_gpu_legacy_sampling(640,480,GL_LINEAR,&s.sampling),"640 sampling");t.bind();glViewport(0,0,640,480);return s;}
static void quad(std::vector<NfsSoftwareTextVertex>&v,float x,float y){
 const NfsSoftwareTextVertex a={x,y,.2f,.2f,1,1,1,1},b={x,y+20,.2f,.8f,1,1,1,1},c={x+24,y,.8f,.2f,1,1,1,1},d={x+24,y+20,.8f,.8f,1,1,1,1};v.insert(v.end(),{a,b,c,c,b,d});
}
static NfsControlsSetupWordsView words(){
 NfsControlsSetupWordsView v{};v.schema=NFS_CONTROLS_SETUP_WORD_SCHEMA;v.count=2;
 for(unsigned i=0;i<28;++i){auto a=nfs_controls_setup_word_display::artwork(i+1);ck(bool(a),"all maintained word artwork decodes");auto&im=v.images[i];im.word=i+1;im.width=a->width;im.height=a->height;im.rgba=a->rgba.data();im.bytes=a->rgba.size();std::copy(a->sha.begin(),a->sha.end(),im.sha256);instrument::pixels[i]=im.rgba;}
 auto&a=v.placements[0];a.word=1;a.color565=65535;a.vertexFirst=6;a.vertexCount=6;a.left=120;a.top=70;a.right=179;a.bottom=102;
 auto&b=v.placements[1];b.word=2;b.color565=65504;b.vertexFirst=18;b.vertexCount=6;b.left=300;b.top=140;b.right=350;b.bottom=172;
 return v;
}
static void uploadedPixels(const NfsControlsSetupWordGpu&s,const NfsControlsSetupWordsView&v,Target&t){
 GLuint fbo=0;glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
 for(unsigned i=0;i<28;++i){auto&a=v.images[i];glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,s.textures[i],0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"each actual word texture readable");std::vector<unsigned char>p(a.bytes);glReadPixels(0,0,a.width,a.height,GL_RGBA,GL_UNSIGNED_BYTE,p.data());ck(glGetError()==GL_NO_ERROR&&!std::memcmp(p.data(),a.rgba,a.bytes),"each uploaded GPU image equals authenticated artwork bytes");}
 glDeleteFramebuffers(1,&fbo);t.bind();
}
static void run(Pbuffer&egl){
 Target t;Owner owner{egl.context};auto s=stage(owner,t);auto guard=s;guard.framebuffer=0;
 auto v=words();std::vector<NfsSoftwareTextVertex>vertices;for(unsigned i=0;i<5;++i)quad(vertices,40+100*i,250);
 std::vector<unsigned char>alpha(1024*512,128);NfsSoftwareTextFontView f{};f.schema=1;f.width=1024;f.height=512;f.identity=19;f.alpha=alpha.data();f.bytes=alpha.size();ck(nfs_texture_sha256(f.alpha,f.bytes,f.sha256),"synthetic dynamic font digest");NfsSoftwareTextGpu font{};ck(nfs_soft_gpu_context_created(&font,1)&&nfs_soft_gpu_prepare(&font,&f),"synthetic dynamic font prepared");
 NfsSoftwareTextLayerView text{640,480,f,vertices.data(),vertices.size()};NfsControlsSetupWordGpu gpu{};
 // Every one of the 28 fresh payloads is authenticated before allocations.
 for(unsigned i=0;i<28;++i){auto bad=v;std::vector<unsigned char>corrupt(bad.images[i].rgba,bad.images[i].rgba+bad.images[i].bytes);corrupt[corrupt.size()/2]^=1;bad.images[i].rgba=corrupt.data();NfsControlsSetupWordGpu fresh{};instrument::reset();auto before=capture();ck(!nfs_controls_setup_word_prepare(&fresh,&bad,&text,&guard),"corrupt fresh word payload rejected");ck(!fresh.program&&!fresh.generation&&!instrument::uploads&&!instrument::allocations&&equal(before,capture()),"corrupt fresh payload publishes no resources or GL state changes");}
 instrument::reset();auto before=capture();ck(nfs_controls_setup_word_prepare(&gpu,&v,&text,&guard)&&equal(before,capture()),"fresh complete word upload restores inherited GL state");ck(instrument::pixelHashes==28&&instrument::uploads==28,"fresh upload hashes and uploads all 28 images exactly once");auto initial=gpu;uploadedPixels(gpu,v,t);
 // Cached resources depend on fixed metadata and owned GPU pixels, not on
 // dereferencing any borrowed pixel buffer again. Supply bounded corrupt
 // storage, so an accidental rehash fails safely rather than reading invalid memory.
 std::vector<unsigned char>unavailable(584u*128u*4u,0);auto borrowed=v;for(auto&im:borrowed.images)im.rgba=unavailable.data();
 instrument::reset();before=capture();ck(nfs_controls_setup_word_prepare(&gpu,&borrowed,&text,&guard)&&nfs_controls_setup_word_ready(&gpu,&borrowed,&text,&guard),"warm cache uses authenticated owned GPU textures");ck(!instrument::pixelHashes&&!instrument::uploads&&!instrument::allocations&&gpu.program==initial.program&&!std::memcmp(gpu.textures,initial.textures,sizeof gpu.textures)&&equal(before,capture()),"warm prepare/ready has no pixel hashes, allocations, reuploads or state changes");
 for(unsigned i=0;i<15;++i){auto bad=v;auto badText=text;switch(i){case 0:bad.schema=0;break;case 1:bad.count=0;break;case 2:bad.count=NFS_CONTROLS_SETUP_WORD_PLACEMENTS+1;break;case 3:bad.images[27].word=1;break;case 4:--bad.images[27].width;break;case 5:--bad.images[27].bytes;break;case 6:bad.images[27].sha256[0]^=1;break;case 7:bad.placements[0].vertexFirst=1;break;case 8:bad.placements[0].vertexCount=7;break;case 9:bad.placements[1].vertexFirst=6;break;case 10:bad.placements[1].right=641;break;case 11:bad.placements[1].top=NAN;break;case 12:bad.placements[1].word=29;break;case 13:badText.vertexCount=29;break;case 14:badText.vertices=nullptr;break;}instrument::reset();before=capture();ck(!nfs_controls_setup_word_prepare(&gpu,&bad,&badText,&guard),"malformed warm metadata/span/placement rejected");ck(!instrument::pixelHashes&&!instrument::uploads&&!instrument::allocations&&equal(before,capture()),"malformed warm view has no pixel reads/resource/state changes");}
 auto stale=guard;stale.generation=2;instrument::reset();ck(!nfs_controls_setup_word_prepare(&gpu,&v,&text,&stale)&&!nfs_controls_setup_word_ready(&gpu,&v,&text,&stale),"mismatched owner generation rejected");ck(!instrument::pixelHashes&&!instrument::uploads&&!instrument::allocations,"stale generation has no payload reads/resources");
 // Independent reference draws only original leading, middle and trailing spans.
 t.seed();s=stage(owner,t);NfsBgSaved saved{};ck(nfs_nested_caption_save(&saved,&s),"reference state save");for(unsigned first:{0u,12u,24u})ck(nfs_controls_setup_word_vertices(font.program,font.texture,font.matrix,font.sampler,vertices.data()+first,6,&s)==1,"original dynamic reference slice");ck(nfs_nested_caption_restore(&saved,&s),"reference state restore");auto reference=t.read();
 t.seed();s=stage(owner,t);glEnable(GL_DEPTH_TEST);glDepthMask(1);glColorMask(0,1,0,1);glEnable(GL_CULL_FACE);glEnable(GL_SCISSOR_TEST);glScissor(7,8,9,10);glActiveTexture(GL_TEXTURE1);before=capture();instrument::reset();ck(nfs_controls_setup_word_draw(&gpu,&borrowed,&font,&text,&s)&&equal(before,capture()),"authored draw restores inherited texture, viewport, blend, attribute, depth and scissor state");ck(!instrument::pixelHashes&&instrument::draws==5,"draw uses no pixel hashes and exactly two replacements plus three original slices");for(unsigned i=0;i<5;++i)ck(instrument::counts[i]==6&&instrument::programs[i]==(i%2?gpu.program:font.program),"leading/intervening/trailing dynamic slices keep exact original order");auto actual=t.read();unsigned dynamic=0,wordInk[2]={0,0};
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t offset=(size_t(479-y)*640+x)*4;int box=-1;for(unsigned i=0;i<2;++i){auto&p=v.placements[i];if(x+.5f>=p.left&&x+.5f<p.right&&y+.5f>=p.top&&y+.5f<p.bottom)box=int(i);}if(box<0){ck(!std::memcmp(actual.data()+offset,reference.data()+offset,4),"all pixels outside authored boxes preserve original dynamic text/background");if(reference[offset]||reference[offset+1])++dynamic;}else if(actual[offset]||actual[offset+1])++wordInk[box];}
 ck(dynamic>100&&wordInk[0]>20&&wordInk[1]>20,"dynamic text and both actual authored images visibly present");
 ck(nfs_controls_setup_word_retire(&gpu,&guard,1)&&!gpu.program&&!gpu.generation&&!glIsProgram(initial.program),"owned retirement deletes program");for(GLuint name:initial.textures)ck(!glIsTexture(name),"owned retirement deletes all word textures");ck(glIsTexture(font.texture),"word retirement preserves dynamic original font");
 owner.generation=2;s=stage(owner,t);guard=s;guard.framebuffer=0;instrument::reset();ck(nfs_controls_setup_word_prepare(&gpu,&v,&text,&guard)&&gpu.generation==2,"fresh generation uploads authenticated resources again");ck(instrument::pixelHashes==28&&instrument::uploads==28,"generation reupload reauthenticates all 28 pixels");uploadedPixels(gpu,v,t);ck(nfs_controls_setup_word_retire(&gpu,&guard,1),"retire second generation");nfs_soft_gpu_context_retired(&font,1,1);ck(glGetError()==GL_NO_ERROR,"no final GLES errors");
}
int main(){try{Pbuffer egl(640,480);run(egl);std::printf("PASS child Controls word GPU checks=%u text=SYNTHETIC actual-GLES=1 no-native-source-authority=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL child Controls word GPU: %s checks=%u\n",e.what(),checks);return 1;}}
