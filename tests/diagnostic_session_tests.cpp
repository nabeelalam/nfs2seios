// SPDX-License-Identifier: MIT
#include "DiagnosticSession.h"
#include <cstdio>
#include <limits>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL session line%d\n",__LINE__);return 1;}}while(0)
struct CommaDecimal : std::numpunct<char>{char do_decimal_point()const override{return ',';}};
int main(){
    std::locale::global(std::locale(std::locale::classic(),new CommaDecimal));
    nfs::DiagnosticSessionLimit limit;
    CHECK(limit.accept(0) && !limit.accept(0) && !limit.accept(-1));
    CHECK(!limit.accept(std::numeric_limits<double>::quiet_NaN()));
    for(unsigned i=1;i<3600;++i)CHECK(limit.accept(i));
    CHECK(limit.count()==3600 && limit.expired(3599.5) && !limit.accept(3600));
    nfs::DiagnosticSessionLimit duration;CHECK(duration.accept(0));CHECK(duration.expired(3600));
    std::fputs(nfs::diagnostic_header(19).c_str(),stdout);
    nfs::DiagnosticSample sample;sample.cores.count=64;
    for(unsigned i=0;i<64;++i){sample.cores.valid[i]=true;sample.cores.percent[i]=double(i)*100/63;}
    sample.fps=60;sample.game_cpu_ms=12.25;sample.game_cpu_id=42;sample.battery_percent=53;
    sample.interval=1.125;sample.process_ms=700;sample.main_ms=50;sample.logical_cores=8;
    sample.app_cpu_percent=7.7778;sample.low_power=true;sample.width=2186;sample.height=1640;
    auto line=nfs::diagnostic_sample_json(sample);CHECK(line.size()<8192 && line.back()=='\n');
    CHECK(line.find("\"game_cpu_sample_id\":42")!=std::string::npos);
    CHECK(line.find("\"hd_timing_requested\":null")!=std::string::npos);
    sample.hd_timing=1;CHECK(nfs::diagnostic_sample_json(sample).find("\"hd_timing_requested\":true")!=std::string::npos);
    sample.hd_timing=0;CHECK(nfs::diagnostic_sample_json(sample).find("\"hd_timing_requested\":false")!=std::string::npos);
    sample.hd_timing=9;CHECK(nfs::diagnostic_sample_json(sample).find("\"hd_timing_requested\":null")!=std::string::npos);
    std::fputs(line.c_str(),stdout);
    sample.elapsed=1;sample.game_cpu_ms=-1;sample.fps=std::numeric_limits<double>::infinity();
    sample.ram_mb=std::numeric_limits<double>::quiet_NaN();sample.battery_percent=101;
    sample.brightness=2;sample.cores.valid[0]=false;sample.cores.percent[1]=101;
    line=nfs::diagnostic_sample_json(sample);CHECK(line.find("\"presentation_fps\":null")!=std::string::npos);
    CHECK(line.find("\"ram_mb\":null")!=std::string::npos);std::fputs(line.c_str(),stdout);
    sample.cores.count=65;line=nfs::diagnostic_sample_json(sample);CHECK(line.find("\"system_core_percent\":[]")!=std::string::npos);std::fputs(line.c_str(),stdout);
    std::fputs(nfs::diagnostic_footer("manual",12.5,13).c_str(),stdout);
    std::fputs(nfs::diagnostic_footer("bad\"input",std::numeric_limits<double>::infinity(),0).c_str(),stdout);
    std::fprintf(stderr,"PASS bounded session rows,strict finite/null fields,64-core bound,units and window identity\n");
}
