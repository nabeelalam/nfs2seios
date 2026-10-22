// SPDX-License-Identifier: MIT
#pragma once
#include <SDL3/SDL.h>
#include <limits>
#include <cmath>
#include <cstdint>
namespace nfs {
// Main-thread desktop trackpad adapter. GameSession enables this only on desktop.
// SDL relative motion uses window logical coordinates, independent of drawable DPI.
// Original buffered mouse consumption adds these deltas and clamps its own cursor.
struct DesktopPointerResult {int32_t dx=0,dy=0;bool click=false;};
class DesktopPointer {
 double carryX=0,carryY=0;int priorWidth=0,priorHeight=0;SDL_WindowID priorWindow=0;bool leftDown=false;
public:
 void reset()noexcept{carryX=carryY=0;priorWidth=priorHeight=0;priorWindow=0;leftDown=false;}
 DesktopPointerResult update(const SDL_Event&e,SDL_WindowID window,int width,int height)noexcept{
  const bool motion=e.type==SDL_EVENT_MOUSE_MOTION;
  const bool button=e.type==SDL_EVENT_MOUSE_BUTTON_DOWN||e.type==SDL_EVENT_MOUSE_BUTTON_UP;
  if(!motion&&!button)return {};
  const SDL_WindowID from=motion?e.motion.windowID:e.button.windowID;
  const SDL_MouseID device=motion?e.motion.which:e.button.which;
  if(!window||from!=window||device==SDL_TOUCH_MOUSEID)return {};
  if(width<=0||height<=0){reset();return {};}
  if(window!=priorWindow||width!=priorWidth||height!=priorHeight){carryX=carryY=0;priorWidth=width;priorHeight=height;priorWindow=window;}
  DesktopPointerResult r;
  if(button){if(e.button.button!=SDL_BUTTON_LEFT)return r;if(e.type==SDL_EVENT_MOUSE_BUTTON_UP){leftDown=false;return r;}if(!leftDown){leftDown=true;r.click=true;}return r;}
  const double x=double(e.motion.xrel)*640/width+carryX,y=double(e.motion.yrel)*480/height+carryY;
  const double bound=double(std::numeric_limits<int32_t>::max());
  if(!std::isfinite(x)||!std::isfinite(y)||x>bound||x< -bound||y>bound||y< -bound){reset();return {};}
  r.dx=int32_t(x);r.dy=int32_t(y);carryX=x-r.dx;carryY=y-r.dy;return r;
 }
};
inline DesktopPointerResult original_desktop_pointer_event(DesktopPointer&p,const SDL_Event&e,SDL_WindowID window,int width,int height)noexcept{return p.update(e,window,width,height);}
}
