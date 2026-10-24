// SPDX-License-Identifier: MIT
#pragma once
// Independent original Steering-sensitivity pre-chart text replay on the full640
// surface. Finish immediately after40FA90, before410330 draws its selected bitmap.
// This is not the final chart-inclusive scene, a sidebar proof or native authority.
#include "ControlsTextGameHooks.inc"
#include <climits>
namespace nfs_controls_steering_text {
using Current=nfs_controls_text::Current;
using Failure=nfs_controls_text::Failure;
constexpr unsigned Width=640,Height=480;constexpr size_t Pixels=Width*Height;
struct Authority {
 nfs_controls_dialog_word::Authority parent{};
 uint32_t form=0,gameType=0,column=0,screenType=0,assignment=0,sensitivity=0,archive=0,table=0;
 std::array<uint32_t,3>header{};
 std::array<uint32_t,4>descriptors{};
 std::array<std::array<uint32_t,5>,4>words{};
 std::array<uint32_t,3>values{};
 std::array<std::array<uint32_t,3>,8>resources{};
 bool operator==(const Authority&o)const noexcept{return parent==o.parent&&form==o.form&&gameType==o.gameType&&column==o.column&&screenType==o.screenType&&assignment==o.assignment&&sensitivity==o.sensitivity&&archive==o.archive&&table==o.table&&header==o.header&&descriptors==o.descriptors&&words==o.words&&values==o.values&&resources==o.resources;}
};
struct State {
 std::shared_ptr<const nfs_soft::FontCatalog>font;
 nfs_soft::FramePacketBuilder builder;
 std::shared_ptr<const nfs_soft::FramePacket>packet;std::vector<uint16_t>pixels;
 uintptr_t game=0;uint32_t form=0,handler=0,ownerWindow=0,sourceSize=0;
 unsigned ownerDepth=0,tailDepth=0,policyDepth=0,glyphDepth=0,stringsInTail=0;
 uint32_t policyResource=0;bool policyResourceCall=false;
 int policyAlignment=-1;uint64_t lifetime=0,ownerEpoch=0,serial=0,nextString=0,string=0,glyph=0,failures=0;
 bool recording=false;Failure failure=Failure::none;Current current{};Authority authority{};
 State(std::shared_ptr<const nfs_soft::FontCatalog>f,unsigned size):font(std::move(f)),builder(font,size),pixels(Pixels),sourceSize(size){}
 void reject(Failure f)noexcept{failure=f;if(f!=Failure::none&&failures!=UINT64_MAX)++failures;builder.invalidate();packet.reset();recording=false;string=glyph=0;glyphDepth=0;tailDepth=0;}
 bool advance(uint64_t&v)noexcept{if(v==UINT64_MAX){reject(Failure::capacity);return false;}++v;return true;}
};
inline uintptr_t token()noexcept{return nfs_logo_publisher::threadToken();}
inline std::atomic<uintptr_t>&ownerThread()noexcept{static std::atomic<uintptr_t> t{0};return t;}
struct Storage {State*steering=nullptr,*active=nullptr;~Storage(){delete steering;if(ownerThread().load()==token())ownerThread().store(0);}};
inline Storage&storage(){static thread_local Storage s;return s;}
inline State*state()noexcept{return ownerThread().load(std::memory_order_acquire)==token()?storage().active:nullptr;}
inline State*current(Game&g)noexcept{auto*s=state();return s&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->ownerDepth==1?s:nullptr;}
inline bool sameSteeringTarget(const Current&a,const Current&b)noexcept{return nfs_controls_text::sameTarget(a,b);}
inline bool sameSteeringFont(const Current&a,const Current&b)noexcept{return nfs_controls_text::sameFont(a,b);}
// All four steering labels and the original raw colon/enum text use18.
inline unsigned expectedStringSize(const State&s)noexcept{
 if(s.sourceSize!=18||s.policyDepth!=1)return 0;
 return !s.policyResourceCall||(s.policyResource>=0x152&&s.policyResource<=0x155)?18:0;
}
inline bool readAuthority(State&s,Authority&a)noexcept{try{
 a={};a.form=s.form;if(!nfs_controls_text::readDialogAuthority(a.parent)||a.parent.selectedIndex!=1||s.form!=uint32_t(guest_address(off_4D36A8))||s.handler!=0x4101c0u)return false;
 nfs::GuestCopyTransaction tx;a.gameType=tx.load<uint8_t>(byte_512DA9);a.column=tx.load<uint8_t>(byte_512DA8);a.screenType=tx.load<uint8_t>(byte_512DFB);a.assignment=tx.load<uint8_t>(inControlAssignMode);a.sensitivity=tx.load<uint8_t>(byte_512E00);a.archive=tx.load<uint32_t>(dword_4D40B8);a.table=tx.load<uint32_t>(dword_4D4AE0);
 if(!a.archive||!a.table||a.assignment||a.sensitivity>3||uint64_t(a.table)+12u*0x15au>0x100000000ULL)return false;
 for(unsigned k=0;k<3;++k)a.header[k]=tx.load<uint32_t>(s.form+4*k);
 if(a.header[0]!=uint32_t(guest_address(aMain))||a.header[1]!=uint32_t(guest_address(aBlk4_0))||(a.header[2]&65535)||((a.header[2]>>16)>=3&&(a.header[2]>>16)!=100)||tx.load<uint32_t>(s.form+28))return false;
 const uint32_t at[]={uint32_t(guest_address(dword_4D3658)),uint32_t(guest_address(dword_4D366C)),uint32_t(guest_address(dword_4D3680)),uint32_t(guest_address(dword_4D3694))};
 const std::array<std::array<uint32_t,5>,4>expected{{{{0x1530002u,0,4,uint32_t(guest_address(byte_512E00)),uint32_t(guest_address(dword_4D364C))}},{{0x1540002u,UINT32_MAX,8,0,uint32_t(guest_address(dword_4D24E8))}},{{0x1550002u,UINT32_MAX,16,0,uint32_t(guest_address(dword_4D24E8))}},{{0x1520000u,0,0,0,0}}}};
 for(unsigned i=0;i<4;++i){a.descriptors[i]=tx.load<uint32_t>(s.form+12+4*i);if(a.descriptors[i]!=at[i])return false;for(unsigned k=0;k<5;++k)a.words[i][k]=tx.load<uint32_t>(at[i]+4*k);if(a.words[i]!=expected[i])return false;}
 for(unsigned k=0;k<3;++k)a.values[k]=tx.load<uint32_t>(uint32_t(guest_address(dword_4D364C))+4*k);if(a.values!=std::array<uint32_t,3>{{0x1570156,0x1590158,0}})return false;
 const uint32_t styles[]={0x05050512,0x04020112,0x04020112,0x03020112,0x03040312,0x03040312,0x03040312,0x03040312};
 const uint32_t geometry[]={435u|(105u<<16),435u|(144u<<16),435u|(187u<<16),455u|(187u<<16),0,0,0,0};
 const char*names[]={"STEERING SENSITIVITY","sensitivity","DONE","CANCEL","full","three quarter","half","one quarter"};
 for(unsigned i=0;i<8;++i){const uint32_t entry=a.table+12*(0x152+i);for(unsigned k=0;k<3;++k)a.resources[i][k]=tx.load<uint32_t>(entry+4*k);if(a.resources[i][0]!=styles[i]||a.resources[i][1]!=geometry[i])return false;const uint64_t text=uint64_t(a.table)+a.resources[i][2];const size_t length=std::strlen(names[i]);if(!text||text+length+1>0x100000000ULL)return false;for(size_t k=0;k<=length;++k)if(tx.load<uint8_t>(uint32_t(text+k))!=uint8_t(names[i][k]))return false;}
 return true;
 }catch(...){return false;}}
inline bool capture(State&s,Current&c,bool pixels,bool fontState)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,c.target);const auto&w=c.target.w;if(w[1]!=Width||w[2]!=Height||w[7]!=0x7810||w[10]!=1280||w[13]||!c.target.owner||(c.target.header[0]&127)!=0x78||c.target.header[1]!=(Width|(Height<<16))||w[3]>w[5]||w[5]>Width||w[4]>w[6]||w[6]>Height)return false;for(unsigned k=0;k<Width;++k)if(c.target.x[k]!=2*k)return false;for(unsigned k=0;k<Height;++k)if(c.target.y[k]!=16+1280*k)return false;for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(c.target.ownerWords[k]!=w[k])return false;
 if(uint64_t(w[8])+16+Pixels*2>0x100000000ULL)return false;c.archive=tx.load<uint32_t>(dword_4D40B8);if(!c.archive)return false;c.identity.lifetime=s.lifetime;std::copy(w.begin(),w.end(),c.identity.geometry.begin());auto&a=c.identity.geometry;a[14]=c.target.x[0];a[15]=c.target.x[1];a[16]=c.target.x[639];a[17]=c.target.y[0];a[18]=c.target.y[1];a[19]=c.target.y[479];if(!c.identity.valid())return false;
 if(fontState){c.font=tx.load<uint32_t>(dword_4DD770);c.callback=tx.load<uint32_t>(dword_4DD77C);c.palette=tx.load<uint32_t>(dword_4DD774);c.color=tx.load<uint32_t>(dword_4DD714);c.mode=tx.load<uint32_t>(dword_4D20A0);c.flags=tx.load<uint8_t>(byte_4DB25E);c.format=tx.load<uint8_t>(byte_4DB25C);c.originX=tx.load<int32_t>(dword_4DD6F4);c.originY=tx.load<int32_t>(dword_4DD6F8);c.size=tx.load<int32_t>(dword_4E54AC);if(!c.font||!c.palette||c.callback!=uint32_t(game_id_sub_4A5830)||c.mode||c.flags||c.format!=16||c.size!=int(expectedStringSize(s))||c.color>65535)return false;if(uint64_t(c.font)+16>0x100000000ULL||uint64_t(c.palette)+64>0x100000000ULL)return false;nfs_logo_publisher::checked(c.font,16);std::memcpy(c.fontHeader.data(),nfs_host_address(c.font,16),16);nfs_logo_publisher::checked(c.palette,64);for(unsigned k=0;k<16;++k)c.paletteWords[k]=tx.load<uint32_t>(c.palette+4*k);}
 if(pixels){nfs_logo_publisher::checked(w[8],16+Pixels*2);std::memcpy(s.pixels.data(),nfs_host_address(w[8]+16,Pixels*2),Pixels*2);}nfs_logo_capture::Target after{};nfs_logo_publisher::target(tx,after);return nfs_logo_capture::same(after,c.target);
 }catch(...){return false;}}
