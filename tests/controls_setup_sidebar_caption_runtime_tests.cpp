// SPDX-License-Identifier: MIT
// Actual child-caption and setup helpers against the existing public synthetic
// guest/authority fixture. No private Proof sealer or game glyph execution.
#include "controls_setup_runtime_fixture.h"
#define NFS_CONTROLS_SETUP_SIDEBAR_TEXT_TEST 1
#include "hd/ControlsSetupSidebarCaptionGameHooks.inc"
namespace cap=nfs_controls_setup_sidebar_text;
using CaptionProof=std::shared_ptr<const nfs_controls_setup_sidebar_caption::Proof>;

static void compactTarget(){
 selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=Plane;w[10]=686;w[11]=0x15000;w[12]=0x16000;selected.owner=0x17000;selected.ownerWords=w;selected.ownerState[1]=selected.owner;selected.ownerState[3]=0x17500;
 selected.header={{0x78,343u|(480u<<16),0,0}};for(unsigned k=0;k<343;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;
 for(unsigned k=0;k<4;++k)put<uint32_t>(Plane+4*k,selected.header[k]);
 put<int32_t>(dword_4E54AC,18);put<uint32_t>(dword_4DD714,65535);std::memcpy(memory.data()+Text,"options",8);
}
static void captionString(Game&g,const char*mode){
 g.eax=Text;g.edx=!std::strcmp(mode,"x")?121:120;g.ebx=364;g.esi=!std::strcmp(mode,"anchor")?169:168;
 if(!std::strcmp(mode,"text"))memory[Text]='X';if(!std::strcmp(mode,"size"))put<int32_t>(dword_4E54AC,16);
 if(std::strcmp(mode,"missing-measure"))cap::measured(g,!std::strcmp(mode,"measure-pointer")?Text+1:Text,!std::strcmp(mode,"width")?47:48,!std::strcmp(mode,"center")?cap::SourceMeasureAlignment::center:cap::SourceMeasureAlignment::right);
 {cap::StringScope string(g);
  {cap::GlyphScope glyph(g);const uint32_t at=Plane+16+369*686+246;
   {cap::WordCapture word(g,at,65535);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?123:65535);}
   if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);
  }
  if(!std::strcmp(mode,"text-exit"))memory[Text]='X';
  if(!std::strcmp(mode,"terminator-exit"))memory[Text+7]='!';
  if(!std::strcmp(mode,"palette"))put<uint32_t>(Palette+4,123);
  if(!std::strcmp(mode,"size-exit"))put<int32_t>(dword_4E54AC,16);
  if(!std::strcmp(mode,"font-header"))memory[Font]^=1;
  if(!std::strcmp(mode,"x-lut"))selected.x[170]^=2;
  if(!std::strcmp(mode,"y-lut"))selected.y[220]^=2;
  if(!std::strcmp(mode,"raw-owner"))selected.ownerWords[8]^=4;
  if(!std::strcmp(mode,"source-entry"))put<uint32_t>(dword_4D3E9C,0x70003);
  if(!std::strcmp(mode,"authority"))put<uint32_t>(off_4D37C8+12,dword_4D3778);
  if(!std::strcmp(mode,"retire"))cap::beforeOwnerRetire(g,selected.owner);
  if(!std::strcmp(mode,"unwind"))throw 7;
  put<int32_t>(dword_4DD6EC,168);
 }
 // Restoring guest text before tail completion must not revive a string whose
 // text/terminator changed while that original StringScope was still active.
 if(!std::strcmp(mode,"text-exit")||!std::strcmp(mode,"terminator-exit"))std::memcpy(memory.data()+Text,"options",8);
}
static CaptionProof captureCaption(Game&g,const char*mode="normal"){
 auto*s=cap::current(g);CHECK(s);g.ebp=!std::strcmp(mode,"ebp")?s->proofForm+4:s->proofForm;
 try{cap::TailScope tail(g);if(!std::strcmp(mode,"nested-tail")){cap::TailScope nested(g);}captionString(g,mode);}catch(int){CHECK(!std::strcmp(mode,"unwind"));}
 return cap::lastProof(g);
}
static std::vector<uint8_t>compactRaw(){return std::vector<uint8_t>(memory.data()+Plane,memory.data()+Plane+16+343*480*2);}
static CaptionProof retained(Game&g,const CaptionProof&p,const nfs_logo_capture::Target&t,const std::vector<uint8_t>&raw){return cap::acquireRetainedProof(g,p,p?p->packet():nullptr,p?p->evidence().serial:0,p?p->evidence().lifetime:0,t,raw.data(),raw.size());}

