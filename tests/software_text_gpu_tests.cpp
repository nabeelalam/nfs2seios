// SPDX-License-Identifier: MIT
#include "HdSceneGlesFixture.h"
#include <array>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
static unsigned draws=0,uploads=0;static bool failUpload=false;
static void countedDraw(GLenum mode,GLint first,GLsizei count){++draws;glDrawArrays(mode,first,count);}
static void countedUpload(GLenum target,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void*p){++uploads;glTexImage2D(failUpload?GL_TEXTURE_CUBE_MAP:target,level,internal,w,h,border,format,type,p);}
#define glDrawArrays countedDraw
#define glTexImage2D countedUpload
#include "hd/SoftwareTextGpu.c.inc"
#undef glDrawArrays
#undef glTexImage2D
using namespace nfs_hd_test;
static unsigned checks=0;
static void ck(bool value,const char*why){require(value,why);++checks;}
struct Target{
 GLuint texture=0,fbo=0;unsigned width,height;
 Target(unsigned w,unsigned h):width(w),height(h){glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);ck(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"owned fixture target");}
 void bind(){glBindFramebuffer(GL_FRAMEBUFFER,fbo);}
 void seed(){bind();glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);}
 std::vector<unsigned char>read(){bind();std::vector<unsigned char>b(size_t(width)*height*4);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,b.data());ck(glGetError()==GL_NO_ERROR,"fixture pixel read");return b;}
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);}
};
static NfsSoftwareTextStage stage(Target&t,unsigned logical=343){NfsSoftwareTextStage s{};s.framebuffer=t.fbo;s.width=t.width;s.height=t.height;s.logicalWidth=logical;s.logicalHeight=480;ck(nfs_frame_gpu_legacy_sampling(t.width,t.height,GL_LINEAR,&s.sampling),"original sampling");t.bind();glViewport(0,0,s.sampling.viewport[2],s.sampling.viewport[3]);return s;}
static std::array<NfsSoftwareTextVertex,6>quad(){
 const NfsSoftwareTextVertex a{20,40,.2f,.2f,.5f,.25f,0,1},b{20,80,.2f,.8f,.5f,.25f,0,1},c{60,40,.8f,.2f,.5f,.25f,0,1},d{60,80,.8f,.8f,.5f,.25f,0,1};return {{a,b,c,c,b,d}};
}
static void sample(const std::vector<unsigned char>&b,const Target&t,unsigned x,unsigned y,std::array<int,3>rgb){const unsigned px=unsigned((x+.5)*t.height/480.),py=t.height-1-unsigned((y+.5)*t.height/480.);const size_t at=(size_t(py)*t.width+px)*4;for(unsigned k=0;k<3;++k)ck(std::abs(int(b[at+k])-rgb[k])<=1,"independent coverage/tint over blue");}
int main(){try{
 Pbuffer context(640,480);ck(std::strstr((const char*)glGetString(GL_VERSION),"OpenGL ES 2.0"),"actual GLES2");
 std::vector<unsigned char>alpha(1024u*512u,128);NfsSoftwareTextFontView font{};font.schema=1;font.width=1024;font.height=512;font.identity=17;font.alpha=alpha.data();font.bytes=alpha.size();ck(nfs_texture_sha256(alpha.data(),alpha.size(),font.sha256),"synthetic coverage pin");
 NfsSoftwareTextGpu gpu{};ck(!nfs_soft_gpu_prepare(&gpu,&font),"no owner rejects");ck(nfs_soft_gpu_context_created(&gpu,1),"context1");
 for(unsigned field=0;field<7;++field){auto invalid=font;if(field==0)invalid.schema++;if(field==1)invalid.width--;if(field==2)invalid.height++;if(field==3)invalid.bytes--;if(field==4)invalid.alpha=nullptr;if(field==5)invalid.identity=0;if(field==6)invalid.sha256[4]^=1;draws=uploads=0;ck(!nfs_soft_gpu_prepare(&gpu,&invalid)&&!draws&&!uploads,"invalid font no GPU work");}
 auto state=capture();ck(nfs_soft_gpu_prepare(&gpu,&font)&&equal(state,capture()),"prepare restores GL state");draws=uploads=0;ck(nfs_soft_gpu_prepare(&gpu,&font)&&!draws&&!uploads,"sealed identical content resource reused");auto changed=font;changed.identity++;ck(!nfs_soft_gpu_prepare(&gpu,&changed),"numeric owner mismatch rejects");changed=font;changed.sha256[0]^=1;ck(!nfs_soft_gpu_prepare(&gpu,&changed),"cached full digest mismatch rejects");
 for(unsigned height:{480u,1640u}){Target target(height*4/3,height),untouched(640,480);target.seed();auto v=quad();auto s=stage(target);glEnable(GL_SCISSOR_TEST);glScissor(0,0,s.sampling.viewport[2],s.sampling.viewport[3]);glEnable(GL_DEPTH_TEST);glDepthMask(1);glEnable(GL_CULL_FACE);glEnable(GL_STENCIL_TEST);glEnable(GL_DITHER);glActiveTexture(GL_TEXTURE1);state=capture();ck(nfs_soft_gpu_draw_into_staging(&gpu,&s,v.data(),v.size())&&equal(state,capture()),"draw preserves inherited state");auto pixels=target.read();sample(pixels,target,40,60,{{64,32,127}});sample(pixels,target,100,60,{{0,0,255}});
  target.seed();s=stage(target);auto clean=target.read();state=capture();
  for(unsigned fault=0;fault<12;++fault){auto bad=s;auto q=v;size_t count=q.size();if(fault==0)bad.framebuffer=untouched.fbo;if(fault==1)bad.logicalWidth=342;if(fault==2)bad.logicalHeight=479;if(fault==3)bad.sampling.projection[0]+=1;if(fault==4)q[2].x=344;if(fault==5)q[1].y=-1;if(fault==6)q[0].u=2;if(fault==7)q[0].r=2;if(fault==8)q[0].a=.1f;if(fault==9)q[0].x=std::numeric_limits<float>::quiet_NaN();if(fault==10)count=5;if(fault==11)count=12294;draws=0;ck(!nfs_soft_gpu_draw_into_staging(&gpu,&bad,q.data(),count)&&!draws&&equal(state,capture()),"malformed batch/stage rejected before drawing");ck(target.read()==clean,"malformed input leaves staging unchanged");}
  target.bind();glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);state=capture();draws=0;ck(!nfs_soft_gpu_draw_into_staging(&gpu,&s,v.data(),v.size())&&!draws&&equal(state,capture()),"restricted inherited scissor rejects before draw");glDisable(GL_SCISSOR_TEST);
  auto wide=quad();for(auto&x:wide)x.x+=400;s=stage(target,640);ck(nfs_soft_gpu_draw_into_staging(&gpu,&s,wide.data(),wide.size()),"full width supported independently");
 }
 {NfsSoftwareTextGpu failed{};ck(nfs_soft_gpu_context_created(&failed,1),"failure fixture owner");state=capture();failUpload=true;ck(!nfs_soft_gpu_prepare(&failed,&font)&&equal(state,capture())&&!failed.ownerGeneration&&!failed.program&&!failed.texture,"upload failure rolls back resource ownership/state");failUpload=false;ck(nfs_soft_gpu_prepare(&failed,&font),"recovery after transient upload failure");nfs_soft_gpu_context_retired(&failed,1,1);}
 nfs_soft_gpu_context_retired(&gpu,2,1);ck(gpu.generation==1,"stale retire ignored");nfs_soft_gpu_context_retired(&gpu,1,0);ck(!gpu.generation&&!gpu.program&&!gpu.texture,"unbound retire abandons resources");context.unbind();context.destroyUnboundContext();context.createContext();ck(nfs_soft_gpu_context_created(&gpu,2)&&nfs_soft_gpu_prepare(&gpu,&font),"new context creates fresh resources");nfs_soft_gpu_context_retired(&gpu,2,1);ck(glGetError()==GL_NO_ERROR,"owned context cleanup");
 std::printf("PASS software text actual GLES checks=%u synthetic-only=1\n",checks);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s checks=%u\n",e.what(),checks);return 1;}}
