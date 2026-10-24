// SPDX-License-Identifier: MIT
// Real maintained collector, public synthetic guest/target/font/word fixtures.
// No original execution, chart proof, native authority or rendered output.
#include "controls_setup_runtime_fixture.h"
static constexpr uint32_t byte_512E00=0x328,byte_512DFB=0x330;
static constexpr uint32_t dword_4D3658=0x12000,dword_4D366C=0x12020,dword_4D3680=0x12040,dword_4D3694=0x12060,dword_4D364C=0x12200;
#include "hd/ControlsSteeringTextGameHooks.inc"
namespace steering=nfs_controls_steering_text;
static void prepareSteering(unsigned level=0){
 prepare(false);put<uint16_t>(off_4D37C8+10,1);put<uint8_t>(byte_512E00,uint8_t(level));put<uint8_t>(byte_512DFB,0);put<uint32_t>(off_4D36A8,aMain);put<uint32_t>(off_4D36A8+4,aBlk4_0);put<uint32_t>(off_4D36A8+8,0);put<uint32_t>(off_4D36A8+28,0);
 const uint32_t at[]={dword_4D3658,dword_4D366C,dword_4D3680,dword_4D3694};
 const uint32_t words[4][5]={{0x1530002,0,4,byte_512E00,dword_4D364C},{0x1540002,UINT32_MAX,8,0,dword_4D24E8},{0x1550002,UINT32_MAX,16,0,dword_4D24E8},{0x1520000,0,0,0,0}};
 for(unsigned i=0;i<4;++i){put<uint32_t>(off_4D36A8+12+4*i,at[i]);for(unsigned k=0;k<5;++k)put<uint32_t>(at[i]+4*k,words[i][k]);}put<uint32_t>(dword_4D364C,0x1570156);put<uint32_t>(dword_4D364C+4,0x1590158);put<uint32_t>(dword_4D364C+8,0);
 const uint32_t styles[]={0x05050512,0x04020112,0x04020112,0x03020112,0x03040312,0x03040312,0x03040312,0x03040312},geometry[]={435u|(105u<<16),435u|(144u<<16),435u|(187u<<16),455u|(187u<<16),0,0,0,0};const char*names[]={"STEERING SENSITIVITY","sensitivity","DONE","CANCEL","full","three quarter","half","one quarter"};
 for(unsigned i=0;i<8;++i){const uint32_t entry=Table+12*(0x152+i),text=0x1c0000+128*i;put<uint32_t>(entry,styles[i]);put<uint32_t>(entry+4,geometry[i]);put<uint32_t>(entry+8,text-Table);std::memcpy(memory.data()+text,names[i],std::strlen(names[i])+1);}
}
#define NFS_CONTROLS_STEERING_SIDEBAR_TEXT_TEST 1
#include "hd/ControlsSteeringSidebarCaptionGameHooks.inc"
namespace cap=nfs_controls_steering_sidebar_text;
using CaptionProof=std::shared_ptr<const nfs_controls_steering_sidebar_caption::Proof>;

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

