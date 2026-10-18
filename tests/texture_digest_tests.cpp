// SPDX-License-Identifier: MIT
#include "TextureDigest.h"
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
static void check(const void *data,size_t size,const char *expected){
    unsigned char result[34];memset(result,0xa5,sizeof(result));assert(nfs_texture_sha256(data,size,result+1));
    assert(result[0]==0xa5 && result[33]==0xa5);char text[65];
    for(unsigned i=0;i<32;++i)std::sprintf(text+i*2,"%02x",result[i+1]);assert(!strcmp(text,expected));
}
int main(int argc,char **argv){
    if(argc==2){FILE *f=fopen(argv[1],"rb");assert(f);std::vector<unsigned char> data;int c;while((c=fgetc(f))!=EOF)data.push_back((unsigned char)c);fclose(f);unsigned char digest[32];assert(nfs_texture_sha256(data.data(),data.size(),digest));for(auto b:digest)printf("%02x",b);puts("");return 0;}
    check(nullptr,0,"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    check("abc",3,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    std::string multi="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    check(multi.data(),multi.size(),"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    std::string million(1000000,'a');check(million.data(),million.size(),"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    unsigned char unaligned[6]={99,'a','b','c',88,77};check(unaligned+1,3,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");assert(unaligned[0]==99 && unaligned[5]==77);
    unsigned char out[32];assert(!nfs_texture_sha256(nullptr,1,out) && !nfs_texture_sha256("a",1,nullptr));
    if(SIZE_MAX>UINT64_MAX/8)assert(!nfs_texture_sha256("a",SIZE_MAX,out));
    puts("PASS texture fingerprint known vectors, padding, large input, alignment and canaries");
}
