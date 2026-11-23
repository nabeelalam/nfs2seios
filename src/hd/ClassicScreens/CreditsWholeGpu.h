// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsWholeRuntimeAbi.h"
#include "hd/GraphicsSceneLayers.h"
// The caller retains its producer view throughout render; input views must not
// alias this GPU object. Calls are confined to the owning render thread.
// Independent unpublished staging only. The backend must separately qualify
// original cached draws and actual front-surface/swap ownership before display.
// The supplied context stage callback must prove actual context/thread/generation
// without depending on the currently bound FBO; it must outlive these resources.
typedef struct NfsCreditsWholeGpu {
 NfsBackgroundSceneStage context;NfsCreditsWholeView frame;
 GLuint program,fbo,color,images[NFS_CREDITS_ART_COUNT],mask;
 GLint matrix,sampler,kind;uint64_t artGeneration,fontIdentity;
 unsigned char fontSha[32];unsigned width,height,ready;
}NfsCreditsWholeGpu;
