// SPDX-License-Identifier: MIT
// Authored complete phrases only. Synthetic packets provide no live authority.
#include "hd/ControlsBindingPhraseDisplay.h"
#include <cstdio>
#include <cstdlib>
#include <string>
using namespace nfs_soft;
using namespace nfs_controls_binding_phrase;
static unsigned checks=0;
#define CHECK(x) do {++checks;if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
static SurfaceIdentity surface(){SurfaceIdentity s;s.lifetime=1;auto&g=s.geometry;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=16;g[8]=0x10000;g[10]=1280;g[11]=0x20000;g[12]=0x30000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=16+479*1280;return s;}
static void setText(TextCommand&c,const char*t){c.text={};c.length=std::strlen(t);CHECK(c.length<c.text.size());std::memcpy(c.text.data(),t,c.length);}
static TextCommand unrelated(){TextCommand c;setText(c,"unrelated fixed label");c.role=1;c.sourceSize=18;c.x=20;c.y=20;c.advance=130;c.color=65535;return c;}
static std::vector<TextCommand>commands(unsigned image,unsigned row,unsigned column=0){
 auto*id=nfs_controls_binding_phrase_identity(image);CHECK(id);std::vector<TextCommand>result;int x=id->sourceSize==18?419:column?501:363,y=(id->sourceSize==18?144:129)+26*int(row);
 for(unsigned token=0;token<id->count;++token)for(unsigned layer=0;layer<2;++layer){TextCommand c;setText(c,id->tokens[token]);c.role=1;c.sourceSize=id->sourceSize;c.advance=id->advances[token];c.x=x+id->positions[token]+(layer?0:1);c.y=y+(layer?0:1);c.color=layer?65535:1;result.push_back(c);}return result;
}
static std::shared_ptr<const FramePacket>packet(const std::vector<TextCommand>&commands){
 FontDescription d;std::vector<uint8_t>alpha(1024*512);for(unsigned role=0;role<2;++role){d.baseline[role]=2;for(auto&g:d.glyphs[role])g={2,2,4,8,0,-8,65536};d.glyphs[role][0]={0,0,0,0,0,0,65536};}for(unsigned y=2;y<10;++y)for(unsigned x=2;x<6;++x)alpha[y*1024+x]=255;auto font=FontCatalog::create(d,alpha.data(),alpha.size(),1);CHECK(font);FramePacketBuilder builder(font);auto s=surface();std::vector<uint16_t>plane(640*480);CHECK(builder.begin(s,1,plane.data(),plane.size()));for(auto c:commands){CHECK(builder.beginString(c,c.sourceSize));CHECK(builder.word(0,plane[0],1,1));plane[0]=1;CHECK(builder.endString(c));}auto result=builder.finish(s,plane.data(),plane.size());CHECK(result);return result;
}
static void append(std::vector<TextCommand>&to,const std::vector<TextCommand>&from){to.insert(to.end(),from.begin(),from.end());}
static bool zero(const NfsControlsBindingPhrasesView&v){
 if(v.schema||v.imageCount||v.count)return false;for(auto&i:v.images){if(i.phrase||i.width||i.height||i.rgba||i.bytes)return false;for(auto n:i.sha256)if(n)return false;}for(auto&p:v.placements)if(p.slot||p.foreground565||p.shadow565||p.vertexFirst||p.vertexCount||p.left||p.top||p.right||p.bottom)return false;return true;
}
static void placement(const Prepared&p,unsigned at,unsigned image,unsigned row,unsigned column){
 const auto view=p.view();CHECK(at<view.count);const auto&g=p.plan()->groups()[at];const auto&v=view.placements[at];auto*id=nfs_controls_binding_phrase_identity(image);CHECK(id&&g.image==image&&g.row==row&&g.column==column);CHECK(v.slot&&v.slot<=view.imageCount);const auto&art=view.images[v.slot-1];CHECK(art.phrase==image&&art.width==id->width&&art.height==128&&art.bytes==size_t(id->width)*128*4);CHECK(art.rgba&&!std::memcmp(art.sha256,id->sha256,32));CHECK(v.vertexFirst==g.vertexFirst&&v.vertexCount==g.vertexCount&&v.foreground565==g.foregroundColor&&v.shadow565==g.shadowColor);CHECK(v.left==g.x&&v.top==g.y&&v.right==g.x+g.width&&v.bottom==g.y+32);
}
int main(){
 CHECK(!artwork(0)&&!artwork(361));CHECK(!Prepared::create({}));
 for(unsigned image=1;image<=360;++image){auto a=artwork(image);auto*id=nfs_controls_binding_phrase_identity(image);CHECK(a&&id);CHECK(a==artwork(image));CHECK(a->image==image&&a->width==id->width&&a->height==128&&a->rgba.size()==size_t(id->width)*128*4);unsigned char digest[32];CHECK(nfs_texture_sha256(a->rgba.data(),a->rgba.size(),digest));CHECK(!std::memcmp(digest,id->sha256,32)&&!std::memcmp(a->sha.data(),id->sha256,32));unsigned ink=0;for(size_t i=0;i<a->rgba.size();i+=4){CHECK(a->rgba[i]==255&&a->rgba[i+1]==255&&a->rgba[i+2]==255);ink+=a->rgba[i+3]!=0;}CHECK(ink);for(unsigned x=0;x<a->width;++x)CHECK(!a->rgba[size_t(x)*4+3]&&!a->rgba[(size_t(127)*a->width+x)*4+3]);for(unsigned y=0;y<128;++y)CHECK(!a->rgba[size_t(y)*a->width*4+3]&&!a->rgba[(size_t(y)*a->width+a->width-1)*4+3]);
  auto prepared=Prepared::create(packet(commands(image,0)));CHECK(prepared&&prepared->view().imageCount==1&&prepared->view().count==1);placement(*prepared,0,image,0,0);
 }
 unsigned short bad[]={0,0};CHECK(!decode(0,bad,2)&&!decode(361,bad,2));CHECK(!decode(1,nullptr,2));CHECK(!decode(1,bad,0)&&!decode(1,bad,1)&&!decode(1,bad,2));bad[0]=1;CHECK(!decode(1,bad,2));bad[1]=256;CHECK(!decode(1,bad,2));const auto&runs=nfs_controls_binding_phrase_runs[0];std::vector<unsigned short>corrupt(runs.data,runs.data+runs.count);corrupt[1]^=1;CHECK(!decode(1,corrupt.data(),corrupt.size()));CHECK(!decode(1,corrupt.data(),size_t(nfs_controls_binding_phrase_identity(1)->width)*128*2+2));
 // Forty strong cache slots are independently observed through weak owners.
 // A caller-retained image survives eviction, and reacquiring its ID decodes
 // another immutable object with the exact same maintained pixels.
 auto retained=artwork(1);CHECK(retained);std::vector<std::weak_ptr<const Artwork>>weak;for(unsigned image=2;image<=42;++image){auto a=artwork(image);CHECK(a);weak.push_back(a);}CHECK(weak.front().expired());unsigned live=0;for(auto&w:weak)live+=!w.expired();CHECK(live==40);auto refreshed=artwork(1);CHECK(refreshed&&refreshed!=retained&&refreshed->rgba==retained->rgba&&refreshed->sha==retained->sha);CHECK(retained->image==1);
 // Two rows share a whole phrase image while keeping independent source spans.
 unsigned single=nfs_controls_binding_phrase_find(18,1,1,"0");std::vector<TextCommand>stream{unrelated()};append(stream,commands(single,0));stream.push_back(unrelated());append(stream,commands(single,1));auto source=packet(stream);auto prepared=Prepared::create(source);CHECK(prepared&&prepared->packet()==source&&prepared->plan()->packet()==source);auto v=prepared->view();CHECK(v.schema==1&&v.imageCount==1&&v.count==2&&v.placements[0].slot==1&&v.placements[1].slot==1);placement(*prepared,0,single,0,0);placement(*prepared,1,single,1,0);CHECK(v.placements[0].vertexFirst>0&&v.placements[0].vertexFirst+v.placements[0].vertexCount<v.placements[1].vertexFirst);
 // Display leases own pixels and source independently of cache and caller.
 auto borrowed=v.images[0].rgba;std::vector<unsigned char>saved(borrowed,borrowed+v.images[0].bytes);std::weak_ptr<const FramePacket>sourceOwner=source;source.reset();for(unsigned image=80;image<130;++image)CHECK(artwork(image));CHECK(!sourceOwner.expired()&&prepared->packet()==sourceOwner.lock());CHECK(prepared->view().images[0].rgba==borrowed&&!std::memcmp(borrowed,saved.data(),saved.size()));prepared.reset();CHECK(sourceOwner.expired());
 unsigned left=nfs_controls_binding_phrase_find(16,0,0,"A"),right=nfs_controls_binding_phrase_find(16,2,2,"C");stream.clear();append(stream,commands(left,0,0));stream.push_back(unrelated());append(stream,commands(right,0,1));append(stream,commands(single,2));auto mixed=Prepared::create(packet(stream));CHECK(mixed&&mixed->view().count==3&&mixed->view().imageCount==3);placement(*mixed,0,left,0,0);placement(*mixed,1,right,0,1);placement(*mixed,2,single,2,0);
 auto unsupportedPacket=packet({unrelated()});auto unsupported=Prepared::create(unsupportedPacket);CHECK(unsupported&&unsupported->packet()==unsupportedPacket&&unsupported->plan()->groups().empty()&&zero(unsupported->view()));
 // One supported phrase in all twenty split rows/columns remains one image.
 stream.clear();for(unsigned row=0;row<10;++row)for(unsigned column=0;column<2;++column)append(stream,commands(left,row,column));auto full=Prepared::create(packet(stream));CHECK(full&&full->view().count==20&&full->view().imageCount==1);for(unsigned i=0;i<20;++i)placement(*full,i,left,i/2,i%2);
 std::printf("PASS binding phrase display checks=%u authored-images=360 cache-strong-bound=40 complete-phrase-bitmaps-only=1 synthetic-source=1\n",checks);
}
