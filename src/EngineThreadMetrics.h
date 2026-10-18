// SPDX-License-Identifier: MIT
#pragma once
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
void nfs_engine_metrics_owner_begin(void);
void nfs_engine_metrics_owner_end(void);
void nfs_set_engine_metrics_capture(int enabled);
void nfs_reset_engine_metrics(void);
void nfs_engine_frame_boundary(void);
double nfs_engine_cpu_ms_per_frame(void);
#ifdef __cplusplus
double nfs_engine_cpu_sample(uint64_t *sample_id);
#endif
#ifdef __cplusplus
}
namespace nfs {
int64_t current_thread_cpu_ns();
struct EngineMetricsOwner {
    EngineMetricsOwner(){nfs_engine_metrics_owner_begin();}
    ~EngineMetricsOwner(){nfs_engine_metrics_owner_end();}
    EngineMetricsOwner(const EngineMetricsOwner &)=delete;
    EngineMetricsOwner &operator=(const EngineMetricsOwner &)=delete;
};
}
#endif
