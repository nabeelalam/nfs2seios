// PRIVATE finite LoadSave display, never a source owner or global FontCatalog.
#pragma once
#include "LivePacketCore.h"
#include "hd/ClassicScreens/private/WordData.h"
#include "hd/SoftwareTextVertices.h"
#include <algorithm>
#include <atomic>
#include <cstring>
namespace nfs_ls_word_display {
namespace source=nfs_loadsave_source;
class Catalog final {
 std::vector<unsigned char>alpha_;std::array<unsigned char,32>sha_{};
 Catalog()=default;
#ifdef NFS_LS_WORD_DISPLAY_CONTRACT
 friend struct ContractAccess;
#endif
 static bool decode(const nfs_ls_words_data::Row&r,const uint16_t*runs,size_t values,std::vector<unsigned char>&out)noexcept{
  if(!runs||!values||values%2||!r.width||r.width>476||r.height!=128||values>2u*r.width*r.height)return false;
  try{const size_t pixels=size_t(r.width)*r.height;size_t total=0;for(size_t i=0;i<values;i+=2){if(!runs[i]||runs[i+1]>255||runs[i]>pixels-total)return false;total+=runs[i];}if(total!=pixels)return false;
   std::vector<unsigned char>next;next.reserve(pixels);for(size_t i=0;i<values;i+=2)next.insert(next.end(),runs[i],uint8_t(runs[i+1]));std::vector<unsigned char>rgba(pixels*4,255);for(size_t i=0;i<pixels;++i)rgba[4*i+3]=next[i];unsigned char digest[32];if(!nfs_texture_sha256(rgba.data(),rgba.size(),digest)||std::memcmp(digest,r.rgbaSha,32))return false;
   for(size_t y=0;y<r.height;++y)if(next[y*r.width]||next[y*r.width+r.width-1])return false;for(size_t x=0;x<r.width;++x)if(next[x]||next[(r.height-1)*r.width+x])return false;out.swap(next);return true;
  }catch(...){return false;}
 }
 static bool bounds(const nfs_ls_words_data::Row&r)noexcept{return r.x<=1024&&r.width<=1024-r.x&&r.y<=512&&r.height<=512-r.y;}
 static std::shared_ptr<const Catalog>build()noexcept{try{auto p=std::shared_ptr<Catalog>(new Catalog);p->alpha_.assign(1024*512,0);for(size_t i=0;i<8;++i){const auto&r=nfs_ls_words_data::rows[i];if(r.width!=4*r.advance||r.height!=128||!bounds(r)||r.advance>119||r.resource!=source::resource(i)||r.top!=unsigned(source::top(i))||std::strcmp(r.text,source::text(i)))return {};std::vector<unsigned char>a;if(!decode(r,r.runs,r.runValues,a))return {};for(size_t y=0;y<r.height;++y)std::copy_n(a.data()+y*r.width,r.width,p->alpha_.data()+(r.y+y)*1024+r.x);}if(!nfs_texture_sha256(p->alpha_.data(),p->alpha_.size(),p->sha_.data())||std::memcmp(p->sha_.data(),nfs_ls_words_data::atlasSha,32))return {};return p;}catch(...){return {};}}
public:
 static std::shared_ptr<const Catalog>builtin()noexcept{static const auto p=build();return p;}
 const std::vector<unsigned char>&alpha()const noexcept{return alpha_;}
 const std::array<unsigned char,32>&sha()const noexcept{return sha_;}
};
class Prepared final {
 std::shared_ptr<const nfs_ls_live::OwnedFrame>frame_;std::shared_ptr<const Catalog>art_;std::array<NfsSoftwareTextVertex,96>vertices_{};
 Prepared()=default;
 static bool binding(const std::shared_ptr<const nfs_ls_live::OwnedFrame>&f)noexcept{
  if(!f||!f->packet()||!f->packet()->font())return false;const auto&p=*f->packet();const auto&e=f->evidence();const auto&profile=f->profile();if(!source::authority(e.entry)||!source::sameAuthority(e.entry,e.exit)||!e.post.valid(profile,e.exit.owner)||p.serial()!=e.exit.owner.serial||!(p.surface()==e.post.surface)||p.font()!=profile.displayFont||p.pixels()!=307200||p.clean().size()!=307200||p.original().size()!=307200||p.commands()!=16||p.records().size()!=16||!p.writes()||!p.surface().valid())return false;
  size_t end=0;for(size_t k=0;k<16;++k){size_t i=k/2;bool shadow=k%2==0;const auto&r=nfs_ls_words_data::rows[i];const auto&record=p.records()[k];const auto&c=record.command;size_t len=std::strlen(r.text);if(!len||len>95||c.length!=len||std::memcmp(c.text.data(),r.text,len)||c.advance!=int32_t(r.advance)||c.x!=426-int32_t(r.advance)+int(shadow)||c.y!=int32_t(r.top)+int(shadow)||c.sourceSize!=18||c.role!=profile.roles[i]||c.alignment!=2||c.originX||c.originY||c.clip.left||c.clip.top||c.clip.right!=640||c.clip.bottom!=480||c.color!=(shadow?1:source::color(source::style(e.entry,i)))||record.vertexFirst!=end||!record.vertexCount||record.vertexCount%6||record.vertexCount>p.vertices().size()-std::min(end,p.vertices().size()))return false;for(size_t j=len;j<c.text.size();++j)if(c.text[j])return false;end+=record.vertexCount;}return end==p.vertices().size();
 }
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_ls_live::OwnedFrame>frame)noexcept{if(!binding(frame))return {};auto art=Catalog::builtin();if(!art)return {};try{auto p=std::shared_ptr<Prepared>(new Prepared);size_t used=0;for(size_t k=0;k<16;++k){const auto&c=frame->packet()->records()[k].command;const auto&r=nfs_ls_words_data::rows[k/2];nfs_soft::Layout layout;layout.count=layout.length=1;layout.quads[0]={float(c.x),float(c.y),float(c.x+c.advance),float(c.y+32),r.x/1024.f,r.y/512.f,(r.x+r.width)/1024.f,(r.y+r.height)/512.f,0};if(!nfs_soft::append_vertices(layout,c.color,p->vertices_.data(),p->vertices_.size(),used))return {};}if(used!=96)return {};p->frame_=std::move(frame);p->art_=std::move(art);return p;}catch(...){return {};}}
 bool currentFor(const std::shared_ptr<const nfs_ls_live::OwnedFrame>&f)const noexcept{return f&&frame_.get()==f.get()&&!frame_.owner_before(f)&&!f.owner_before(frame_)&&binding(f)&&art_&&art_->alpha().size()==524288;}
 NfsSoftwareTextLayerView layer()const noexcept{NfsSoftwareTextLayerView v{};if(!currentFor(frame_))return v;v.logicalWidth=640;v.logicalHeight=480;v.font.schema=1;v.font.width=1024;v.font.height=512;v.font.identity=nfs_ls_words_data::identity;v.font.alpha=art_->alpha().data();v.font.bytes=art_->alpha().size();std::copy(art_->sha().begin(),art_->sha().end(),v.font.sha256);v.vertices=vertices_.data();v.vertexCount=vertices_.size();return v;}
 const std::shared_ptr<const nfs_ls_live::OwnedFrame>&frame()const noexcept{return frame_;}
};
// This owns display borrows; the unchanged live Game acquire/reacquire gates
// still provide current source authority, never Prepared or this neutral C view.
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const nfs_ls_live::Lease>source_;std::shared_ptr<const Prepared>words_;std::shared_ptr<const nfs_ls_live::OwnedFrame>frame_;
 Lease()=default;static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}static void release(void*p)noexcept{auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
#ifdef NFS_LS_WORD_DISPLAY_CONTRACT
 friend struct ContractAccess;
#endif
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const nfs_ls_live::OwnedFrame>f)noexcept{auto words=Prepared::create(f);if(!words)return {};auto source=nfs_ls_live::Lease::create(f);if(!source)return {};try{auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*p){release(const_cast<Lease*>(p));});p->frame_=std::move(f);p->source_=std::move(source);p->words_=std::move(words);return p;}catch(...){return {};}}
 NfsLoadSaveLiveView view(uint64_t request)const noexcept{NfsLoadSaveLiveView v{};if(!request||!source_||!words_||!words_->currentFor(frame_))return v;v=source_->view(request);auto display=words_->layer();if(v.schema!=1||display.vertexCount!=96||v.content!=frame_->packet()->serial())return NfsLoadSaveLiveView{};v.text=display;v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}
};
} // namespace nfs_ls_word_display
