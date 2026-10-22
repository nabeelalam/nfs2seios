// SPDX-License-Identifier: MIT
#include "hd/PlayerSwatchPacket.h"
#include <iostream>
#include <stdexcept>
#include <functional>
namespace sw=nfs_player_swatch;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x))throw std::runtime_error(#x);}while(0)
static sw::Target target(){sw::Target t{};t.w[1]=640;t.w[2]=480;t.w[5]=640;t.w[6]=480;t.w[7]=0x7810;t.w[8]=0x100000;t.w[10]=1280;t.w[11]=0x30000;t.w[12]=0x31000;t.header[0]=0x78;t.header[1]=640|(480u<<16);for(unsigned k=0;k<640;++k)t.x[k]=2*k;for(unsigned k=0;k<480;++k)t.y[k]=16+1280*k;t.owner=0x41000;t.ownerWords=t.w;return t;}
struct Fixture{
 sw::Authority authority;sw::Input in;sw::Plane base,current;
 Fixture(unsigned player=1,sw::Origin origin=sw::Origin::Refresh,std::array<uint32_t,4>clip={{0,0,640,480}}){auto&b=authority.binding;b.epoch=11;b.serial=12;b.lifetime=13;b.owner=0x77000;b.archive=0x78000;b.player=player;b.origin=origin;b.form=player==1?0x4d3050:0x4d3100;b.row=player==1?sw::Player1Row:(origin==sw::Origin::Refresh?sw::Player2Row:sw::Player2BodyRow);b.car=4;b.color=7;b.carAddress=player==1?0x512dbf:0x512dda;b.colorAddress=(player==1?0x512dc1:0x512ddc)+b.car;b.tableAddress=sw::ColorTable+16*b.color;b.table={{0,255,255,82}};authority.events={20,21,22,23,24,25,26};
  in.binding=b;in.events=authority.events;in.complete=1;in.rgb={{255,0,0}};in.rawColor=0xffff0000u;base.valid=1;base.width=640;base.height=480;base.target=target();for(unsigned k=0;k<4;++k)base.target.w[3+k]=clip[k];for(unsigned k=0;k<sw::Full;++k)base.pixels[k]=uint16_t(k*13+7);in.pre=base;in.post=base;
  // Independent oracle: this synthetic table selects undimmed red; first fill
  // black, second fill RGB565 red. Do not call implementation color/pack here.
  for(unsigned k=0;k<2;++k){auto&f=in.fills[k];unsigned x=k?366:367,y=k?141:142;uint16_t value=k?0xf800:0;f.before=f.after=base.target;f.call={{x,y,84,21}};f.rawColor=k?0xffff0000u:0xff000000u;f.pattern=value|(uint32_t(value)<<16);f.callback=sw::FillCallback;f.format=16;f.complete=1;f.tableBefore=f.tableAfter={{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
   const unsigned left=std::max(x,clip[0]),top=std::max(y,clip[1]),right=std::min(x+84,clip[2]),bottom=std::min(y+21,clip[3]);
   if(left>=right||top>=bottom){f.callback=0;continue;}
   for(unsigned yy=top;yy<bottom;++yy){sw::Row row;row.callback=sw::FillCallback;row.format=16;row.pattern=f.pattern;row.args[1]=base.target.w[8]+16+(yy*640+left)*2;row.args[2]=right-left;for(unsigned xx=left;xx<right;++xx){auto&pixel=in.post.pixels[yy*640+xx];row.before.push_back(pixel);row.after.push_back(value);pixel=value;}f.rows.push_back(std::move(row));}
  }current=in.post;
 }
};
int main(){try{
 uint32_t raw=0;std::array<uint32_t,3>rgb{};
 CHECK((sw::color({{0,255,255,82}},raw,&rgb)&&raw==0xffff0000u&&rgb==std::array<uint32_t,3>({{255,0,0}})));
 CHECK(sw::color({{64,255,255,82}},raw)&&raw==0xff7fff00u);
 // Carry regression: 1*32768 discards bit15=1, so ADC rounds to1, not0.
 CHECK(sw::color({{64,1,255,82}},raw)&&raw==0xfffefffeu);
 CHECK(sw::color({{64,254,255,82}},raw)&&raw==0xff80ff01u);
 CHECK(sw::color({{128,255,255,82}},raw)&&raw==0xff00ffffu);
 CHECK(sw::color({{192,255,255,82}},raw)&&raw==0xff7f00ffu);
 CHECK(sw::color({{255,255,255,82}},raw)&&raw==0xffff0006u);
 CHECK(sw::color({{0,255,255,164}},raw)&&raw==0xff7f0000u);
 CHECK(sw::color({{0,0,210,80}},raw)&&raw==0xffd3d3d3u);
 CHECK(sw::color({{0,255,255,0}},raw)&&raw==0xffff0000u);
 CHECK(sw::color({{254,255,255,231}},raw));
 CHECK(!sw::color({{256,0,0,0}},raw));CHECK(sw::pack(0xffff0000u)==0xf800&&sw::pack(0xff00ff00u)==0x07e0&&sw::pack(0xff0000ffu)==31);
 sw::Failure failure=sw::Failure::None;
 for(auto origin:{sw::Origin::Refresh,sw::Origin::Body})for(unsigned player:{1u,2u}){auto f=std::unique_ptr<Fixture>(new Fixture(player,origin));auto packet=sw::createPacket(f->in,f->authority,f->base,f->current,&failure);CHECK(packet&&failure==sw::Failure::None);CHECK(packet->post()==std::vector<uint16_t>(f->current.pixels.begin(),f->current.pixels.end()));CHECK(packet->pixels().size()==85*22&&packet->coverage().size()==85*22);CHECK(packet->pixels()[0]==0xf800&&packet->pixels()[21*85+84]==0);CHECK(packet->coverage()[84]==0&&packet->coverage()[21*85]==0);CHECK(packet->matches(f->authority,f->current.target,f->current.pixels.data(),sw::Full));
  CHECK(packet->preMatches(f->base));CHECK(packet->preMatches(f->base.target,f->base.pixels.data(),sw::Full));
  CHECK(!packet->preMatches(f->base.target,nullptr,sw::Full));CHECK(!packet->preMatches(f->base.target,f->base.pixels.data(),sw::Full-1));
  f->base.pixels[0]^=1;CHECK(!packet->preMatches(f->base));f->base.pixels[0]^=1;
  f->base.target.owner++;CHECK(!packet->preMatches(f->base));f->base.target.owner--;
  CHECK(!packet->preMatches(f->current));
  auto old=f->authority;f->authority.binding.epoch++;CHECK(!packet->matches(f->authority,f->current.target,f->current.pixels.data(),sw::Full));f->authority=old;
  std::vector<uint8_t>other(sw::Full);CHECK(packet->disjoint(other.data(),other.size()));other[141*640+366]=1;CHECK(!packet->disjoint(other.data(),other.size()));other[141*640+366]=0;other[162*640+366]=1;CHECK(packet->disjoint(other.data(),other.size()));CHECK(!packet->disjoint(nullptr,sw::Full));
  const auto preserved=packet->post();f->in.post.pixels.fill(0);f->in.fills[1].rows.clear();f->current.pixels.fill(0);CHECK(packet->post()==preserved&&packet->pixels()[0]==0xf800);
 }
 // Actual selected640 clip can expose only18/17 columns and19/18 rows.
 {auto f=std::unique_ptr<Fixture>(new Fixture(1,sw::Origin::Body,{{433,144,640,480}}));CHECK(f->in.fills[0].rows.size()==19&&f->in.fills[0].rows[0].args[2]==18);CHECK(f->in.fills[1].rows.size()==18&&f->in.fills[1].rows[0].args[2]==17);auto p=sw::createPacket(f->in,f->authority,f->base,f->current,&failure);CHECK(p);CHECK(std::count(p->coverage().begin(),p->coverage().end(),uint8_t(255))==342);CHECK(p->coverage()[0]==0&&p->coverage()[3*85+67]==255);CHECK(p->post()[141*640+366]==f->base.pixels[141*640+366]);}
 for(const std::array<uint32_t,4>&clip:{std::array<uint32_t,4>{{0,0,343,480}},std::array<uint32_t,4>{{640,480,640,480}}}){auto f=std::unique_ptr<Fixture>(new Fixture(1,sw::Origin::Body,clip));CHECK(f->in.fills[0].rows.empty()&&f->in.fills[1].rows.empty());auto p=sw::createPacket(f->in,f->authority,f->base,f->current,&failure);CHECK(p&&std::all_of(p->coverage().begin(),p->coverage().end(),[](uint8_t v){return v==0;}));CHECK(p->post()==std::vector<uint16_t>(f->base.pixels.begin(),f->base.pixels.end()));f->in.post.pixels[0]^=1;f->current=f->in.post;CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));}
 // One-row color fill already contains red. Unchanged value still has coverage;
 // the clipped-out shadow contributes no fabricated callback or write.
 {auto f=std::unique_ptr<Fixture>(new Fixture(2,sw::Origin::Body,{{366,141,450,142}}));CHECK(f->in.fills[0].rows.empty()&&f->in.fills[1].rows.size()==1);f->base=f->current;f->in.pre=f->base;f->in.fills[1].rows[0].before.assign(84,0xf800);auto p=sw::createPacket(f->in,f->authority,f->base,f->current,&failure);CHECK(p);CHECK(std::count(p->coverage().begin(),p->coverage().end(),uint8_t(255))==84);CHECK(p->post()==std::vector<uint16_t>(f->base.pixels.begin(),f->base.pixels.end()));}
 // Selected343 storage remains explicitly unsupported, not a640 no-write event.
 {auto f=std::unique_ptr<Fixture>(new Fixture);f->base.width=343;CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));}
 {auto f=std::unique_ptr<Fixture>(new Fixture(1,sw::Origin::Body,{{0,0,343,480}}));f->in.fills[0].callback=sw::FillCallback;CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));}
 {auto f=std::unique_ptr<Fixture>(new Fixture(1,sw::Origin::Body,{{433,144,640,480}}));f->in.fills[1].rows[0].args[2]=18;CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));}
 unsigned negatives=0;auto bad=[&](const std::function<void(Fixture&)>&mutate){auto f=std::unique_ptr<Fixture>(new Fixture);mutate(*f);CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));CHECK(failure!=sw::Failure::None);++negatives;};
 bad([](Fixture&f){f.in.binding.lifetime++;});bad([](Fixture&f){f.authority.binding.epoch++;});bad([](Fixture&f){f.in.binding.archive++;});bad([](Fixture&f){f.in.binding.owner++;});bad([](Fixture&f){f.in.binding.serial++;});bad([](Fixture&f){f.in.events.table++;});
 bad([](Fixture&f){f.authority.events.rgb=f.authority.events.table;f.in.events=f.authority.events;});bad([](Fixture&f){f.authority.binding.player=3;f.in.binding=f.authority.binding;});bad([](Fixture&f){f.authority.binding.row=sw::Player2Row;f.in.binding=f.authority.binding;});bad([](Fixture&f){f.authority.binding.carAddress++;f.in.binding=f.authority.binding;});bad([](Fixture&f){f.authority.binding.colorAddress++;f.in.binding=f.authority.binding;});bad([](Fixture&f){f.authority.binding.tableAddress+=16;f.in.binding=f.authority.binding;});
 bad([](Fixture&f){f.in.binding.table[0]++;});bad([](Fixture&f){f.in.rgb[0]--;});bad([](Fixture&f){f.in.rawColor^=1;});bad([](Fixture&f){f.in.complete=0;});bad([](Fixture&f){f.in.error=1;});bad([](Fixture&f){f.in.pre.pixels[0]^=1;});bad([](Fixture&f){f.in.post.target.x[639]--;});bad([](Fixture&f){f.current.target.owner++;});
 bad([](Fixture&f){f.in.fills[0].call[0]--;});bad([](Fixture&f){f.in.fills[0].rawColor=0;});bad([](Fixture&f){f.in.fills[1].pattern^=1;});bad([](Fixture&f){f.in.fills[1].tableAfter[3]++;});bad([](Fixture&f){f.in.fills[1].callback++;});bad([](Fixture&f){f.in.fills[1].converter=1;});bad([](Fixture&f){f.in.fills[1].rows.pop_back();});bad([](Fixture&f){f.in.fills[1].before.y[479]--;});
 bad([](Fixture&f){f.in.fills[1].rows[0].args[1]+=2;});bad([](Fixture&f){f.in.fills[1].rows[0].args[2]--;});bad([](Fixture&f){f.in.fills[1].rows[0].before[0]^=1;});bad([](Fixture&f){f.in.fills[1].rows[0].after[0]^=1;});bad([](Fixture&f){f.in.fills[1].rows[0].pattern^=1;});bad([](Fixture&f){f.in.fills[1].rows[0].callback++;});bad([](Fixture&f){f.in.fills[1].rows[0].format=15;});bad([](Fixture&f){f.in.fills[1].rows[0].converter=1;});
 bad([](Fixture&f){f.in.post.pixels[0]^=1;f.current=f.in.post;});bad([](Fixture&f){f.current.pixels[100]^=1;});bad([](Fixture&f){std::swap(f.in.fills[0],f.in.fills[1]);});
 bad([](Fixture&f){f.authority.binding.form=0x4d3100;f.in.binding=f.authority.binding;});bad([](Fixture&f){f.authority.binding.origin=static_cast<sw::Origin>(99);f.in.binding=f.authority.binding;});
 for(auto origin:{sw::Origin::Refresh,sw::Origin::Body}){auto f=std::unique_ptr<Fixture>(new Fixture(2,origin));f->authority.binding.row=origin==sw::Origin::Refresh?sw::Player2BodyRow:sw::Player2Row;f->in.binding=f->authority.binding;CHECK(!sw::createPacket(f->in,f->authority,f->base,f->current,&failure));}
 CHECK(negatives==41);std::cout<<"PASS player swatch packet checks="<<checks<<" negatives="<<negatives<<" synthetic-only=1\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<"\n";return 1;}}