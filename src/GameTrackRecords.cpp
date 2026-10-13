// SPDX-License-Identifier: MIT
#include "TrackRecords.h"
#include "GameContexts.h"
#include "FunctionIds.h"
extern "C" void nfs_game_record_path(void *context,uint32_t track,uint32_t output){nfs_dispatch_game_regparm2(context,game_id_sub_41B250,track,output);}
