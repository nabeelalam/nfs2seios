// SPDX-License-Identifier: MIT
#ifndef NFS_TEXTURE_DIGEST_H
#define NFS_TEXTURE_DIGEST_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* SHA-256, FIPS180-4. Content identification only; no security protocol API. */
static uint32_t nfs_digest_rotr(uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));}
static void nfs_digest_block(uint32_t h[8],const unsigned char data[64]){
    static const uint32_t k[64]={
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
    uint32_t w[64];
    for(unsigned i=0;i<16;++i)w[i]=((uint32_t)data[4*i]<<24)|((uint32_t)data[4*i+1]<<16)|((uint32_t)data[4*i+2]<<8)|data[4*i+3];
    for(unsigned i=16;i<64;++i){uint32_t x=w[i-15],y=w[i-2];w[i]=w[i-16]+(nfs_digest_rotr(x,7)^nfs_digest_rotr(x,18)^(x>>3))+w[i-7]+(nfs_digest_rotr(y,17)^nfs_digest_rotr(y,19)^(y>>10));}
    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],z=h[7];
    for(unsigned i=0;i<64;++i){
        uint32_t t1=z+(nfs_digest_rotr(e,6)^nfs_digest_rotr(e,11)^nfs_digest_rotr(e,25))+((e&f)^((~e)&g))+k[i]+w[i];
        uint32_t t2=(nfs_digest_rotr(a,2)^nfs_digest_rotr(a,13)^nfs_digest_rotr(a,22))+((a&b)^(a&c)^(b&c));
        z=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
    h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=z;
}
static int nfs_texture_sha256(const void *input,size_t bytes,unsigned char output[32]){
    if(!output || (!input && bytes) || bytes>UINT64_MAX/8)return 0;
    uint32_t h[8]={0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    const unsigned char *data=(const unsigned char *)input;size_t complete=bytes/64;
    for(size_t i=0;i<complete;++i)nfs_digest_block(h,data+64*i);
    unsigned char tail[128]={0};size_t rest=bytes%64;if(rest)memcpy(tail,data+64*complete,rest);tail[rest]=128;
    size_t length=rest<56?64:128;uint64_t bits=(uint64_t)bytes*8;
    for(unsigned i=0;i<8;++i)tail[length-1-i]=(unsigned char)(bits>>(8*i));
    nfs_digest_block(h,tail);if(length==128)nfs_digest_block(h,tail+64);
    for(unsigned i=0;i<8;++i)for(unsigned j=0;j<4;++j)output[4*i+j]=(unsigned char)(h[i]>>(24-8*j));
    return 1;
}
#endif
