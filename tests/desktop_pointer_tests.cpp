#include "SDLDesktopPointer.h"
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
static unsigned checks=0;static void ck(bool ok){++checks;if(!ok)throw std::runtime_error("desktop pointer contract");}
static SDL_Event motion(float x,float y,SDL_MouseID id=1,SDL_WindowID window=7){SDL_Event e{};e.type=SDL_EVENT_MOUSE_MOTION;e.motion.which=id;e.motion.windowID=window;e.motion.xrel=x;e.motion.yrel=y;return e;}
static SDL_Event button(bool down,unsigned b=SDL_BUTTON_LEFT,SDL_MouseID id=1){SDL_Event e{};e.type=down?SDL_EVENT_MOUSE_BUTTON_DOWN:SDL_EVENT_MOUSE_BUTTON_UP;e.button.windowID=7;e.button.which=id;e.button.button=b;return e;}
int main(){try{nfs::DesktopPointer p;auto send=[&](SDL_Event e,int w=1280,int h=960){return nfs::original_desktop_pointer_event(p,e,7,w,h);};
 auto r=send(motion(2,2));ck(r.dx==1&&r.dy==1);r=send(motion(-2,-2));ck(r.dx==-1&&r.dy==-1);
 int x=0,y=0;for(unsigned k=0;k<10;++k){r=send(motion(1,1),2560,1920);x+=r.dx;y+=r.dy;}ck(x==2&&y==2);r=send(motion(-2,-2),2560,1920);ck(!r.dx&&!r.dy);r=send(motion(-4,-4),2560,1920);ck(r.dx==-1&&r.dy==-1);
 p.reset();send(motion(1,1));r=send(motion(1,1),640,480);ck(r.dx==1&&r.dy==1); // resize clears prior fractional carry.
 p.reset();send(motion(1,1));r=send(motion(100,100,SDL_TOUCH_MOUSEID));ck(!r.dx&&!r.dy);r=send(motion(100,100,1,8));ck(!r.dx&&!r.dy);r=send(motion(1,1));ck(r.dx==1&&r.dy==1);
 ck(send(button(true)).click);ck(send(button(true)).click==false);ck(send(button(false)).click==false);ck(send(button(true)).click);ck(send(button(true,SDL_BUTTON_RIGHT)).click==false);p.reset();ck(send(button(true,SDL_BUTTON_LEFT,SDL_TOUCH_MOUSEID)).click==false);ck(send(button(true)).click);
 p.reset();send(motion(1,1));p.reset();r=send(motion(1,1));ck(!r.dx&&!r.dy);r=send(motion(std::numeric_limits<float>::infinity(),0));ck(!r.dx&&!r.dy);r=send(motion(std::numeric_limits<float>::quiet_NaN(),0));ck(!r.dx&&!r.dy);r=send(motion(1e30f,0));ck(!r.dx&&!r.dy);r=send(motion(1,1),0,480);ck(!r.dx&&!r.dy);SDL_Event finger{};finger.type=SDL_EVENT_FINGER_MOTION;r=send(finger);ck(!r.dx&&!r.dy&&!r.click);
 // Accepted relative sweeps drive a source-clamped original cursor to a known corner.
 p.reset();int cx=400,cy=250;r=send(motion(-1280,-960));cx=std::max(0,std::min(639,cx+r.dx));cy=std::max(0,std::min(479,cy+r.dy));ck(!cx&&!cy);r=send(motion(800,600));cx+=r.dx;cy+=r.dy;ck(cx==400&&cy==300);
 p.reset();ck(!send(button(false)).click);ck(send(button(true)).click);ck(!send(button(false,SDL_BUTTON_LEFT,SDL_TOUCH_MOUSEID)).click);ck(!send(button(true)).click);ck(!send(button(false)).click);ck(send(button(true)).click);
 p.reset();r=nfs::original_desktop_pointer_event(p,motion(1,1),0,640,480);ck(!r.dx&&!r.dy&&!r.click);
 p.reset();x=y=0;for(unsigned k=0;k<1920;++k){r=send(motion(1,.75f),1920,1440);x+=r.dx;y+=r.dy;}ck(x==640&&y==480);
 std::printf("PASS desktop real mouse checks=%u fractional640x480=1 syntheticExcluded=1 singleClick=1\n",checks);return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
