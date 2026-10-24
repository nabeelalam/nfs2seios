// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <limits>

// Pure original two-line402E80 layout arithmetic. This does not authenticate
// measurements, source owners, bitmap pixels, artwork or native presentation.
namespace nfs_feedback_modal_geometry {
struct Geometry {
 int32_t width=0,height=0,left=0,top=0,titleAnchor=0,titleY=0,messageAnchor=0,messageY=0;
};
inline bool make(int32_t screenWidth,int32_t screenHeight,int32_t titleMeasuredWidth,
                 int32_t messageMeasuredWidth,Geometry&out)noexcept {
 if(screenWidth<=0||screenHeight<=0||titleMeasuredWidth<0||messageMeasuredWidth<0)return false;
 const int64_t title=int64_t(titleMeasuredWidth)+60;
 const int64_t width=(title>messageMeasuredWidth?title:int64_t(messageMeasuredWidth))+30;
 const int64_t height=60,left=(int64_t(screenWidth)-width)/2,top=(int64_t(screenHeight)-height)/2;
 const int64_t titleAnchor=left+60+(width-60)/2,titleY=top-25;
 const int64_t messageAnchor=left+width/2,messageY=top+15;
 const auto fits=[](int64_t v)noexcept{return v>=std::numeric_limits<int32_t>::min()&&v<=std::numeric_limits<int32_t>::max();};
 if(!fits(width)||!fits(height)||!fits(left)||!fits(top)||!fits(titleAnchor)||!fits(titleY)||!fits(messageAnchor)||!fits(messageY))return false;
 const Geometry result{int32_t(width),int32_t(height),int32_t(left),int32_t(top),int32_t(titleAnchor),int32_t(titleY),int32_t(messageAnchor),int32_t(messageY)};
 out=result;return true;
}
}
