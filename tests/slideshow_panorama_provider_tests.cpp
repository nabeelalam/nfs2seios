// SPDX-License-Identifier: MIT
#include "hd/SlideshowPanoramaProvider.h"
#include "TextureDigest.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <zlib.h>
using namespace nfs_slideshow_panorama;
static Digest sha(const std::vector<unsigned char>&v){Digest d{};assert(nfs_texture_sha256(v.data(),v.size(),d.data()));return d;}
static std::vector<unsigned char>pixels(std::initializer_list<unsigned char>values){
    std::vector<unsigned char>v;for(auto x:values){v.push_back(x);v.push_back(x+1);v.push_back(x+2);v.push_back(255);}return v;
}
static std::vector<unsigned char>zip(const std::vector<unsigned char>&v){
    uLongf n=compressBound(v.size());std::vector<unsigned char>out(n);
    assert(compress2(out.data(),&n,v.data(),v.size(),9)==Z_OK);out.resize(n);return out;
}
struct Fixture {
    // Independent hand-labelled rows: L | four protected pixels | R.
    std::vector<unsigned char>center=pixels({10,20,30,40,50,60,70,80,90,100,110,120});
    std::vector<unsigned char>sides=pixels({1,2,3,4,5,6});
    std::vector<unsigned char>full=pixels({1,10,20,30,40,4,2,50,60,70,80,5,3,90,100,110,120,6});
    Entry entry;
    Fixture(){entry.key.archiveSha[0]=1;entry.key.entrySha[0]=2;
        entry.key.centerRgbaSha=sha(center);entry.key.footerRgbaSha[0]=3;
        entry.width=6;entry.height=3;entry.centerWidth=4;entry.reviewed=true;
        entry.sideSha=sha(sides);entry.composedSha=sha(full);entry.compressedSides=zip(sides);}
};
int main(){
    Fixture f;Registry registry;
    assert(registry.install({f.entry}));
    auto first=registry.acquire(f.entry.key,4,3,f.center);
    assert(first&&first->current()&&first->rgba()==f.full&&first->centerLeft()==1);
    assert(first->width()==6&&first->height()==3&&first->centerWidth()==4&&first->sha()==sha(f.full));
    // Direct center byte check, including every alpha byte, independent of replay.
    for(size_t row=0;row<3;++row)for(size_t byte=0;byte<16;++byte)
        assert(first->rgba()[row*24+4+byte]==f.center[row*16+byte]);
    bool remoteCurrent=false;std::thread reader([&]{remoteCurrent=first->current();});reader.join();assert(remoteCurrent);
    for(unsigned field=0;field<4;++field){auto key=f.entry.key;
        Digest*parts[]={&key.archiveSha,&key.entrySha,&key.centerRgbaSha,&key.footerRgbaSha};
        (*parts[field])[0]^=128;assert(!registry.acquire(key,4,3,f.center));}
    auto changed=f.center;changed[0]^=1;assert(!registry.acquire(f.entry.key,4,3,changed));
    changed=f.center;changed[3]=254;assert(!registry.acquire(f.entry.key,4,3,changed));
    assert(!registry.acquire(f.entry.key,3,4,f.center));
    changed=f.center;changed.pop_back();assert(!registry.acquire(f.entry.key,4,3,changed));
    // Invalid metadata never replaces a live generation.
    for(unsigned fault=0;fault<9;++fault){auto e=f.entry;
        switch(fault){case 0:e.reviewed=false;break;case 1:e.width=5;break;
        case 2:e.width=0xffffffffu;break;case 3:e.height=0;break;
        case 4:e.centerWidth=e.width;break;case 5:e.centerWidth=2;break;
        case 6:e.key.footerRgbaSha={};break;case 7:e.compressedSides.clear();break;
        case 8:e.composedSha={};break;}
        assert(!registry.install({e}));assert(first->current());}
    assert(!registry.install({f.entry,f.entry}));assert(first->current());
    assert(!registry.install(std::vector<Entry>(99,f.entry)));assert(first->current());
    // Unselected entries are never inflated. Bad streams reject only acquisition.
    auto bad=f.entry;bad.key.entrySha[0]=9;bad.compressedSides={1,2,3};
    assert(registry.install({f.entry,bad}));assert(!first->current());
    auto second=registry.acquire(f.entry.key,4,3,f.center);assert(second&&second->generation()>first->generation());
    assert(!registry.acquire(bad.key,4,3,f.center));
    for(unsigned fault=0;fault<8;++fault){auto e=f.entry;
        switch(fault){case 0:e.compressedSides.push_back(0);break;
        case 1:e.compressedSides.pop_back();break;
        case 2:e.compressedSides.insert(e.compressedSides.end(),f.entry.compressedSides.begin(),f.entry.compressedSides.end());break;
        case 3:e.sideSha[0]^=1;break;case 4:e.composedSha[0]^=1;break;
        case 5:{auto v=f.sides;v.push_back(1);e.compressedSides=zip(v);break;}
        case 6:{auto v=f.sides;v[3]=254;e.sideSha=sha(v);e.compressedSides=zip(v);break;}
        case 7:{auto v=f.sides;v[0]^=1;e.compressedSides=zip(v);break;}}
        assert(registry.install({e}));assert(!registry.acquire(e.key,4,3,f.center));}
    assert(!second->current());assert(registry.install({f.entry}));
    auto last=registry.acquire(f.entry.key,4,3,f.center);assert(last);
    registry.clear();assert(!last->current()&&!registry.acquire(f.entry.key,4,3,f.center));
    assert(registry.install({f.entry}));auto newer=registry.acquire(f.entry.key,4,3,f.center);
    assert(newer&&newer->generation()>last->generation());
    std::shared_ptr<const Panorama>orphan;
    {Registry temporary;assert(temporary.install({f.entry}));orphan=temporary.acquire(f.entry.key,4,3,f.center);assert(orphan&&orphan->current());}
    assert(!orphan->current());
    // Caller changes to compressed metadata cannot mutate the installed copy.
    f.entry.compressedSides.assign(1,0);assert(newer->current()&&newer->rgba()==f.full);
    assert(registry.acquire(f.entry.key,4,3,f.center));
    // Alpha rejection remains independent of a matching supplied center hash.
    Fixture transparent;transparent.center[3]=0;
    transparent.entry.key.centerRgbaSha=sha(transparent.center);
    assert(registry.install({transparent.entry}));
    assert(!registry.acquire(transparent.entry.key,4,3,transparent.center));
    std::cout<<"slideshow panorama provider: selected replay, routing, bounds, streams and lifetime checks passed\n";
}