static void positiveRetained(unsigned level){
 prepareSteering(level);Game g;g.eax=off_4D36A8;steering::OwnerScope parent(g,0x4101c0);CHECK(steering::current(g));cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();
 auto proof=captureCaption(g);CHECK(proof&&proof->current());auto packet=cap::acquireCurrentPacket(g);CHECK(packet&&packet==proof->packet()&&packet->commands()==1&&packet->writes()==1);
 CHECK(nfs_nested_sidebar_caption::exactCommand(packet->records()[0].command));CHECK(packet->clean()[369*343+123]==0&&packet->original()[369*343+123]==65535);
 const auto target=selected;auto raw=compactRaw();CHECK(retained(g,proof,target,raw)==proof);
 // Caption history remains usable after valid row/sensitivity changes, but the
 // current independent steering descriptors/resources must still authenticate.
 for(unsigned next=0;next<4;++next){put<uint8_t>(byte_512E00,uint8_t(next));put<uint16_t>(off_4D36A8+10,uint16_t(next%3));CHECK(retained(g,proof,target,raw)==proof);}
 selected.w[1]=640;g.ebp=0x135;CHECK(!cap::acquireCurrentPacket(g));CHECK(retained(g,proof,target,raw)==proof);selected=target;
 auto corrupt=raw;corrupt[50]^=1;CHECK(!retained(g,proof,target,corrupt));auto badTarget=target;badTarget.x[100]^=2;CHECK(!retained(g,proof,badTarget,raw));badTarget=target;badTarget.owner++;CHECK(!retained(g,proof,badTarget,raw));
 const auto epoch=steering::current(g)->ownerEpoch;++steering::current(g)->ownerEpoch;CHECK(!cap::current(g)&&!retained(g,proof,target,raw));steering::current(g)->ownerEpoch=epoch;CHECK(retained(g,proof,target,raw)==proof);
 const auto life=cap::current(g)->lifetime;++cap::current(g)->lifetime;CHECK(!retained(g,proof,target,raw));cap::current(g)->lifetime=life;CHECK(retained(g,proof,target,raw)==proof);
 for(uint32_t at:{dword_4D3658,Table+12*0x153}){auto old=nfs::GuestCopyTransaction{}.load<uint32_t>(at);put<uint32_t>(at,old^1);CHECK(!retained(g,proof,target,raw));put<uint32_t>(at,old);CHECK(retained(g,proof,target,raw)==proof);}
 memory[Text+7]='!';CHECK(!retained(g,proof,target,raw));memory[Text+7]=0;CHECK(retained(g,proof,target,raw)==proof);
 Game other=g;CHECK(!cap::current(other)&&!retained(other,proof,target,raw));const auto beforeReads=reads;std::thread foreign([&]{Game other;other.eax=off_4D36A8;cap::OwnerScope denied(other);CHECK(!cap::current(other));});foreign.join();CHECK(reads==beforeReads);
 // A separate refcounted scratch descriptor sharing LUTs is not retirement;
 // the last reference or a shared bitmap is conservatively rejected.
 const uint32_t scratch=0x1d0000;for(unsigned k=0;k<14;++k)put<uint32_t>(scratch+4*k,target.w[k]);put<uint32_t>(scratch+32,0x1e0000);
 put<uint32_t>(target.w[11]-4,2);put<uint32_t>(target.w[12]-4,2);CHECK(!cap::descriptorRetires(*cap::current(g),scratch));cap::beforeDescriptorRetire(g,scratch);CHECK(retained(g,proof,target,raw)==proof);
 put<uint32_t>(target.w[11]-4,1);CHECK(cap::descriptorRetires(*cap::current(g),scratch));put<uint32_t>(target.w[11]-4,2);put<uint32_t>(scratch+32,target.w[8]);CHECK(cap::descriptorRetires(*cap::current(g),scratch));
 cap::beforeDescriptorRetire(g,target.owner);CHECK(!retained(g,proof,target,raw)&&cap::current(g)->lifetime>life);CHECK(proof->packet()->original()[369*343+123]==65535);
}
static void historyAfterOwnerReturn(){
 CaptionProof history;Game g;
 {prepareSteering();g.eax=off_4D36A8;steering::OwnerScope parent(g,0x4101c0);cap::OwnerScope owner(g);compactTarget();history=captureCaption(g);CHECK(history);}
 CHECK(!steering::current(g)&&!cap::current(g)&&!cap::lastProof(g));CHECK(history->current()&&history->packet()->original()[369*343+123]==65535);
}
static void failures(){
 for(const char*mode:{"ebp","text","size","missing-measure","measure-pointer","width","center","anchor","x","bad-word","extra-write","palette","size-exit","font-header","x-lut","y-lut","raw-owner","source-entry","authority","retire","text-exit","terminator-exit","nested-tail","unwind"}){
  prepareSteering();Game g;g.eax=off_4D36A8;steering::OwnerScope parent(g,0x4101c0);cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();CHECK(!captureCaption(g,mode));CHECK(!leaseDepth&&allocationsInLease==0);
 }
}
static void owners(){
 for(unsigned mode=0;mode<4;++mode){prepareSteering();Game g;g.eax=off_4D36A8;steering::OwnerScope parent(g,0x4101c0);auto*p=steering::current(g);CHECK(p);const auto form=p->form,handler=p->handler;const auto epoch=p->ownerEpoch;
  if(mode==0)g.eax=off_4D37C8;if(mode==1)p->handler=0x403970;if(mode==2)p->ownerEpoch=0;if(mode==3)p->form=off_4D349C;
  {cap::OwnerScope denied(g);CHECK(!cap::current(g));}p->form=form;p->handler=handler;p->ownerEpoch=epoch;
 }
 prepareSteering();Game g;g.eax=off_4D36A8;steering::OwnerScope parent(g,0x4101c0);cap::OwnerScope owner(g);CHECK(cap::current(g));compactTarget();CHECK(captureCaption(g));g.eax=off_4D36A8;{cap::OwnerScope nested(g);CHECK(!cap::lastProof(g));}CHECK(!cap::lastProof(g));
}
int main(){CHECK(nfs_soft::installFontCatalog(font()));for(unsigned level=0;level<4;++level)positiveRetained(level);failures();owners();historyAfterOwnerReturn();CHECK(!leaseDepth&&allocationsInLease==0);std::printf("PASS steering sidebar caption runtime checks=%u synthetic-only=1 no_fake_sealer=1\n",checks);}
