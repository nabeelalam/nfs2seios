// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "SoftwareFontProvider.h"
#include "LocationCaptionProof.h"
#ifndef NFS_LOCATION_CAPTION_TEST
#include "ContinuousLogoGameHooks.inc"
#endif
#include <atomic>
#include <exception>
#include <limits>
#include <new>
#include <cstring>
namespace nfs_location_caption {
constexpr unsigned Width=343,Height=480;constexpr size_t Pixels=Width*Height;
enum class Failure {none,owner,nested,read,target,fontState,unsupportedText,palette,sourceChanged,command,word,unwind,capacity,retired,selection,postPlane};
inline const char*failureName(Failure f)noexcept{switch(f){case Failure::none:return "none";case Failure::owner:return "owner";case Failure::nested:return "nested";case Failure::read:return "read";case Failure::target:return "compact-target";case Failure::fontState:return "font-state";case Failure::unsupportedText:return "caption-text";case Failure::palette:return "palette";case Failure::sourceChanged:return "state-changed";case Failure::command:return "command";case Failure::word:return "original-word";case Failure::unwind:return "unwind";case Failure::capacity:return "capacity";case Failure::retired:return "retired";case Failure::selection:return "selection";case Failure::postPlane:return "post-plane";}return "unknown";}
struct Current {nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};uint32_t archive=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0;int32_t originX=0,originY=0,size=0;std::array<uint32_t,16> paletteWords{};std::array<uint8_t,16> fontHeader{};};
struct State {
 std::shared_ptr<const nfs_soft::FontCatalog> font; nfs_soft::FramePacketBuilder builder;
 std::shared_ptr<const nfs_soft::FramePacket> packet;std::vector<uint16_t> pixels;
 std::shared_ptr<const nfs_location_caption_source::Proof>proof;
 nfs_soft::TextCommand proofCommand{};nfs_location_caption_source::Snapshot proofStringBefore{},proofStringAfter{};
 bool proofStringComplete=false;uint32_t formGuest=0,proofTextGuest=0;
 uintptr_t game=0;unsigned ownerDepth=0,tailDepth=0,glyphDepth=0,stringsInTail=0;uint64_t lifetime=0,serial=0,nextString=0,string=0,glyph=0,failures=0;
 bool recording=false;Failure failure=Failure::none;Current current{};uint32_t ownerWindow=0;
 explicit State(std::shared_ptr<const nfs_soft::FontCatalog> f):font(std::move(f)),builder(font),pixels(Pixels){}
 void reject(Failure f)noexcept{failure=f;if(f!=Failure::none&&failures!=UINT64_MAX)++failures;builder.invalidate();packet.reset();proof.reset();proofStringComplete=false;recording=false;string=glyph=0;glyphDepth=0;}
 bool advance(uint64_t&v)noexcept{if(v==UINT64_MAX){reject(Failure::capacity);return false;}++v;return true;}
};
inline uintptr_t token()noexcept{static thread_local unsigned char t;return reinterpret_cast<uintptr_t>(&t);}
inline std::atomic<uintptr_t>&ownerThread()noexcept{static std::atomic<uintptr_t> t{0};return t;}
struct Storage{State*p=nullptr;~Storage(){delete p;if(ownerThread().load()==token())ownerThread().store(0);}};
inline Storage&storage(){static thread_local Storage s;return s;}
inline State*state()noexcept{if(ownerThread().load(std::memory_order_acquire)!=token())return nullptr;return storage().p;}
inline State*current(Game&g)noexcept{auto*s=state();return s&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->ownerDepth==1?s:nullptr;}
inline bool sameTarget(const Current&a,const Current&b)noexcept{return a.archive==b.archive&&a.identity==b.identity&&nfs_logo_capture::same(a.target,b.target);}
inline bool sameFont(const Current&a,const Current&b)noexcept{return sameTarget(a,b)&&a.font==b.font&&a.callback==b.callback&&a.palette==b.palette&&a.color==b.color&&a.mode==b.mode&&a.flags==b.flags&&a.format==b.format&&a.size==b.size&&a.originX==b.originX&&a.originY==b.originY&&a.paletteWords==b.paletteWords&&a.fontHeader==b.fontHeader;}
// Reads complete actual owner/header and all343X/480Y entries; never pads to640.
inline bool capture(State&s,Current&c,bool pixels,bool fontState)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,c.target);const auto&w=c.target.w;if(w[1]!=Width||w[2]!=Height||w[7]!=0x7810||w[10]!=686||w[13]||!c.target.owner||(c.target.header[0]&127)!=0x78||c.target.header[1]!=(Width|(Height<<16))||w[3]>w[5]||w[5]>Width||w[4]>w[6]||w[6]>Height)return false;for(unsigned k=0;k<Width;++k)if(c.target.x[k]!=2*k)return false;for(unsigned k=0;k<Height;++k)if(c.target.y[k]!=16+686*k)return false;for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(c.target.ownerWords[k]!=w[k])return false;
 if(uint64_t(w[8])+16+Pixels*2>0x100000000ULL)return false;c.archive=tx.load<uint32_t>(dword_4D40B8);if(!c.archive)return false;c.identity.lifetime=s.lifetime;std::copy(w.begin(),w.end(),c.identity.geometry.begin());auto&a=c.identity.geometry;a[14]=c.target.x[0];a[15]=c.target.x[1];a[16]=c.target.x[342];a[17]=c.target.y[0];a[18]=c.target.y[1];a[19]=c.target.y[479];if(!c.identity.valid())return false;
 if(fontState){c.font=tx.load<uint32_t>(dword_4DD770);c.callback=tx.load<uint32_t>(dword_4DD77C);c.palette=tx.load<uint32_t>(dword_4DD774);c.color=tx.load<uint32_t>(dword_4DD714);c.mode=tx.load<uint32_t>(dword_4D20A0);c.flags=tx.load<uint8_t>(byte_4DB25E);c.format=tx.load<uint8_t>(byte_4DB25C);c.originX=tx.load<int32_t>(dword_4DD6F4);c.originY=tx.load<int32_t>(dword_4DD6F8);c.size=tx.load<int32_t>(dword_4E54AC);if(!c.font||!c.palette||c.callback!=uint32_t(game_id_sub_4A5830)||c.mode||c.flags||c.format!=16||c.size!=18||c.color>65535)return false;if(uint64_t(c.font)+16>0x100000000ULL||uint64_t(c.palette)+64>0x100000000ULL)return false;nfs_logo_publisher::checked(c.font,16);std::memcpy(c.fontHeader.data(),nfs_host_address(c.font,16),16);nfs_logo_publisher::checked(c.palette,64);for(unsigned k=0;k<16;++k)c.paletteWords[k]=tx.load<uint32_t>(c.palette+4*k);}
 if(pixels){nfs_logo_publisher::checked(w[8],16+Pixels*2);std::memcpy(s.pixels.data(),nfs_host_address(w[8]+16,Pixels*2),Pixels*2);}nfs_logo_capture::Target after{};nfs_logo_publisher::target(tx,after);return nfs_logo_capture::same(after,c.target);
 }catch(...){return false;}}
