// PRIVATE neutral display lease. Live Game authority remains in currentScene().
#pragma once
#include "LiveLayerAbi.h"
#include "WordDisplay.h"
#include "FullSceneSource.h"
#include "hd/CompactCaptionWordArtwork.h"
#include <atomic>
#include <algorithm>
namespace nfs_ls_scene_display {
class Lease final {
 std::atomic<unsigned>refs_{1};
 std::shared_ptr<const nfs_ls_live::OwnedFrame>frame_;
 std::shared_ptr<const nfs_ls_scene::OwnedScene>scene_;
 std::shared_ptr<const nfs_ls_word_display::Lease>words_;
 std::shared_ptr<const nfs_compact_caption_display::Artwork>caption_;
 NfsBackgroundSceneLayers layers_{};
 Lease()=default;
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 template<class T>static bool sameOwner(const std::shared_ptr<const T>&a,const std::shared_ptr<const T>&b)noexcept{return a&&b&&a.get()==b.get()&&!a.owner_before(b)&&!b.owner_before(a);}
 static bool binding(const std::shared_ptr<const nfs_ls_live::OwnedFrame>&f,const std::shared_ptr<const nfs_ls_scene::OwnedScene>&s)noexcept{
  if(!f||!s||!f->packet()||!s->cache()||!s->cache()->trusted()||s->mapRole()>1||s->cache()->wordCount()>4||s->clean().size()!=307200||f->packet()->clean()!=s->clean())return false;
  const auto&o=f->evidence().exit.owner;const auto&owner=s->cache()->owner();
  return o.game==owner.game&&o.formSymbol==owner.form&&o.handlerSymbol==owner.handler&&o.type==owner.type&&o.runtimeForm&&o.archive==owner.archive;
 }
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const nfs_ls_live::OwnedFrame>f,std::shared_ptr<const nfs_ls_scene::OwnedScene>s)noexcept{
  if(!binding(f,s))return {};auto words=nfs_ls_word_display::Lease::create(f);auto art=nfs_compact_caption_display::artwork(3);if(!words||!art||art->word!=3||art->width!=192||art->height!=128)return {};
  try{auto out=std::shared_ptr<Lease>(new Lease,[](const Lease*p){release(const_cast<Lease*>(p));});out->frame_=std::move(f);out->scene_=std::move(s);out->words_=std::move(words);out->caption_=std::move(art);auto&layers=out->layers_;const auto&cache=*out->scene_->cache();
   layers.mapRole=out->scene_->mapRole();layers.wordCount=cache.wordCount();layers.panelClip[0]=layers.panelClip[1]=0;layers.panelClip[2]=343;layers.panelClip[3]=480;std::copy(cache.words().begin(),cache.words().end(),layers.words);
   const auto&group=cache.logos();const auto&catalog=cache.trusted()->catalog().logos;const auto&w=group.pre.target.w;
   if(group.draws!=3||!group.complete||group.error)return {};
   for(unsigned i=0;i<3;++i){const auto&d=group.draw[i];if(d.role>=5)return {};const auto&id=catalog.identities[d.role];auto&l=layers.logos[i];const int x=nfs_small_logo_live::signed12(id.header[3]),y=nfs_small_logo_live::signed12(id.header[3]>>16);l.role=d.role;l.scalar256=i==1?(d.args[1]&511u):256u;l.left=float(x);l.top=float(y);l.right=float(x+int(id.width));l.bottom=float(y+int(id.height));l.clip[0]=float(std::max(0,int(w[3])));l.clip[1]=float(std::max(0,int(w[4])));l.clip[2]=float(std::min(343,int(w[5])));l.clip[3]=float(std::min(480,int(w[6])));if(l.scalar256>256||l.clip[0]>=l.clip[2]||l.clip[1]>=l.clip[3])return {};}
   return out;
  }catch(...){return {};}
 }
 bool currentFor(const std::shared_ptr<const nfs_ls_live::OwnedFrame>&f,const std::shared_ptr<const nfs_ls_scene::OwnedScene>&s)const noexcept{return sameOwner(frame_,f)&&sameOwner(scene_,s)&&binding(f,s)&&words_&&caption_;}
 NfsLoadSaveLiveView view(uint64_t request)const noexcept{
  NfsLoadSaveLiveView v{};if(!request||!currentFor(frame_,scene_))return v;v=words_->view(request);if(v.schema!=1||v.text.vertexCount!=96)return {};v.schema=2;v.sourceGame=scene_->cache()->owner().game;v.sourceEpoch=scene_->cache()->owner().epoch;v.scene=&layers_;v.caption.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.caption.word=3;v.caption.width=caption_->width;v.caption.height=caption_->height;v.caption.rgba=caption_->rgba.data();v.caption.bytes=caption_->rgba.size();std::copy(caption_->sha.begin(),caption_->sha.end(),v.caption.sha256);v.caption.left=120;v.caption.top=364;v.caption.right=168;v.caption.bottom=396;v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;
 }
};
} // namespace nfs_ls_scene_display
