// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <SDL3/SDL_video.h>
// Original GLES backend configuration, owned and changed on the app main thread.
extern "C" {
int32_t keepAspectRatio=1,needRecreateGl=0,windowResized=0,linearFiltering=1,fixedFramebufferSize=1,framebufferLinearFiltering=1;
int32_t vSync=1,winWidth=640,winHeight=480,initialWinWidth=640,initialWinHeight=480;
float dpr=1;
SDL_Window *sdlWin=nullptr;
}
