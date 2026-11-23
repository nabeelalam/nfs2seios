// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsScenePrepare.h"
#include "ClassicScreens/FullSceneSource.h"
#include "NestedSidebarCaptionDisplay.h"
#include "Main343OriginLifetime.h"
#include "Main343OriginProof.h"
namespace nfs_options_sidebar_origin {
// Immutable semantic content only. A retired Options owner grants no authority
// over a child draw; the new child must prove its complete source and raw copy.
struct Sidebar final {
 std::shared_ptr<const nfs_soft::FramePacket>caption;
 std::shared_ptr<const nfs_compact_caption_display::Prepared>captionWord;
 std::shared_ptr<const nfs_nested_sidebar_caption::Display>nestedCaptionWord;
 std::shared_ptr<const nfs_ls_scene::Cache>loadsaveCache;
 std::shared_ptr<const nfs_compact_caption_display::Artwork>loadsaveCaptionArt;
 bool captionCurrent()const noexcept{return caption&&((captionWord&&!nestedCaptionWord&&captionWord->current()&&captionWord->packet()==caption)||(!captionWord&&nestedCaptionWord&&nestedCaptionWord->current()&&nestedCaptionWord->packet()==caption)||(!captionWord&&!nestedCaptionWord&&loadsaveCache&&loadsaveCaptionArt&&loadsaveCaptionArt->word==3&&loadsaveCache->captionPacket()==caption));}
 NfsCompactCaptionWordView wordView()const noexcept{
  if(!captionCurrent())return {};if(captionWord)return captionWord->view();if(nestedCaptionWord)return nestedCaptionWord->view();
  NfsCompactCaptionWordView v{};v.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.word=3;v.width=loadsaveCaptionArt->width;v.height=loadsaveCaptionArt->height;v.rgba=loadsaveCaptionArt->rgba.data();v.bytes=loadsaveCaptionArt->rgba.size();std::copy(loadsaveCaptionArt->sha.begin(),loadsaveCaptionArt->sha.end(),v.sha256);v.left=120;v.top=364;v.right=168;v.bottom=396;return v;
 }
 std::array<NfsBackgroundSceneWordLayer,4>words{};
 std::array<NfsSmallLogoLayer,3>logos{};unsigned wordCount=0;
};
class Origin final {
 Origin()=default;
public:
 std::shared_ptr<const Sidebar>frame;
 std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>catalog;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>life;
 std::vector<uint8_t>raw;uintptr_t game=0,thread=0;uint32_t archive=0;uint64_t epoch=0;
 bool current()const noexcept{return frame&&frame->captionCurrent()&&catalog&&life&&life->current();}
 bool matches(uintptr_t g,uintptr_t t,uint32_t a,const uint8_t*p,size_t n)const noexcept{
  return current()&&g==game&&t==thread&&a==archive&&p&&n==raw.size()&&!std::memcmp(p,raw.data(),n);
 }
 static std::shared_ptr<const Origin>create(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const std::shared_ptr<const nfs_options_scene_cpu::Prepared>&frame,
 const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog,const uint8_t*raw,size_t count)noexcept{
  if(!game||!thread||!archive||!epoch||!frame||!frame->valid||!catalog||!raw||count!=16+343*480*2||!frame->caption||frame->caption->original().size()!=343*480)return {};
  if(nfs_background_scene_cpu::le32(raw)!=0x78||nfs_background_scene_cpu::le32(raw+4)!=(343u|(480u<<16))||nfs_background_scene_cpu::le32(raw+8)||nfs_background_scene_cpu::le32(raw+12))return {};
  for(size_t k=0;k<343*480;++k)if(nfs_background_scene_cpu::le16(raw+16+2*k)!=frame->caption->original()[k])return {};
  try{auto o=std::shared_ptr<Origin>(new Origin);o->game=game;o->thread=thread;o->archive=archive;o->epoch=epoch;auto side=std::make_shared<Sidebar>();side->caption=frame->caption;side->captionWord=frame->captionWord;side->words=frame->words;side->logos=frame->logos;side->wordCount=frame->wordCount;o->frame=std::move(side);o->catalog=catalog;o->raw.assign(raw,raw+count);o->life=nfs_main_sidebar_origin::acquireGameLife(game);return o->current()?o:std::shared_ptr<const Origin>{};}catch(...){return {};}
 }


