// SPDX-License-Identifier: MIT
#include "ArtworkPack.h"
#include <fstream>
#include <cstring>
#include <limits>
namespace nfs_hd {
namespace {
uint32_t u32(const uint8_t*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
uint64_t u64(const uint8_t*p){return u32(p)|(uint64_t(u32(p+4))<<32);}
bool nonzero(const uint8_t*p,size_t n){uint8_t v=0;for(size_t k=0;k<n;++k)v|=p[k];return v!=0;}
bool zero(const uint8_t*p,size_t n){return !nonzero(p,n);}
bool hash(const uint8_t*p,size_t n,const uint8_t*expected){uint8_t actual[32];return nfs_texture_sha256(p,n,actual)&&std::memcmp(actual,expected,32)==0;}
bool rgba(const uint8_t*p,size_t n,bool opaque){for(size_t k=0;k<n;k+=4)if(p[k]>p[k+3]||p[k+1]>p[k+3]||p[k+2]>p[k+3]||(opaque&&p[k+3]!=255))return false;return true;}
struct Section {uint32_t tag,id;const uint8_t*data;size_t size;};
bool dimensions(uint32_t w,uint32_t h,size_t n){return w&&h&&w<=4096&&h<=4096&&uint64_t(w)*h*4==n;}
size_t byteLimit(const uint8_t*header){return u32(header+8)==3&&std::memcmp(header,"NFSHDPK3",8)==0?ArtworkPack::MaxBytes:ArtworkPack::LegacyMaxBytes;}
void failure(std::string*out,const char*message)noexcept{if(out)try{*out=message;}catch(...){}}
}
std::shared_ptr<const ArtworkPack> ArtworkPack::loadFile(const std::string&path,const Digest&expected,std::string*error)noexcept{
    try{
        std::ifstream file(path,std::ios::binary|std::ios::ate);
        if(!file){failure(error,"HD artwork pack could not be opened");return {};}
        const auto end=file.tellg();
        if(end<0||uint64_t(end)>MaxBytes||uint64_t(end)<832){failure(error,"HD artwork pack size is unsupported");return {};}
        uint8_t header[12];file.seekg(0);
        if(!file.read(reinterpret_cast<char*>(header),sizeof header)||uint64_t(end)>byteLimit(header)){failure(error,"HD artwork pack size is unsupported");return {};}
        std::vector<uint8_t> bytes(static_cast<size_t>(end));file.seekg(0);
        if(!file.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(bytes.size()))||file.peek()!=std::char_traits<char>::eof()){
            failure(error,"HD artwork pack read was incomplete");return {};
        }
        return loadOwned(std::move(bytes),expected,error);
    }catch(...){failure(error,"HD artwork pack could not be loaded");return {};}
}
std::shared_ptr<const ArtworkPack> ArtworkPack::loadOwned(std::vector<uint8_t>bytes,const Digest&expected,std::string*error)noexcept{
    try{
        if(bytes.size()<832||bytes.size()>byteLimit(bytes.data())||!nonzero(expected.data(),32)||!hash(bytes.data(),bytes.size(),expected.data())){
            failure(error,"HD artwork pack does not match its trusted content pin");return {};
        }
        auto pack=std::shared_ptr<ArtworkPack>(new ArtworkPack(std::move(bytes)));pack->digest_=expected;
        if(!pack->parse()){failure(error,"HD artwork pack structure is invalid");return {};}
        if(error)error->clear();return pack;
    }catch(...){failure(error,"HD artwork pack could not be prepared");return {};}
}
bool ArtworkPack::parse(){
    const auto*b=bytes_.data();
    const uint32_t version=u32(b+8),sectionCount=u32(b+24);
    const bool legacy=version==1&&std::memcmp(b,"NFSHDPK1",8)==0;
    const bool mixed=version==3&&std::memcmp(b,"NFSHDPK3",8)==0;
    const bool extended=(version==2&&std::memcmp(b,"NFSHDPK2",8)==0)||mixed;
    if((!legacy&&!extended)||u32(b+12)!=64||u64(b+16)!=bytes_.size()||
       u32(b+28)!=32||u64(b+32)!=64||!zero(b+40,24))return false;
    if((legacy&&sectionCount!=24)||(extended&&(sectionCount<26||sectionCount>25+(mixed?NFS_NATIVE_MARK_MAX_TEXTURES:NFS_NATIVE_MARK_V2_MAX_TEXTURES))))return false;
    const uint32_t markTextures=extended?sectionCount-25:0;
    std::array<Section,25+NFS_NATIVE_MARK_MAX_TEXTURES> sections{};
    size_t at=64+32*size_t(sectionCount);if(at>bytes_.size())return false;
    for(unsigned k=0;k<sectionCount;++k){
        const auto*t=b+64+32*k;uint32_t tag,id;
        if(k==0){tag=1;id=0;}else if(k<=5){tag=2;id=k-1;}else if(k==6){tag=3;id=0;}else if(k<=18){tag=4;id=k-6;}else if(k<=23){tag=5;id=k-19;}else if(k==24){tag=6;id=0;}else{tag=7;id=k-25;}
        const auto n=u64(t+16);
        if(u32(t)!=tag||u32(t+4)!=id||u64(t+8)!=at||u64(t+24)||n>bytes_.size()-at)return false;
        sections[k]={tag,id,b+at,size_t(n)};at+=size_t(n);
    }
    if(at!=bytes_.size())return false;
    const auto&cs=sections[0];
    if(cs.size!=1616||u32(cs.data)!=0x31544143||u32(cs.data+4)!=1||u32(cs.data+8)!=20||u32(cs.data+12)!=80)return false;
    auto c=std::make_shared<nfs_background_scene_cpu::SourceCatalog>();
    for(unsigned k=0;k<20;++k){
        const auto*p=cs.data+16+80*k;uint32_t kind=k<13?1:(k<18?2:3),role=k<13?k:(k<18?k-13:k-18);
        if(u32(p)!=kind||u32(p+4)!=role)return false;
        if(kind==1){
            auto&i=c->identities[role];i.span=u32(p+8);i.width=u32(p+12);i.height=u32(p+16);i.paletteOffset=u32(p+20);
            if(u32(p+24)||u32(p+28))return false;
            std::copy_n(p+32,16,i.header.begin());std::copy_n(p+48,32,i.sha.begin());c->roles[role]=role;
        }else if(kind==2){
            auto&i=c->logos.identities[role];i.span=u32(p+8);i.width=u32(p+12);i.height=u32(p+16);i.paletteOffset=u32(p+20);i.paletteCount=u32(p+24);i.maxIndex=u32(p+28);
            for(unsigned z=0;z<4;++z)i.header[z]=u32(p+32+4*z);
            std::copy_n(p+48,32,i.sha.begin());c->logos.roles[role]=role;
        }else{
            if(!zero(p+8,40))return false;
            c->mapRoles[role]=role;std::copy_n(p+48,32,c->mapSha[role].begin());
        }
    }
    if(!nfs_background_scene_cpu::validCatalog(*c))return false;
    for(unsigned k=0;k<6;++k){
        const auto&s=sections[k+1];if(s.size<112)return false;const auto*p=s.data;const size_t n=s.size-112;
        const uint32_t width=u32(p),height=u32(p+4),opaque=u32(p+24);
        if(u32(p+8)!=1||u32(p+12)!=(k<5?k:0)||u32(p+16)!=1||u32(p+20)||opaque>1||u32(p+28)||!u64(p+32)||u64(p+40)!=n||!dimensions(width,height,n)||!hash(p+112,n,p+48)||!rgba(p+112,n,opaque!=0))return false;
        if(k<5){
            if((k<2&&opaque!=1)||(k>=2&&opaque!=0)||(k<3&&!nonzero(p+80,32))||(k>=3&&!zero(p+80,32)))return false;
            if(k<2&&std::memcmp(p+80,c->mapSha[k].data(),32))return false;
            if(k==2&&std::memcmp(p+80,c->identities[0].sha.data(),32))return false;
            auto&t=textures_[k];t.pixels=p+112;t.bytes=n;t.width=width;t.height=height;t.schema=1;t.role=k;t.premultiplied=1;t.opacityBaked=0;t.opaque=opaque;t.identity=u64(p+32);std::copy_n(p+48,32,t.sha256);std::copy_n(p+80,32,t.sourceSha256);
        }else{
            if(opaque||!zero(p+80,32))return false;
            logo_.pixels=p+112;logo_.bytes=n;logo_.width=width;logo_.height=height;logo_.schema=1;logo_.premultiplied=1;logo_.opacityBaked=0;logo_.identity=u64(p+32);std::copy_n(p+48,32,logo_.sha256);
        }
    }
    static const char names[12][5]={"nedb","neds","spdb","spds","forb","fors","bspb","bsps","bndb","bnds","bfrb","bfrs"};
    for(unsigned k=0;k<12;++k){
        const auto&s=sections[7+k];if(s.size!=112)return false;const auto*p=s.data;auto&w=words_[k];
        w.schema=u32(p);w.sourceRole=u32(p+4);w.textureRole=u32(p+8);w.x=u32(p+12);w.y=u32(p+16);w.width=u32(p+20);w.height=u32(p+24);w.scale=u32(p+28);w.logicalWidth=u32(p+32);w.logicalHeight=u32(p+36);
        if(w.schema!=1||w.sourceRole!=k+1||w.textureRole<3||w.textureRole>4||!w.scale||w.scale>16||!w.width||!w.height||w.logicalWidth!=c->identities[k+1].width||w.logicalHeight!=c->identities[k+1].height||uint64_t(w.logicalWidth)*w.scale!=w.width||uint64_t(w.logicalHeight)*w.scale!=w.height||!zero(p+45,3)||std::memcmp(p+40,names[k],5)||std::memcmp(p+48,c->identities[k+1].sha.data(),32)||!nonzero(p+80,32))return false;
        const auto&t=textures_[w.textureRole];if(uint64_t(w.x)+w.width>t.width||uint64_t(w.y)+w.height>t.height)return false;
        std::copy_n(reinterpret_cast<const char*>(p+40),5,w.entry);std::copy_n(p+48,32,w.sourceSha256);std::copy_n(p+80,32,w.authoredStraightSha256);
    }
    for(unsigned k=0;k<5;++k){
        const auto&s=sections[19+k];if(s.size!=40)return false;const auto*p=s.data;auto&t=regions_[k];
        t.role=u32(p);t.x=u32(p+4);t.y=u32(p+8);t.width=u32(p+12);t.height=u32(p+16);t.scale=u32(p+20);
        auto signed32=[](uint32_t v)->int32_t{return v<=uint32_t(INT32_MAX)?int32_t(v):int32_t(int64_t(v)-0x100000000LL);};
        t.logicalWidth=signed32(u32(p+24));t.logicalHeight=signed32(u32(p+28));t.sourceOriginX=signed32(u32(p+32));t.sourceOriginY=signed32(u32(p+36));
        auto signed16=[](uint32_t v)->int32_t{v&=65535u;return v<32768u?int32_t(v):int32_t(v)-65536;};
        if(t.sourceOriginX!=signed16(c->logos.identities[k].header[3])||t.sourceOriginY!=signed16(c->logos.identities[k].header[3]>>16))return false;
        if(t.role!=k||!t.scale||t.scale>16||t.logicalWidth!=int32_t(c->logos.identities[k].width)||t.logicalHeight!=int32_t(c->logos.identities[k].height)||uint64_t(uint32_t(t.logicalWidth))*t.scale!=t.width||uint64_t(uint32_t(t.logicalHeight))*t.scale!=t.height||uint64_t(t.x)+t.width>logo_.width||uint64_t(t.y)+t.height>logo_.height)return false;
    }
    if(extended){
        const auto&s=sections[24];if(s.size<16)return false;const auto*p=s.data;
        const uint32_t count=u32(p+4);
        if(u32(p)!=1||!count||count>NFS_NATIVE_MARK_MAX_BINDINGS||u32(p+8)!=64||u32(p+12)||
           uint64_t(s.size)!=16+uint64_t(count)*64)return false;
        for(unsigned k=0;k<count;++k){
            const auto*q=p+16+size_t(k)*64;const uint32_t texture=u32(q+16),encoding=u32(q+4),edge=u32(q+8);
            NfsNativeMarkProfile profile{};
            if(u32(q)!=1||!nfs_native_mark_profile(encoding,edge,&profile)||
               (!mixed&&(encoding!=NFS_NATIVE_MARK_SOURCE_RGBA5551_LE||edge!=NFS_NATIVE_MARK_SOURCE_EDGE))||
               u32(q+12)!=profile.preparedBytes||texture>=markTextures||
               u32(q+20)!=profile.logicalEdge||u32(q+24)!=profile.logicalEdge||u32(q+28)||!nonzero(q+32,32))return false;
            for(unsigned j=0;j<k;++j)if(std::memcmp(nativeBindings_[j].prepared.data(),q+32,32)==0&&
                (!mixed||(nativeBindings_[j].encoding==encoding&&nativeBindings_[j].edge==edge)))return false;
            auto&binding=nativeBindings_[k];binding.texture=texture;binding.encoding=encoding;binding.edge=edge;
            std::copy_n(q+32,32,binding.prepared.begin());
        }
        for(unsigned k=0;k<markTextures;++k){
            const auto&s=sections[25+k];if(s.size<112)return false;const auto*p=s.data;
            const uint32_t artEdge=u32(p);NfsNativeMarkProfile profile{};
            const bool shape=artEdge==512?nfs_native_mark_profile(NFS_NATIVE_MARK_SOURCE_RGBA5551_LE,128,&profile):
                (mixed&&artEdge==128?nfs_native_mark_profile(NFS_NATIVE_MARK_SOURCE_RGBA4444_LE,32,&profile):false);
            if(!shape||s.size!=112+profile.artBytes||u32(p+4)!=profile.artEdge||u32(p+8)!=1||u32(p+12)!=k||u32(p+16)!=1||
               u32(p+20)||u32(p+24)||u32(p+28)||!u64(p+32)||u64(p+40)!=profile.artBytes||
               !hash(p+112,profile.artBytes,p+48)||!zero(p+80,32)||!rgba(p+112,profile.artBytes,false))return false;
            for(unsigned j=0;j<k;++j)if(nativeTextures_[j].identity==u64(p+32))return false;
            auto&a=nativeTextures_[k];a.pixels=p+112;a.bytes=profile.artBytes;a.width=a.height=profile.artEdge;
            a.logicalWidth=a.logicalHeight=profile.logicalEdge;a.schema=NFS_NATIVE_MARK_ART_SCHEMA;
            a.premultiplied=1;a.opacityBaked=0;a.identity=u64(p+32);std::copy_n(p+48,32,a.sha256);
        }
        for(unsigned k=0;k<count;++k)if(nativeTextures_[nativeBindings_[k].texture].logicalWidth!=nativeBindings_[k].edge)return false;
        nativeBindingCount_=count;nativeTextureCount_=markTextures;
    }
    catalog_=std::move(c);return true;
}
NfsHdArtProvider ArtworkPack::provider()const noexcept{return {NFS_HD_ART_PROVIDER_SCHEMA,const_cast<ArtworkPack*>(this),texture,word,logo,region};}
NfsNativeMarkProvider ArtworkPack::nativeMarkProvider()const noexcept{
    if(!nativeBindingCount_||!nativeTextureCount_)return {};
    return {NFS_NATIVE_MARK_PROVIDER_SCHEMA,const_cast<ArtworkPack*>(this),nativeMark};
}
int ArtworkPack::nativeMark(void*p,uint32_t encoding,uint32_t edge,const uint8_t digest[32],NfsNativeMarkArt*out){
    NfsNativeMarkProfile profile{};
    if(!p||!out||!digest||!nfs_native_mark_profile(encoding,edge,&profile))return 0;
    const auto&pack=*static_cast<const ArtworkPack*>(p);
    for(unsigned k=0;k<pack.nativeBindingCount_;++k)if(pack.nativeBindings_[k].encoding==encoding&&pack.nativeBindings_[k].edge==edge&&
        std::memcmp(pack.nativeBindings_[k].prepared.data(),digest,32)==0){
        *out=pack.nativeTextures_[pack.nativeBindings_[k].texture];return 1;
    }
    return 0;
}
int ArtworkPack::texture(void*p,uint32_t role,NfsBackgroundSceneTexture*out){if(!p||!out||role>=5)return 0;*out=static_cast<const ArtworkPack*>(p)->textures_[role];return 1;}
int ArtworkPack::word(void*p,uint32_t role,NfsBackgroundSceneWord*out){if(!p||!out||!role||role>12)return 0;*out=static_cast<const ArtworkPack*>(p)->words_[role-1];return 1;}
int ArtworkPack::logo(void*p,NfsSmallLogoArt*out){if(!p||!out)return 0;*out=static_cast<const ArtworkPack*>(p)->logo_;return 1;}
int ArtworkPack::region(void*p,uint32_t role,NfsSmallLogoRegion*out){if(!p||!out||role>=5)return 0;*out=static_cast<const ArtworkPack*>(p)->regions_[role];return 1;}
}
