// SPDX-License-Identifier: MIT
#pragma once
// Actual original-call collection for the finite unavailable-Feedback popup.
// Numeric replay never grants native publication. The sole lexical Invocation
// authenticates the source owner; all original draw/store calls still execute.
#include "FeedbackModalInvocationGameHooks.inc"
namespace nfs_feedback_modal_backdrop_live {
namespace bd=nfs_feedback_backdrop;
using Invocation=nfs_feedback_modal_live::Invocation;
enum class Event {mainSelect,allocate,select,fillColor,fill,body,retire,clip,named,draw,hotspot};
struct State;
struct ModalScope;
class Pending final {
 std::shared_ptr<const Invocation>invocation_;
 std::shared_ptr<const bd::VerifiedBackdrop>verified_;
 Pending(std::shared_ptr<const Invocation>i,std::shared_ptr<const bd::VerifiedBackdrop>v):invocation_(std::move(i)),verified_(std::move(v)){}
 friend struct ModalScope;
public:
 const auto&invocation()const noexcept{return invocation_;}
 const auto&verified()const noexcept{return verified_;}
};
struct State {
 Game*game=nullptr;std::shared_ptr<const Invocation>invocation;bd::Record record{};bd::SelectorContract required{};
 std::shared_ptr<const bd::VerifiedBackdrop>verified;std::shared_ptr<const Pending>pending;
 bool failed=false,backdrop=true,closed=false;const char*failure="none";
 unsigned mainDepth=0,eventDepth=0,widths=0,rowSequence=0;int draw=-1;bd::Row*row=nullptr;size_t stores=0;
 explicit State(Game&g,std::shared_ptr<const Invocation>i):game(&g),invocation(std::move(i)){
  record.binding=invocation->binding();record.initial=invocation->initialTarget();record.before=invocation->before();record.steps.reserve(15);record.body.rows.reserve(60);for(auto&d:record.draws)d.rows.reserve(61);
  required.outer=uint32_t(game_id_sub_4A1090);required.convertSlot=uint32_t(guest_address(dword_571264));required.convert=uint32_t(game_id_sub_499A88);required.blendSlot=uint32_t(guest_address(dword_571260));required.blend=uint32_t(game_id_sub_4829E4);required.opacitySlot=uint32_t(guest_address(dword_5636E8));required.opacity=180;required.lutAddress=uint32_t(guest_address(byte_5635E4));for(unsigned k=0;k<256;++k)required.lut[k]=uint8_t(k*180/256);
 }
 void fail(const char*why)noexcept{if(!failed)failure=why;failed=true;record.error=1;verified.reset();pending.reset();}
};
inline std::shared_ptr<State>&slot()noexcept{static thread_local std::shared_ptr<State>s;return s;}
inline State*current(Game&g)noexcept{auto s=slot();return s&&s->game==&g&&s->invocation&&nfs_feedback_modal_live::activeInvocation(g)==s->invocation?s.get():nullptr;}
inline bool read(uint32_t p,void*out,size_t bytes)noexcept{return nfs_feedback_modal_live::read(p,out,bytes);}
inline bool sourceStable(State&s)noexcept{
 for(unsigned k=0;k<2;++k){const auto&bytes=*s.invocation->catalog()->bytes(k);std::vector<uint8_t>now;try{now.resize(bytes.size());}catch(...){return false;}if(!read(s.record.binding.sources[k],now.data(),now.size())||now!=bytes)return false;}
 return nfs_feedback_modal_live::activeInvocation(*s.game)==s.invocation;
}
inline bool selectedTarget(bd::Target&t)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);bd::Target after{};nfs_logo_publisher::target(tx,after);return nfs_logo_capture::same(t,after);}catch(...){return false;}}
inline bool plane(bd::Target&t,bd::Plane&p)noexcept{return nfs_feedback_modal_live::explicitMain(t,p);}
inline bool scratch(State&s)noexcept{auto&b=s.record.body;if(!b.descriptor||!read(b.descriptor,b.words.data(),56))return false;const auto&w=b.words;if(w[1]!=186||w[2]!=60||w[7]!=0x7810||w[10]!=372||!w[8]||!w[11]||!w[12])return false;b.source=w[8];try{b.x.resize(186);b.y.resize(60);}catch(...){return false;}return read(b.source,b.header.data(),16)&&read(w[12],b.x.data(),186*4)&&read(w[11],b.y.data(),60*4);}
inline bool scratchBytes(State&s,std::vector<uint8_t>&out)noexcept{try{out.resize(16+186*60*2);return read(s.record.body.source,out.data(),out.size());}catch(...){return false;}}
inline bool step(State&s,bd::Step wanted)noexcept{static const bd::Step order[]={bd::Step::initialMain,bd::Step::allocateScratch,bd::Step::selectScratch,bd::Step::fillColor,bd::Step::fillScratch,bd::Step::restoreMainAfterScratch,bd::Step::bodyBlit,bd::Step::retireScratch,bd::Step::bannerClip,bd::Step::bannerResolve,bd::Step::bannerDraw,bd::Step::restoreClip,bd::Step::emblemResolve,bd::Step::emblemHotspot,bd::Step::emblemDraw};if(s.record.steps.size()>=15||order[s.record.steps.size()]!=wanted){s.fail("event-order");return false;}s.record.steps.push_back(wanted);return true;}
struct ModalScope {
 Game&g;std::shared_ptr<State>s;int unwind=0;
 explicit ModalScope(Game&game)noexcept:g(game){auto invocation=nfs_feedback_modal_live::activeInvocation(g);if(!invocation||uint32_t(g.eax)!=2||uint32_t(g.edx)!=invocation->binding().owner)return;if(auto*old=current(g)){old->fail("nested-modal");return;}try{s=std::make_shared<State>(g,invocation);if(!s->required.valid()||!sourceStable(*s)){s.reset();return;}unwind=nfs_logo_publisher::exceptions();slot()=s;}catch(...){s.reset();}}
 ~ModalScope(){if(!s)return;s->closed=true;if(s->failed)return;if(nfs_logo_publisher::exceptions()>unwind||current(g)!=s.get()||s->backdrop||s->eventDepth||s->row||!s->verified||!sourceStable(*s)){s->fail("modal-return");return;}bd::Target t;bd::Plane ignored;if(!plane(t,ignored)||!nfs_logo_capture::same(t,s->record.pretextTarget)){s->fail("modal-return-target");return;}try{s->pending=std::shared_ptr<const Pending>(new Pending(s->invocation,s->verified));}catch(...){s->fail("pending-allocation");}}
 ModalScope(const ModalScope&)=delete;ModalScope&operator=(const ModalScope&)=delete;
};
struct WidthScope {
 Game&g;State*s=nullptr;uint32_t text=0;int unwind=0;
 explicit WidthScope(Game&game)noexcept:g(game){s=current(g);if(!s||s->failed||!s->backdrop){s=nullptr;return;}text=uint32_t(g.eax);if(s->widths>=2||text!=(s->widths?s->record.binding.messageResource:s->record.binding.titleResource)){s->fail("width-source");s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();}
 ~WidthScope(){if(!s||s->failed)return;if(current(g)!=s||nfs_logo_publisher::exceptions()>unwind||!sourceStable(*s)){s->fail("width-drift");return;}int width=int32_t(g.eax);if(width!=(s->widths?108:96)){s->fail("width-value");return;}(s->widths?s->record.messageWidth:s->record.titleWidth)=width;++s->widths;}
};
struct EventScope {
 Game&g;State*s=nullptr;Event kind;uint32_t a=0,d=0,b=0,c=0;unsigned role=0;bool entered=false,ignore=false;int unwind=0;
 explicit EventScope(Game&game,Event event)noexcept:g(game),kind(event){s=current(g);if(!s||s->failed||!s->backdrop){s=nullptr;return;}a=uint32_t(g.eax);d=uint32_t(g.edx);b=uint32_t(g.ebx);c=uint32_t(g.ecx);unwind=nfs_logo_publisher::exceptions();try{
  if(kind==Event::select&&s->mainDepth){ignore=true;return;}if(s->eventDepth>=4){s->fail("event-nesting");return;}
  auto&r=s->record;auto&body=r.body;const size_t n=r.steps.size();bd::Step wanted=bd::Step::initialMain;
  switch(kind){
   case Event::mainSelect:wanted=n?bd::Step::restoreMainAfterScratch:bd::Step::initialMain;++s->mainDepth;break;
   case Event::allocate:wanted=bd::Step::allocateScratch;break;
   case Event::select:wanted=bd::Step::selectScratch;break;
   case Event::fillColor:wanted=bd::Step::fillColor;body.fillColor=a;break;
   case Event::fill:wanted=bd::Step::fillScratch;break;
   case Event::body:wanted=bd::Step::bodyBlit;body.opacity=c;if(a!=body.source){s->fail("body-source");return;}break;
   case Event::retire:wanted=bd::Step::retireScratch;if(a!=body.descriptor||!read(a,body.retireWords.data(),56)){s->fail("retire-source");return;}break;
   case Event::clip:wanted=n==8?bd::Step::bannerClip:bd::Step::restoreClip;break;
   case Event::named:role=n==9?0:1;wanted=role?bd::Step::emblemResolve:bd::Step::bannerResolve;break;
   case Event::hotspot:wanted=bd::Step::emblemHotspot;role=1;r.draws[1].inputX=int32_t(d);r.draws[1].inputY=int32_t(b);r.draws[1].hotspot=true;if(a!=r.binding.sources[1]){s->fail("hotspot-source");return;}break;
   case Event::draw:role=n==10?0:1;wanted=role?bd::Step::emblemDraw:bd::Step::bannerDraw;break;
  }
  if(!step(*s,wanted))return;entered=true;++s->eventDepth;
  if(kind==Event::named){auto&draw=r.draws[role];draw.role=bd::Role(role);draw.sequence=role;draw.archive=a;draw.sourceAddress=r.binding.sources[role];if(a!=r.binding.archive||!read(d,draw.name.data(),4)){s->fail("named-source");return;}draw.sourceBefore.resize(s->invocation->catalog()->bytes(role)->size());if(!read(draw.sourceAddress,draw.sourceBefore.data(),draw.sourceBefore.size())||draw.sourceBefore!=*s->invocation->catalog()->bytes(role)){s->fail("named-bytes");return;}}
  if(kind==Event::draw){auto&draw=r.draws[role];if(s->draw>=0||a!=r.binding.sources[role]||!selectedTarget(draw.before)){s->fail("draw-entry");return;}s->draw=int(role);draw.x=int32_t(d);draw.y=int32_t(b);draw.mode=0;if(!role){draw.inputX=draw.x;draw.inputY=draw.y;}const auto&id=*s->invocation->catalog()->identity(role);const int left=std::max(int(draw.before.w[3]),draw.x),top=std::max(int(draw.before.w[4]),draw.y),right=std::min(int(draw.before.w[5]),draw.x+int(id.width)),bottom=std::min(int(draw.before.w[6]),draw.y+int(id.height));if(right<=left||bottom<=top){s->fail("draw-clip");return;}draw.uv={{uint32_t(left-draw.x),uint32_t(top-draw.y),uint32_t(right-draw.x),uint32_t(bottom-draw.y)}};}
 }catch(...){s->fail("event-allocation");}}
 ~EventScope(){if(!s||ignore)return;if(entered&&s->eventDepth)--s->eventDepth;if(kind==Event::mainSelect&&s->mainDepth)--s->mainDepth;if(!entered||s->failed)return;if(current(g)!=s||nfs_logo_publisher::exceptions()>unwind){s->fail("event-unwind");return;}try{auto&r=s->record;auto&body=r.body;switch(kind){
  case Event::mainSelect:{bd::Target t;bd::Plane pixels;if(!plane(t,pixels)||!nfs_logo_capture::same(t,r.initial)){s->fail("main-restore");return;}if(r.steps.size()==1){r.initialMainRestoreComplete=1;r.initialRestored=std::move(pixels);}else{r.restoredMainAfterScratch=t;r.restored=std::move(pixels);}break;}
  case Event::allocate:body.descriptor=uint32_t(g.eax);if(!scratch(*s)){s->fail("allocate-descriptor");return;}body.allocated=1;break;
  case Event::select:{std::array<uint32_t,14>actual{};if(a!=body.descriptor||uint32_t(g.eax)!=1||!read(uint32_t(guest_address(dword_4DB240)),actual.data(),56)||actual!=body.words){s->fail("scratch-select");return;}body.selected=uint32_t(g.eax);break;}
  case Event::fillColor:body.fillWord=uint16_t(g.eax);break;
  case Event::fill:if(!scratchBytes(*s,body.postFill)){s->fail("fill-bytes");return;}body.fillComplete=1;break;
  case Event::body:if(!scratchBytes(*s,body.afterBlit)){s->fail("body-source-after");return;}break;
  case Event::retire:body.retired=1;break;
  case Event::named:{auto&draw=r.draws[role];draw.resolvedAddress=uint32_t(g.eax);if(draw.resolvedAddress!=draw.sourceAddress){s->fail("named-result");return;}draw.namedResolves=1;break;}
  case Event::draw:{auto&draw=r.draws[role];if(!selectedTarget(draw.after)){s->fail("draw-after");return;}draw.sourceAfter.resize(draw.sourceBefore.size());if(!read(draw.sourceAddress,draw.sourceAfter.data(),draw.sourceAfter.size())){s->fail("draw-source-after");return;}draw.complete=1;s->draw=-1;break;}
  default:break;
 }}catch(...){s->fail("event-exit-allocation");}}
 EventScope(const EventScope&)=delete;EventScope&operator=(const EventScope&)=delete;
};
inline bool selectors(bd::SelectorContract&out)noexcept{
 out.outer=uint32_t(game_id_sub_4A1090);out.convertSlot=uint32_t(guest_address(dword_571264));out.blendSlot=uint32_t(guest_address(dword_571260));out.opacitySlot=uint32_t(guest_address(dword_5636E8));out.lutAddress=uint32_t(guest_address(byte_5635E4));return read(out.convertSlot,&out.convert,4)&&read(out.blendSlot,&out.blend,4)&&read(out.opacitySlot,&out.opacity,4)&&read(out.lutAddress,out.lut.data(),256);
}
struct RowScope {
 Game&g;State*s=nullptr;bd::Row row{};bool bulk=false;int unwind=0;
 explicit RowScope(Game&game)noexcept:g(game){s=current(g);if(!s||s->failed||!s->backdrop){s=nullptr;return;}if(s->row||s->rowSequence>=168){s->fail("row-order");s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();try{
  row.callerStackBefore=uint32_t(g.esp);if(uint64_t(row.callerStackBefore)+0x3c>UINT64_C(0x100000000)){s->fail("dispatch-span");return;}row.dispatchSlot=row.callerStackBefore+0x38;if(!read(row.dispatchSlot,&row.dispatchBefore,4)){s->fail("dispatch-before");return;}row.callback=row.dispatchBefore;row.sourceAddress=uint32_t(g.eax);row.destinationAddress=uint32_t(g.edx);row.count=uint32_t(g.ebx);
  if(row.callback==uint32_t(game_id_sub_4885D4)&&s->record.steps.size()==5&&row.count==186*60&&row.destinationAddress==s->record.body.source+16){bulk=true;auto&body=s->record.body;body.fillCallback=row.callback;body.fillDestination=row.destinationAddress;body.fillCount=row.count;return;}
  if(!row.count||row.count>640||(row.callback!=uint32_t(game_id_sub_4A1090)&&row.callback!=uint32_t(game_id_sub_483288))){s->fail("row-source");return;}row.sequence=s->rowSequence;row.format=row.callback==uint32_t(game_id_sub_4A1090)?0x78:0x7b;row.source.resize(row.count*(row.format==0x78?2:1));row.before.resize(row.count);row.after.resize(row.count);row.stores.reserve(row.count);if(!read(row.sourceAddress,row.source.data(),row.source.size())||!read(row.destinationAddress,row.before.data(),row.count*2)){s->fail("row-before");return;}
  if(row.format==0x78){if(!selectors(row.selectors.before)){s->fail("row-selectors-before");return;}}else{if(!read(uint32_t(guest_address(dword_5636EC)),&row.paletteAddress,4)||!read(row.paletteAddress,row.paletteBefore.data(),1024)){s->fail("row-palette-before");return;}}s->row=&row;
 }catch(...){s->fail("row-allocation");}}
 ~RowScope(){if(!s)return;if(s->row==&row)s->row=nullptr;if(s->failed)return;if(current(g)!=s||nfs_logo_publisher::exceptions()>unwind){s->fail("row-unwind");return;}try{row.callerStackAfter=uint32_t(g.esp);if(!read(row.dispatchSlot,&row.dispatchAfter,4)||row.callerStackAfter!=row.callerStackBefore||row.dispatchAfter!=row.dispatchBefore){s->fail("row-dispatch-after");return;}if(bulk)return;row.sourceAfter.resize(row.source.size());if(!read(row.sourceAddress,row.sourceAfter.data(),row.sourceAfter.size())||!read(row.destinationAddress,row.after.data(),row.count*2)){s->fail("row-after");return;}if(row.format==0x78){if(!selectors(row.selectors.after)){s->fail("row-selectors-after");return;}row.selectors.complete=1;s->record.body.rows.push_back(std::move(row));}else{uint32_t palette=0;if(s->draw<0||!read(uint32_t(guest_address(dword_5636EC)),&palette,4)||palette!=row.paletteAddress||!read(palette,row.paletteAfter.data(),1024)){s->fail("row-palette-after");return;}s->record.draws[unsigned(s->draw)].rows.push_back(std::move(row));}++s->rowSequence;
 }catch(...){s->fail("row-exit-allocation");}}
 RowScope(const RowScope&)=delete;RowScope&operator=(const RowScope&)=delete;
};
struct StoreScope {
 State*s=nullptr;bd::Row*row=nullptr;bd::Store store{};int unwind=0;
 explicit StoreScope(Game&g,uint32_t address,uint16_t computed)noexcept{s=current(g);if(!s||s->failed||!s->backdrop||!s->row){s=nullptr;return;}row=s->row;if(s->stores>=bd::MaxStores||row->stores.size()>=row->count||address<row->destinationAddress||uint64_t(address)>=uint64_t(row->destinationAddress)+row->count*2||(address-row->destinationAddress)%2){s->fail("store-range");s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();store.address=address;store.computed=computed;if(!read(address,&store.before,2)){s->fail("store-before");s=nullptr;}}
 ~StoreScope(){if(!s||s->failed)return;if(s->row!=row||nfs_logo_publisher::exceptions()>unwind||!read(store.address,&store.after,2)){s->fail("store-after");return;}try{row->stores.push_back(store);++s->stores;}catch(...){s->fail("store-allocation");}}
};
inline void pretext(Game&g)noexcept{auto*s=current(g);if(!s||s->failed||!s->backdrop)return;if(s->eventDepth||s->row||s->widths!=2||s->record.steps.size()!=15||!plane(s->record.pretextTarget,s->record.pretext)||!sourceStable(*s)){s->fail("pretext-source");return;}s->backdrop=false;s->record.complete=1;s->verified=bd::ReplayVerifier::verify(s->record,s->invocation->binding(),s->invocation->catalog(),s->required);if(!s->verified)s->fail("numeric-replay");}
inline std::shared_ptr<const Pending>acquireCurrentPending(Game&g)noexcept{auto*s=current(g);if(!s||s->failed||!s->closed||s->backdrop||!s->pending||!sourceStable(*s))return {};bd::Target target;bd::Plane ignored;if(!plane(target,ignored)||!nfs_logo_capture::same(target,s->record.pretextTarget)){s->fail("current-target");return {};}return s->pending;}
} // namespace nfs_feedback_modal_backdrop_live
