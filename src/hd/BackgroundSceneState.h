// SPDX-License-Identifier: MIT
#pragma once
typedef struct NfsBgSaved {NfsFrameSaved common;GLint depthFunc,scissor[4];GLboolean depthMask;GLfloat depthClear;}NfsBgSaved;
static void nfs_bg_save(NfsBgSaved*s){nfs_frame_save(&s->common);glGetIntegerv(GL_DEPTH_FUNC,&s->depthFunc);glGetIntegerv(GL_SCISSOR_BOX,s->scissor);glGetBooleanv(GL_DEPTH_WRITEMASK,&s->depthMask);glGetFloatv(GL_DEPTH_CLEAR_VALUE,&s->depthClear);}
static void nfs_bg_restore(const NfsBgSaved*s){glDepthFunc(s->depthFunc);glDepthMask(s->depthMask);glClearDepthf(s->depthClear);glScissor(s->scissor[0],s->scissor[1],s->scissor[2],s->scissor[3]);nfs_frame_restore(&s->common);}
