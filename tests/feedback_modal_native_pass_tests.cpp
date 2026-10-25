// SPDX-License-Identifier: MIT
// Synthetic renderer observations only; no GL/Game/source authority fabricated.
#include "hd/FeedbackModalNativePass.h"
#include <array>
#include <vector>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <limits>
static unsigned checks;
#define CK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
static NfsAudioNativeVertex source(unsigned t,unsigned c){NfsAudioNativeVertex v{};v.x=786432.f+float((t/15)*32+(c==1||c==2?32:0));v.y=786432.f+float((t%15)*32+(c>=2?32:0));v.r=v.g=v.b=v.a=255;v.oow=1;v.sow=c==1||c==2?255:0;v.tow=c>=2?255:0;return v;}
static std::array<NfsFeedbackActualVertex,6>actual(unsigned t){const unsigned corners[]={0,1,2,0,2,3};std::array<NfsFeedbackActualVertex,6>v{};for(unsigned i=0;i<6;++i){auto s=source(t,corners[i]);v[i]={s.x-786432.f,s.y-786432.f,1,s.sow/256,s.tow/256,0,1,255,255,255,255};}return v;}
struct F {
 std::unique_ptr<NfsFeedbackNativePass>p{new NfsFeedbackNativePass{}};
 NfsAudioNativeStamp stamp{1,17,29,11,2186,1640};uint64_t serial=0;
 std::array<float,16>matrix{{2.f/640,0,0,0,0,-2.f/480,0,0,0,0,1,0,-1,1,0,1}};
 std::vector<uint16_t>pixels;
 F():pixels(307200){for(unsigned i=0;i<pixels.size();++i)pixels[i]=uint16_t(i*79u^(i/63u));CK(nfs_feedback_native_created(p.get(),1,17,29));}
 void begin(uint64_t epoch=1){CK(nfs_feedback_native_begin(p.get(),&stamp,++serial,epoch,19,matrix.data(),pixels.data(),pixels.size()));}
 std::array<unsigned char,2048>payload(unsigned t){std::array<unsigned char,2048>b{};for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){auto w=pixels[((t%15)*32+y)*640+(t/15)*32+x];b[(y*32+x)*2]=uint8_t(w);b[(y*32+x)*2+1]=uint8_t(w>>8);}return b;}
 void upload(unsigned t,unsigned id=7){auto b=payload(t);CK(nfs_feedback_native_upload(p.get(),&stamp,64,id,10,32,b.data(),b.size()));}
 void select(unsigned id=7){CK(nfs_feedback_native_select(p.get(),&stamp,0,64,id,3,10,32,3,3,3));}
 void triangles(unsigned t){auto a=source(t,0),b=source(t,1),c=source(t,2),d=source(t,3);CK(nfs_feedback_native_triangle(p.get(),&stamp,&a,&b,&c,1,0));CK(nfs_feedback_native_triangle(p.get(),&stamp,&a,&c,&d,1,0));}
 NfsFeedbackActualBatch state(unsigned id=7){NfsFeedbackActualBatch s{};s.program=19;s.texture0=id;s.sampler=0;std::copy(matrix.begin(),matrix.end(),s.matrix);s.textureEnabled=1;s.blendEnabled=1;s.equationRGB=s.equationAlpha=NFS_FEEDBACK_GL_FUNC_ADD;s.sourceRGB=s.sourceAlpha=NFS_FEEDBACK_GL_SRC_ALPHA;s.destinationRGB=s.destinationAlpha=NFS_FEEDBACK_GL_ONE_MINUS_SRC_ALPHA;return s;}
 void batch(unsigned t,unsigned id=7){auto v=actual(t);auto s=state(id);CK(nfs_feedback_native_actual_batch(p.get(),&stamp,&s,v.data(),v.size()));}
 void tile(unsigned t,unsigned id=7){upload(t,id);select(id);triangles(t);batch(t,id);}
 void prefix(unsigned count){for(unsigned t=0;t<count;++t)tile(t);}
 void ready(){begin();upload(0);select();triangles(0);}
};
int main(){F f;f.begin();f.prefix(300);CK(nfs_feedback_native_finish(f.p.get(),&f.stamp,f.serial));
 f.begin();for(unsigned t=0;t<300;++t)f.tile(t,t+11);CK(nfs_feedback_native_finish(f.p.get(),&f.stamp,f.serial));
 // Every missing actual tile draw must fail before the next texture replacement,
 // including when all source uploads/selects/triangles reached the final tile.
 for(unsigned missing=0;missing<300;++missing){f.begin();f.prefix(missing);f.upload(missing);f.select();f.triangles(missing);if(missing<299){auto b=f.payload(missing+1);CK(!nfs_feedback_native_upload(f.p.get(),&f.stamp,64,7,10,32,b.data(),b.size()));}else CK(!nfs_feedback_native_finish(f.p.get(),&f.stamp,f.serial));CK(!f.p->original.active);}
 for(unsigned vertex=0;vertex<6;++vertex)for(unsigned field=0;field<11;++field){f.ready();auto v=actual(0);auto s=f.state();auto&q=v[vertex];switch(field){case 0:q.x+=1;break;case 1:q.y+=1;break;case 2:q.z=0;break;case 3:q.s+=.25f;break;case 4:q.t+=.25f;break;case 5:q.marker=72;break;case 6:q.q=std::numeric_limits<float>::quiet_NaN();break;case 7:q.r--;break;case 8:q.g--;break;case 9:q.b--;break;default:q.a--;}
  CK(!nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6)&&!f.p->original.active);}
 for(unsigned field=0;field<31;++field){f.ready();auto v=actual(0);auto s=f.state();if(field<16)s.matrix[field]+=.125f;else switch(field){case 16:s.program++;break;case 17:s.texture0++;break;case 18:s.sampler=1;break;case 19:s.textureEnabled=0;break;case 20:s.fogEnabled=1;break;case 21:s.hdActive=1;break;case 22:s.markActive=1;break;case 23:s.blendEnabled=2;break;case 24:s.equationRGB++;break;case 25:s.equationAlpha++;break;case 26:s.sourceRGB=0;break;case 27:s.destinationRGB=1;break;case 28:s.sourceAlpha=0;break;case 29:s.destinationAlpha=1;break;default:s.matrix[0]=std::numeric_limits<float>::infinity();}
  CK(!nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));}
 for(unsigned field=0;field<6;++field){f.ready();auto v=actual(0);auto s=f.state();auto now=f.stamp;if(field==0)now.generation++;if(field==1)now.context++;if(field==2)now.thread++;if(field==3)now.framebuffer++;if(field==4)now.width++;if(field==5)now.height++;CK(!nfs_feedback_native_actual_batch(f.p.get(),&now,&s,v.data(),6));}
 for(unsigned field=0;field<5;++field){f.ready();auto v=actual(0);auto s=f.state();CK(!nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,field==0?nullptr:&s,field==1?nullptr:field==4?reinterpret_cast<const NfsFeedbackActualVertex*>(f.p.get()):v.data(),field==2?5:field==3?7:6));}
 f.begin();f.upload(0);f.select();auto v=actual(0);auto s=f.state();CK(!nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));
 f.ready();f.batch(0);CK(!nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));
 f.begin();auto b=f.payload(0);b[37]^=1;CK(!nfs_feedback_native_upload(f.p.get(),&f.stamp,64,7,10,32,b.data(),2048));
 f.begin();CK(!nfs_feedback_native_upload(f.p.get(),&f.stamp,64,7,10,32,f.p->original.expected[0],2048));
 // Inherited HD state is deliberately not checked at begin/select/source input.
 // Only the post-sync actual draw state is authoritative for substitutions.
 f.ready();s=f.state();s.hdActive=0;s.markActive=0;CK(nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));nfs_feedback_native_other(f.p.get(),&f.stamp);
 f.ready();s=f.state();s.blendEnabled=0;s.sourceRGB=s.sourceAlpha=0;s.destinationRGB=s.destinationAlpha=1;CK(nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));nfs_feedback_native_other(f.p.get(),&f.stamp);
 for(unsigned sf=0;sf<2;++sf)for(unsigned df=0;df<2;++df){f.ready();s=f.state();s.sourceRGB=s.sourceAlpha=sf?NFS_FEEDBACK_GL_ONE:NFS_FEEDBACK_GL_SRC_ALPHA;s.destinationRGB=s.destinationAlpha=df?NFS_FEEDBACK_GL_ZERO:NFS_FEEDBACK_GL_ONE_MINUS_SRC_ALPHA;CK(nfs_feedback_native_actual_batch(f.p.get(),&f.stamp,&s,v.data(),6));nfs_feedback_native_other(f.p.get(),&f.stamp);}
 f.begin(2);nfs_feedback_native_other(f.p.get(),&f.stamp);CK(!nfs_feedback_native_begin(f.p.get(),&f.stamp,++f.serial,1,19,f.matrix.data(),f.pixels.data(),307200));
 for(unsigned fault=0;fault<5;++fault){auto projection=f.matrix;uint64_t epoch=fault==0?0:fault==1?UINT64_MAX:2;uint32_t program=fault==2?0:19;if(fault==3)projection[7]=std::numeric_limits<float>::quiet_NaN();CK(!nfs_feedback_native_begin(f.p.get(),&f.stamp,++f.serial,epoch,program,fault==4?f.p->matrix:projection.data(),f.pixels.data(),307200));}
 f.begin(2);nfs_feedback_native_retired(f.p.get(),99);CK(f.p->original.active);nfs_feedback_native_retired(f.p.get(),1);CK(!f.p->original.active&&!f.p->original.generation);CK(nfs_feedback_native_created(f.p.get(),2,31,37));f.stamp.generation=2;f.stamp.context=31;f.stamp.thread=37;f.begin(2);f.prefix(300);CK(nfs_feedback_native_finish(f.p.get(),&f.stamp,f.serial));
 CK(!nfs_feedback_native_created(nullptr,1,1,1)&&!nfs_feedback_native_actual_batch(nullptr,nullptr,nullptr,nullptr,0));
 std::printf("PASS %u synthetic modal native source+actual-batch checks; no live GL/native admission\n",checks);
}
