// SPDX-License-Identifier: MIT
#include "hd/SlideshowSession.h"
#include <cassert>
#ifdef NFS_TEST_DEFAULT_SESSION_LINK
// Existing users which do not opt in must link without provider symbols.
int main(){nfs_slideshow::Session s;assert(!s.select(nullptr,0,nullptr,0,nullptr,0,nullptr));s.close();assert(!s.frame());}
#else
#include <cstdio>
#include <fstream>
#include <iostream>
using namespace nfs_slideshow_panorama;
static Digest sha(const std::vector<unsigned char>&v){Digest d{};assert(nfs_texture_sha256(v.data(),v.size(),d.data()));return d;}
static void le32(std::vector<unsigned char>&v,size_t at,uint32_t n){for(unsigned k=0;k<4;++k)v[at+k]=static_cast<unsigned char>(n>>(8*k));}
static std::vector<unsigned char>pixels(std::initializer_list<unsigned char>values){std::vector<unsigned char>v;for(auto x:values){v.insert(v.end(),{x,x,x,255});}return v;}
static std::vector<unsigned char>zip(const std::vector<unsigned char>&v){uLongf n=compressBound(v.size());std::vector<unsigned char>out(n);assert(compress2(out.data(),&n,v.data(),v.size(),9)==Z_OK);out.resize(n);return out;}
int main(int argc,char**argv){
    assert(argc==2);const char*path=argv[1];
    struct Remove{const char*path;~Remove(){std::remove(path);}}remove{path};
    auto center=pixels({10,20,30,40,50,60,70,80,90,100,110,120});
    auto sides=pixels({1,2,3,4,5,6});
    auto full=pixels({1,10,20,30,40,4,2,50,60,70,80,5,3,90,100,110,120,6});
    // Synthetic original identity uses the actual bounded640x480 source protocol.
    std::vector<unsigned char>original(16+16+640*480*2,0);
    le32(original,0,0x49504853);le32(original,4,original.size());
    le32(original,16,120);le32(original,20,640u|(480u<<16));
    nfs_slideshow::SourcePin source{};
    source.archiveBytes=original.size();source.entryOffset=16;source.entryBytes=original.size()-16;
    source.format=120;source.width=640;source.height=480;source.archiveSha=sha(original);
    std::vector<unsigned char>entryBytes(original.begin()+16,original.end());source.entrySha=sha(entryBytes);
    const auto compressed=zip(center);
    std::vector<unsigned char>pack={'N','F','S','S','L','D','R','2',2,0,0,0,98,0,0,0};
    pack.insert(pack.end(),compressed.begin(),compressed.end());
    {std::ofstream f(path,std::ios::binary|std::ios::trunc);assert(f);f.write(reinterpret_cast<const char*>(pack.data()),pack.size());assert(f.good());}
    nfs_slideshow::RuntimeImagePin pin{};pin.fileBytes=pack.size();pin.offset=16;pin.bytes=compressed.size();
    pin.sourceSha=source.archiveSha;pin.entrySha=source.entrySha;pin.chunkSha=sha(compressed);
    pin.pixels.width=4;pin.pixels.height=3;pin.pixels.rgbaSha=sha(center);pin.accepted=true;
    auto footer=nfs_slideshow_footer::FooterImage::create(4,3,center,sha(center));assert(footer);
    auto footers=nfs_slideshow_footer::Catalog::create({footer},{{source.archiveSha,source.entrySha,0}});assert(footers);
    Entry panorama;panorama.key={source.archiveSha,source.entrySha,sha(center),footer->sha()};
    panorama.width=6;panorama.height=3;panorama.centerWidth=4;panorama.sideSha=sha(sides);
    panorama.composedSha=sha(full);panorama.reviewed=true;panorama.compressedSides=zip(sides);
    Registry registry;assert(registry.install({panorama}));
    nfs_slideshow::Session session;
    auto select=[&](bool preview=false){return session.select(original.data(),original.size(),&source,1,&pin,1,path,preview,preview?nullptr:footers);};
    assert(select());auto base=session.frame();assert(base&&!base->panorama);
    assert(session.attachPanorama(registry));auto first=session.frame();
    assert(first!=base&&first->epoch==base->epoch&&first->panorama&&first->panorama->rgba()==full);
    assert(first->rgba==base->rgba&&first->originalWords==base->originalWords&&first->footer==base->footer);
    auto other=panorama;other.key.footerRgbaSha[0]^=1;assert(registry.install({other}));
    assert(!first->panorama->current());assert(!session.attachPanorama(registry));
    assert(session.frame()==base&&!session.frame()->panorama&&session.frame()->rgba==center);
    assert(registry.install({panorama}));assert(session.attachPanorama(registry));auto second=session.frame();
    assert(select());assert(session.frame()!=second&&session.frame()->epoch!=second->epoch&&!session.frame()->panorama);
    assert(session.attachPanorama(registry));auto third=session.frame();session.close();
    assert(!session.frame()&&!session.attachPanorama(registry));assert(third->rgba==center);
    // Preview lacks a separately authenticated footer and cannot attach.
    assert(select(true));base=session.frame();assert(!session.attachPanorama(registry)&&session.frame()==base);
    // Refused new source clears both base and attachment, never old-car fallback.
    original.back()^=1;assert(!select()&&!session.frame()&&!session.attachPanorama(registry));
    std::cout<<"slideshow panorama session: exact attachment, fallback removal and selection lifetime passed\n";
}
#endif