struct OwnerScope {State*s=nullptr;bool entered=false,nested=false;int unwind=0;explicit OwnerScope(Game&g,unsigned handler)noexcept{
 unwind=nfs_logo_publisher::exceptions();if(auto*active=state()){s=active;if(!s->ownerDepth){s->reject(Failure::owner);return;}s->reject(Failure::nested);if(s->ownerDepth==UINT_MAX){s->reject(Failure::capacity);return;}++s->ownerDepth;entered=nested=true;return;}
 const uint32_t form=uint32_t(g.eax);
 if(form!=uint32_t(guest_address(off_4D36A8))||handler!=0x4101c0u||!nfs_logo_publisher::enabled())return;auto font=nfs_soft::acquireFontCatalog();if(!font)return;uintptr_t expected=0,t=token();if(!ownerThread().compare_exchange_strong(expected,t)&&expected!=t)return;
 auto&slot=storage();State*&cell=slot.steering;if(!cell){try{cell=new State(std::move(font),18);}catch(...){return;}}s=cell;s->form=form;s->handler=handler;s->game=reinterpret_cast<uintptr_t>(&g);s->ownerDepth=1;s->policyDepth=0;s->reject(Failure::none);s->policyAlignment=-1;slot.active=s;entered=true;
 if(!s->advance(s->lifetime)||!s->advance(s->ownerEpoch)){s->ownerDepth=0;return;}Authority a;if(!readAuthority(*s,a)){s->reject(Failure::owner);s->ownerDepth=0;}
 }~OwnerScope(){if(!entered)return;s->reject(nfs_logo_publisher::exceptions()>unwind?Failure::unwind:Failure::owner);if(nested){if(s->ownerDepth>1)--s->ownerDepth;return;}s->game=0;s->ownerDepth=0;if(storage().active==s)storage().active=nullptr;}};