 // A completed nested sidebar has its own independently replayed word/logo/
 // caption proof. Retain that immutable content, never the closed dialog owner.
 template<class Packet>static std::shared_ptr<const Origin>createNested(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const std::shared_ptr<const Packet>&packet,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog)noexcept{
  if(!game||!thread||!archive||!epoch||!packet||!catalog||!packet->catalog()||!nfs_main_sidebar_origin::sameCatalog(*packet->catalog(),*catalog)||!packet->binding().valid()||packet->binding().archive!=archive||packet->binding().ownerEpoch!=epoch||packet->binding().thread!=thread||packet->raw().size()!=16+343*480*2||packet->wordCount()>4||!packet->caption()||!packet->captionWord()||!packet->captionWord()->current()||packet->captionWord()->packet()!=packet->caption()||!packet->captionWord()->proof()||packet->captionWord()->proof()->evidence().game!=game||packet->captionWord()->proof()->evidence().form!=packet->binding().form)return {};
  try{auto o=std::shared_ptr<Origin>(new Origin);auto side=std::make_shared<Sidebar>();side->caption=packet->caption();side->nestedCaptionWord=packet->captionWord();side->words=packet->words();side->logos=packet->logos();side->wordCount=packet->wordCount();o->frame=std::move(side);o->catalog=catalog;o->raw=packet->raw();o->game=game;o->thread=thread;o->archive=archive;o->epoch=epoch;o->life=nfs_main_sidebar_origin::acquireGameLife(game);return o->current()?o:std::shared_ptr<const Origin>{};}catch(...){return {};}
 }

 // Load/Save verifies its own original caption callbacks/stores rather than
 // the compact-caption route. Its private-constructed Cache is the complete
 // immutable source proof; a later owner still proves its exact bytes anew.
 // Cache owner and shared publisher use distinct TLS token domains; the live
 // issuer supplies both while executing on the verified original owner thread.
 static std::shared_ptr<const Origin>createLoadSave(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const std::shared_ptr<const nfs_ls_scene::Cache>&cache,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog,uintptr_t publisherThread)noexcept{
  if(!publisherThread||!cache||!catalog||!cache->trusted()||!nfs_main_sidebar_origin::sameCatalog(cache->trusted()->catalog(),*catalog)||!cache->owner().valid()||cache->owner().game!=game||cache->owner().thread!=thread||cache->owner().archive!=archive||cache->owner().epoch!=epoch||!cache->captionPacket()||cache->raw().size()!=16+343*480*2||cache->wordCount()>4||cache->logos().draws!=3)return {};
  try{auto art=nfs_compact_caption_display::artwork(3);if(!art)return {};auto o=std::shared_ptr<Origin>(new Origin);auto side=std::make_shared<Sidebar>();side->caption=cache->captionPacket();side->loadsaveCache=cache;side->loadsaveCaptionArt=std::move(art);side->words=cache->words();side->wordCount=cache->wordCount();
   const auto&g=cache->logos();const auto&t=cache->slots().target[cache->slot()];
   for(unsigned k=0;k<3;++k){const auto&d=g.draw[k];if(d.role>=5)return {};const auto&a=catalog->logos.identities[d.role];auto&v=side->logos[k];v.role=d.role;v.scalar256=k==1?(d.args[1]&511):256;v.left=float(nfs_small_logo_live::signed12(a.header[3]));v.top=float(nfs_small_logo_live::signed12(a.header[3]>>16));v.right=v.left+a.width;v.bottom=v.top+a.height;v.clip[0]=float(t.w[3]);v.clip[1]=float(t.w[4]);v.clip[2]=float(t.w[5]);v.clip[3]=float(t.w[6]);}
   o->frame=std::move(side);o->catalog=catalog;o->raw=cache->raw();o->game=game;o->thread=publisherThread;o->archive=archive;o->epoch=epoch;o->life=nfs_main_sidebar_origin::acquireGameLife(game);return o->current()?o:std::shared_ptr<const Origin>{};
  }catch(...){return {};}
 }

