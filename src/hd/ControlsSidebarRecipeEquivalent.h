// SPDX-License-Identifier: MIT
#pragma once
// Immutable image-recipe equality only. This creates no Game/current-owner,
// source-copy, parent-plane, native-pass or publication authority.
#include "AudioSidebarPacket.h"
#include <cmath>
#include <cstring>
namespace nfs_controls_sidebar_recipe {
namespace detail {
inline bool number(float a,float b)noexcept{return std::isfinite(a)&&std::isfinite(b)&&a==b;}
inline bool word(const NfsBackgroundSceneWordLayer&a,const NfsBackgroundSceneWordLayer&b)noexcept{
 if(a.sourceRole!=b.sourceRole||a.scalar256!=b.scalar256||a.left!=b.left||a.top!=b.top)return false;
 for(unsigned i=0;i<4;++i)if(a.clip[i]!=b.clip[i])return false;return true;
}
inline bool logo(const NfsSmallLogoLayer&a,const NfsSmallLogoLayer&b)noexcept{
 if(a.role!=b.role||a.scalar256!=b.scalar256||!number(a.left,b.left)||!number(a.top,b.top)||!number(a.right,b.right)||!number(a.bottom,b.bottom))return false;
 for(unsigned i=0;i<4;++i)if(!number(a.clip[i],b.clip[i]))return false;return true;
}
inline bool catalog(const nfs_background_scene_cpu::SourceCatalog&a,const nfs_background_scene_cpu::SourceCatalog&b)noexcept{
 if(a.roles!=b.roles||a.mapRoles!=b.mapRoles||a.mapSha!=b.mapSha||a.logos.roles!=b.logos.roles)return false;
 for(size_t k=0;k<a.identities.size();++k){const auto&x=a.identities[k];const auto&y=b.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.header!=y.header||x.sha!=y.sha)return false;}
 for(size_t k=0;k<a.logos.identities.size();++k){const auto&x=a.logos.identities[k];const auto&y=b.logos.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.paletteCount!=y.paletteCount||x.maxIndex!=y.maxIndex||x.header!=y.header||x.sha!=y.sha)return false;}
 return true;
}
inline bool clip(const nfs_soft::Clip&a,const nfs_soft::Clip&b)noexcept{return number(a.left,b.left)&&number(a.top,b.top)&&number(a.right,b.right)&&number(a.bottom,b.bottom);}
inline bool command(const nfs_soft::TextCommand&a,const nfs_soft::TextCommand&b)noexcept{
 return a.text==b.text&&a.length==b.length&&a.role==b.role&&a.alignment==b.alignment&&a.sourceSize==b.sourceSize&&a.x==b.x&&a.y==b.y&&a.originX==b.originX&&a.originY==b.originY&&a.advance==b.advance&&a.color==b.color&&clip(a.clip,b.clip);
}
inline bool vertex(const nfs_soft::Vertex&a,const nfs_soft::Vertex&b)noexcept{
 return number(a.x,b.x)&&number(a.y,b.y)&&number(a.u,b.u)&&number(a.v,b.v)&&number(a.r,b.r)&&number(a.g,b.g)&&number(a.b,b.b)&&number(a.a,b.a);
}
inline bool font(const nfs_soft::FontCatalog&a,const nfs_soft::FontCatalog&b)noexcept{
 if(&a==&b)return true; // FontCatalog owns immutable metadata and alpha bytes.
 if(a.identity()!=b.identity()||a.alpha()!=b.alpha())return false;
 const auto&x=a.description();const auto&y=b.description();if(x.baseline!=y.baseline||x.kern!=y.kern)return false;
 for(unsigned r=0;r<nfs_soft::FontDescription::roles;++r)for(unsigned k=0;k<nfs_soft::FontDescription::glyphCount;++k){const auto&g=x.glyphs[r][k];const auto&h=y.glyphs[r][k];if(g.x!=h.x||g.y!=h.y||g.w!=h.w||g.h!=h.h||g.left!=h.left||g.top!=h.top||g.advance!=h.advance)return false;}
 return true;
}
inline bool surface(const nfs_soft::SurfaceIdentity&a,const nfs_soft::SurfaceIdentity&b)noexcept{
 if(!a.valid()||!b.valid())return false;
 // Only lifetime and three guest allocation addresses are versioned ownership.
 // All other descriptor values and relative LUT samples remain exact.
 for(unsigned k=0;k<a.geometry.size();++k)if(k!=8&&k!=11&&k!=12&&a.geometry[k]!=b.geometry[k])return false;return true;
}
inline bool caption(const nfs_soft::FramePacket&a,const nfs_soft::FramePacket&b)noexcept{
 if(!a.font()||!b.font()||!font(*a.font(),*b.font())||!surface(a.surface(),b.surface())||a.pixels()!=b.pixels()||a.commands()!=b.commands()||a.writes()!=b.writes()||a.clean()!=b.clean()||a.original()!=b.original()||a.records().size()!=b.records().size()||a.vertices().size()!=b.vertices().size())return false;
 // Serial is a source generation, never an image-content field.
 for(size_t k=0;k<a.records().size();++k){const auto&x=a.records()[k];const auto&y=b.records()[k];if(x.vertexFirst!=y.vertexFirst||x.vertexCount!=y.vertexCount||!command(x.command,y.command))return false;}
 for(size_t k=0;k<a.vertices().size();++k)if(!vertex(a.vertices()[k],b.vertices()[k]))return false;return true;
}
inline bool captionWord(const NfsCompactCaptionWordView&a,const NfsCompactCaptionWordView&b)noexcept{
 constexpr size_t Bytes=192u*128u*4u;
 if(a.schema!=NFS_COMPACT_CAPTION_WORD_SCHEMA||b.schema!=a.schema||a.word!=3||b.word!=a.word||a.width!=192||b.width!=a.width||a.height!=128||b.height!=a.height||a.bytes!=Bytes||b.bytes!=Bytes||!a.rgba||!b.rgba||!number(a.left,b.left)||!number(a.top,b.top)||!number(a.right,b.right)||!number(a.bottom,b.bottom))return false;
 return std::memcmp(a.sha256,b.sha256,sizeof a.sha256)==0&&std::memcmp(a.rgba,b.rgba,Bytes)==0;
}
} // namespace detail
inline bool equivalent(const std::shared_ptr<const nfs_audio_sidebar::Packet>&a,const std::shared_ptr<const nfs_audio_sidebar::Packet>&b)noexcept{
 if(!a||!b||!a->catalog()||!b->catalog()||!a->caption()||!b->caption()||!a->captionWord()||!b->captionWord()||!a->captionWord()->current()||!b->captionWord()->current()||!nfs_background_scene_cpu::validCatalog(*a->catalog())||!nfs_background_scene_cpu::validCatalog(*b->catalog())||a->raw().size()!=nfs_audio_sidebar::Bytes||a->raw()!=b->raw()||a->wordCount()>a->words().size()||a->wordCount()!=b->wordCount()||!detail::catalog(*a->catalog(),*b->catalog())||!detail::caption(*a->caption(),*b->caption())||!detail::captionWord(a->captionWord()->view(),b->captionWord()->view()))return false;
 for(unsigned k=0;k<a->wordCount();++k)if(!detail::word(a->words()[k],b->words()[k]))return false;
 for(size_t k=0;k<a->logos().size();++k)if(!detail::logo(a->logos()[k],b->logos()[k]))return false;
 // The caller separately proves current packet ownership and the retained base
 // map digest's unique role in this catalogue. No binding is normalized here.
 return true;
}
} // namespace nfs_controls_sidebar_recipe
