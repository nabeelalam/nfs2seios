// SPDX-License-Identifier: MIT
#pragma once
static void nfs_player_intro_wide_created(void);
static void nfs_player_intro_wide_retired(int);
static void nfs_player_intro_wide_other(void);
static void nfs_player_intro_wide_clear(uint16_t,float,float,float);
static void nfs_player_intro_wide_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_player_intro_wide_triangle(const GrVertex*,const GrVertex*,const GrVertex*);
static void nfs_player_intro_wide_batch(unsigned);
static GLuint nfs_player_intro_wide_present(GLuint,GLuint,int,int);
static void nfs_player_intro_wide_after_present(int);