inline void beginTail(Game&g)noexcept{auto*s=current(g);if(!s)return;if(s->recording||s->tailDepth||s->string||s->glyph||s->policyDepth){s->reject(Failure::nested);return;}s->reject(Failure::none);s->stringsInTail=0;s->policyAlignment=-1;s->policyResource=0;s->policyResourceCall=false;if(uint32_t(g.eax)!=s->form||!readAuthority(*s,s->authority)){s->reject(Failure::owner);return;}Current c;if(!capture(*s,c,true,false)){s->reject(Failure::target);return;}s->current=c;s->ownerWindow=c.target.owner;s->tailDepth=1;s->recording=true;}
inline void finishTail(Game&g)noexcept{auto*s=current(g);if(!s||!s->recording)return;Current c;Authority a;if(s->tailDepth!=1||!s->stringsInTail||s->string||s->glyph||s->glyphDepth||s->policyDepth||!readAuthority(*s,a)||!(a==s->authority)||!capture(*s,c,true,false)||!sameSteeringTarget(c,s->current)){s->reject(Failure::postPlane);return;}auto p=s->builder.finish(c.identity,s->pixels.data(),Pixels);s->recording=false;s->tailDepth=0;if(!p){s->reject(Failure::postPlane);return;}s->packet=std::move(p);s->failure=Failure::none;}
struct PolicyScope {Game&g;State*s=nullptr;bool active=false;int unwind=0;PolicyScope(Game&game,bool stacked)noexcept:g(game){s=current(g);if(!s||!s->recording||s->tailDepth!=1)return;active=true;unwind=nfs_logo_publisher::exceptions();if(s->policyDepth==UINT_MAX){s->reject(Failure::capacity);return;}if(s->policyDepth++){s->reject(Failure::nested);return;}s->policyAlignment=-1;s->policyResourceCall=!stacked;s->policyResource=stacked?0:uint32_t(g.eax);if(stacked){try{nfs::GuestCopyTransaction tx;if(uint64_t(uint32_t(g.esp))+8>0x100000000ULL)throw 1;int32_t v=tx.load<int32_t>(uint32_t(g.esp)+4);if(v<0||v>5)throw 1;s->policyAlignment=v;}catch(...){s->reject(Failure::read);}}}~PolicyScope(){if(!active)return;if(nfs_logo_publisher::exceptions()>unwind)s->reject(Failure::unwind);if(s->policyDepth)--s->policyDepth;s->policyAlignment=-1;s->policyResource=0;s->policyResourceCall=false;}};
inline void resourceAlignment(Game&g)noexcept{auto*s=current(g);if(!s||!s->recording||s->policyDepth!=1)return;const int32_t v=int32_t(g.eax);if(v<0||v>5)s->reject(Failure::unsupportedText);else s->policyAlignment=v;}
struct StringScope {Game&g;State*s=nullptr;nfs_soft::TextCommand command{};uint64_t string=0;uint32_t textGuest=0;Current entered{};int unwind=0;explicit StringScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording||s->tailDepth!=1)return;if(s->string||s->glyph||s->stringsInTail>=nfs_soft::FramePacketBuilder::maxCommands){s->reject(Failure::nested);return;}unwind=nfs_logo_publisher::exceptions();textGuest=uint32_t(g.eax);try{{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<96;++k){uint64_t at=uint64_t(textGuest)+k;if(at>UINT32_MAX)throw 1;uint8_t ch=tx.load<uint8_t>(uint32_t(at));if(!ch){command.length=k;break;}if(k==95||ch<32||ch>126)throw 1;command.text[k]=ch;}}if(!command.length){s->reject(Failure::unsupportedText);return;}if(!capture(*s,entered,false,true)||!sameSteeringTarget(entered,s->current)){s->reject(Failure::fontState);return;}command.x=int32_t(g.edx);command.y=int32_t(g.ebx);if(s->policyDepth!=1||s->policyAlignment<0||s->policyAlignment>5){s->reject(Failure::unsupportedText);return;}command.role=1;command.alignment=(s->policyAlignment%3==1)?2:(s->policyAlignment%3==2)?1:0;if(!s->stringsInTail){if(!s->advance(s->serial)||!s->builder.begin(s->current.identity,s->serial,s->pixels.data(),Pixels)){s->reject(Failure::command);return;}}command.sourceSize=entered.size;command.originX=entered.originX;command.originY=entered.originY;command.color=uint16_t(entered.color);const auto&w=entered.target.w;command.clip={float(w[3]),float(w[4]),float(w[5]),float(w[6])};if(!s->builder.beginString(command,uint32_t(entered.size))||!s->advance(s->nextString)){s->reject(Failure::command);return;}string=s->nextString;s->string=string;++s->stringsInTail;
 }catch(...){s->reject(Failure::read);}}
 ~StringScope(){if(!string||!s||s!=current(g)||!s->recording||s->string!=string)return;if(nfs_logo_publisher::exceptions()>unwind||s->glyph||s->glyphDepth){s->reject(Failure::unwind);return;}Current after;if(!capture(*s,after,false,true)||!sameSteeringFont(entered,after)){s->reject(Failure::sourceChanged);return;}try{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<=command.length;++k)if(tx.load<uint8_t>(textGuest+k)!=(k<command.length?command.text[k]:0))throw 1;int64_t advance=int64_t(tx.load<int32_t>(dword_4DD6EC))-command.x;if(advance<=0||advance>4096)throw 1;command.advance=int32_t(advance);}catch(...){s->reject(Failure::read);return;}if(!s->builder.endString(command)){s->reject(Failure::command);return;}s->string=0;}};
