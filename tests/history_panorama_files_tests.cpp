// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/HistoryWideFiles.h"
#include <cassert>
#include <iterator>
static uint64_t base=0;
static uint64_t generation(){return base;}
static std::array<unsigned char,32>pin(const char*text){std::string s(text);assert(s.size()==64);std::array<unsigned char,32>out{};auto hex=[](char c)->unsigned{if(c>='0'&&c<='9')return unsigned(c-'0');if(c>='a'&&c<='f')return unsigned(c-'a'+10);if(c>='A'&&c<='F')return unsigned(c-'A'+10);assert(false);return 0;};for(unsigned k=0;k<32;++k)out[k]=static_cast<unsigned char>((hex(s[2*k])<<4)|hex(s[2*k+1]));return out;}
int main(int argc,char**argv){assert(argc==5);using namespace nfs_history_wide;Registry registry;Files files(generation,registry);auto zeroPin=pin(argv[2]),sevenPin=pin(argv[4]);std::string missing=std::string(argv[3])+".missing";
 assert(files.configure(0,argv[1],zeroPin));assert(!files.configure(0,argv[1],zeroPin));assert(!files.configure(1,argv[1],zeroPin));assert(!files.configure(7,argv[3],{}));assert(files.configure(7,missing,sevenPin));assert(files.configure(14,argv[1],zeroPin));auto wrongPin=zeroPin;wrongPin[0]^=1;assert(files.configure(21,argv[1],wrongPin));
 base=7;auto art=files.acquire(0,7);assert(art);Artwork::View view{};assert(art->view(7,0,view)&&view.width==128&&view.height==72);for(unsigned y=0;y<72;++y)for(unsigned x=0;x<128;++x){auto*p=view.pixels+4*(y*128+x);assert(p[0]==x%251&&p[1]==y%251&&p[2]==(x^y)%251&&p[3]==255);}
 assert(files.acquire(0,7)==art);assert(!files.configure(28,argv[1],zeroPin));assert(!files.acquire(7,7));assert(!files.acquire(14,7));assert(!files.acquire(21,7));
 {std::ifstream check(missing,std::ios::binary);assert(!check.good());std::ifstream input(argv[3],std::ios::binary);std::vector<char>bytes((std::istreambuf_iterator<char>(input)),{});assert(!bytes.empty());std::ofstream output(missing,std::ios::binary);output.write(bytes.data(),bytes.size());assert(output.good());}
 assert(!files.acquire(7,7)); // Failure is attempted once per base generation.
 base=8;registry.clear();auto seven=files.acquire(7,8);assert(seven&&seven->view(8,7,view));assert(!files.acquire(0,7));auto renewed=files.acquire(0,8);assert(renewed&&renewed!=art);assert(!registry.current(art,7,0));assert(art->view(7,0,view)&&view.pixels[3]==255);
 base=9;assert(!registry.install(renewed,8,0,generation));assert(!files.acquire(0,8));base=0;registry.clear();assert(!files.acquire(0,8));assert(!registry.current(renewed,8,0));
}