inline nfs_location_caption_source::Snapshot proofSnapshot(const Current&c)noexcept{
 nfs_location_caption_source::Snapshot p;p.target=c.target;p.identity=c.identity;p.archive=c.archive;p.font=c.font;p.callback=c.callback;p.palette=c.palette;p.color=c.color;p.mode=c.mode;p.flags=c.flags;p.format=c.format;p.originX=c.originX;p.originY=c.originY;p.size=c.size;p.paletteWords=c.paletteWords;p.fontHeader=c.fontHeader;return p;
}
inline bool exactSourceText(uint32_t at)noexcept{
 if(!at||uint64_t(at)+9>UINT64_C(0x100000000))return false;
 try{nfs::GuestCopyTransaction tx;for(unsigned i=0;i<8;++i)if(tx.load<uint8_t>(at+i)!=uint8_t("location"[i]))return false;return !tx.load<uint8_t>(at+8);}catch(...){return false;}
}
inline bool exactProofCommand(const nfs_soft::TextCommand&c)noexcept{
 if(c.length!=8||std::memcmp(c.text.data(),"location",8)||c.role!=1||c.alignment||c.sourceSize!=18||c.x!=116||c.y!=162||c.originX||c.originY||c.advance!=52||c.color!=0xffff||c.clip.left||c.clip.top||c.clip.right!=343||c.clip.bottom!=480)return false;
 for(size_t i=8;i<c.text.size();++i)if(c.text[i])return false;
 return true;
}
struct OwnerScope {State*s=nullptr;bool entered=false;int unwind=0;explicit OwnerScope(Game&g)noexcept{
 if(uint32_t(g.eax)!=uint32_t(guest_address(off_4D2A20))||!nfs_logo_publisher::enabled())return;
 auto font=nfs_soft::acquireFontCatalog();if(!font)return;uintptr_t expected=0,t=token();if(!ownerThread().compare_exchange_strong(expected,t)&&expected!=t)return;
 auto&slot=storage();if(!slot.p){try{slot.p=new State(std::move(font));}catch(...){ownerThread().store(0);return;}}s=slot.p;unwind=nfs_logo_publisher::exceptions();entered=true;if(s->ownerDepth++){s->reject(Failure::nested);return;}s->reject(Failure::owner);if(!s->advance(s->lifetime))return;s->game=reinterpret_cast<uintptr_t>(&g);s->formGuest=uint32_t(g.eax);s->ownerWindow=0;s->tailDepth=0;s->failure=Failure::none;
 }~OwnerScope(){if(!entered)return;if(nfs_logo_publisher::exceptions()>unwind)s->reject(Failure::unwind);if(s->ownerDepth&&!--s->ownerDepth){s->reject(Failure::owner);s->game=0;s->tailDepth=0;}}};