struct GlyphScope {Game&g;State*s=nullptr;uint64_t string=0;int unwind=0;explicit GlyphScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording||!s->string)return;if(s->glyph||s->glyphDepth){s->reject(Failure::nested);return;}string=s->string;s->glyph=string;s->glyphDepth=1;unwind=nfs_logo_publisher::exceptions();}~GlyphScope(){if(!string||s!=current(g)||s->glyph!=string)return;if(nfs_logo_publisher::exceptions()>unwind){s->reject(Failure::unwind);return;}s->glyph=0;s->glyphDepth=0;}};
struct WordCapture {State*s=nullptr;uint64_t string=0;uint32_t address=0;size_t index=0;uint16_t before=0,computed=0;int unwind=0;WordCapture(Game&g,uint32_t at,uint16_t value)noexcept{s=current(g);if(!s||!s->recording||!s->glyph||s->glyph!=s->string||s->glyphDepth!=1)return;const uint64_t first=uint64_t(s->current.identity.geometry[8])+16;if(at<first||uint64_t(at)>=first+Pixels*2||(at-first)%2){s->reject(Failure::word);return;}index=size_t((at-first)/2);const auto&w=s->current.target.w;unsigned x=unsigned(index%Width),y=unsigned(index/Width);if(x<w[3]||x>=w[5]||y<w[4]||y>=w[6]){s->reject(Failure::word);return;}unwind=nfs_logo_publisher::exceptions();address=at;computed=value;try{nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);string=s->string;}catch(...){s->reject(Failure::read);}}
 ~WordCapture(){if(!string||!s||!s->recording||s->string!=string||s->glyph!=string)return;if(nfs_logo_publisher::exceptions()>unwind){s->reject(Failure::unwind);return;}uint16_t after=0;try{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}catch(...){s->reject(Failure::read);return;}if(!s->builder.word(index,before,computed,after))s->reject(Failure::word);}};
