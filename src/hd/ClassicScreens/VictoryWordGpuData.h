// SPDX-License-Identifier: MIT
#pragma once
#include "VictoryWordData.h"
#include <stddef.h>
#include "VictoryWord_econ.inc"
#include "VictoryWord_scon.inc"
#include "VictoryWord_icon.inc"
#include "VictoryWord_fcon.inc"
#include "VictoryWord_gcon.inc"
#include "VictoryWord_wcon.inc"
#include "VictoryWord_eng2.inc"
#include "VictoryWord_spa2.inc"
#include "VictoryWord_ita2.inc"
#include "VictoryWord_fre2.inc"
#include "VictoryWord_ger2.inc"
#include "VictoryWord_swe2.inc"
typedef struct NfsVictoryWordCoverage{const uint16_t*runs;size_t count;} NfsVictoryWordCoverage;
static const NfsVictoryWordCoverage nfs_victory_word_coverage[12]={
{nfs_victory_word_econ,sizeof(nfs_victory_word_econ)/sizeof(uint16_t)},
{nfs_victory_word_scon,sizeof(nfs_victory_word_scon)/sizeof(uint16_t)},
{nfs_victory_word_icon,sizeof(nfs_victory_word_icon)/sizeof(uint16_t)},
{nfs_victory_word_fcon,sizeof(nfs_victory_word_fcon)/sizeof(uint16_t)},
{nfs_victory_word_gcon,sizeof(nfs_victory_word_gcon)/sizeof(uint16_t)},
{nfs_victory_word_wcon,sizeof(nfs_victory_word_wcon)/sizeof(uint16_t)},
{nfs_victory_word_eng2,sizeof(nfs_victory_word_eng2)/sizeof(uint16_t)},
{nfs_victory_word_spa2,sizeof(nfs_victory_word_spa2)/sizeof(uint16_t)},
{nfs_victory_word_ita2,sizeof(nfs_victory_word_ita2)/sizeof(uint16_t)},
{nfs_victory_word_fre2,sizeof(nfs_victory_word_fre2)/sizeof(uint16_t)},
{nfs_victory_word_ger2,sizeof(nfs_victory_word_ger2)/sizeof(uint16_t)},
{nfs_victory_word_swe2,sizeof(nfs_victory_word_swe2)/sizeof(uint16_t)},
};