struct TailScope {Game&g;State*s=nullptr;bool entered=false;int unwind=0;explicit TailScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();entered=true;if(s->tailDepth++){s->reject(Failure::nested);return;}s->reject(Failure::none);s->stringsInTail=0;Current c;if(!capture(*s,c,true,false)){s->reject(Failure::target);return;}s->current=c;s->ownerWindow=c.target.owner;if(!s->advance(s->serial)||!s->builder.begin(c.identity,s->serial,s->pixels.data(),Pixels)){s->reject(Failure::command);return;}s->failure=Failure::none;s->recording=true;
 }~TailScope(){if(!entered)return;if(nfs_logo_publisher::exceptions()>unwind)s->reject(Failure::unwind);if(s->tailDepth==1&&s->recording){Current c;if(s->string||s->glyph||s->glyphDepth||!capture(*s,c,true,false)||!sameTarget(c,s->current)){s->reject(Failure::postPlane);}else{auto packet=s->builder.finish(c.identity,s->pixels.data(),Pixels);s->recording=false;if(!packet)s->reject(Failure::postPlane);else{s->packet=std::move(packet);s->failure=Failure::none;
 // Supplemental sealing does not alter legacy packet admission or source replay.
 if(s->proofStringComplete&&s->stringsInTail==1&&s->packet->commands()==1&&s->packet->records().size()==1&&s->formGuest==uint32_t(guest_address(off_4D2A20))&&exactProofCommand(s->proofCommand)&&exactProofCommand(s->packet->records()[0].command)&&exactSourceText(s->proofTextGuest)){
  try{nfs_location_caption_source::Evidence e;e.formGuest=s->formGuest;e.textGuest=s->proofTextGuest;e.game=s->game;e.policyThread=token();e.lifetime=s->lifetime;e.serial=s->serial;e.command=s->proofCommand;e.pre=proofSnapshot(s->current);e.stringBefore=s->proofStringBefore;e.stringAfter=s->proofStringAfter;e.post=proofSnapshot(c);s->proof=std::shared_ptr<const nfs_location_caption_source::Proof>(new nfs_location_caption_source::Proof(s->packet,e));}catch(...){s->proof.reset();}
 }
}}}if(s->tailDepth)--s->tailDepth;}};
struct StringScope {Game&g;State*s=nullptr;nfs_soft::TextCommand command{};uint64_t string=0;uint32_t textGuest=0;Current entered{};int unwind=0;explicit StringScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording||s->tailDepth!=1)return;if(s->string||s->glyph||s->stringsInTail){s->reject(Failure::nested);return;}unwind=nfs_logo_publisher::exceptions();try{{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<96;++k){uint64_t at=uint64_t(uint32_t(g.eax))+k;if(at>UINT32_MAX)throw 1;uint8_t ch=tx.load<uint8_t>(uint32_t(at));if(!ch){command.length=k;break;}if(k==95||ch<32||ch>126)throw 1;command.text[k]=ch;}}if(command.length!=8||std::memcmp(command.text.data(),"location",8)||g.edx!=116||g.ebx!=162){s->reject(Failure::unsupportedText);return;}textGuest=uint32_t(g.eax);if(!capture(*s,entered,false,true)||!sameTarget(entered,s->current)){s->reject(Failure::fontState);return;}command.x=int32_t(g.edx);command.y=int32_t(g.ebx);command.role=1;command.sourceSize=entered.size;command.originX=entered.originX;command.originY=entered.originY;command.color=uint16_t(entered.color);const auto&w=entered.target.w;command.clip={float(w[3]),float(w[4]),float(w[5]),float(w[6])};if(!s->builder.beginString(command)||!s->advance(s->nextString)){s->reject(Failure::command);return;}string=s->nextString;s->string=string;++s->stringsInTail;
 }catch(...){s->reject(Failure::read);}}
 ~StringScope(){if(!string||!s||s!=current(g)||!s->recording||s->string!=string)return;if(nfs_logo_publisher::exceptions()>unwind||s->glyph||s->glyphDepth){s->reject(Failure::unwind);return;}Current after;if(!capture(*s,after,false,true)||!sameFont(entered,after)){s->reject(Failure::sourceChanged);return;}try{nfs::GuestCopyTransaction tx;int64_t advance=int64_t(tx.load<int32_t>(dword_4DD6EC))-command.x;if(advance<=0||advance>4096)throw 1;command.advance=int32_t(advance);}catch(...){s->reject(Failure::read);return;}if(!s->builder.endString(command)){s->reject(Failure::command);return;}s->proofCommand=command;s->proofStringBefore=proofSnapshot(entered);s->proofStringAfter=proofSnapshot(after);s->proofTextGuest=textGuest;s->proofStringComplete=exactSourceText(textGuest);s->string=0;}};