inline bool related(State&s,uint32_t at,bool nullIsSelected)noexcept{if(!s.ownerWindow||(!at&&!nullIsSelected))return false;if(at==s.ownerWindow)return true;try{nfs::GuestCopyTransaction tx;uint32_t p=at?at:uint32_t(guest_address(dword_4DB240));if(uint64_t(p)+56>0x100000000ULL)return false;nfs_logo_publisher::checked(p,56);const auto&w=s.current.target.w;return tx.load<uint32_t>(p+32)==w[8]||tx.load<uint32_t>(p+44)==w[11]||tx.load<uint32_t>(p+48)==w[12];}catch(...){return false;}}
inline void beforeOwnerRetire(Game&g,uint32_t at,bool nullIsSelected=false)noexcept{auto*s=current(g);if(!s||!related(*s,at,nullIsSelected))return;s->reject(Failure::retired);(void)s->advance(s->lifetime);s->ownerWindow=0;}
// Descriptor destruction releases refcounted LUT references. A distinct scratch
// descriptor sharing a table does not retire the still-owned Controls text table.
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
struct SelectionScope {Game&g;State*s=nullptr;explicit SelectionScope(Game&game)noexcept:g(game){s=current(g);if(!s||!s->recording)s=nullptr;}~SelectionScope(){if(!s||s!=current(g)||!s->recording)return;Current c;if(!capture(*s,c,false,false)||!sameSteeringTarget(c,s->current))s->reject(Failure::selection);}};
// Once410330 writes the chart, this pre-chart packet no longer matches the
// current plane. Historical retention never grants final-scene admission.
inline std::shared_ptr<const nfs_soft::FramePacket> acquireCurrentPacket(Game&g)noexcept{auto*s=current(g);if(!s||!s->packet||s->recording||s->tailDepth||s->string||s->glyph||s->glyphDepth||s->policyDepth)return {};Authority a;Current c;if(!readAuthority(*s,a)||!(a==s->authority)||!capture(*s,c,true,false)||!s->packet->matches(c.identity,s->serial,s->pixels.data(),Pixels))return {};return s->packet;}
} // namespace nfs_controls_steering_text
