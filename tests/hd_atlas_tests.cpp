// SPDX-License-Identifier: MIT
#include "HdAtlas.h"
#include <vector>
#undef NDEBUG
#include <cassert>
#include <cstdio>
static void word(std::vector<unsigned char> &v,unsigned at,uint32_t n){for(unsigned i=0;i<4;++i)v[at+i]=(n>>(i*8))&255;}
static std::vector<unsigned char> pack(unsigned old_size=2,unsigned new_size=4){
    std::vector<unsigned char> v(32+old_size*old_size*2+new_size*new_size*4,0);
    memcpy(v.data(),"NFSHDA1\0",8);word(v,8,old_size);word(v,12,new_size);
    word(v,16,old_size*old_size*2);word(v,20,new_size*new_size*4);
    for(unsigned i=32;i<v.size();++i)v[i]=i&255;
    return v;
}
static bool read(NfsHdAtlas &atlas,const std::vector<unsigned char> &v){
    FILE *f=tmpfile();assert(f);assert(fwrite(v.data(),1,v.size(),f)==v.size());rewind(f);
    const bool result=nfs_hd_atlas_read(&atlas,f);fclose(f);return result;
}
int main(){
    NfsHdAtlas a{};auto valid=pack();assert(read(a,valid));
    assert(a.original_size==2 && a.replacement_size==4);
    assert(nfs_hd_atlas_matches(&a,2,valid.data()+32));
    valid[32]^=1;assert(!nfs_hd_atlas_matches(&a,2,valid.data()+32));valid[32]^=1;
    assert(!nfs_hd_atlas_matches(&a,1,valid.data()+32) && !nfs_hd_atlas_matches(&a,2,nullptr));
    assert(memcmp(a.rgba,valid.data()+40,64)==0); // exact straight alpha and RGB preserved
    for(unsigned length=0;length<valid.size();++length){
        auto short_file=valid;short_file.resize(length);assert(!read(a,short_file));assert(!a.original && !a.rgba);
    }
    auto extra=valid;extra.push_back(0);assert(!read(a,extra));
    for(unsigned at:{0u,7u,8u,12u,16u,20u,24u,28u}){
        auto bad=valid;bad[at]^=128;assert(!read(a,bad));assert(!a.original && !a.rgba);
    }
    for(auto dims:std::vector<std::pair<unsigned,unsigned>>{{0,4},{3,4},{512,1024},{2,2},{2,32},{256,4096}}){
        auto bad=valid;word(bad,8,dims.first);word(bad,12,dims.second);assert(!read(a,bad));
    }
    assert(read(a,valid));assert(!nfs_hd_atlas_read(&a,nullptr));assert(!a.original && !a.rgba);
    auto largest=pack(256,2048);assert(read(a,largest));nfs_hd_atlas_free(&a);nfs_hd_atlas_free(&a);
    puts("PASS: bounded HD atlas parsing, full-byte identity, alpha, malformed/truncated/trailing rejection and ownership");
}
