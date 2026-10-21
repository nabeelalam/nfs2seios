// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <SDL3/SDL_video.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
// Original GLES backend configuration, owned and changed on the app main thread.
extern "C" {
int32_t keepAspectRatio=1,needRecreateGl=0,windowResized=0,linearFiltering=1,framebufferLinearFiltering=1;
#if defined(__APPLE__) && TARGET_OS_IOS
int32_t fixedFramebufferSize=1; // Existing iPad resolution selection owns this policy.
#else
int32_t fixedFramebufferSize=0; // Desktop follows the actual drawable pixel size.
#endif
int32_t vSync=1,winWidth=640,winHeight=480,initialWinWidth=640,initialWinHeight=480;
float dpr=1;
SDL_Window *sdlWin=nullptr;
}
