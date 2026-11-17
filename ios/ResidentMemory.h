// SPDX-License-Identifier: MIT
#pragma once
#include <mach/mach.h>

namespace nfs {
inline double resident_memory_mib() noexcept {
    mach_task_basic_info_data_t info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS ||
        count != MACH_TASK_BASIC_INFO_COUNT) return -1;
    return double(info.resident_size) / 1048576.0;
}
}
