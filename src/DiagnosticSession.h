// SPDX-License-Identifier: MIT
#pragma once
#include "SystemCpuLoad.h"
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
namespace nfs {
struct DiagnosticSample {
    double elapsed=0,interval=-1,fps=-1,fps_age=-1,game_cpu_ms=-1;
    uint64_t game_cpu_id=0;
    double process_ms=-1,main_ms=-1,app_cpu_percent=-1,ram_mb=-1;
    double battery_percent=-1,brightness=-1;
    unsigned logical_cores=0;
    int width=0,height=0,frame_limit=0,thermal=-1,battery_state=0,app_state=0;
    bool low_power=false;
    int hd_timing=-1;
    CpuLoadReading cores{};
};
inline void diagnostic_number(std::ostream &out,double value,double maximum=1e15){
    if(std::isfinite(value) && value>=0 && value<=maximum)out<<value;
    else out<<"null";
}
inline std::string diagnostic_header(unsigned build){
    std::ostringstream out;out.imbue(std::locale::classic());
    out<<"{\"type\":\"session\",\"schema\":\"nfsiise-session-v1\",\"build\":"<<build
       <<",\"sample_target_s\":1,\"game_cpu_window_submissions\":60,"
       "\"fps_scope\":\"latest presentation-rate window;null if stale\","
       "\"game_cpu_scope\":\"latest engine-thread 60-submission window;sample_id identifies repeats\","
       "\"core_scope\":\"whole system,not per-core app usage\","
       "\"battery_scope\":\"OS percentage,not watts or measured energy\","
       "\"battery_states\":\"0 unknown,1 unplugged,2 charging,3 full\","
       "\"thermal_states\":\"0 nominal,1 fair,2 serious,3 critical\","
       "\"app_states\":\"0 active,1 inactive,2 background\","
       "\"limits\":\"60 minutes or 3600 rows;sampling gaps are not interpolated\"}\n";
    return out.str();
}
inline std::string diagnostic_footer(const std::string &reason,double elapsed,unsigned rows){
    const bool allowed=reason=="manual" || reason=="background" || reason=="window_changed" || reason=="time_limit" || reason=="write_error" || reason=="writer_backlog";
    std::ostringstream out;out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(4);
    out<<"{\"type\":\"end\",\"reason\":\""<<(allowed?reason:"stopped")<<"\",\"elapsed_s\":";
    diagnostic_number(out,elapsed);out<<",\"attempted_rows\":"<<rows<<"}\n";return out.str();
}
inline std::string diagnostic_sample_json(const DiagnosticSample &s){
    std::ostringstream out;out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(4);
    out<<"{\"type\":\"sample\",\"elapsed_s\":";diagnostic_number(out,s.elapsed);
    out<<",\"interval_s\":";diagnostic_number(out,s.interval);
    out<<",\"presentation_fps\":";diagnostic_number(out,s.fps,10000);
    out<<",\"fps_age_s\":";diagnostic_number(out,s.fps_age);
    out<<",\"game_cpu_ms_per_frame\":";diagnostic_number(out,s.game_cpu_ms);
    out<<",\"game_cpu_sample_id\":"<<s.game_cpu_id;
    out<<",\"process_cpu_ms_interval\":";diagnostic_number(out,s.process_ms);
    out<<",\"main_cpu_ms_interval\":";diagnostic_number(out,s.main_ms);
    out<<",\"app_cpu_percent_all_cores\":";diagnostic_number(out,s.app_cpu_percent,100);
    out<<",\"logical_cores\":"<<s.logical_cores;
    out<<",\"ram_mb\":";diagnostic_number(out,s.ram_mb);
    out<<",\"battery_percent\":";diagnostic_number(out,s.battery_percent,100);
    out<<",\"battery_state\":"<<s.battery_state;
    out<<",\"brightness\":";diagnostic_number(out,s.brightness,1);
    out<<",\"thermal_state\":"<<s.thermal<<",\"app_state\":"<<s.app_state;
    out<<",\"low_power_mode\":"<<(s.low_power?"true":"false");
    out<<",\"hd_timing_requested\":"<<(s.hd_timing==0?"false":s.hd_timing==1?"true":"null");
    out<<",\"render_width\":"<<s.width<<",\"render_height\":"<<s.height;
    out<<",\"frame_limit\":"<<s.frame_limit<<",\"system_core_percent\":[";
    const auto count=s.cores.count<=64?s.cores.count:0;
    for(std::size_t i=0;i<count;++i){if(i)out<<',';diagnostic_number(out,s.cores.valid[i]?s.cores.percent[i]:-1,100);}
    out<<"]}\n";return out.str();
}
class DiagnosticSessionLimit {
    double last=-1;unsigned rows=0;
public:
    bool expired(double elapsed)const{return !std::isfinite(elapsed)||elapsed<0||elapsed>=3600||rows>=3600;}
    bool accept(double elapsed){
        if(expired(elapsed) || elapsed<=last)return false;
        last=elapsed;++rows;return true;
    }
    unsigned count()const{return rows;}
};
}
