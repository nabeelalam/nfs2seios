// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
namespace nfs {
enum class TouchPhase {down,motion,up,cancel};
enum class TouchAction {none,click,escape,enter};
struct TouchResult {int32_t dx=0,dy=0;TouchAction action=TouchAction::none;bool clear=false;};
// Main-thread relative trackpad. Coordinates use the original 640x480 UI,
// independent of render resolution. A chord commits only after its last lift.
class TouchPointer {
    struct Finger {uint64_t id=0;double x=0,y=0;bool used=false;};
    std::array<Finger,10> fingers{};
    uint64_t device=0;uint32_t started=0;
    unsigned count=0,peak=0;bool rejected=false;
    double path=0,remainder_x=0,remainder_y=0;
    void clear_state(){for(auto &f:fingers)f.used=false;count=peak=0;rejected=false;path=remainder_x=remainder_y=0;}
public:
    TouchResult reset(){clear_state();TouchResult r;r.clear=true;return r;}
    TouchResult update(TouchPhase phase,uint64_t touch,uint64_t finger,double x,double y,uint32_t now){
        TouchResult result;
        if(count && device!=touch)return result;
        Finger *found=nullptr;for(auto &f:fingers)if(f.used && f.id==finger){found=&f;break;}
        if(phase==TouchPhase::cancel){if(found)return reset();return result;}
        if(!std::isfinite(x) || !std::isfinite(y)){if(found)return reset();return result;}
        x=std::fmax(0,std::fmin(1,x));y=std::fmax(0,std::fmin(1,y));
        if(phase==TouchPhase::down){
            if(found)return result;
            if(!count){clear_state();device=touch;started=now;}
            for(auto &f:fingers)if(!f.used){found=&f;break;}
            if(!found){rejected=true;return result;}
            *found={finger,x,y,true};++count;if(count>peak)peak=count;
            if(peak>3)rejected=true;
            return result;
        }
        if(!found)return result;
        const double dx=(x-found->x)*640,dy=(y-found->y)*480;
        path+=std::hypot(dx,dy);found->x=x;found->y=y;
        if(peak==1){
            remainder_x+=dx;remainder_y+=dy;
            result.dx=int32_t(remainder_x);result.dy=int32_t(remainder_y);
            remainder_x-=result.dx;remainder_y-=result.dy;
        }
        if(phase==TouchPhase::up){
            found->used=false;--count;
            if(!count){
                if(!rejected && uint32_t(now-started)<=350 && path<=8)
                    result.action=peak==1?TouchAction::click:peak==2?TouchAction::escape:TouchAction::enter;
                clear_state();
            }
        }
        return result;
    }
};
}