 // A complete sidebar is a source in its own right. The original may abort
 // after its caption and before a full copy/upload; never invent a full frame.
 static std::shared_ptr<const Origin>createSidebar(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const nfs_background_scene_cpu::Scene&s,const nfs_logo_capture::Group&g,
 const std::shared_ptr<const nfs_soft::FramePacket>&caption,const std::shared_ptr<const nfs_compact_caption::Proof>&proof,
 uint64_t serial,uint64_t lifetime,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog)noexcept{
  namespace bg=nfs_background_scene_cpu;namespace sl=nfs_small_logo_live;
  if(!game||!thread||!archive||!epoch||!catalog||!bg::validCatalog(*catalog)||s.form!=nfs_options_scene_cpu::OptionsForm||s.error||s.draws.empty()||s.draws.size()>5||!caption||!proof||proof->evidence().route!=nfs_compact_caption::Route::OrdinaryEbp||!serial||!lifetime||g.thread!=thread||!bg::plane(s.preSide,343)||!bg::plane(s.postSide,343)||!bg::plane(s.postWords,343)||!bg::plane(s.tailPre,343)||!bg::plane(s.tailPost,343))return {};
  const auto&t=s.preSide.target;
  if(!nfs_logo_capture::same(t,s.postSide.target)||!nfs_logo_capture::same(t,s.postWords.target)||!nfs_logo_capture::same(t,s.tailPre.target)||!nfs_logo_capture::same(t,s.tailPost.target))return {};
  nfs_soft::SurfaceIdentity identity{};
  if(!nfs_options_scene_cpu::captionIdentity(t,lifetime,identity)||caption->serial()!=serial||caption->pixels()!=bg::Side||!caption->matches(identity,serial,s.tailPost.pixels.data(),bg::Side))return {};
  try{
   auto work=std::unique_ptr<bg::Workspace>(new bg::Workspace);auto content=std::make_shared<Sidebar>();
   if(bg::side(s.draws[0],t,*work,*catalog)!=bg::Failure::None||!std::equal(work->side.begin(),work->side.end(),s.postSide.pixels.begin()))return {};
   for(unsigned k=1;k<s.draws.size();++k)if(bg::word(s.draws[k],t,*work,content->words[k-1],*catalog)!=bg::Failure::None)return {};
   if(!std::equal(work->side.begin(),work->side.end(),s.postWords.pixels.begin())||!bg::samePlane(s.postWords,g.pre,343)||sl::verifyGroup(g,work->logo,catalog->logos)!=sl::Failure::None||!bg::samePlane(g.post,s.tailPre,343)||!std::equal(caption->clean().begin(),caption->clean().end(),g.post.pixels.begin()))return {};
   content->captionWord=nfs_compact_caption_display::Prepared::create(proof,caption,{nfs_options_scene_cpu::OptionsForm,proof->evidence().formGuest,archive,game,serial,lifetime});
   if(!content->captionWord||!content->captionWord->current())return {};
   content->caption=caption;content->wordCount=unsigned(s.draws.size()-1);
   for(unsigned k=0;k<3;++k){const auto&d=g.draw[k];const auto&a=catalog->logos.identities[d.role];auto&v=content->logos[k];v.role=d.role;v.scalar256=k==1?(d.args[1]&511):256;v.left=float(sl::signed12(a.header[3]));v.top=float(sl::signed12(a.header[3]>>16));v.right=v.left+a.width;v.bottom=v.top+a.height;v.clip[0]=float(t.w[3]);v.clip[1]=float(t.w[4]);v.clip[2]=float(t.w[5]);v.clip[3]=float(t.w[6]);}
   auto o=std::shared_ptr<Origin>(new Origin);o->frame=std::move(content);o->catalog=catalog;o->game=game;o->thread=thread;o->archive=archive;o->epoch=epoch;o->life=nfs_main_sidebar_origin::acquireGameLife(game);o->raw.resize(16+bg::Side*2);
   for(unsigned k=0;k<4;++k)for(unsigned b=0;b<4;++b)o->raw[4*k+b]=uint8_t(t.header[k]>>(8*b));
   for(size_t k=0;k<bg::Side;++k){o->raw[16+2*k]=uint8_t(s.tailPost.pixels[k]);o->raw[17+2*k]=uint8_t(s.tailPost.pixels[k]>>8);}
   return o->current()?o:std::shared_ptr<const Origin>{};
  }catch(...){return {};}
 }

};
inline std::shared_ptr<const Origin>&slot(){static thread_local std::shared_ptr<const Origin>p;return p;}
// Original343 caches alternate storage. Keep the last completed content for
// each actual bitmap; the receiving owner must still prove exact current bytes.
struct History {std::array<std::shared_ptr<const Origin>,4>entries{};unsigned next=0;};
inline History&history(){static thread_local History h;return h;}
inline void remember(std::shared_ptr<const Origin>p)noexcept{
 if(!p||!p->current())return;auto&h=history();const auto bitmap=p->frame->caption->surface().geometry[8];
 for(auto&old:h.entries)if(old&&old->game==p->game&&old->thread==p->thread&&old->archive==p->archive&&old->catalog==p->catalog&&old->frame->caption->surface().geometry[8]==bitmap){old=p;slot()=std::move(p);return;}
 h.entries[h.next++%h.entries.size()]=p;slot()=std::move(p);
}

inline void publish(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const std::shared_ptr<const nfs_options_scene_cpu::Prepared>&frame,
 const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog,const uint8_t*raw,size_t count)noexcept{
 const auto prior=slot();if(prior&&prior->catalog==catalog&&prior->matches(game,thread,archive,raw,count))return;
 remember(Origin::create(game,thread,archive,epoch,frame,catalog,raw,count));
}
inline void publishSidebar(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,
 const nfs_background_scene_cpu::Scene&scene,const nfs_logo_capture::Group&logos,
 const std::shared_ptr<const nfs_soft::FramePacket>&caption,const std::shared_ptr<const nfs_compact_caption::Proof>&proof,
 uint64_t serial,uint64_t lifetime,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog)noexcept{
 const auto p=Origin::createSidebar(game,thread,archive,epoch,scene,logos,caption,proof,serial,lifetime,catalog);if(p)remember(std::move(p));
}
template<class Packet>inline void publishNested(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,const std::shared_ptr<const Packet>&packet,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog)noexcept{
 auto p=Origin::createNested(game,thread,archive,epoch,packet,catalog);if(p)remember(std::move(p));
}
inline void publishLoadSave(uintptr_t game,uintptr_t thread,uint32_t archive,uint64_t epoch,const std::shared_ptr<const nfs_ls_scene::Cache>&cache,const std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>&catalog,uintptr_t publisherThread)noexcept{
 auto p=Origin::createLoadSave(game,thread,archive,epoch,cache,catalog,publisherThread);if(p)remember(std::move(p));
}
inline std::shared_ptr<const Origin>acquire(uintptr_t game,uintptr_t thread,uint32_t archive,const uint8_t*raw,size_t count)noexcept{
 const auto p=slot();if(p&&p->matches(game,thread,archive,raw,count))return p;for(const auto&old:history().entries)if(old&&old->matches(game,thread,archive,raw,count))return old;return {};
}

}
