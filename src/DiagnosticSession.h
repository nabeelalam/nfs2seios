// SPDX-License-Identifier: MIT
#pragma once
#include "SystemCpuLoad.h"
#include "MainWaitTiming.h"
#include "FramePresentation.h"
#include "TransformTrial.h"
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
namespace nfs {
struct DiagnosticSwapIntervals {
    unsigned count=0,over_18ms=0,over_20ms=0;
    unsigned swap_calls=0;
    uint64_t longest_ns=0,swap_total_ns=0,swap_longest_ns=0;
    void observe(const NfsFrameTiming &frame,uint64_t start,uint64_t end,uint64_t epoch){
        if(frame.window_epoch!=epoch || !frame.interval_ns || frame.timestamp_ns<start ||
           frame.timestamp_ns>end || frame.interval_ns>frame.timestamp_ns-start)return;
        ++count;
        if(frame.interval_ns>18000000)++over_18ms;
        if(frame.interval_ns>20000000)++over_20ms;
        if(frame.interval_ns>longest_ns)longest_ns=frame.interval_ns;
        if(frame.swap_call_ns && frame.swap_call_ns<=frame.timestamp_ns-start){
            ++swap_calls;
            swap_total_ns+=frame.swap_call_ns;
            if(frame.swap_call_ns>swap_longest_ns)swap_longest_ns=frame.swap_call_ns;
        }
    }
};
struct DiagnosticDisplayWindow {
    unsigned callbacks=0,late_callbacks=0;
    double target_seconds=0,max_late_ms=0;

