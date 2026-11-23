// SPDX-License-Identifier: MIT
#ifndef NFS_NESTED_SCENE_RENDERER_H
#define NFS_NESTED_SCENE_RENDERER_H
#include "AudioSceneRuntimeAbi.h"
#include "OptionsBitmapPreludeAbi.h"
#include "GraphicsSceneRuntimeAbi.h"
#include "ControlsSceneRuntimeAbi.h"
#include "NativeSceneTarget.h"
static int nfs_nested_options_bitmap_prepare(const NfsOptionsBitmapPreludeView*,const NfsNativeLogoTargetProof*);
static void nfs_nested_audio_prepare(const NfsAudioSceneFrameView*,const NfsNativeLogoTargetProof*);
static void nfs_nested_graphics_prepare(const NfsGraphicsSceneFrameView*,const NfsNativeLogoTargetProof*);
static void nfs_nested_controls_prepare(const NfsControlsSceneFrameView*,const NfsNativeLogoTargetProof*);
static void nfs_nested_retire(int);
static void nfs_nested_revoke(void);
static void nfs_nested_tail(unsigned);
static GLuint nfs_nested_present(GLuint,GLuint,int,int);
#define NFS_AUDIO_SCENE_WIDE_FINISH nfs_nested_audio_prepare
#define NFS_GRAPHICS_SCENE_WIDE_FINISH nfs_nested_graphics_prepare
#define NFS_CONTROLS_SCENE_WIDE_FINISH nfs_nested_controls_prepare
#endif
