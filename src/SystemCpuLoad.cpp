// SPDX-License-Identifier: MIT
#include "SystemCpuLoad.h"
#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/processor_info.h>
#endif
namespace nfs {
bool read_system_cpu_ticks(CpuLoadSnapshot &out){
    out.count=0;
#if defined(__APPLE__)
    const mach_port_t host=mach_host_self();
    if(host==MACH_PORT_NULL)return false;
    natural_t count=0;processor_info_array_t raw=nullptr;mach_msg_type_number_t words=0;
    const kern_return_t status=host_processor_info(host,PROCESSOR_CPU_LOAD_INFO,&count,&raw,&words);
    mach_port_deallocate(mach_task_self(),host);
    const bool valid=status==KERN_SUCCESS && raw && count>0 && count<=max_cpu_load_cores &&
        words==count*PROCESSOR_CPU_LOAD_INFO_COUNT;
    if(valid){
        const auto *values=reinterpret_cast<const processor_cpu_load_info_data_t *>(raw);
        out.count=count;bool any_ticks=false;
        for(std::size_t i=0;i<out.count;++i){
            out.cores[i].user=uint32_t(values[i].cpu_ticks[CPU_STATE_USER]);
            out.cores[i].system=uint32_t(values[i].cpu_ticks[CPU_STATE_SYSTEM]);
            out.cores[i].idle=uint32_t(values[i].cpu_ticks[CPU_STATE_IDLE]);
            out.cores[i].nice=uint32_t(values[i].cpu_ticks[CPU_STATE_NICE]);
            const auto &v=out.cores[i];any_ticks=any_ticks || v.user || v.system || v.idle || v.nice;
        }
        // Some platform policies may expose a shape but no activity counters.
        if(!any_ticks)out.count=0;
    }
    if(raw)vm_deallocate(mach_task_self(),reinterpret_cast<vm_address_t>(raw),vm_size_t(words)*sizeof(integer_t));
    return valid && out.count>0;
#else
    return false;
#endif
}
}
