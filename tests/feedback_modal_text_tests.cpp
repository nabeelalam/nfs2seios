// SPDX-License-Identifier: MIT
// Synthetic owned data only. Consistency checks are not lexical/source proof.
#include "hd/FeedbackModalTextProof.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <vector>
#include <iostream>
// Load production and standard headers globally before isolating existing pure
// fixtures; their unrelated test mains are renamed and never called here.
namespace backdrop_fixture {
#define main unusedBackdropFixtureMain
#include "feedback_modal_backdrop_tests.cpp"
#undef main
}
#undef CHECK
namespace parent_fixture {
#define main unusedParentFixtureMain
#include "controls_dialog_word_proof_tests.cpp"
#undef main
}
#undef CHECK
namespace modal=nfs_feedback_modal_text;
using Packet=std::shared_ptr<const nfs_soft::FramePacket>;
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL modal text line %d: %s\n",__LINE__,#x);std::abort();}}while(0)
static_assert(!std::is_default_constructible<modal::Proof>::value,"no public source proof");
static_assert(!std::is_constructible<modal::Proof,std::shared_ptr<const modal::CheckedText>>::value,"pure consistency cannot seal lexical proof");
static_assert(!std::is_convertible<std::shared_ptr<const modal::CheckedText>,std::shared_ptr<const modal::Proof>>::value,"checked text grants no source/native authority");
struct Fixture {
 std::unique_ptr<backdrop_fixture::Fixture>raw{new backdrop_fixture::Fixture};
 std::shared_ptr<const nfs_feedback_backdrop::VerifiedBackdrop>backdrop;
 modal::Evidence e{};std::vector<nfs_soft::TextCommand>commands;Packet p;
 Fixture(){
  backdrop=nfs_feedback_backdrop::ReplayVerifier::verify(raw->r,raw->r.binding,raw->catalog,raw->selectors);CHECK(backdrop);
  auto parent=parent_fixture::evidence(2);e.binding=backdrop->binding();e.source.controls=parent.source;e.source.controls.form=e.binding.form;e.source.controls.parentMenu=e.binding.parent;
  e.source.caller=0xf0100010;e.source.modal=0xf0100020;e.source.stringDraw=0xf0100030;e.source.measure=0xf0100040;e.source.glyph=e.source.controls.glyphCallback;
  e.authority=parent.authority;auto&a=e.authority;e.controlsArchive=parent.authority.archive;a.archive=e.controlsArchive;CHECK(a.archive!=e.binding.archive);a.table=0x80000;a.parentMenu=a.parentMenuChild=e.binding.parent;a.parentWords[1]=e.binding.form;a.texts[2]=e.binding.titleResource;
  for(unsigned i=0;i<5;++i)parent_fixture::put32(a.headers[i],8,a.texts[i]-a.table);
  e.game=123;e.thread=456;e.serial=17;e.fontIdentity=17;e.caller=e.source.caller;e.modal=e.source.modal;e.count=2;e.feedbackAvailable=0;
  e.post=parent.post;e.post.identity=e.binding.surface;e.post.target=raw->r.pretextTarget;e.post.archive=e.controlsArchive;e.post.color=65535;
  const char*names[]={"force feedback","device not found"};const int x[]={302,266},y[]={185,225},adv[]={96,108};
  for(unsigned i=0;i<2;++i){auto&r=e.resources[i];r.id=i?0x301:0x14a;r.table=a.table;r.headerAddress=r.table+12*r.id;r.textAddress=i?e.binding.messageResource:e.binding.titleResource;r.headerBefore={{18,1,2,4,0xa0,1,0xe0,0,0,0,0,0}};parent_fixture::put32(r.headerBefore,8,r.textAddress-r.table);r.headerAfter=r.headerBefore;std::memcpy(r.textBefore.data(),names[i],std::strlen(names[i]));r.textAfter=r.textBefore;r.alignment=2;r.colorIndex=i?2:0;r.anchor=i?320:350;r.y=y[i];r.measuredWidth=adv[i];r.resolves=r.measures=r.strings=1;
   r.before=r.resolved=r.measured=e.post;r.before.color=r.resolved.color=r.measured.color=i?1:65535;r.stringBefore=r.after=e.post;r.stringBefore.color=r.after.color=i?65535:1;
   // Distinct stable per-string palette contents are allowed. The selected
   // color must not silently change within the original string operation.
   for(auto*s:{&r.stringBefore,&r.after})s->paletteWords[5]=10+i;
   commands.push_back(parent_fixture::string(names[i],x[i],y[i],adv[i],i,1,i?65535:1));
  }
  e.post=e.resources[1].after;p=packet(commands);CHECK(p);
 }
 Packet packet(const std::vector<nfs_soft::TextCommand>&cmds,uint64_t serial=17,bool wrongClean=false){auto font=parent_fixture::font();nfs_soft::FramePacketBuilder b(font);auto plane=backdrop->pretext();if(wrongClean)plane.back()^=1;CHECK(b.begin(e.binding.surface,serial,plane.data(),plane.size()));for(size_t i=0;i<cmds.size();++i){CHECK(b.beginString(cmds[i]));auto at=size_t(180*640+300+i);auto val=uint16_t(plane[at]^0x7fff);CHECK(b.word(at,plane[at],val,val));plane[at]=val;CHECK(b.endString(cmds[i]));}return b.finish(e.binding.surface,plane.data(),plane.size());}
 modal::Current current()const{modal::Current c;c.observed=e;c.packet=p;c.original565=p->original().data();c.planeWords=p->pixels();return c;}
};
int main(){
 Fixture f;CHECK(modal::consistent(f.p,f.e,f.backdrop));auto checked=modal::CheckedText::create(f.p,f.e,f.backdrop);CHECK(checked);CHECK(checked->matches(f.current()));CHECK(checked->packet()==f.p&&checked->backdrop()==f.backdrop);
 CHECK(!modal::CheckedText::create({},f.e,f.backdrop));CHECK(!modal::CheckedText::create(f.p,f.e,{}));
 const std::vector<std::function<void(modal::Evidence&)>>bad={
 [](auto&e){e.resources[0].headerBefore[1]++;e.resources[0].headerAfter=e.resources[0].headerBefore;},[](auto&e){e.game=0;},[](auto&e){e.thread=0;},[](auto&e){e.serial++;},[](auto&e){e.fontIdentity++;},[](auto&e){e.source.caller=0;},[](auto&e){e.caller++;},[](auto&e){e.modal++;},[](auto&e){e.count=1;},[](auto&e){e.feedbackAvailable=1;},[](auto&e){e.controlsArchive=0;},[](auto&e){e.controlsArchive=e.binding.archive;},[](auto&e){e.authority.archive=e.binding.archive;},[](auto&e){e.resources[0].stringBefore.archive=e.binding.archive;},[](auto&e){e.authority.selectedIndex=1;},[](auto&e){e.authority.terminator=1;},[](auto&e){e.source.controls.form++;},[](auto&e){e.binding.modalEpoch++;},[](auto&e){e.binding.resourceEpoch++;},[](auto&e){e.binding.lifetime++;},[](auto&e){e.binding.targetOwner++;},[](auto&e){e.resources[0].id++;},[](auto&e){e.resources[1].textAddress++;},[](auto&e){e.resources[0].headerAfter[4]++;},[](auto&e){e.resources[0].headerAddress++;},[](auto&e){e.resources[0].textBefore[0]='F';},[](auto&e){e.resources[0].textAfter[0]='F';},[](auto&e){e.resources[1].textBefore[16]='x';e.resources[1].textAfter=e.resources[1].textBefore;},[](auto&e){e.resources[0].alignment=1;},[](auto&e){e.resources[0].colorIndex=2;},[](auto&e){e.resources[0].anchor=416;},[](auto&e){e.resources[0].y=224;},[](auto&e){e.resources[1].measuredWidth++;},[](auto&e){e.resources[0].strings=2;},[](auto&e){e.resources[0].measures=0;},[](auto&e){e.resources[0].resolved.callback++;},[](auto&e){e.resources[0].stringBefore.paletteWords[0]++;},[](auto&e){e.resources[0].after.fontHeader[0]++;},[](auto&e){e.resources[0].before.target.x[300]++;},[](auto&e){e.post.target.y[300]++;},[](auto&e){e.resources[0].after.identity.lifetime++;},[](auto&e){e.resources[1].after.color=1;},[](auto&e){e.post.font++;},[](auto&e){e.post.color=1;}
 };
 for(const auto&mutate:bad){auto e=f.e;mutate(e);CHECK(!modal::CheckedText::create(f.p,e,f.backdrop));}
 for(unsigned fault=0;fault<12;++fault){auto cmds=f.commands;if(fault==0)cmds[0].text[0]='F';if(fault==1)cmds[0].x++;if(fault==2)cmds[1].y++;if(fault==3)cmds[0].advance++;if(fault==4)cmds[0].alignment=2;if(fault==5)cmds[1].role=0;if(fault==6)cmds[0].color=65535;if(fault==7)cmds[0].clip.right=639;if(fault==8)std::swap(cmds[0],cmds[1]);if(fault==9)cmds.pop_back();if(fault==10)cmds.push_back(cmds[0]);if(fault==11)cmds[0].originX=1;auto p=f.packet(cmds);CHECK(p&&!modal::CheckedText::create(p,f.e,f.backdrop));}
 CHECK(!modal::CheckedText::create(f.packet(f.commands,18),f.e,f.backdrop));CHECK(!modal::CheckedText::create(f.packet(f.commands,17,true),f.e,f.backdrop));
 for(unsigned fault=0;fault<16;++fault){auto c=f.current();if(fault==0)c.observed.game++;if(fault==1)c.observed.thread++;if(fault==2)c.observed.source.modal++;if(fault==3)c.observed.binding.ownerEpoch++;if(fault==4)c.observed.binding.modalEpoch++;if(fault==5)c.observed.binding.resourceEpoch++;if(fault==6)c.observed.resources[0].headerBefore[1]++;if(fault==7)c.observed.resources[0].after.palette++;if(fault==8)c.observed.post.target.ownerWords[0]++;if(fault==9)c.original565=nullptr;if(fault==10)c.planeWords--;if(fault==11)c.packet=f.packet(f.commands);if(fault==12)c.observed.count=0;if(fault==13)c.observed.serial++;if(fault==14)c.observed.controlsArchive=0;if(fault==15)c.observed.controlsArchive=c.observed.binding.archive;CHECK(!checked->matches(c));}
 auto plane=f.p->original();plane.back()^=1;auto c=f.current();c.original565=plane.data();CHECK(!checked->matches(c));
 // Owning copies outlive input evidence, fixture planes and caller shared owners.
 auto retainedCurrent=f.current();std::weak_ptr<const nfs_soft::FramePacket>weak=f.p;f.e.resources[0].textBefore[0]='x';f.raw->r.pretext.clear();f.p.reset();CHECK(!weak.expired());CHECK(checked->matches(retainedCurrent));CHECK(checked->evidence().resources[0].textBefore[0]=='f');
 std::printf("PASS %u synthetic modal text consistency/current/ownership checks; no lexical or native admission claimed\n",checks);
}
