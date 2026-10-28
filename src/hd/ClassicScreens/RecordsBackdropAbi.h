#pragma once
#include <stdint.h>
typedef struct NfsRecordsBackdropView {uint32_t schema,trackIndex,statistics,language;uint64_t frameEpoch,content;uint32_t sourceArchive;} NfsRecordsBackdropView;
#ifdef __cplusplus
inline bool nfs_records_backdrop_same(const NfsRecordsBackdropView&a,const NfsRecordsBackdropView&b)noexcept{return a.schema==b.schema&&a.trackIndex==b.trackIndex&&a.statistics==b.statistics&&a.language==b.language&&a.frameEpoch==b.frameEpoch&&a.content==b.content&&a.sourceArchive==b.sourceArchive;}
#endif