static void positiveRetained(bool split){
 prepare(split);Game g;g.eax=split?off_4D35E8:off_4D349C;setup::OwnerScope parent(g,split?0x403970:0x4101c0);CHECK(setup::current(g));cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();
 auto proof=captureCaption(g);CHECK(proof&&proof->current());auto packet=cap::acquireCurrentPacket(g);CHECK(packet&&packet==proof->packet()&&packet->commands()==1&&packet->writes()==1);
 CHECK(nfs_nested_sidebar_caption::exactCommand(packet->records()[0].command));CHECK(packet->clean()[369*343+123]==0&&packet->original()[369*343+123]==65535);
 const auto target=selected;auto raw=compactRaw();CHECK(retained(g,proof,target,raw)==proof);
 // Mutable selection, binding values and active split column are observed live;
 // immutable caption history is not authority to freeze them at capture time.
 const unsigned columns=split?2:1;
 for(unsigned column=0;column<columns;++column){put<uint8_t>(byte_512DA8,uint8_t(column));put<uint16_t>((split?off_4D35E8:off_4D349C)+10,1);put<uint32_t>(split?dword_512E34:dword_512E0C,0x450004+column);CHECK(retained(g,proof,target,raw)==proof);}
 put<uint16_t>((split?off_4D35E8:off_4D349C)+10,0);
 for(unsigned column=0;column<columns;++column){put<uint8_t>(byte_512DA8,uint8_t(column));put<uint8_t>(inControlAssignMode,1);g.eax=setup::current(g)->form;g.edx=split?dual[0]:single[0];
  {setup::PromptScope prompt(g);g.ebp=0x135;CHECK(retained(g,proof,target,raw)==proof);}
  put<uint8_t>(inControlAssignMode,0);CHECK(retained(g,proof,target,raw)==proof);
 }
 // Independently supplied copy arguments remain mandatory in an ambient full640
 // phase. Neither that phase's selected target nor its EBP135 grants a caption.
 selected.w[1]=640;g.ebp=0x135;CHECK(!cap::acquireCurrentPacket(g));CHECK(retained(g,proof,target,raw)==proof);selected=target;
 auto corrupt=raw;corrupt[50]^=1;CHECK(!retained(g,proof,target,corrupt));auto badTarget=target;badTarget.x[100]^=2;CHECK(!retained(g,proof,badTarget,raw));badTarget=target;badTarget.owner++;CHECK(!retained(g,proof,badTarget,raw));
 const auto epoch=setup::current(g)->ownerEpoch;++setup::current(g)->ownerEpoch;CHECK(!cap::current(g)&&!retained(g,proof,target,raw));setup::current(g)->ownerEpoch=epoch;CHECK(retained(g,proof,target,raw)==proof);
 const auto captionLife=cap::current(g)->lifetime;++cap::current(g)->lifetime;CHECK(!retained(g,proof,target,raw));cap::current(g)->lifetime=captionLife;CHECK(retained(g,proof,target,raw)==proof);
 const auto entryWord=uint32_t(0x70002);put<uint32_t>(dword_4D3E9C,0x70003);CHECK(!retained(g,proof,target,raw));put<uint32_t>(dword_4D3E9C,entryWord);CHECK(retained(g,proof,target,raw)==proof);
 memory[Text+7]='!';CHECK(!retained(g,proof,target,raw));memory[Text+7]=0;CHECK(retained(g,proof,target,raw)==proof);
 Game other=g;CHECK(!cap::current(other)&&!retained(other,proof,target,raw));const auto beforeReads=reads;std::thread foreign([&]{Game other;other.eax=split?off_4D35E8:off_4D349C;cap::OwnerScope denied(other);CHECK(!cap::current(other));});foreign.join();CHECK(reads==beforeReads);
 const auto life=cap::current(g)->lifetime;cap::beforeDescriptorRetire(g,target.owner);CHECK(!retained(g,proof,target,raw)&&cap::current(g)->lifetime>life);CHECK(proof->packet()->original()[369*343+123]==65535);
}
static void historyAfterOwnerReturn(){
 CaptionProof history;Game g;
 {prepare(false);g.eax=off_4D349C;setup::OwnerScope parent(g,0x4101c0);cap::OwnerScope owner(g);compactTarget();history=captureCaption(g);CHECK(history);}
 CHECK(!setup::current(g)&&!cap::current(g)&&!cap::lastProof(g));CHECK(history->current()&&history->packet()->original()[369*343+123]==65535);
}
static void failures(bool split){
 for(const char*mode:{"ebp","text","size","missing-measure","measure-pointer","width","center","anchor","x","bad-word","extra-write","palette","size-exit","font-header","x-lut","y-lut","raw-owner","source-entry","authority","retire","text-exit","terminator-exit","nested-tail","unwind"}){
  prepare(split);Game g;g.eax=split?off_4D35E8:off_4D349C;setup::OwnerScope parent(g,split?0x403970:0x4101c0);cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();CHECK(!captureCaption(g,mode));CHECK(!leaseDepth&&allocationsInLease==0);
 }
}
static void owners(){
 for(unsigned mode=0;mode<4;++mode){prepare(false);Game g;g.eax=off_4D349C;setup::OwnerScope parent(g,0x4101c0);auto*p=setup::current(g);CHECK(p);const auto form=p->form,handler=p->handler;const auto epoch=p->ownerEpoch;
  if(mode==0)g.eax=off_4D37C8;if(mode==1)p->handler=0x403970;if(mode==2)p->ownerEpoch=0;if(mode==3)p->form=off_4D36A8;
  {cap::OwnerScope denied(g);CHECK(!cap::current(g));}p->form=form;p->handler=handler;p->ownerEpoch=epoch;
 }
 prepare(false);Game g;g.eax=off_4D349C;setup::OwnerScope parent(g,0x4101c0);cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();CHECK(captureCaption(g));g.eax=off_4D349C;{cap::OwnerScope nested(g);CHECK(!cap::lastProof(g));}CHECK(!cap::lastProof(g));
}
int main(){CHECK(nfs_soft::installFontCatalog(font()));for(bool split:{false,true}){positiveRetained(split);failures(split);}owners();historyAfterOwnerReturn();CHECK(!leaseDepth&&allocationsInLease==0);std::printf("PASS child Setup sidebar genuine helper capture over synthetic authority: checks=%u no_fake_sealer=1 no_game_execution=1\n",checks);}
