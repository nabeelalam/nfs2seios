// SPDX-License-Identifier: MIT
// Drive the actual child caption collector over public synthetic guest memory.
// No forged Proof, original assets, original glyph execution or GPU publication.
#include "controls_setup_runtime_fixture.h"
#define NFS_CONTROLS_SETUP_SIDEBAR_TEXT_TEST 1
#include "hd/ControlsSetupSidebarCaptionGameHooks.inc"
#include "hd/ControlsSetupSidebarCaptionDisplay.h"
namespace cap=nfs_controls_setup_sidebar_text;
namespace display=nfs_controls_setup_sidebar_caption;
using ProofOwner=std::shared_ptr<const display::Proof>;

static void compactTarget(){
 selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;
 w[7]=0x7810;w[8]=Plane;w[10]=686;w[11]=0x15000;w[12]=0x16000;
 selected.owner=0x17000;selected.ownerWords=w;selected.ownerState[1]=selected.owner;selected.ownerState[3]=0x17500;
 selected.header={{0x78,343u|(480u<<16),0,0}};
 for(unsigned k=0;k<343;++k)selected.x[k]=2*k;
 for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;
 for(unsigned k=0;k<4;++k)put<uint32_t>(Plane+4*k,selected.header[k]);
 put<int32_t>(dword_4E54AC,18);put<uint32_t>(dword_4DD714,65535);
 std::memcpy(memory.data()+Text,"options",8);
}
static ProofOwner capture(Game&g){
 auto*s=cap::current(g);CHECK(s);g.ebp=s->proofForm;
 {
  cap::TailScope tail(g);g.eax=Text;g.edx=120;g.ebx=364;g.esi=168;
  cap::measured(g,Text,48,cap::SourceMeasureAlignment::right);
  {
   cap::StringScope string(g);
   // Seven checked RGB565 writes in the real343 target. The synthetic fixture
   // supplies the observed computed words; it does not execute original glyphs.
   for(unsigned k=0;k<7;++k){cap::GlyphScope glyph(g);
    const uint32_t at=Plane+16+369*686+(123+k)*2;
    {cap::WordCapture word(g,at,65535);put<uint16_t>(at,65535);}
   }
   put<int32_t>(dword_4DD6EC,168);
  }
 }
 auto p=cap::lastProof(g);CHECK(p&&p->packet()&&p->packet()->writes()==7);
 return p;
}
static display::Binding binding(const ProofOwner&p){
 const auto&e=p->evidence();display::Binding b;
 b.form=e.form;b.handler=e.handler;b.game=e.game;b.ownerEpoch=e.ownerEpoch;
 b.archive=e.post.archive;b.serial=e.serial;b.lifetime=e.lifetime;return b;
}
static std::shared_ptr<const display::Display>exercise(bool split){
 prepare(split);Game g;g.eax=split?off_4D35E8:off_4D349C;
 setup::OwnerScope setupOwner(g,split?0x403970:0x4101c0);CHECK(setup::current(g));
 cap::OwnerScope captionOwner(g);CHECK(cap::current(g));compactTarget();
 auto proof=capture(g);auto packet=proof->packet();const auto b=binding(proof);
 auto viewOwner=display::Display::create(proof,packet,b);
 CHECK(viewOwner&&viewOwner->current()&&viewOwner->proof()==proof&&viewOwner->packet()==packet);
 CHECK(nfs_nested_sidebar_caption::exactCommand(packet->records()[0].command));
 for(unsigned k=0;k<7;++k){CHECK(packet->clean()[369*343+123+k]==0);CHECK(packet->original()[369*343+123+k]==65535);}
 const auto view=viewOwner->view();auto art=nfs_compact_caption_display::artwork(3);
 CHECK(art&&view.schema==NFS_COMPACT_CAPTION_WORD_SCHEMA&&view.word==3);
 CHECK(view.width==192&&view.height==128&&view.bytes==192u*128u*4u);
 CHECK(view.left==120&&view.top==364&&view.right==168&&view.bottom==396);
 CHECK(view.rgba==art->rgba.data()&&!std::memcmp(view.sha256,art->sha.data(),32));
 std::array<unsigned char,32>sha{};CHECK(nfs_texture_sha256(view.rgba,view.bytes,sha.data()));
 CHECK(!std::memcmp(sha.data(),view.sha256,32));
 CHECK(!display::Display::create({},packet,b));CHECK(!display::Display::create(proof,{},b));
 // Each required independently supplied binding field rejects both zero and
 // drift, including a plausible OTHER child handler rather than just garbage.
 for(unsigned k=0;k<7;++k)for(bool zero:{false,true}){auto bad=b;
  if(k==0)bad.form=zero?0:b.form+4;
  if(k==1)bad.handler=zero?0:(split?0x4101c0:0x403970);
  if(k==2)bad.game=zero?0:b.game+1;
  if(k==3)bad.ownerEpoch=zero?0:b.ownerEpoch+1;
  if(k==4)bad.archive=zero?0:b.archive+4;
  if(k==5)bad.serial=zero?0:b.serial+1;
  if(k==6)bad.lifetime=zero?0:b.lifetime+1;
  CHECK(!display::Display::create(proof,packet,bad));
 }
 // A second real helper capture has a distinct serial. Equal dimensions and
 // text cannot authorize cross-pairing its packet with the first sealed Proof.
 auto second=capture(g);CHECK(second!=proof&&second->packet()!=packet);
 CHECK(!display::Display::create(proof,second->packet(),b));
 CHECK(!display::Display::create(second,packet,binding(second)));
 CHECK(display::Display::create(second,second->packet(),binding(second)));
 // Historical display retains immutable approved artwork and source packet;
 // changing guest memory or retiring the live owner grants no live authority.
 memory[Text]='X';put<uint16_t>(Plane+16+369*686+246,123);
 CHECK(!cap::acquireCurrentPacket(g));CHECK(viewOwner->current());
 CHECK(viewOwner->packet()->original()[369*343+123]==65535);
 const auto again=viewOwner->view();CHECK(again.rgba==view.rgba&&!std::memcmp(again.sha256,view.sha256,32));
 cap::beforeOwnerRetire(g,selected.owner);CHECK(!cap::lastProof(g));CHECK(viewOwner->current());
 CHECK(!leaseDepth&&allocationsInLease==0);return viewOwner;
}
int main(){
 static_assert(!std::is_default_constructible<display::Display>::value,"typed factory only");
 static_assert(!std::is_convertible<std::shared_ptr<const nfs_nested_sidebar_caption::Proof>,ProofOwner>::value,"ancestor caption proof is a distinct type");
 CHECK(nfs_soft::installFontCatalog(font()));
 auto singleDisplay=exercise(false);auto splitDisplay=exercise(true);
 Game inactive;CHECK(!setup::state()&&!cap::current(inactive));
 CHECK(singleDisplay->current()&&splitDisplay->current());
 CHECK(singleDisplay->binding().form==off_4D349C&&splitDisplay->binding().form==off_4D35E8);
 CHECK(!display::Display::create(singleDisplay->proof(),splitDisplay->packet(),singleDisplay->binding()));
 CHECK(!display::Display::create(splitDisplay->proof(),singleDisplay->packet(),splitDisplay->binding()));
 CHECK(singleDisplay->view().rgba==splitDisplay->view().rgba);
 CHECK(singleDisplay->packet()->writes()==7&&splitDisplay->packet()->writes()==7);
 CHECK(!leaseDepth&&allocationsInLease==0);
 std::printf("PASS typed authored child Setup OPTIONS display: checks=%u no_fake_sealer=1 no_native_claim=1\n",checks);
}
