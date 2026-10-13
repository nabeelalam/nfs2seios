// SPDX-License-Identifier: MIT
#include "MenuInput.h"
namespace nfs {
void MenuInput::update(uint8_t desired,uint64_t now,MenuMessageSink sink,void *context){
    const uint32_t keys[]={0x25,0x26,0x27,0x28,13,27};
    const uint32_t scans[]={0x4b,0x48,0x4d,0x50,0x1c,1};
    for(unsigned i=0;i<6;++i){
        const uint8_t bit=uint8_t(1u<<i);const bool was=held&bit,on=desired&bit;
        const bool repeat=i<4 && was && on && now>=repeat_at[i];
        if(was==on && !repeat)continue;
        uint32_t flags=1|(scans[i]<<16);
        if(i<4)flags|=1u<<24;
        if(!on)flags|=0xc0000000u;else if(repeat)flags|=1u<<30;
        const GameMessage message{0,on?0x100u:0x101u,keys[i],flags,uint32_t(now),0,0};
        if(!sink(context,message))continue; // Retry if the original window is not ready.
        if(on){held|=bit;if(i<4)repeat_at[i]=now+(repeat?120:400);}
        else held&=uint8_t(~bit);
        if(on && i>=4)sink(context,{0,0x102,keys[i],flags,uint32_t(now),0,0});
    }
}
}
