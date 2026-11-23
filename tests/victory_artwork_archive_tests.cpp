// SPDX-License-Identifier: MIT
// Fixture-driven archive integration check. Inputs are generated privately by
// victory_image_pack.py; no original game data is required by this program.
#include "hd/ClassicScreens/VictoryArtworkArchive.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cstdio>
using namespace nfs_victory_art;
static void check(bool value,const char*message){if(!value){std::cerr<<message<<"\n";std::exit(1);}}
static Digest hex(const char*s){Digest d{};check(s&&std::strlen(s)==64,"digest length");for(unsigned i=0;i<32;++i){char pair[]={s[2*i],s[2*i+1],0};char*end=nullptr;unsigned long v=std::strtoul(pair,&end,16);check(end==pair+2&&v<=255,"digest encoding");d[i]=unsigned(v);}return d;}
static std::vector<unsigned char>read(const char*path){std::ifstream f(path,std::ios::binary);check(bool(f),"fixture open");return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
static void write(const char*path,const std::vector<unsigned char>&bytes){std::ofstream f(path,std::ios::binary|std::ios::trunc);check(bool(f)&&bool(f.write(reinterpret_cast<const char*>(bytes.data()),bytes.size())),"fixture write");}
static ArchivePin repin(const std::vector<unsigned char>&bytes){ArchivePin p;p.fileBytes=bytes.size();check(nfs_texture_sha256(bytes.data(),4120,p.metadataSha.data()),"fixture metadata hash");return p;}
int main(int argc,char**argv){
 check(argc==8,"Expected preview path/size/metadataSHA, complete-fixture path/size/metadataSHA, scratch path");
 ArchivePin previewPin;previewPin.fileBytes=std::strtoull(argv[2],nullptr,10);previewPin.metadataSha=hex(argv[3]);
 ArchivePin fullPin;fullPin.fileBytes=std::strtoull(argv[5],nullptr,10);fullPin.metadataSha=hex(argv[6]);
 check(!Archive::open(argv[1],previewPin),"production admitted preview");auto preview=Archive::open(argv[1],previewPin,true);check(preview&&preview->count()==16&&preview->draft(),"preview open");
 auto full=Archive::open(argv[4],fullPin);check(full&&full->count()==32&&!full->draft(),"complete two-aspect open");
 unsigned images=0;size_t largest=0;
 for(auto a:{preview,full})for(unsigned aspect=0;aspect<2;++aspect)for(unsigned index=0;index<16;++index){
  const auto*e=a->entry(aspect,index);if(!e)continue;std::vector<unsigned char>pixels;check(a->load(aspect,index,e->sourceSha,pixels),"image decode/digest");check(pixels.size()==size_t(e->width)*e->height*4,"image size");largest=std::max(largest,pixels.size());++images;
 }
 std::vector<unsigned char>sentinel={9,7,5};Digest wrong=full->entry(0,0)->sourceSha;wrong[0]^=1;check(!full->load(0,0,wrong,sentinel)&&sentinel==std::vector<unsigned char>({9,7,5}),"source mismatch modified output");
 check(!full->entry(2,0)&&!full->entry(0,16),"out-of-range scene");check(!preview->load(1,0,preview->entry(0,0)->sourceSha,sentinel),"missing aspect");
 auto badPin=fullPin;badPin.metadataSha[0]^=1;check(!Archive::open(argv[4],badPin),"metadata digest mismatch");badPin=fullPin;++badPin.fileBytes;check(!Archive::open(argv[4],badPin),"file-size mismatch");
 const auto original=read(argv[4]);check(original.size()==fullPin.fileBytes&&original.size()<32768,"bounded synthetic fixture");const auto*wide=full->entry(1,0);nfs_slideshow::PixelPin wp{wide->width,wide->height,wide->rgbaSha,2};
 check(!nfs_slideshow::inflatePixels(original.data()+wide->offset,wide->bytes,wp,sentinel),"legacy slideshow accepted widescreen");check(nfs_slideshow::inflateCanvasPixels(original.data()+wide->offset,wide->bytes,wp,sentinel),"adaptive decoder rejected widescreen");
 {std::ifstream existing(argv[7]);check(!existing,"scratch already exists");}
 unsigned negatives=0;
 auto reject=[&](std::vector<unsigned char>bad){write(argv[7],bad);check(!Archive::open(argv[7],repin(bad),true),"malformed pinned metadata admitted");++negatives;};
 auto bad=original;bad[24+24]^=1;reject(bad); // noncontiguous first chunk
 bad=original;bad[24+8]^=1;reject(bad); // wrong 4:3 dimensions
 bad=original;bad[24+16]=3;reject(bad); // unsupported predictor
 bad=original;std::memset(bad.data()+24+96,0,32);reject(bad); // absent original identity
 bad=original;bad[24+16*128+96]^=1;reject(bad); // inconsistent source across aspects
 bad=original;bad[24+4]=1;reject(bad); // duplicate image index
 bad=original;bad[24+14]=1;reject(bad); // excessive height
 bad=original;bad[24+20]=0;bad[24+21]=0;bad[24+22]=0;bad[24+23]=0;reject(bad); // empty chunk
 bad=original;const auto*first=full->entry(0,0);bad[first->offset+first->bytes-1]^=1;write(argv[7],bad);auto damaged=Archive::open(argv[7],fullPin);check(bool(damaged),"valid metadata refused due to independent chunk damage");sentinel={9,7,5};check(!damaged->load(0,0,first->sourceSha,sentinel)&&sentinel==std::vector<unsigned char>({9,7,5}),"damaged chunk changed output");check(damaged->load(0,1,full->entry(0,1)->sourceSha,sentinel),"unrelated chunk could not decode");++negatives;
 check(std::remove(argv[7])==0,"scratch cleanup");check(images==48,"fixture image count");std::cout<<"{\"decodedImages\":"<<images<<",\"malformedCases\":"<<negatives<<",\"largestDecodedBytes\":"<<largest<<",\"legacyAspectPreserved\":true,\"draftProductionRejected\":true}\n";
}
