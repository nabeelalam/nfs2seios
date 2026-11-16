// SPDX-License-Identifier: MIT
#pragma once
// Original compatibility metadata only; no image/palette bytes or local paths.
#include "OpponentPreviewSourceCatalog.h"
#include <algorithm>
#include "PlayerPhotoSourceCatalog.h"
namespace nfs_opponent_preview_catalog {
inline std::shared_ptr<const nfs_opponent_preview_source::Catalog>catalog(){
 std::array<nfs_opponent_preview_source::Entry,15>entries{};
 for(unsigned i=0;i<12;++i){const auto&s=nfs_player_photo_sources[i];auto&e=entries[i];if(s.car!=i)return {};std::copy_n(reinterpret_cast<const uint8_t*>(s.name),4,e.name.begin());e.width=s.width;e.height=s.height;e.bytes=s.span;e.archiveOffset=s.offset;std::copy_n(s.sha,32,e.sha.begin());}
 entries[12].name={{99,108,115,97}};
 entries[12].width=306u; entries[12].height=146u; entries[12].bytes=45732u;
 entries[12].archiveOffset=3075444u;
 entries[12].sha={{136,134,201,7,115,13,215,73,232,239,84,210,156,28,101,81,99,32,236,23,166,138,117,132,246,89,62,66,14,225,190,138}};
 entries[13].name={{99,108,115,98}};
 entries[13].width=306u; entries[13].height=146u; entries[13].bytes=45732u;
 entries[13].archiveOffset=3121176u;
 entries[13].sha={{215,103,44,130,17,111,1,30,152,175,107,248,227,153,112,197,191,182,141,48,160,110,156,116,67,9,93,46,182,96,193,223}};
 entries[14].name={{99,108,115,99}};
 entries[14].width=306u; entries[14].height=146u; entries[14].bytes=45732u;
 entries[14].archiveOffset=3166908u;
 entries[14].sha={{252,122,27,234,154,32,194,80,141,56,16,231,42,145,193,252,198,49,64,50,242,203,49,51,222,231,175,15,63,23,87,172}};
 const nfs_opponent_preview_source::Digest archive={{1,81,20,122,205,4,240,82,115,105,168,74,139,112,141,200,36,166,207,143,96,197,56,66,222,85,245,178,41,29,38,99}};
 const nfs_opponent_preview_source::Digest decoded={{43,235,4,189,178,255,192,87,1,39,155,116,133,74,58,142,102,67,168,158,94,149,137,156,52,87,24,95,215,220,172,129}};
 return nfs_opponent_preview_source::Catalog::create(archive,decoded,entries);
}
}
