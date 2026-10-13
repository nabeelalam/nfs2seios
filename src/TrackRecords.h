// SPDX-License-Identifier: MIT
#pragma once
#include "FileBridge.h"
#include <array>
namespace nfs {
struct TrackRecordEntry {char name[10];int16_t car;int32_t time,mode;};
static_assert(sizeof(TrackRecordEntry)==20,"Original STF entry ABI");
using TrackRecords=std::array<TrackRecordEntry,31>;
static_assert(sizeof(TrackRecords)==620,"Original STF ABI");
// Conversion preserves valid original .ssf records. Malformed source is rejected
// before opening/truncating the output; absent source retains original zero-file behavior.
bool convert_track_records(FileBridge &,const std::string &source,const std::string &destination,bool clear);
bool parse_track_records(const std::string &,TrackRecords &);
void bind_track_record_files(FileBridge *);
}
extern "C" void nfs_game_record_path(void *,uint32_t,uint32_t);
extern "C" int32_t fetchTrackRecords(int32_t,int32_t,int32_t);
