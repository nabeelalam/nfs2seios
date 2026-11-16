// SPDX-License-Identifier: MIT
#pragma once
/* Optional independently owned wide image; never authorizes a native pass. */
static void nfs_mt_wide_begin(const NfsMenuTransitionView*,const NfsNativeLogoTargetProof*);
static void nfs_mt_wide_finish(uint64_t,int);
static void nfs_mt_wide_cancel(void);
static void nfs_mt_wide_other(void);
static void nfs_mt_wide_retired(int);
static void nfs_mt_wide_created(void);
static GLuint nfs_mt_wide_present(GLuint,GLuint,int,int);
static void nfs_mt_wide_after_present(int);
static void nfs_mt_wide_nonpaint(void);
static int nfs_mt_wide_defer_triangle(void);
static void nfs_mt_wide_tail(unsigned);
