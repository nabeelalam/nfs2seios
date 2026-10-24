// SPDX-License-Identifier: MIT
// Synthetic owned immutable packets exercise presentation parsing only.
#include "hd/ControlsBindingPhrasePlan.h"
#include <cstdio>
#include <cstdlib>
#include <string>
using namespace nfs_soft;
using nfs_controls_binding_phrase::Plan;
static unsigned checks=0;
#define CHECK(x) do {++checks;if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
static SurfaceIdentity surface(){SurfaceIdentity s;s.lifetime=1;auto&g=s.geometry;g[1]=640;g[2]=480;g[5]=640;g[6]=480;g[7]=16;g[8]=0x10000;g[10]=1280;g[11]=0x20000;g[12]=0x30000;g[15]=2;g[16]=1278;g[17]=16;g[18]=1296;g[19]=16+479*1280;return s;}
static void setText(TextCommand&c,const char*t){c.text={};c.length=std::strlen(t);CHECK(c.length<c.text.size());std::memcpy(c.text.data(),t,c.length);}
static TextCommand unrelated(){TextCommand c;setText(c,"unrelated fixed label");c.role=1;c.sourceSize=18;c.x=20;c.y=20;c.advance=130;c.color=65535;return c;}
static std::vector<TextCommand>commands(const NfsControlsBindingPhraseIdentity&id,unsigned row=0,unsigned column=0){
 std::vector<TextCommand>result;int x=id.sourceSize==18?419:column?501:363,y=(id.sourceSize==18?144:129)+26*int(row);
 for(unsigned token=0;token<id.count;++token)for(unsigned layer=0;layer<2;++layer){TextCommand c;setText(c,id.tokens[token]);c.role=1;c.sourceSize=id.sourceSize;c.advance=id.advances[token];c.x=x+id.positions[token]+(layer?0:1);c.y=y+(layer?0:1);c.color=layer?65535:1;result.push_back(c);}return result;
}
static std::shared_ptr<const FramePacket>packet(const std::vector<TextCommand>&commands){
 FontDescription d;std::vector<uint8_t>alpha(1024*512);for(unsigned role=0;role<2;++role){d.baseline[role]=2;for(auto&g:d.glyphs[role])g={2,2,4,8,0,-8,65536};d.glyphs[role][0]={0,0,0,0,0,0,65536};}for(unsigned y=2;y<10;++y)for(unsigned x=2;x<6;++x)alpha[y*1024+x]=255;auto font=FontCatalog::create(d,alpha.data(),alpha.size(),1);CHECK(font);
 FramePacketBuilder builder(font);auto s=surface();std::vector<uint16_t>plane(640*480);CHECK(builder.begin(s,1,plane.data(),plane.size()));for(auto c:commands){CHECK(builder.beginString(c,c.sourceSize));CHECK(builder.word(0,plane[0],1,1));plane[0]=1;CHECK(builder.endString(c));}auto result=builder.finish(s,plane.data(),plane.size());CHECK(result);return result;
}
static void checkGroup(const std::shared_ptr<const FramePacket>&p,unsigned image,size_t first,unsigned row=0,unsigned column=0){auto plan=Plan::create(p);CHECK(plan&&plan->packet()==p&&plan->groups().size()==1);auto&g=plan->groups()[0];auto*id=nfs_controls_binding_phrase_identity(image);CHECK(id);CHECK(g.image==image&&g.sourceSize==id->sourceSize&&g.row==row&&g.column==column);CHECK(g.firstRecord==first&&g.recordCount==2*id->count);CHECK(g.vertexFirst==p->records()[first].vertexFirst);const auto&last=p->records()[first+g.recordCount-1];CHECK(g.vertexCount==last.vertexFirst+last.vertexCount-g.vertexFirst);CHECK(g.width==int(id->width/4)&&g.shadowColor==1&&g.foregroundColor==65535);}
int main(){
 CHECK(!Plan::create({}));CHECK(!nfs_controls_binding_phrase_identity(0));CHECK(!nfs_controls_binding_phrase_identity(361));
 // Every finite image is independently reachable from its complete value, and
 // every grammar parses through real FramePacketBuilder command/vertex storage.
 for(unsigned image=1;image<=360;++image){auto*id=nfs_controls_binding_phrase_identity(image);CHECK(id&&id->image==image);CHECK(nfs_controls_binding_phrase_find(id->sourceSize,id->kind,id->device,id->name)==image);auto stream=commands(*id);auto p=packet(stream);checkGroup(p,image,0);}
 CHECK(!nfs_controls_binding_phrase_find(17,0,0,"A"));CHECK(!nfs_controls_binding_phrase_find(18,0,0,"a"));CHECK(!nfs_controls_binding_phrase_find(16,1,3,"0"));CHECK(!nfs_controls_binding_phrase_find(16,1,1,"16"));CHECK(!nfs_controls_binding_phrase_find(18,2,1,"a"));CHECK(!nfs_controls_binding_phrase_find(18,2,1,"AA"));
 auto*axis=nfs_controls_binding_phrase_identity(nfs_controls_binding_phrase_find(18,1,1,"0"));CHECK(axis);
 for(unsigned mode=0;mode<17;++mode){auto c=commands(*axis);switch(mode){
 case 0:setText(c[0],"joyStick");break;
 case 1:++c[0].advance;break;
 case 2:++c[1].x;break;
 case 3:++c[2].x;break;
 case 4:c[3].color=37904;break;
 case 5:c[2].sourceSize=16;break;
 case 6:c[2].alignment=1;break;
 case 7:c[2].originX=1;break;
 case 8:setText(c[2],"3");setText(c[3],"3");break;
 case 9:std::swap(c[2],c[4]);std::swap(c[3],c[5]);break;
 case 10:for(auto&q:c)q.clip.right=419+axis->width/4;break;
 case 11:c[0].clip.right=639;break;
 case 12:for(auto&q:c)++q.y;break;
 case 13:c[2].role=0;break;
 case 14:setText(c[6],"16");setText(c[7],"16");break;
 case 15:c.insert(c.begin()+4,unrelated());break;
 case 16:c.pop_back();break;
 }auto plan=Plan::create(packet(c));CHECK(plan&&plan->groups().empty());}
 // A malformed earlier group never discards a later unrelated valid binding.
 auto malformed=commands(*axis);malformed[2].x++;auto good=commands(*axis,1);malformed.insert(malformed.end(),good.begin(),good.end());checkGroup(packet(malformed),axis->image,8,1);
 // Both split columns and source16/source18 observations remain separate; no
 // presentation grouping claims that these synthetic mixed forms are live.
 auto single=commands(*axis,2);auto*split=nfs_controls_binding_phrase_identity(nfs_controls_binding_phrase_find(16,0,0,"A"));CHECK(split);auto left=commands(*split,3,0),right=commands(*split,3,1);std::vector<TextCommand>mixed{unrelated()};mixed.insert(mixed.end(),single.begin(),single.end());mixed.push_back(unrelated());mixed.insert(mixed.end(),left.begin(),left.end());mixed.push_back(unrelated());mixed.insert(mixed.end(),right.begin(),right.end());mixed.push_back(unrelated());auto mixedPacket=packet(mixed);auto plan=Plan::create(mixedPacket);CHECK(plan&&plan->groups().size()==3);CHECK(plan->groups()[0].firstRecord==1&&plan->groups()[0].sourceSize==18);CHECK(plan->groups()[1].firstRecord==10&&plan->groups()[1].sourceSize==16&&plan->groups()[1].column==0);CHECK(plan->groups()[2].firstRecord==15&&plan->groups()[2].column==1);CHECK(plan->groups()[0].vertexFirst>0);CHECK(plan->groups()[0].vertexFirst+plan->groups()[0].vertexCount<mixedPacket->records()[10].vertexFirst);
 // The plan retains the exact immutable packet after the caller releases it.
 std::weak_ptr<const FramePacket>owner=mixedPacket;mixedPacket.reset();CHECK(!owner.expired()&&plan->packet()==owner.lock());plan.reset();CHECK(owner.expired());
 // Entire nonstandard layouts remain original while keeping an inspectable plan.
 auto exception=commands(*axis);for(unsigned i=4;i<exception.size();++i)exception[i].x+=8;auto declined=Plan::create(packet(exception));CHECK(declined&&declined->groups().empty());
 // The parser checks actual component-vertex box separation. Builder fitting
 // keeps this synthetic font inside each identity's separated allocation, so
 // this fixture does not claim coverage of an impossible overlapping packet.
 std::printf("PASS binding phrase presentation plan checks=%u identities=360 immutable-packets=owned source-authority=none overlap-malformed-packet=not-injected\n",checks);
}
