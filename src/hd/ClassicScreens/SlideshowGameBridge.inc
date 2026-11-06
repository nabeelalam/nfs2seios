// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowGameHooks.inc"
#include "hd/SlideshowSession.h"
#include "GuestCopyTransaction.h"
#include <string>
#include "hd/SlideshowStartup.h"
#if __has_include("hd/ClassicScreens/private/SlideshowCatalogData.h")
#include "hd/ClassicScreens/private/SlideshowCatalogData.h"
#define NFS_SLIDESHOW_PRIVATE_CATALOG_AVAILABLE 1
#endif
namespace nfs_slideshow_live {
struct Configuration {
 const nfs_slideshow::SourcePin* sources=nullptr;size_t sourceCount=0;
 const nfs_slideshow::RuntimeImagePin* images=nullptr;size_t imageCount=0;
 std::string path;uint32_t expectedForm=0;bool preview=false;
 std::shared_ptr<const nfs_slideshow_footer::Catalog> footers;
};
struct Bridge {
 Configuration configuration;nfs_slideshow::Session session;
 Game*game=nullptr;uint32_t form=0,archive=0;bool presenting=false;
 void retire()noexcept{session.close();archive=0;presenting=false;}
};
inline Bridge&bridge(){static thread_local Bridge value;return value;}
inline bool exactForm(nfs::GuestCopyTransaction&tx,uint32_t form) {
 return form==uint32_t(guest_address(dword_4D2C64)) && tx.load<uint16_t>(form+8)==12 &&
  tx.load<uint32_t>(form+12)==uint32_t(guest_address(dword_4D2C3C)) &&
  tx.load<uint32_t>(form+16)==uint32_t(guest_address(dword_4D2C28)) &&
  tx.load<uint32_t>(form+20)==uint32_t(guest_address(dword_4D2C50)) &&
  tx.load<uint32_t>(form+24)==0;
}

inline void observeText(const Snapshot&)noexcept;
inline void receive(const Snapshot&event) {
 observeText(event);
 auto&b=bridge();
 if(event.event==Event::Enter){
  b.retire();b.game=nullptr;b.form=0;
  if(!event.game || !event.form || event.form!=b.configuration.expectedForm)return;
  {nfs::GuestCopyTransaction tx;if(!exactForm(tx,event.form))return;}
  b.game=event.game;b.form=event.form;return;
 }
 if(b.game!=event.game || b.form!=event.form)return;
 if(event.event==Event::Exit){b.retire();b.game=nullptr;b.form=0;return;}
 if(event.event==Event::ArchiveBefore){b.retire();return;}
 if(event.event==Event::ArchiveAfter){
  b.retire();if(!event.archive)return;
  uint32_t bytes=0;
  {nfs::GuestCopyTransaction tx;
   if(tx.load<uint32_t>(dword_4D40B8)!=event.archive ||
      tx.load<uint32_t>(event.archive)!=0x49504853u)return;
   bytes=tx.load<uint32_t>(event.archive+4);
  }
  if(bytes<16 || bytes>1024u*1024u || uint64_t(event.archive)+bytes>UINT64_C(0x100000000))return;
  // Allocate outside the guest lock, copy while the exact archive is still current,
  // then release the lock before hashing, disk I/O or decompression.
  std::vector<unsigned char> source(bytes);
  {nfs::GuestCopyTransaction tx;
   if(tx.load<uint32_t>(dword_4D40B8)!=event.archive ||
      tx.load<uint32_t>(event.archive+4)!=bytes)return;
   std::memcpy(source.data(),nfs_host_address(event.archive,bytes),bytes);
  }
  const auto&c=b.configuration;
  if(b.session.select(source.data(),source.size(),c.sources,c.sourceCount,
                      c.images,c.imageCount,c.path.c_str(),c.preview,c.footers))b.archive=event.archive;
  return;
 }
 if(event.event==Event::PresentBefore){
  nfs::GuestCopyTransaction tx;
  b.presenting=b.archive && b.archive==event.archive &&
   tx.load<uint32_t>(dword_4D40B8)==b.archive && exactForm(tx,b.form);
 }
 if(event.event==Event::PresentAfter)b.presenting=false;
}
inline bool configure(Configuration value)noexcept {
 try {
  const uint32_t originalForm=uint32_t(guest_address(dword_4D2C64));
  if(!value.expectedForm)value.expectedForm=originalForm;
  if(value.expectedForm!=originalForm || !value.sources || !value.images ||
     value.sourceCount!=98 || value.imageCount!=98 || value.path.empty())return false;
  auto&b=bridge();b.retire();b.game=nullptr;b.form=0;
  b.configuration=std::move(value);install(receive);return true;
 }catch(...){return false;}
}
inline void configureOnEntry(Game&g)noexcept {
 Preserve preserve(g);
#ifdef NFS_SLIDESHOW_PRIVATE_CATALOG_AVAILABLE
 try {
  auto value=nfs_slideshow::startup();if(value.path.empty())return;
  auto&b=bridge();if(b.configuration.path==value.path&&b.configuration.preview==value.preview&&b.configuration.footers==value.footers&&sink()==receive)return;
  Configuration c;c.path=std::move(value.path);c.preview=value.preview;c.footers=std::move(value.footers);
  c.sources=nfs_slideshow_private::sources;c.sourceCount=sizeof(nfs_slideshow_private::sources)/sizeof(*c.sources);
  c.images=nfs_slideshow_private::images;c.imageCount=sizeof(nfs_slideshow_private::images)/sizeof(*c.images);
  (void)configure(std::move(c));
 }catch(...){}
#endif
}
inline std::shared_ptr<const nfs_slideshow::ImageFrame> presentationFrame()noexcept {
 // The original slideshow redraws its controls only on changes, while its
 // idle/cursor loop can present the same complete scene again. Keep admission
 // bound to the actual owner lifetime; every lease still proves the current
 // form, archive, target plane and full original native draw independently.
 auto&b=bridge();return b.game&&b.form&&b.archive?b.session.frame():nullptr;
}
} // namespace nfs_slideshow_live

#include "SlideshowTextCapture.inc"