struct GlyphScope {Game&g;State*s=nullptr;uint64_t string=0;int unwind=0;explicit GlyphScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording||!s->string)return;if(s->glyph||s->glyphDepth){s->reject(Failure::nested);return;}string=s->string;s->glyph=string;s->glyphDepth=1;unwind=nfs_logo_publisher::exceptions();}~GlyphScope(){if(!string||s!=current(g)||s->glyph!=string)return;if(nfs_logo_publisher::exceptions()>unwind){s->reject(Failure::unwind);return;}s->glyph=0;s->glyphDepth=0;}};
struct WordCapture {State*s=nullptr;uint64_t string=0;uint32_t address=0;size_t index=0;uint16_t before=0,computed=0;int unwind=0;WordCapture(Game&g,uint32_t at,uint16_t value)noexcept{s=current(g);if(!s||!s->recording||!s->glyph||s->glyph!=s->string||s->glyphDepth!=1)return;const uint64_t first=uint64_t(s->current.identity.geometry[8])+16;if(at<first||uint64_t(at)>=first+Pixels*2||(at-first)%2){s->reject(Failure::word);return;}index=size_t((at-first)/2);const auto&w=s->current.target.w;unsigned x=unsigned(index%Width),y=unsigned(index/Width);if(x<w[3]||x>=w[5]||y<w[4]||y>=w[6]){s->reject(Failure::word);return;}unwind=nfs_logo_publisher::exceptions();address=at;computed=value;try{nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);string=s->string;}catch(...){s->reject(Failure::read);}}
 ~WordCapture(){if(!string||!s||!s->recording||s->string!=string||s->glyph!=string)return;if(nfs_logo_publisher::exceptions()>unwind){s->reject(Failure::unwind);return;}uint16_t after=0;try{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}catch(...){s->reject(Failure::read);return;}if(!s->builder.word(index,before,computed,after))s->reject(Failure::word);}};
