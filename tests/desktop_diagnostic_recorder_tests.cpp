// SPDX-License-Identifier: MIT
// Focused recorder tests use mocked SDL clocks/frame/engine metrics, not a game baseline.
#include "DesktopDiagnosticRecorder.h"
#include "FramePresentation.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <deque>
#include <string>
#include <unistd.h>
static uint64_t now_ns=1000000000,latest=900,drops=17;static bool enabled=false,engine=false;static unsigned clocks=0;static std::deque<NfsFrameTiming> pending;
extern "C" Uint64 SDL_GetTicksNS(){++clocks;return now_ns;}
extern "C" int SDL_GetNumLogicalCPUCores(){return 4;}
extern "C" void nfs_frame_timing_capture(int value){enabled=value;}
extern "C" unsigned nfs_frame_timing_drain(NfsFrameTiming*out,unsigned cap){unsigned n=0;while(n<cap&&!pending.empty()){out[n++]=pending.front();pending.pop_front();}return n;}
extern "C" NfsFrameTimingSnapshot nfs_frame_timing_snapshot(){return {latest,now_ns,drops,3,0,0,enabled?1:0};}
extern "C" void nfs_set_engine_metrics_capture(int value){engine=value;}
extern "C" double nfs_engine_cpu_sample(uint64_t*id){*id=42;return 12.5;}
extern "C" int nfs_render_width(){return 1280;}
extern "C" int nfs_render_height(){return 720;}
extern "C" int nfs_frame_limit(){return 0;}
std::string read(const std::string&p){std::ifstream f(p);return {std::istreambuf_iterator<char>(f),{}};}
bool exists(const std::string&p){return ::access(p.c_str(),F_OK)==0;}
int main(){
 char folder[]="/tmp/nfs-desktop-recorder-XXXXXX";assert(mkdtemp(folder));std::string dir=folder;
 {char a[]="app",b[]="game";char*args[]={a,b,nullptr};int n=2;assert(nfs::desktop_recording_option(n,args).empty());assert(n==2&&clocks==0&&!engine&&!enabled);}
 {char a[]="app",b[]="--record-session",c[]="new.jsonl",d[]="game";char*args[]={a,b,c,d,nullptr};int n=4;assert(nfs::desktop_recording_option(n,args)=="new.jsonl");assert(n==2&&std::string(args[1])=="game"&&args[2]==nullptr);}
 {char a[]="app",b[]="--record-session",c[]="one.jsonl",d[]="--record-session",e[]="two.jsonl";char*args[]={a,b,c,d,e,nullptr};int n=5;bool refused=false;try{nfs::desktop_recording_option(n,args);}catch(...){refused=true;}assert(refused);}
 {char a[]="app",b[]="--record-session";char*args[]={a,b,nullptr};int n=2;bool refused=false;try{nfs::desktop_recording_option(n,args);}catch(...){refused=true;}assert(refused);}
 {char a[]="app",b[]="--record-session=bad.jsonl";char*args[]={a,b,nullptr};int n=2;bool refused=false;try{nfs::desktop_recording_option(n,args);}catch(...){refused=true;}assert(refused);}
 {std::ofstream(dir+"/exclusive.metadata.json")<<"keep";bool refused=false;try{nfs::DesktopDiagnosticRecorder r(dir+"/exclusive.jsonl");}catch(...){refused=true;}assert(refused);assert(read(dir+"/exclusive.metadata.json")=="keep");assert(!exists(dir+"/exclusive.jsonl")&&!exists(dir+"/exclusive.frames.csv"));assert(!engine&&!enabled);}
 {nfs::DesktopDiagnosticRecorder::Limits limits;limits.samples=2;pending.push_back({900,10,0,2});nfs::DesktopDiagnosticRecorder r(dir+"/samples.jsonl",limits);assert(pending.empty()&&engine&&enabled);pending.push_back({901,2000000000,0,3});latest=901;now_ns+=1000000000;r.poll();now_ns+=1000000000;r.poll();now_ns+=1000000000;r.poll();assert(!engine&&!enabled);r.stop();}
 auto sample=read(dir+"/samples.jsonl"),meta=read(dir+"/samples.metadata.json"),csv=read(dir+"/samples.frames.csv");assert(sample.find("\"low_power_mode\":null")!=std::string::npos);assert(sample.find("\"presentation_fps\":null")!=std::string::npos);assert(sample.find("\"game_cpu_sample_id\":42")!=std::string::npos);assert(sample.find("\"game_cpu_ms_per_frame\":12.5000")!=std::string::npos);assert(meta.find("\"samples_written\":2")!=std::string::npos);assert(meta.find("\"start_frame_id\":900")!=std::string::npos);assert(meta.find("sample_limit")!=std::string::npos);assert(csv.find("900,")==std::string::npos&&csv.find("901,2000000000,0,3")!=std::string::npos);
 {nfs::DesktopDiagnosticRecorder::Limits limits;limits.frames=2;nfs::DesktopDiagnosticRecorder r(dir+"/frames.jsonl",limits);for(unsigned i=0;i<1024;++i)pending.push_back({902+i,now_ns+i,1,3});latest=1925;r.poll();r.stop();assert(pending.empty());}meta=read(dir+"/frames.metadata.json");assert(meta.find("\"frames_written\":2")!=std::string::npos);assert(meta.find("\"discarded_frames\":1022")!=std::string::npos);assert(meta.find("frame_limit")!=std::string::npos);
 {nfs::DesktopDiagnosticRecorder::Limits limits;limits.queue_bytes=1024;nfs::DesktopDiagnosticRecorder r(dir+"/backlog.jsonl",limits);for(unsigned i=0;i<1024;++i)pending.push_back({2000+i,now_ns+i,1,3});r.poll();r.stop();assert(pending.empty());}meta=read(dir+"/backlog.metadata.json");assert(meta.find("writer_backlog")!=std::string::npos);assert(meta.find("\"discarded_frames\":1024")!=std::string::npos);assert(!enabled&&!engine);
 assert(read(dir+"/backlog.jsonl").find("writer_backlog")!=std::string::npos);
 {nfs::DesktopDiagnosticRecorder r(dir+"/time.jsonl");now_ns+=3600000000000ull;r.poll();r.stop();}meta=read(dir+"/time.metadata.json");assert(meta.find("time_limit")!=std::string::npos);assert(read(dir+"/time.jsonl").find("time_limit")!=std::string::npos);
 {nfs::DesktopDiagnosticRecorder::Limits limits;limits.bytes=8192;nfs::DesktopDiagnosticRecorder r(dir+"/bytes.jsonl",limits);for(unsigned i=0;i<128;++i)pending.push_back({100000000000000ull+i,now_ns+i,1000000000,3});r.poll();r.stop();}meta=read(dir+"/bytes.metadata.json");assert(meta.find("byte_limit")!=std::string::npos);
 std::printf("PASS: mocked-clock recorder limits, default-off, exclusivity, unsupported fields, IDs and backlog; artifacts %s\n",folder);
}
