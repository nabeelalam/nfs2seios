// SPDX-License-Identifier: MIT
#include "hd/ControlsSteeringBarArtwork.h"
#include <cstdio>
#include <cstdlib>
static unsigned checks=0;
#define CHECK(x)do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line%d %s\n",__LINE__,#x);std::abort();}}while(0)
int main(){namespace bar=nfs_controls_steering_bar;CHECK(!bar::artwork(4));CHECK(!bar::artwork(UINT32_MAX));
 for(unsigned level=0;level<4;++level){auto a=bar::artwork(level);CHECK(a&&a==bar::artwork(level));const auto v=a->view();CHECK(v.schema==1&&v.level==level&&v.width==1424&&v.height==196&&v.bytes==size_t(1424)*196*4&&v.left==280&&v.top==280&&v.right==636&&v.bottom==329);CHECK(v.rgba==a->rgba.data());unsigned char sha[32];CHECK(nfs_texture_sha256(v.rgba,v.bytes,sha)&&!std::memcmp(sha,nfs_controls_steering_bar_sha256[level],32)&&!std::memcmp(sha,v.sha256,32));size_t visible=0,partial=0;for(unsigned y=0;y<196;++y)for(unsigned x=0;x<1424;++x){const auto*p=v.rgba+(size_t(y)*1424+x)*4;CHECK(p[0]<=p[3]&&p[1]<=p[3]&&p[2]<=p[3]);if(x<4||x>=1420||y<4||y>=192)CHECK(!p[3]);visible+=p[3]!=0;partial+=p[3]&&p[3]!=255;}CHECK(visible>100000&&partial>1000);
  const auto*data=nfs_controls_steering_bar_rle[level];const size_t count=nfs_controls_steering_bar_rle_count[level];CHECK(count&&count%2==0&&count<=size_t(1424)*196*2);auto decoded=bar::decode(level,data,count);CHECK(decoded&&decoded->rgba==a->rgba);CHECK(!bar::decode(level,nullptr,count)&&!bar::decode(level,data,0)&&!bar::decode(level,data,count-1)&&!bar::decode(4,data,count));
  for(unsigned mode=0;mode<5;++mode){std::vector<uint32_t>bad(data,data+count);if(mode==0)bad[0]=0;if(mode==1)bad[0]=UINT32_MAX;if(mode==2)bad[1]=255;if(mode==3)bad.back()^=1;if(mode==4)bad[1]=0xff000000u;CHECK(!bar::decode(level,bad.data(),bad.size()));}CHECK(a->view().rgba==v.rgba&&a->sha==decoded->sha);
 }
 std::printf("PASS steering authored bar artwork checks=%u levels=4 premultiplied=1 original-game-data=none\n",checks);
}
