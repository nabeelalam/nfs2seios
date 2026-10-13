// SPDX-License-Identifier: MIT
#include "InputAxes.h"
#include "GuestAccess.h"
namespace nfs {
int32_t calibrated_axis(uint32_t input, int32_t axis, bool deadzone){
    if(axis<0)axis=0;
    if(axis>65535)axis=65535;
    int32_t high=int32_t(input & 0xff00),low=int32_t((input & 0xff)<<8);
    int32_t numerator=0,denominator=0;
    if(high==0x6800){
        const int32_t threshold=deadzone?high:0x8000;
        if(low==0x4e00)low=0x1a00;else if(low==0x1a00)low=0x4e00;
        if(axis<threshold){numerator=threshold-axis;denominator=(threshold-low)/256;}
    }else if(high==0x9800){
        const int32_t threshold=deadzone?high:0x8000;
        if(low==0xb200)low=0xe600;else if(low==0xe600)low=0xb200;
        low+=0x100;
        if(axis>threshold){numerator=axis-threshold;denominator=(low-threshold)/256;}
    }else if(high==0x1400 || low==0x1400){
        if(low==0x1400){axis=65535-axis;const int32_t temp=low;low=high;high=temp;}
        if(low==0xb600)low=0x4a00;else if(low==0x4a00)low=0xb600;
        numerator=axis-(deadzone?high:0);denominator=(low-(deadzone?high:0))/256;
    }
    // Malformed calibration must not divide by zero. Valid game records use
    // positive denominators; reject inverted bounds instead of reversing input.
    if(denominator<=0)return 0;
    const int32_t result=numerator/denominator;
    return result<0?0:(result>255?255:result);
}
int32_t read_calibrated_axis(uint32_t input,uint32_t axes,bool deadzone){
    const uint32_t controller=input>>20;
    if(controller>=16)return 0;
    const uint32_t index=controller*16+((input>>16)&7);
    const uint64_t address=uint64_t(axes)+uint64_t(index)*4;
    if(address+4>uint64_t(UINT32_MAX)+1)return 0;
    return calibrated_axis(input,guest_load<int32_t>(uint32_t(address)),deadzone);
}
}
