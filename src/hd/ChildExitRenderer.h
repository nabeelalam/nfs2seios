// SPDX-License-Identifier: MIT
#pragma once
/* Legacy API name: source-authenticated40D1B0 PREVIEW CHANGE21.
 * This channel does not cover actual Escape/return animations. */
static void nfs_child_exit_created(void);
static void nfs_child_exit_retired(int);
static void nfs_child_exit_other(void);
static void nfs_child_exit_state(void);
static void nfs_child_exit_clear(uint16_t,float,float,float);
static void nfs_child_exit_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_child_exit_triangle(const GrVertex*,const GrVertex*,const GrVertex*);
static void nfs_child_exit_batch(unsigned);
static GLuint nfs_child_exit_present(GLuint,GLuint,int,int);
static void nfs_child_exit_after_present(int);
