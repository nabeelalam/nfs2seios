// SPDX-License-Identifier: MIT
// Actual GLES mechanics with synthetic spans; no original-source/FIFO authority.
#include "HdSceneGlesFixture.h"
#include "hd/BackgroundSceneCompositor.c.inc"
#include "hd/NestedCaptionGpu.c.inc"
#include "hd/ControlsSetupWordArtwork.h"
#include "hd/ControlsBindingPhraseArtwork.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <thread>
#include <vector>
using namespace nfs_hd_test;
static unsigned checks=0;
static void ck(bool v,const char*why){require(v,why);++checks;}
namespace instrument {
static unsigned hashes=0,uploads=0,allocations=0,draws=0,glCalls=0;
static std::vector<GLuint>programs;
static int digest(const void*p,size_t n,unsigned char*out){if(n>=128u*128u*4u)++hashes;return nfs_texture_sha256(p,n,out);}
static void upload(GLenum t,GLint l,GLint i,GLsizei w,GLsizei h,GLint b,GLenum f,GLenum ty,const void*p){++glCalls;++uploads;::glTexImage2D(t,l,i,w,h,b,f,ty,p);}
static GLuint program(){++glCalls;++allocations;return ::glCreateProgram();}
static void textures(GLsizei n,GLuint*p){++glCalls;allocations+=unsigned(n);::glGenTextures(n,p);}
static GLboolean texture(GLuint n){++glCalls;return ::glIsTexture(n);}
static GLboolean isProgram(GLuint n){++glCalls;return ::glIsProgram(n);}
static void integer(GLenum e,GLint*p){++glCalls;::glGetIntegerv(e,p);}
static void draw(GLenum mode,GLint first,GLsizei count){GLint p=0;::glGetIntegerv(GL_CURRENT_PROGRAM,&p);++glCalls;++draws;programs.push_back(GLuint(p));::glDrawArrays(mode,first,count);}
static void reset(){hashes=uploads=allocations=draws=glCalls=0;programs.clear();}
}
#define nfs_texture_sha256 instrument::digest
#define glTexImage2D instrument::upload
#define glCreateProgram instrument::program
#define glGenTextures instrument::textures
#define glIsTexture instrument::texture
#define glIsProgram instrument::isProgram
#define glGetIntegerv instrument::integer
#define glDrawArrays instrument::draw
#include "hd/ControlsSetupWordGpu.c.inc"
// Root includes the phrase implementation after its fixed-word vertex helper.
#ifndef NFS_CONTROLS_BINDING_PHRASE_GPU_IMPLEMENTATION
#include "hd/ControlsBindingPhraseGpu.c.inc"
#endif
#undef nfs_texture_sha256
#undef glTexImage2D
#undef glCreateProgram
#undef glGenTextures
#undef glIsTexture
#undef glIsProgram
#undef glGetIntegerv
#undef glDrawArrays
struct Target {
 GLuint texture=0,fbo=0;
 Target(){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);bind();glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"staging target complete");}
 void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);}
 void seed(){bind();glViewport(0,0,640,480);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);}
 std::vector<unsigned char>read(){bind();std::vector<unsigned char>p(640*480*4);glReadPixels(0,0,640,480,GL_RGBA,GL_UNSIGNED_BYTE,p.data());ck(glGetError()==GL_NO_ERROR,"target readback");return p;}
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);}
};
struct Owner {EGLContext context;uint64_t generation=1;std::thread::id thread=std::this_thread::get_id();};
static int current(void*u,const NfsBackgroundSceneStage*s){auto&o=*static_cast<Owner*>(u);return s&&eglGetCurrentContext()==o.context&&std::this_thread::get_id()==o.thread&&s->contextIdentity==reinterpret_cast<uintptr_t>(o.context)&&s->generation==o.generation;}
static NfsBackgroundSceneStage stage(Owner&o,Target&t){NfsBackgroundSceneStage s{};s.generation=o.generation;s.contextIdentity=reinterpret_cast<uintptr_t>(o.context);s.ownerCurrent=current;s.ownerUser=&o;s.framebuffer=t.fbo;s.width=640;s.height=480;ck(nfs_frame_gpu_legacy_sampling(640,480,GL_LINEAR,&s.sampling),"legacy sampling");t.bind();glViewport(0,0,640,480);return s;}
static void quad(std::vector<NfsSoftwareTextVertex>&v,float x,float y){const NfsSoftwareTextVertex a={x,y,.2f,.2f,1,1,1,1},b={x,y+20,.2f,.8f,1,1,1,1},c={x+24,y,.8f,.2f,1,1,1,1},d={x+24,y+20,.8f,.8f,1,1,1,1};v.insert(v.end(),{a,b,c,c,b,d});}
struct Phrases {
 std::vector<std::shared_ptr<const nfs_controls_binding_phrase::Artwork>>owners;
 NfsControlsBindingPhrasesView view{};
 explicit Phrases(const std::vector<unsigned>&ids){ck(!ids.empty()&&ids.size()<=20,"bounded test phrase list");view.schema=1;view.imageCount=view.count=unsigned(ids.size());
  for(unsigned i=0;i<ids.size();++i){auto a=nfs_controls_binding_phrase::artwork(ids[i]);ck(bool(a),"maintained complete phrase decodes");owners.push_back(a);auto&im=view.images[i];im.phrase=ids[i];im.width=a->width;im.height=128;im.rgba=a->rgba.data();im.bytes=a->rgba.size();std::copy(a->sha.begin(),a->sha.end(),im.sha256);auto&p=view.placements[i];p.slot=i+1;p.foreground565=65535;p.shadow565=0;p.vertexFirst=i*6;p.vertexCount=6;p.left=40;p.top=40+float(i)*14;p.right=p.left+float(a->width)/4.f;p.bottom=p.top+32;}
 }
};
static unsigned cached(const NfsControlsBindingPhraseGpu&s){unsigned n=0;for(unsigned i=0;i<20;++i)if(s.textures[i])++n;return n;}
static void readImages(const NfsControlsBindingPhraseGpu&s,const NfsControlsBindingPhrasesView&v,Target&t){GLuint fbo=0;glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);for(unsigned i=0;i<v.imageCount;++i){auto&a=v.images[i];int slot=nfs_controls_binding_phrase_slot(&s,a.phrase);ck(slot>=0,"requested phrase cached");glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,s.textures[slot],0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"complete phrase texture readable");std::vector<unsigned char>p(a.bytes);glReadPixels(0,0,a.width,a.height,GL_RGBA,GL_UNSIGNED_BYTE,p.data());ck(glGetError()==GL_NO_ERROR&&!std::memcmp(p.data(),a.rgba,a.bytes),"uploaded RGBA equals exact full authored phrase");}glDeleteFramebuffers(1,&fbo);t.bind();}
static void combined(Pbuffer&egl){
 Target t;Owner owner{egl.context};auto s=stage(owner,t),guard=s;guard.framebuffer=0;
 std::vector<NfsSoftwareTextVertex>vertices;for(unsigned i=0;i<5;++i)quad(vertices,40+float(i)*100,250);
 std::vector<unsigned char>alpha(1024u*512u,128);NfsSoftwareTextFontView f{};f.schema=1;f.width=1024;f.height=512;f.identity=79;f.alpha=alpha.data();f.bytes=alpha.size();ck(nfs_texture_sha256(f.alpha,f.bytes,f.sha256),"combined synthetic font digest");NfsSoftwareTextGpu font{};ck(nfs_soft_gpu_context_created(&font,1)&&nfs_soft_gpu_prepare(&font,&f),"combined synthetic original font prepared");
 NfsSoftwareTextLayerView text{640,480,f,vertices.data(),vertices.size()};
 NfsControlsSetupWordsView fixed{};fixed.schema=1;fixed.count=1;
 std::vector<std::shared_ptr<const nfs_controls_setup_word_display::Artwork>>owners;
 for(unsigned i=0;i<28;++i){auto a=nfs_controls_setup_word_display::artwork(i+1);ck(bool(a),"combined mandatory finite word artwork");owners.push_back(a);auto&im=fixed.images[i];im.word=i+1;im.width=a->width;im.height=a->height;im.rgba=a->rgba.data();im.bytes=a->rgba.size();std::copy(a->sha.begin(),a->sha.end(),im.sha256);}
 auto&w=fixed.placements[0];w.word=1;w.color565=65504;w.vertexFirst=6;w.vertexCount=6;w.left=120;w.top=70;w.right=w.left+float(fixed.images[0].width)/4.f;w.bottom=102;
 Phrases phrases({33});auto&b=phrases.view.placements[0];b.vertexFirst=18;b.left=300;b.top=140;b.right=b.left+float(phrases.view.images[0].width)/4.f;b.bottom=172;
 NfsControlsSetupWordGpu words{};NfsControlsBindingPhraseGpu bindings{};ck(nfs_controls_setup_word_prepare(&words,&fixed,&text,&guard)&&nfs_controls_binding_phrase_prepare(&bindings,&phrases.view,&text,&guard),"combined independent finite word and phrase caches prepared");
 // Independent reference contains only the three original gaps that survive.
 t.seed();s=stage(owner,t);NfsBgSaved saved{};ck(nfs_nested_caption_save(&saved,&s),"combined reference save");for(unsigned first:{0u,12u,24u})ck(nfs_controls_setup_word_vertices(font.program,font.texture,font.matrix,font.sampler,vertices.data()+first,6,&s)==1,"combined independent surviving original reference slice");ck(nfs_nested_caption_restore(&saved,&s),"combined reference restore");auto reference=t.read();
 // Both individually valid replacements cannot claim the same source span.
 auto overlap=phrases.view;overlap.placements[0].vertexFirst=6;instrument::reset();auto before=capture();ck(!nfs_controls_setup_word_draw_with_bindings(&words,&fixed,&font,&text,&bindings,&overlap,&s),"combined overlapping fixed and binding source spans refused");ck(!instrument::glCalls&&!instrument::hashes&&!instrument::uploads&&!instrument::allocations&&equal(before,capture()),"combined overlap refuses before helper GL with state intact");
 t.seed();s=stage(owner,t);glEnable(GL_DEPTH_TEST);glDepthMask(1);glEnable(GL_CULL_FACE);glEnable(GL_SCISSOR_TEST);glScissor(7,8,9,10);glColorMask(0,1,0,1);glActiveTexture(GL_TEXTURE1);before=capture();instrument::reset();
 ck(nfs_controls_setup_word_draw_with_bindings(&words,&fixed,&font,&text,&bindings,&phrases.view,&s)&&equal(before,capture()),"combined transaction restores inherited GL state once around all ordered slices");
 const GLuint expected[]={font.program,words.program,font.program,bindings.program,bindings.program,font.program};ck(instrument::draws==6&&instrument::programs.size()==6,"combined draws three original gaps, one fixed word, whole phrase shadow and foreground");for(unsigned i=0;i<6;++i)ck(instrument::programs[i]==expected[i],"combined exact original/fixed/original/binding-shadow/binding-foreground/original order");ck(!instrument::hashes&&!instrument::uploads&&!instrument::allocations,"combined warm draw has no payload hashes or allocations");
 auto actual=t.read();unsigned fixedInk=0,bindingInk=0,dynamicInk=0;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t k=(size_t(479-y)*640+x)*4;bool fixedBox=x+.5f>=w.left&&x+.5f<w.right&&y+.5f>=w.top&&y+.5f<w.bottom;bool bindingBox=x+.5f>=b.left&&x+.5f<b.right+1&&y+.5f>=b.top&&y+.5f<b.bottom+1;if(!fixedBox&&!bindingBox){ck(!std::memcmp(actual.data()+k,reference.data()+k,4),"combined all pixels beyond replacement boxes preserve independent original reference");if(reference[k]||reference[k+1])++dynamicInk;}else if(fixedBox&&(actual[k]||actual[k+1]))++fixedInk;else if(bindingBox&&(actual[k]||actual[k+1]))++bindingInk;}
 ck(fixedInk>20&&bindingInk>20&&dynamicInk>100,"combined actual fixed phrase and original gaps all visibly present");
 ck(nfs_controls_setup_word_retire(&words,&guard,1)&&nfs_controls_binding_phrase_retire(&bindings,&guard,1),"combined caches retire independently");nfs_soft_gpu_context_retired(&font,1,1);ck(glGetError()==GL_NO_ERROR,"combined no final GLES errors");
}
static void run(Pbuffer&egl){
 Target t,untouched;Owner owner{egl.context};auto s=stage(owner,t),guard=s;guard.framebuffer=0;
 std::vector<NfsSoftwareTextVertex>vertices;for(unsigned i=0;i<30;++i)quad(vertices,30+float(i%6)*90,280+float(i/6)*30);
 NfsSoftwareTextLayerView text{};text.logicalWidth=640;text.logicalHeight=480;text.vertices=vertices.data();text.vertexCount=vertices.size();
 NfsControlsBindingPhraseGpu gpu{};NfsControlsBindingPhrasesView zero{};instrument::reset();auto before=capture();
 ck(nfs_controls_binding_phrase_empty(&zero)&&nfs_controls_binding_phrase_valid(&zero,nullptr)&&nfs_controls_binding_phrase_prepare(&gpu,&zero,nullptr,nullptr)&&nfs_controls_binding_phrase_ready(&gpu,&zero,nullptr,nullptr)&&nfs_controls_binding_phrase_draw_into_scope(&gpu,&zero,0,nullptr),"exact zero is original fallback without context");ck(!instrument::glCalls&&!instrument::hashes&&!instrument::uploads&&equal(before,capture()),"zero view emits no helper GL or payload work");
 Phrases first({33,181});instrument::reset();before=capture();ck(nfs_controls_binding_phrase_prepare(&gpu,&first.view,&text,&guard)&&equal(before,capture()),"cold requested phrase upload restores state");ck(instrument::hashes==2&&instrument::uploads==2&&cached(gpu)==2,"cold cache authenticates/uploads only two requested images");readImages(gpu,first.view,t);
 std::vector<unsigned char>unavailable(4096u*128u*4u,0);auto borrowed=first.view;for(unsigned i=0;i<borrowed.imageCount;++i)borrowed.images[i].rgba=unavailable.data();
 instrument::reset();before=capture();ck(nfs_controls_binding_phrase_prepare(&gpu,&borrowed,&text,&guard)&&nfs_controls_binding_phrase_ready(&gpu,&borrowed,&text,&guard),"cached corrupt borrowed storage is never reread");ck(!instrument::hashes&&!instrument::uploads&&!instrument::allocations&&equal(before,capture()),"warm unchanged has no payload or allocation work");
 borrowed.placements[0].left+=3;borrowed.placements[0].right+=3;borrowed.placements[0].foreground565=65504;instrument::reset();ck(nfs_controls_binding_phrase_prepare(&gpu,&borrowed,&text,&guard)&&nfs_controls_binding_phrase_ready(&gpu,&borrowed,&text,&guard),"changed placements use same authenticated GPU phrases");ck(!instrument::hashes&&!instrument::uploads&&!instrument::allocations,"placement update requires no pixel hash/upload");
 Phrases fresh({34});auto corrupt=fresh.view;std::vector<unsigned char>broken(corrupt.images[0].rgba,corrupt.images[0].rgba+corrupt.images[0].bytes);broken[broken.size()/2]^=1;corrupt.images[0].rgba=broken.data();auto held=gpu;instrument::reset();before=capture();ck(!nfs_controls_binding_phrase_prepare(&gpu,&corrupt,&text,&guard),"new corrupt phrase refused");ck(instrument::hashes==1&&!instrument::glCalls&&!instrument::uploads&&!instrument::allocations&&!std::memcmp(&held,&gpu,sizeof gpu)&&equal(before,capture()),"corrupt NEW image refuses before helper GL and retains cache");
 for(unsigned i=0;i<12;++i){auto bad=first.view;auto bt=text;switch(i){case 0:bad.images[0].width+=4;break;case 1:bad.images[0].sha256[0]^=1;break;case 2:bad.placements[0].slot=0;break;case 3:bad.placements[0].slot=3;break;case 4:bad.placements[1].vertexFirst=0;break;case 5:bad.placements[0].right+=1;break;case 6:bad.placements[0].bottom+=1;break;case 7:bad.placements[0].vertexFirst=1;break;case 8:bad.count=21;break;case 9:bad.imageCount=21;break;case 10:bt.vertexCount=1;break;case 11:bad.placements[0].left=NAN;break;}instrument::reset();ck(!nfs_controls_binding_phrase_prepare(&gpu,&bad,&bt,&guard),"malformed metadata/order/slot/whole-image bounds refused");ck(!instrument::glCalls&&!instrument::hashes&&!instrument::uploads&&!instrument::allocations,"malformed metadata refuses before GL or pixels");}
 auto stale=guard;stale.generation=2;instrument::reset();ck(!nfs_controls_binding_phrase_prepare(&gpu,&first.view,&text,&stale)&&!nfs_controls_binding_phrase_ready(&gpu,&first.view,&text,&stale),"stale generation refused");ck(!instrument::glCalls&&!instrument::hashes&&!instrument::uploads,"stale generation does no helper GL or pixels");
 // One group draws the complete phrase shadow and foreground into staging only.
 Phrases drawPhrase({33});ck(nfs_controls_binding_phrase_prepare(&gpu,&drawPhrase.view,&text,&guard),"prepare draw group");untouched.seed();auto original=untouched.read();t.seed();s=stage(owner,t);glEnable(GL_DEPTH_TEST);glDepthMask(1);glEnable(GL_CULL_FACE);glEnable(GL_SCISSOR_TEST);glScissor(7,8,9,10);glColorMask(0,1,0,1);glActiveTexture(GL_TEXTURE1);before=capture();NfsBgSaved saved{};ck(nfs_nested_caption_save(&saved,&s),"outer transaction saves state");instrument::reset();ck(nfs_controls_binding_phrase_draw_into_scope(&gpu,&drawPhrase.view,0,&s)==1,"whole shadow then foreground draw");ck(instrument::draws==2&&!instrument::hashes&&!instrument::uploads&&!instrument::allocations,"group draws exactly shadow and foreground without allocations/pixels");ck(nfs_nested_caption_restore(&saved,&s)&&equal(before,capture()),"outer transaction restores all inherited state");auto pixels=t.read();ck(untouched.read()==original,"separate original target unchanged");unsigned white=0,black=0;auto&p=drawPhrase.view.placements[0];for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t k=(size_t(479-y)*640+x)*4;bool box=x+.5f>=p.left&&x+.5f<p.right+1&&y+.5f>=p.top&&y+.5f<p.bottom+1;if(!box)ck(!pixels[k]&&!pixels[k+1]&&pixels[k+2]==255&&pixels[k+3]==255,"phrase draws only its whole-image shadow bounds");else{if(pixels[k]>200&&pixels[k+1]>200)++white;if(pixels[k+2]<150)++black;}}ck(white>20&&black>5,"foreground and offset shadow both visible");
 t.bind();glViewport(0,0,640,480);auto wrong=s;wrong.framebuffer=untouched.fbo;instrument::reset();ck(!nfs_controls_binding_phrase_draw_into_scope(&gpu,&drawPhrase.view,0,&wrong)&&!instrument::draws,"wrong staging FBO refuses before draw");
 // Fill bounded cache, then prove an inherited unit-1 binding prevents eviction.
 ck(nfs_controls_binding_phrase_retire(&gpu,&guard,1),"retire initial phrase cache");std::vector<unsigned>ids;for(unsigned i=1;i<=20;++i)ids.push_back(i);Phrases full(ids);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,0);ck(nfs_controls_binding_phrase_prepare(&gpu,&full.view,&text,&guard)&&cached(gpu)==20,"exactly twenty retained GPU textures");Phrases next({21});GLuint protectedTexture=gpu.textures[0];glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,protectedTexture);before=capture();held=gpu;instrument::reset();ck(!nfs_controls_binding_phrase_prepare(&gpu,&next.view,&text,&guard),"eviction of inherited bound texture refused");ck(!instrument::uploads&&!instrument::allocations&&!std::memcmp(&held,&gpu,sizeof gpu)&&equal(before,capture()),"refused eviction preserves all owned resources and caller state");glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,0);
 for(unsigned id=21;id<=43;++id){Phrases one({id});instrument::reset();ck(nfs_controls_binding_phrase_prepare(&gpu,&one.view,&text,&guard)&&nfs_controls_binding_phrase_ready(&gpu,&one.view,&text,&guard),"lazy cache rotates through more than twenty distinct complete phrases");ck(instrument::hashes==1&&instrument::uploads==1&&cached(gpu)<=20,"each new phrase uploads once with bounded retained cache");readImages(gpu,one.view,t);}
 held=gpu;ck(nfs_controls_binding_phrase_retire(&gpu,&guard,1)&&!gpu.generation&&!gpu.program&&!glIsProgram(held.program),"owned retirement deletes program");for(GLuint n:held.textures)if(n)ck(!glIsTexture(n),"owned retirement deletes retained phrases");
 owner.generation=2;s=stage(owner,t);guard=s;guard.framebuffer=0;instrument::reset();ck(nfs_controls_binding_phrase_prepare(&gpu,&first.view,&text,&guard)&&gpu.generation==2,"new owner generation reuploads");ck(instrument::hashes==2&&instrument::uploads==2,"new generation reauthenticates only requested pixels");readImages(gpu,first.view,t);ck(nfs_controls_binding_phrase_retire(&gpu,&guard,1),"retire new generation");ck(glGetError()==GL_NO_ERROR,"no final GLES error");
}
int main(){try{Pbuffer egl(640,480);run(egl);combined(egl);std::printf("PASS Controls binding phrase GPU checks=%u actual-GLES=1 synthetic-spans=1 no-source-authority=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL Controls binding phrase GPU: %s checks=%u\n",e.what(),checks);return 1;}}
