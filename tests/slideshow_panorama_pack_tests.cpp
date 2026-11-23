// SPDX-License-Identifier: MIT
#include "hd/SlideshowPanoramaPack.h"
#include "TextureDigest.h"
#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>
#include <zlib.h>
using namespace nfs_slideshow_panorama;
static Digest sha(const std::vector<unsigned char>&v){Digest d{};assert(nfs_texture_sha256(v.data(),v.size(),d.data()));return d;}
static void put(std::vector<unsigned char>&v,uint64_t n,unsigned bytes){for(unsigned i=0;i<bytes;++i)v.push_back(static_cast<unsigned char>(n>>(8*i)));}
static void set(std::vector<unsigned char>&v,size_t at,uint64_t n,unsigned bytes){for(unsigned i=0;i<bytes;++i)v[at+i]=static_cast<unsigned char>(n>>(8*i));}
static bool load(const std::vector<unsigned char>&v,Registry&r){std::istringstream s(std::string(v.begin(),v.end()),std::ios::binary);return loadPack(s,r);}
int main(int argc,char**argv){
 assert(argc==2);std::vector<unsigned char>center(4*3*4,255),sides(2*3*4,255),full(6*3*4,255);
 Entry e;e.width=6;e.height=3;e.centerWidth=4;e.reviewed=true;e.key.archiveSha[0]=1;e.key.entrySha[0]=2;e.key.footerRgbaSha[0]=3;e.key.centerRgbaSha=sha(center);e.sideSha=sha(sides);e.composedSha=sha(full);
 uLongf n=compressBound(sides.size());e.compressedSides.resize(n);assert(compress2(e.compressedSides.data(),&n,sides.data(),sides.size(),9)==Z_OK);e.compressedSides.resize(n);
 std::vector<unsigned char>pack={'N','F','S','S','P','A','N',0};put(pack,1,4);put(pack,1,4);put(pack,216,4);put(pack,0,4);put(pack,n,8);
 for(const Digest*d:{&e.key.archiveSha,&e.key.entrySha,&e.key.centerRgbaSha,&e.key.footerRgbaSha,&e.sideSha,&e.composedSha})pack.insert(pack.end(),d->begin(),d->end());
 put(pack,6,4);put(pack,3,4);put(pack,4,4);put(pack,1,4);put(pack,n,8);pack.insert(pack.end(),e.compressedSides.begin(),e.compressedSides.end());
 Registry r;assert(load(pack,r));auto live=r.acquire(e.key,4,3,center);assert(live&&live->rgba()==full);
 for(size_t k=0;k<pack.size();++k){auto bad=pack;bad.resize(k);assert(!load(bad,r)&&live->current());}
 for(unsigned f=0;f<12;++f){auto bad=pack;switch(f){
 case 0:bad[0]^=1;break;case 1:set(bad,8,2,4);break;case 2:set(bad,12,0,4);break;case 3:set(bad,12,99,4);break;case 4:set(bad,16,215,4);break;case 5:set(bad,20,1,4);break;
 case 6:set(bad,24,Registry::MaximumCompressedBytes+1,8);break;case 7:set(bad,240,Registry::MaximumCompressedEntryBytes+1,8);break;case 8:set(bad,236,0,4);break;case 9:set(bad,224,5,4);break;case 10:bad.push_back(0);break;case 11:set(bad,24,n+1,8);break;}
 assert(!load(bad,r)&&live->current());}
 auto duplicate=pack;duplicate.insert(duplicate.begin()+248,pack.begin()+32,pack.begin()+248);duplicate.insert(duplicate.end(),e.compressedSides.begin(),e.compressedSides.end());set(duplicate,12,2,4);set(duplicate,24,2*n,8);assert(!load(duplicate,r)&&live->current());
 auto badStream=pack;badStream.back()^=1;assert(load(badStream,r)&&!live->current());assert(!r.acquire(e.key,4,3,center)); // no startup inflation
 assert(load(pack,r));live=r.acquire(e.key,4,3,center);assert(live);
 {std::ofstream out(argv[1],std::ios::binary);out.write(reinterpret_cast<const char*>(pack.data()),pack.size());assert(out);}
 const auto pin=sha(pack);auto wrong=pin;wrong[0]^=1;
 assert(!loadPack(argv[1],wrong,pack.size(),r)&&live->current());assert(!loadPack(argv[1],pin,pack.size()+1,r)&&live->current());assert(!loadPack(argv[1],Digest{},pack.size(),r)&&live->current());
 assert(loadPack(argv[1],pin,pack.size(),r)&&!live->current());assert(r.acquire(e.key,4,3,center));
 assert(!loadPack(std::string(argv[1])+".missing",r));
 std::cout<<"panorama pack: bounds, truncation, exact EOF, duplicate routing, lazy streams and file pins passed\n";
}