    void observe(double previous,double target,double now) {
        const double interval=target-previous;
        if(!(interval>0 && interval<=1) || !std::isfinite(now))return;
        ++callbacks;
        target_seconds+=interval;
        if(now>target){
            ++late_callbacks;
            const double late_ms=(now-target)*1000;
            if(late_ms>max_late_ms)max_late_ms=late_ms;
        }
    }
};
struct DiagnosticSample {
    double elapsed=0,interval=-1,fps=-1,fps_age=-1,game_cpu_ms=-1;
    double display_link_callbacks=-1,display_link_target_ms=-1;
    double display_link_late_callbacks=-1,display_link_max_late_ms=-1;
    uint64_t game_cpu_id=0;
    double swap_intervals=-1,swap_over_18ms=-1,swap_over_20ms=-1,swap_max_ms=-1;
    double swap_calls=-1,swap_call_total_ms=-1,swap_call_max_ms=-1;
    double classified_presentations=-1,race_world_presentations=-1;
    double renderer_calls=-1,renderer_queue_ms=-1,renderer_execution_ms=-1,renderer_execution_max_ms=-1;
    double process_ms=-1,main_ms=-1,app_cpu_percent=-1,ram_mb=-1;
    double foreground_event_pumps=-1;
    bool main_wait_valid=false;MainWaitTiming main_wait{};
    double battery_percent=-1,brightness=-1;
    unsigned logical_cores=0;
    int width=0,height=0,frame_limit=0,thermal=-1,battery_state=0,app_state=0;
    bool low_power=false;
    int hd_timing=-1;
    int transform_trial_requested=-1;uint64_t transform_trial_generation=0;
    bool transform_trial_interval_stable=false;
    CpuLoadReading cores{};
};
inline void record_transform_trial(DiagnosticSample &sample,TransformTrialSnapshot current,
                                   TransformTrialInterval &interval){
    sample.transform_trial_requested=current.mode();
    sample.transform_trial_generation=current.generation();
    sample.transform_trial_interval_stable=interval.observe(current);
}
inline void record_fps_snapshot(DiagnosticSample &sample,const NfsFrameTimingSnapshot &snapshot,uint64_t now){
    if(!snapshot.fps_timestamp_ns || now<snapshot.fps_timestamp_ns)return;
    sample.fps_age=double(now-snapshot.fps_timestamp_ns)/1e9;
    if(sample.fps_age<=2.5)sample.fps=snapshot.fps;
}
inline void record_frame_content(DiagnosticSample &sample,NfsFrameContentTotals current,
                                 NfsFrameContentTotals &previous,bool valid){
    if(valid && current.window_epoch==previous.window_epoch &&
       current.classified>previous.classified && current.race_world>=previous.race_world){
        const auto classified=current.classified-previous.classified;
        const auto race=current.race_world-previous.race_world;
        if(race<=classified){
            sample.classified_presentations=double(classified);
            sample.race_world_presentations=double(race);
        }
    }
    previous=current;
}
inline void diagnostic_number(std::ostream &out,double value,double maximum=1e15){
    if(std::isfinite(value) && value>=0 && value<=maximum)out<<value;
    else out<<"null";
}
inline std::string diagnostic_header(unsigned build,const std::string &source={}){
    const bool valid=source.size()==40 && source.find_first_not_of("0123456789abcdef")==std::string::npos;
    std::ostringstream out;out.imbue(std::locale::classic());
    out<<"{\"type\":\"session\",\"schema\":\"nfsiise-session-v1\",\"build\":"<<build;
    if(valid)out<<",\"source_commit\":\""<<source<<"\"";
    out<<",\"sample_target_s\":1,\"game_cpu_window_submissions\":60,"
       "\"fps_scope\":\"latest presentation-rate window;null if stale\","
       "\"game_cpu_scope\":\"latest engine-thread 60-submission window;sample_id identifies repeats\","
       "\"swap_interval_scope\":\"complete swap-submission intervals inside sample, no display or GPU timestamps\","
       "\"swap_call_scope\":\"host elapsed time inside SDL_GL_SwapWindow for successful swaps, including any display or driver wait\","
       "\"renderer_scope\":\"timed main-thread renderer operations since prior sample, host elapsed time, no GPU timestamps\","
       "\"event_pump_scope\":\"foreground GameSession polls,not kernel wakeups or idle timeout counts\","
       "\"main_wait_scope\":\"iOS owner run-loop calls wholly within sample;elapsed includes nested callbacks/rendering,not idle CPU or kernel wakeups\","
       "\"transform_trial_scope\":\"requested dispatch configuration only;stable means unchanged generation since prior sample,not per-call accounting;rolling CPU/FPS windows may overlap a change\","
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
    out<<",\"display_link_callbacks\":";diagnostic_number(out,s.display_link_callbacks);
    out<<",\"display_link_mean_target_ms\":";diagnostic_number(out,s.display_link_target_ms);
    out<<",\"display_link_late_callbacks\":";diagnostic_number(out,s.display_link_late_callbacks);
    out<<",\"display_link_max_late_ms\":";diagnostic_number(out,s.display_link_max_late_ms);
    out<<",\"game_cpu_ms_per_frame\":";diagnostic_number(out,s.game_cpu_ms);
    out<<",\"game_cpu_sample_id\":"<<s.game_cpu_id;
    out<<",\"classified_presentations\":";diagnostic_number(out,s.classified_presentations);
    out<<",\"race_world_presentations\":";diagnostic_number(out,s.race_world_presentations);
    out<<",\"swap_intervals_count\":";diagnostic_number(out,s.swap_intervals);
    out<<",\"swap_intervals_over_18ms\":";diagnostic_number(out,s.swap_over_18ms);
    out<<",\"swap_intervals_over_20ms\":";diagnostic_number(out,s.swap_over_20ms);
    out<<",\"swap_interval_max_ms\":";diagnostic_number(out,s.swap_max_ms);
    out<<",\"swap_calls_count\":";diagnostic_number(out,s.swap_calls);
    out<<",\"swap_call_total_ms\":";diagnostic_number(out,s.swap_call_total_ms);
    out<<",\"swap_call_max_ms\":";diagnostic_number(out,s.swap_call_max_ms);
    out<<",\"renderer_calls_interval\":";diagnostic_number(out,s.renderer_calls);
    out<<",\"renderer_queue_ms_interval\":";diagnostic_number(out,s.renderer_queue_ms);
    out<<",\"renderer_execution_ms_interval\":";diagnostic_number(out,s.renderer_execution_ms);
    out<<",\"renderer_execution_max_ms\":";diagnostic_number(out,s.renderer_execution_max_ms);
    out<<",\"process_cpu_ms_interval\":";diagnostic_number(out,s.process_ms);
    out<<",\"main_cpu_ms_interval\":";diagnostic_number(out,s.main_ms);
    out<<",\"foreground_event_pumps_interval\":";diagnostic_number(out,s.foreground_event_pumps);
    const auto wait_number=[&](uint64_t value){diagnostic_number(out,s.main_wait_valid?double(value):-1);};
    out<<",\"main_wait_calls\":";wait_number(s.main_wait.calls);
    out<<",\"main_wait_elapsed_ms\":";diagnostic_number(out,s.main_wait_valid?double(s.main_wait.elapsed_ns)/1e6:-1);
    out<<",\"main_wait_max_ms\":";diagnostic_number(out,s.main_wait_valid?double(s.main_wait.max_ns)/1e6:-1);
    out<<",\"main_wait_finished\":";wait_number(s.main_wait.finished);
    out<<",\"main_wait_stopped\":";wait_number(s.main_wait.stopped);
    out<<",\"main_wait_timed_out\":";wait_number(s.main_wait.timed_out);
    out<<",\"main_wait_handled_source\":";wait_number(s.main_wait.handled_source);
    out<<",\"main_wait_other\":";wait_number(s.main_wait.other);
    out<<",\"app_cpu_percent_all_cores\":";diagnostic_number(out,s.app_cpu_percent,100);
    out<<",\"logical_cores\":"<<s.logical_cores;
    out<<",\"ram_mb\":";diagnostic_number(out,s.ram_mb);
    out<<",\"battery_percent\":";diagnostic_number(out,s.battery_percent,100);
    out<<",\"battery_state\":"<<s.battery_state;
    out<<",\"brightness\":";diagnostic_number(out,s.brightness,1);
    out<<",\"thermal_state\":"<<s.thermal<<",\"app_state\":"<<s.app_state;
    out<<",\"low_power_mode\":"<<(s.low_power?"true":"false");
    out<<",\"transform_trial_requested\":\""<<(s.transform_trial_requested==0?"baseline":s.transform_trial_requested==1?"cache":"unavailable")<<"\"";
    out<<",\"transform_trial_generation\":"<<s.transform_trial_generation;
    out<<",\"transform_trial_interval_stable\":"<<(s.transform_trial_interval_stable?"true":"false");
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
