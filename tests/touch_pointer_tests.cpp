#include "TouchPointer.h"
#include <cstdio>
#include <stdexcept>
#include <limits>
#define CHECK(x) do{if(!(x))throw std::runtime_error("touch pointer line " + std::to_string(__LINE__));}while(0)
#include <string>
using namespace nfs;
int main(){try{
    TouchPointer p;
    auto event=[&](TouchPhase phase,uint64_t finger,double x,double y,uint32_t now){return p.update(phase,0,finger,x,y,now);};
    auto down=[&](uint64_t f,uint32_t t){return event(TouchPhase::down,f,.5,.5,t);};
    auto up=[&](uint64_t f,uint32_t t){return event(TouchPhase::up,f,.5,.5,t);};
    CHECK(down(0,100).action==TouchAction::none);
    CHECK(up(0,110).action==TouchAction::click);CHECK(up(0,111).action==TouchAction::none);
    down(1,200);down(2,210);CHECK(up(1,220).action==TouchAction::none);CHECK(up(2,230).action==TouchAction::escape);
    down(1,300);down(2,301);down(3,302);CHECK(up(3,303).action==TouchAction::none);up(1,304);CHECK(up(2,305).action==TouchAction::enter);
    down(1,400);down(2,401);down(3,402);down(4,403);up(4,404);up(3,405);up(2,406);CHECK(up(1,407).action==TouchAction::diagnostics);
    for(unsigned i=0;i<5;++i)down(i,420+i);
    for(unsigned i=0;i<5;++i)CHECK(up(i,430+i).action==TouchAction::none);
    for(unsigned i=0;i<4;++i)down(i,450+i);
    CHECK(event(TouchPhase::cancel,2,.5,.5,460).clear);
    for(unsigned i=0;i<4;++i)CHECK(up(i,470+i).action==TouchAction::none);
    down(1,500);CHECK(up(1,851).action==TouchAction::none);
    down(1,900);auto r=event(TouchPhase::motion,1,.6,.55,910);CHECK(r.dx==63 || r.dx==64);CHECK(r.dy==24);
    r=event(TouchPhase::up,1,.6,.55,920);CHECK(r.action==TouchAction::none);
    down(1,1000);int total=0;for(int i=1;i<=4;++i)total+=event(TouchPhase::motion,1,.5+i/1024.0,.5,1000+i).dx;CHECK(total==2);CHECK(up(1,1010).action==TouchAction::click);
    // Returning to the initial point does not turn a pan into a click.
    down(1,1100);event(TouchPhase::motion,1,.6,.5,1110);CHECK(up(1,1120).action==TouchAction::none);
    down(1,1200);down(2,1201);r=event(TouchPhase::motion,1,.6,.5,1202);CHECK(r.dx==0 && r.dy==0);up(1,1203);CHECK(up(2,1204).action==TouchAction::none);
    down(1,1300);CHECK(event(TouchPhase::cancel,1,.5,.5,1301).clear);CHECK(up(1,1302).action==TouchAction::none);
    down(1,1400);CHECK(p.reset().clear);CHECK(up(1,1401).action==TouchAction::none);
    down(1,1500);CHECK(event(TouchPhase::motion,1,std::numeric_limits<double>::quiet_NaN(),.5,1501).clear);CHECK(up(1,1502).action==TouchAction::none);
    // Other devices, duplicate down and unmatched lifts do not change ownership.
    down(1,1600);down(1,1601);CHECK(p.update(TouchPhase::up,1,1,.5,.5,1602).action==TouchAction::none);CHECK(up(99,1603).action==TouchAction::none);CHECK(up(1,1604).action==TouchAction::click);
    down(1,UINT32_MAX-10);CHECK(up(1,5).action==TouchAction::click);
    for(unsigned i=0;i<11;++i)down(i,1700+i);
    for(unsigned i=0;i<11;++i)CHECK(up(i,1720+i).action==TouchAction::none);
    CHECK(up(0,1800).action==TouchAction::none);down(0,1801);CHECK(up(0,1802).action==TouchAction::click);
    std::puts("PASS: relative motion, tap chords, timing, cancellation and bounded ownership");return 0;
}catch(const std::exception &e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