inline bool related(State&s,uint32_t at,bool nullIsSelected)noexcept{if(!s.ownerWindow||(!at&&!nullIsSelected))return false;if(at==s.ownerWindow)return true;try{nfs::GuestCopyTransaction tx;uint32_t p=at?at:uint32_t(guest_address(dword_4DB240));if(uint64_t(p)+56>0x100000000ULL)return false;nfs_logo_publisher::checked(p,56);const auto&w=s.current.target.w;return tx.load<uint32_t>(p+32)==w[8]||tx.load<uint32_t>(p+44)==w[11]||tx.load<uint32_t>(p+48)==w[12];}catch(...){return false;}}
inline void beforeOwnerRetire(Game&g,uint32_t at,bool nullIsSelected=false)noexcept{auto*s=current(g);if(!s||!related(*s,at,nullIsSelected))return;s->reject(Failure::retired);(void)s->advance(s->lifetime);s->ownerWindow=0;}
// Descriptor destruction releases refcounted LUT references. A distinct scratch
// descriptor sharing a table does not retire the still-owned caption table.
// Bitmap aliases remain conservative because some descriptors coallocate pixels.
inline bool descriptorRetires(State&s,uint32_t at)noexcept{
 if(!s.ownerWindow||!at)return false;if(at==s.ownerWindow)return true;
 try{nfs::GuestCopyTransaction tx;if(uint64_t(at)+56>0x100000000ULL)return true;nfs_logo_publisher::checked(at,56);const auto&w=s.current.target.w;
  if(tx.load<uint32_t>(at+32)==w[8])return true;
  for(unsigned k:{11u,12u})if(tx.load<uint32_t>(at+4*k)==w[k]){if(w[k]<4)return true;const uint32_t refs=tx.load<uint32_t>(w[k]-4);if(refs<=1||refs>uint32_t(INT32_MAX))return true;}
  return false;
 }catch(...){return true;}
}
inline void beforeDescriptorRetire(Game&g,uint32_t at)noexcept{auto*s=current(g);if(!s||!descriptorRetires(*s,at))return;s->reject(Failure::retired);(void)s->advance(s->lifetime);s->ownerWindow=0;}
struct SelectionScope {Game&g;State*s=nullptr;explicit SelectionScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording)s=nullptr;}~SelectionScope(){if(!s||s!=current(g)||!s->recording)return;Current c;if(!capture(*s,c,false,false)||!sameTarget(c,s->current))s->reject(Failure::selection);}};
// Immutable artifact only: caller must separately join current base/copy/upload/GPU proofs.
inline std::shared_ptr<const nfs_soft::FramePacket> lastPacket(Game&g)noexcept{auto*s=current(g);return s&&!s->recording&&!s->tailDepth?s->packet:std::shared_ptr<const nfs_soft::FramePacket>{};}
inline std::shared_ptr<const nfs_soft::FramePacket> acquireCurrentPacket(Game&g)noexcept{auto*s=current(g);if(!s||!s->packet||s->recording||s->tailDepth)return {};Current c;if(!capture(*s,c,true,false)||!s->packet->matches(c.identity,s->serial,s->pixels.data(),Pixels))return {};return s->packet;}
// Unlike the logical Location handler identity, formGuest is the descriptor
// observed by the actual lexical menu owner. The selected bitmap owner is a
// third identity, re-read and compared here without widening packet-only rules.
inline std::shared_ptr<const nfs_location_caption_source::Proof>lastProof(Game&g)noexcept{
 auto*s=current(g);return s&&!s->recording&&!s->tailDepth?s->proof:std::shared_ptr<const nfs_location_caption_source::Proof>{};
}
inline std::shared_ptr<const nfs_location_caption_source::Proof>acquireCurrentProof(Game&g)noexcept{
 auto*s=current(g);if(!s||!nfs_logo_publisher::enabled()||s->recording||s->tailDepth||!s->proof||!s->packet)return {};
 const auto proof=s->proof;const auto&e=proof->evidence();
 if(!proof->current()||proof->packet()!=s->packet||e.logicalForm!=nfs_location_caption_source::LogicalForm||e.formGuest!=uint32_t(guest_address(off_4D2A20))||s->formGuest!=e.formGuest||e.game!=reinterpret_cast<uintptr_t>(&g)||e.game!=s->game||e.policyThread!=token()||e.lifetime!=s->lifetime||e.serial!=s->serial||!exactSourceText(e.textGuest))return {};
 Current c;if(!capture(*s,c,true,false)||c.archive!=e.post.archive||c.archive!=e.stringAfter.archive||c.archive!=s->current.archive||c.target.owner!=s->ownerWindow||!nfs_logo_capture::same(c.target,e.post.target)||!nfs_logo_capture::same(c.target,s->current.target)||!s->packet->matches(c.identity,s->serial,s->pixels.data(),Pixels))return {};
 return s==current(g)&&s->proof==proof?proof:std::shared_ptr<const nfs_location_caption_source::Proof>{};
}
} // namespace nfs_location_caption
