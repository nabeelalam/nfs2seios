// SPDX-License-Identifier: MIT
#pragma once
#include "AudioDialogRecords.h"
namespace nfs_audio_frame {
class BackgroundCatalog final {
 std::vector<uint8_t>source_;BackgroundCatalog()=default;
public:
 static std::shared_ptr<const BackgroundCatalog>create(const std::array<unsigned char,32>&expected,const std::vector<uint8_t>&source)noexcept{if(source.size()!=614416||!nonzero(expected)||nfs_small_logo_live::le32(source.data())!=0x78||nfs_small_logo_live::le32(source.data()+4)!=(640u|(480u<<16))||nfs_small_logo_live::le32(source.data()+8)||nfs_small_logo_live::le32(source.data()+12))return {};try{auto p=std::shared_ptr<BackgroundCatalog>(new BackgroundCatalog);p->source_=source;if(!nfs_small_logo_live::hashMatches(p->source_.data(),614416,expected))return {};return p;}catch(...){return {};}}
 const std::vector<uint8_t>&source()const noexcept{return source_;}
};
class BackgroundReplayVerifier;
class VerifiedPreBars final {
 Binding binding_;std::shared_ptr<const BackgroundCatalog>catalog_;std::vector<uint16_t>pixels_;VerifiedPreBars()=default;friend class BackgroundReplayVerifier;
public:
 const Binding&binding()const noexcept{return binding_;}const std::vector<uint16_t>&pixels()const noexcept{return pixels_;}
};
}
