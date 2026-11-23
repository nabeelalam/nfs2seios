// SPDX-License-Identifier: MIT
#pragma once
#include "CompactCaptionProof.h"
namespace nfs_player_initial_sidebar {
namespace bg=nfs_background_scene_cpu;
struct Binding {
 uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0,thread=0;
 uint64_t epoch=0;bg::Event captionEvent{};
};
inline bool sameBinding(const Binding&a,const Binding&b)noexcept{return a.form==b.form&&a.formGuest==b.formGuest&&a.archive==b.archive&&a.game==b.game&&a.thread==b.thread&&a.epoch==b.epoch&&a.captionEvent.ownerEpoch==b.captionEvent.ownerEpoch&&a.captionEvent.sequence==b.captionEvent.sequence&&a.captionEvent.iteration==b.captionEvent.iteration;}
class Store;
// Historical Main ownership/events and current Player caption ownership remain
// distinct. This pending source proof grants no copy/upload/GPU authorization.
class Proof final {
 struct Life{std::atomic<bool>alive{true};};
 std::shared_ptr<Life>life_;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin_;
 std::shared_ptr<const nfs_compact_caption::Proof>caption_;Binding binding_;bg::Plane post_;
 std::shared_ptr<const bg::SourceCatalog>showcaseCatalog_;std::shared_ptr<const bg::Prepared>showcaseFrame_;
 Proof()=default;friend class Store;
public:
 Proof(const Proof&)=delete;Proof&operator=(const Proof&)=delete;
 const std::shared_ptr<const nfs_main_sidebar_origin::Proof>&origin()const noexcept{return origin_;}
 bool showcaseReturn()const noexcept{return bool(showcaseCatalog_);}
 std::shared_ptr<const bg::Prepared>frame()const noexcept{return origin_?origin_->frame():showcaseFrame_;}
 const std::shared_ptr<const nfs_compact_caption::Proof>&captionProof()const noexcept{return caption_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&caption()const noexcept{return caption_->packet();}
 const bg::Plane&post()const noexcept{return post_;}const Binding&binding()const noexcept{return binding_;}
 uint64_t captionSerial()const noexcept{return caption_->evidence().serial;}
 uint64_t captionLifetime()const noexcept{return caption_->evidence().lifetime;}
 bool current()const noexcept{return life_&&life_->alive.load(std::memory_order_acquire)&&((origin_&&origin_->current())||(showcaseCatalog_&&showcaseFrame_&&showcaseFrame_->valid))&&caption_&&caption_->current();}
 // Validate the actual full-page copy against this captured return source.
 // No synthetic Group is needed: every converter row and destination word is
 // checked independently, including unchanged pixels outside the sidebar.
 bool verifiesShowcaseCopy(const nfs_logo_capture::Copy&c,bg::Workspace&w)const noexcept{
  namespace sl=nfs_small_logo_live;
  if(!showcaseReturn()||!current()||!c.serial||c.thread!=binding_.thread||c.linkedGroup||c.error||c.complete!=1||c.span!=16+bg::Side*2||!bg::plane(c.pre,640)||!bg::plane(c.post,640)||!nfs_logo_capture::same(c.pre.target,c.post.target)||!nfs_main_sidebar_origin::sameRetainedTarget(c.sourceOwner,post_.target)||c.args[0]!=post_.target.w[8]||c.args[1]!=0x2800||c.args[2]||c.args[3]||c.rows!=480||c.sourceBefore!=c.sourceAfter||c.shaBefore!=c.shaAfter||!sl::hashMatches(c.sourceBefore.data(),c.span,c.shaBefore)||sl::overlap(c.args[0],c.span,c.pre.target.w[8],16+bg::Full*2)||!sl::tables(c.tableBefore,c.tableAfter,true,0,0x7d))return false;
  for(unsigned k=0;k<4;++k)if(bg::le32(c.sourceBefore.data()+4*k)!=post_.target.header[k])return false;
  for(unsigned k=0;k<bg::Side;++k)if(bg::le16(c.sourceBefore.data()+16+2*k)!=post_.pixels[k])return false;
  std::copy_n(c.pre.pixels.begin(),bg::Full,w.logo.copyReplay.begin());const uint32_t key=c.row[0].colorKey;
  for(unsigned y=0;y<480;++y){const auto&row=c.row[y];const auto&rr=row.row;const uint64_t dest=uint64_t(c.pre.target.w[8])+16+y*1280;
   if(rr.callback!=0xf0001844||row.finalCallback!=rr.callback||row.converter!=0xf0001f44||row.colorKey!=key||row.bytes!=4*343||!rr.args[0]||uint64_t(rr.args[0])+row.bytes>UINT64_C(0x100000000)||rr.args[1]!=dest||rr.args[2]!=343||sl::overlap(rr.args[0],row.bytes,c.pre.target.w[8],16+bg::Full*2))return false;
   for(unsigned x=0;x<343;++x){const auto value=sl::expanded(post_.pixels[y*343+x],key);if(bg::le32(row.source.data()+4*x)!=value)return false;auto&pixel=w.logo.copyReplay[y*640+x];pixel=sl::pixel(pixel,value,false);}
  }
  return std::equal(w.logo.copyReplay.begin(),w.logo.copyReplay.end(),c.post.pixels.begin())&&current();
 }
 bool catalogMatches(const bg::SourceCatalog&catalog)const noexcept{const auto c=origin_?origin_->catalog():showcaseCatalog_;return c&&nfs_main_sidebar_origin::sameCatalog(*c,catalog);}
 bool catalogMatches(const std::shared_ptr<const bg::SourceCatalog>&catalog)const noexcept{return catalog&&catalogMatches(*catalog);}
};
class Store final {
 std::shared_ptr<const Proof>pending_;
 static bool planeMatches(const Proof&p,const bg::Plane&actual)noexcept{return bg::plane(actual,343)&&nfs_main_sidebar_origin::sameRetainedTarget(p.post().target,actual.target)&&std::equal(p.post().pixels.begin(),p.post().pixels.begin()+bg::Side,actual.pixels.begin());}
public:
 Store()=default;Store(const Store&)=delete;Store&operator=(const Store&)=delete;~Store(){clear();}
 void clear()noexcept{if(pending_&&pending_->life_)pending_->life_->alive.store(false,std::memory_order_release);pending_.reset();}
 void reset()noexcept{clear();}
 // Current here means unrevoked historical source. Caller must acquire with
 // independently captured live Binding/343 before reuse, and reset on Player
 // owner exit/retirement. Numeric form/epoch alone never seals a source.
 std::shared_ptr<const Proof>current()const noexcept{return pending_&&pending_->current()?pending_:std::shared_ptr<const Proof>{};}
 std::shared_ptr<const Proof>accept(std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin,std::shared_ptr<const nfs_compact_caption::Proof>caption,const Binding&binding,const bg::Plane&actual,bg::Workspace&workspace,std::shared_ptr<const bg::SourceCatalog>catalog)noexcept{
  clear();if(!origin||!origin->current()||!origin->frame()||!origin->catalog()||!caption||!caption->current()||caption->origin()!=origin||!catalog||!bg::validCatalog(*catalog)||!nfs_main_sidebar_origin::sameCatalog(*origin->catalog(),*catalog)||!bg::plane(actual,343))return {};
  const auto&e=caption->evidence();const auto&packet=caption->packet();
  const bool player1=e.policy==nfs_compact_caption::Policy::Player1;
  if((!player1&&e.policy!=nfs_compact_caption::Policy::Player2)||binding.form!=(player1?0x4d3050u:0x4d3100u)||!binding.formGuest||binding.formGuest!=e.formGuest||!binding.game||binding.game!=origin->game()||binding.game!=e.game||!binding.thread||binding.thread!=origin->publisherThread()||!binding.archive||binding.archive!=origin->archive()||!binding.epoch||binding.captionEvent.ownerEpoch!=binding.epoch||!binding.captionEvent.sequence||binding.captionEvent.iteration||e.route!=nfs_compact_caption::Route::InitialMain||!e.policyThread||!e.lifetime||!e.serial||!e.resourceSerial||!packet||packet->serial()!=e.serial||packet->surface().lifetime!=e.lifetime||packet->pixels()!=bg::Side||packet->commands()!=1||!packet->font())return {};
  if(e.incomingEbp!=origin->formGuest()&&e.incomingEbp!=0&&e.incomingEbp!=UINT32_MAX&&e.incomingEbp!=UINT32_MAX-1)return {};
  // Recheck exact immutable source metadata; policy-local TLS is deliberately
  // not equated to the independently supplied publisher-thread occurrence.
  if(e.requestedResource!=(player1?2u:3u)||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=(player1?198:234)||e.measureWidth!=(player1?68:70)||e.measureAnchor!=168||e.measureKind!=2||e.command.role!=1||e.command.sourceSize!=18||e.command.alignment!=2||e.command.color!=0xffff||e.command.x!=(player1?100:98)||e.command.y!=e.inputY||e.command.advance!=e.measureWidth||e.command.length!=10||std::memcmp(e.command.text.data(),player1?"player one":"player two",10))return {};
  const auto&original=origin->post();if(!bg::plane(original,343)||!nfs_main_sidebar_origin::sameRetainedTarget(original.target,actual.target)||!packet->surface().valid()||packet->clean().size()!=bg::Side||packet->original().size()!=bg::Side)return {};
  for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})if(s->archive!=binding.archive||!nfs_main_sidebar_origin::sameRetainedTarget(s->target,actual.target)||s->identity.lifetime!=e.lifetime||s->selector!=(player1?2u:3u)||s->resource!=(player1?2u:3u))return {};
  if(!(e.stringAfter.identity==packet->surface()))return {};for(unsigned k=0;k<14;++k)if(packet->surface().geometry[k]!=actual.target.w[k])return {};
  if(nfs_small_logo_live::verifyGroup(origin->group(),workspace.logo,catalog->logos)!=nfs_small_logo_live::Failure::None)return {};
  for(unsigned k=0;k<bg::Side;++k)if(packet->clean()[k]!=original.pixels[k]||packet->original()[k]!=actual.pixels[k]||(workspace.logo.groupUnion[k]&&packet->original()[k]!=packet->clean()[k]))return {};
  if(!origin->current()||!caption->current())return {};
  try{std::shared_ptr<Proof>p(new Proof);p->life_=std::make_shared<Proof::Life>();p->origin_=std::move(origin);p->caption_=std::move(caption);p->binding_=binding;p->post_=actual;if(!p->current())return {};pending_=p;return p;}catch(...){return {};}
 }
 // Separate return source. Its caption seal already owns the exact lexical
 // Showcase caller, selected pane and complete original word replay. No Main
 // epoch or logo draw group is invented for this original return route.
 std::shared_ptr<const Proof>acceptShowcaseReturn(std::shared_ptr<const nfs_compact_caption::Proof>caption,const Binding&b,const bg::Plane&actual,std::shared_ptr<const bg::SourceCatalog>catalog,unsigned phase,uint32_t previousForm)noexcept{
  clear();if(!caption||!caption->current()||caption->origin()||!catalog||!bg::validCatalog(*catalog)||phase>=320||!bg::plane(actual,343)||actual.target.header[3])return {};
  const auto&e=caption->evidence();const auto&packet=caption->packet();const bool one=b.form==0x4d3050;
  if((!one&&b.form!=0x4d3100)||!b.formGuest||e.formGuest!=b.formGuest||!b.game||e.game!=b.game||!b.thread||!b.archive||!b.epoch||b.captionEvent.ownerEpoch!=b.epoch||!b.captionEvent.sequence||b.captionEvent.iteration||!previousForm||e.incomingEbp!=previousForm||e.route!=nfs_compact_caption::Route::PlayerShowcaseReturn||e.policy!=(one?nfs_compact_caption::Policy::Player1:nfs_compact_caption::Policy::Player2)||!e.serial||!e.lifetime||!e.resourceSerial||!packet||!packet->font()||packet->serial()!=e.serial||packet->surface().lifetime!=e.lifetime||packet->pixels()!=bg::Side||packet->commands()!=1||packet->original().size()!=bg::Side||packet->clean().size()!=bg::Side)return {};
  const auto&c=e.command;if(e.requestedResource!=(one?2u:3u)||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=(one?198:234)||e.measureWidth!=(one?68:70)||e.measureAnchor!=168||e.measureKind!=2||c.role!=1||c.sourceSize!=18||c.alignment!=2||c.color!=0xffff||c.x!=(one?100:98)||c.y!=e.inputY||c.advance!=e.measureWidth||c.length!=10||std::memcmp(c.text.data(),one?"player one":"player two",10))return {};
  for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})if(s->archive!=b.archive||s->selector!=(one?2u:3u)||s->resource!=(one?2u:3u)||s->identity.lifetime!=e.lifetime||!nfs_main_sidebar_origin::sameRetainedTarget(s->target,actual.target))return {};
  if(!(packet->surface()==e.stringAfter.identity)||!std::equal(packet->original().begin(),packet->original().end(),actual.pixels.begin()))return {};
  try{auto frame=std::make_shared<bg::Prepared>();frame->serial=b.captionEvent.sequence;frame->contentRevision=e.serial;
   const unsigned first=phase/80,roles[]={first+1,(first+1)%4+1,0},scalars[]={256,phase%80*255/80,256};
   for(unsigned i=0;i<3;++i){const auto&a=catalog->logos.identities[roles[i]];auto&l=frame->logos[i];l.role=roles[i];l.scalar256=scalars[i];l.left=float(nfs_small_logo_live::signed12(a.header[3]));l.top=float(nfs_small_logo_live::signed12(a.header[3]>>16));l.right=l.left+a.width;l.bottom=l.top+a.height;l.clip[2]=343;l.clip[3]=480;}
   for(unsigned y=0;y<480;++y)std::copy_n(actual.pixels.begin()+y*343,343,frame->original.begin()+y*640);frame->valid=true;
   std::shared_ptr<Proof>p(new Proof);p->life_=std::make_shared<Proof::Life>();p->showcaseCatalog_=std::move(catalog);p->showcaseFrame_=std::move(frame);p->caption_=std::move(caption);p->binding_=b;p->post_=actual;if(!p->current())return {};pending_=p;return p;
  }catch(...){return {};}
 }
 std::shared_ptr<const Proof>acquire(const Binding&binding,const bg::Plane&actual,const std::shared_ptr<const bg::SourceCatalog>&catalog)noexcept{
  if(!pending_||!pending_->current()||!sameBinding(pending_->binding(),binding)||!pending_->catalogMatches(catalog)||!planeMatches(*pending_,actual)||!pending_->current()){clear();return {};}return pending_;
 }
};
} // namespace nfs_player_initial_sidebar
