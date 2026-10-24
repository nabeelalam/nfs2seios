// SPDX-License-Identifier: MIT
#include "hd/ControlsSetupWordDisplay.h"
#include <cstdio>
#include <cstdlib>
using namespace nfs_soft;
using nfs_controls_setup_word_display::Prepared;
static unsigned checks=0;
#define CHECK(x) do { ++checks; if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);std::exit(1);} } while(0)
static SurfaceIdentity surface(){SurfaceIdentity s;s.lifetime=1;auto&g=s.geometry;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=16;g[8]=0x10000;g[10]=1280;g[11]=0x20000;g[12]=0x30000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=16+479*1280;return s;}
static TextCommand label(unsigned id){TextCommand c;const char*t=nfs_controls_setup_word_text[id-1];c.length=std::strlen(t);std::memcpy(c.text.data(),t,c.length);c.sourceSize=nfs_controls_setup_word_source_size[id-1];c.advance=nfs_controls_setup_word_advance[id-1];c.x=20;c.y=30;c.color=0xffe0;return c;}
static std::shared_ptr<const FramePacket> packet(const std::vector<TextCommand>&commands){
 FontDescription d;std::vector<uint8_t>alpha(1024*512);
 for(unsigned role=0;role<2;++role){d.baseline[role]=2;for(auto&g:d.glyphs[role])g={2,2,8,8,0,-8,3*65536};d.glyphs[role][0]={0,0,0,0,0,0,2*65536};}
 for(unsigned y=2;y<10;++y)for(unsigned x=2;x<10;++x)alpha[y*1024+x]=255;
 auto font=FontCatalog::create(d,alpha.data(),alpha.size(),1);CHECK(font);
 FramePacketBuilder b(font);std::vector<uint16_t>plane(640*480);auto s=surface();CHECK(b.begin(s,1,plane.data(),plane.size()));
 for(auto c:commands){CHECK(b.beginString(c,c.sourceSize));CHECK(b.word(0,plane[0],1,1));plane[0]=1;CHECK(b.endString(c));}
 auto result=b.finish(s,plane.data(),plane.size());CHECK(result);return result;
}
int main(){
 using namespace nfs_controls_setup_word_display;
 CHECK(!Prepared::create({}));CHECK(!artwork(0));CHECK(!artwork(29));
 for(unsigned i=1;i<=28;++i){auto a=artwork(i);CHECK(a);CHECK(a->word==i);CHECK(a->width==4*nfs_controls_setup_word_advance[i-1]);CHECK(a->height==128);CHECK(!std::memcmp(a->sha.data(),nfs_controls_setup_word_sha256[i-1],32));
  auto f=packet({label(i)});auto p=Prepared::create(f);CHECK(p&&p->packet()==f);auto v=p->view();CHECK(v.schema==1&&v.count==1);CHECK(v.placements[0].word==i);CHECK(v.placements[0].vertexFirst==0);CHECK(v.placements[0].vertexCount==f->vertices().size());CHECK(v.placements[0].left==20&&v.placements[0].right==20+label(i).advance);CHECK(v.placements[0].color565==0xffe0);
 }
 // Exact labels replace only their source spans, preserving the dynamic binding
 // command between them and both source-size variants independently.
 auto dynamic=label(1);dynamic.text={};std::memcpy(dynamic.text.data(),"keyboard A",10);dynamic.length=10;dynamic.advance=60;
 auto f=packet({label(1),dynamic,label(2)});auto p=Prepared::create(f);CHECK(p);auto v=p->view();CHECK(v.count==2);CHECK(v.placements[0].vertexFirst==f->records()[0].vertexFirst);CHECK(v.placements[1].vertexFirst==f->records()[2].vertexFirst);CHECK(v.placements[0].vertexFirst+v.placements[0].vertexCount<=f->records()[1].vertexFirst);
 for(unsigned mode=0;mode<6;++mode){auto c=label(1);if(mode==0)c.advance++;if(mode==1)c.text[0]='S';if(mode==2)c.sourceSize=16;if(mode==3)c.originX=1;if(mode==4)c.clip.right=c.x+c.advance-1;if(mode==5)c.y=449;CHECK(!Prepared::create(packet({c})));}
 auto c=label(21);std::vector<TextCommand>full(48,c);CHECK(Prepared::create(packet(full))->view().count==48);full.push_back(c);CHECK(!Prepared::create(packet(full)));
 CHECK(!decode(1,nullptr,2));unsigned short bad[]={0,0};CHECK(!decode(1,bad,2));bad[0]=1;bad[1]=256;CHECK(!decode(1,bad,2));auto copied=std::vector<unsigned short>(std::begin(nfs_controls_setup_word_alpha_rle_1),std::end(nfs_controls_setup_word_alpha_rle_1));copied[3]^=1;CHECK(!decode(1,copied.data(),copied.size()));
 std::printf("PASS Controls setup words checks=%u artwork=28 dynamic-spans=preserved synthetic-only=1\n",checks);
}
