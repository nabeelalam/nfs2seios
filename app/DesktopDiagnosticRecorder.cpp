// SPDX-License-Identifier: MIT
#include "DesktopDiagnosticRecorder.h"
#include "DiagnosticSession.h"
#include "EngineThreadMetrics.h"
#include "FramePresentation.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <cerrno>
#include <fcntl.h>
#if defined(_WIN32)
#include <io.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <time.h>
#endif
extern "C" int nfs_render_width();
namespace nfs {
std::string desktop_recording_option(int &argc,char **argv){
 std::string path;int out=1;
 for(int i=1;i<argc;++i){
  const std::string value=argv[i];
  if(value=="--record-session"){
   if(!path.empty() || i+1>=argc || !argv[i+1] || !argv[i+1][0] || std::strncmp(argv[i+1],"--",2)==0)throw std::invalid_argument("Invalid or duplicate recording option");
   path=argv[++i];
   if(path.size()<7 || path.substr(path.size()-6)!=".jsonl")throw std::invalid_argument("Recording output must be a new .jsonl file");
  }else if(value.compare(0,17,"--record-session=")==0)throw std::invalid_argument("Use --record-session followed by a new .jsonl file");
  else argv[out++]=argv[i];
 }
 argc=out;argv[out]=nullptr;return path;
}
namespace {
FILE *exclusive(const std::string&p){
#if defined(_WIN32)
 int fd=_open(p.c_str(),_O_WRONLY|_O_CREAT|_O_EXCL|_O_BINARY,_S_IREAD|_S_IWRITE);
 if(fd<0)return nullptr;FILE*f=_fdopen(fd,"wb");if(!f){_close(fd);std::remove(p.c_str());}
#else
 int fd=::open(p.c_str(),O_WRONLY|O_CREAT|O_EXCL,0600);
 if(fd<0)return nullptr;FILE*f=fdopen(fd,"wb");if(!f){::close(fd);std::remove(p.c_str());}
#endif
 return f;
}
double cpu_ns(bool main){
#if !defined(_WIN32) && defined(CLOCK_PROCESS_CPUTIME_ID) && defined(CLOCK_THREAD_CPUTIME_ID)
 timespec ts{};if(clock_gettime(main?CLOCK_THREAD_CPUTIME_ID:CLOCK_PROCESS_CPUTIME_ID,&ts)==0)return double(ts.tv_sec)*1e9+ts.tv_nsec;
#else
 (void)main;
#endif
 return -1;
}
const char *safe_reason(const char*p){
 for(const char*value:{"session_end","session_error","time_limit","sample_limit","frame_limit","byte_limit","writer_backlog","write_error"})if(p&&std::strcmp(p,value)==0)return value;
 return "session_end";
}
}
struct DesktopDiagnosticRecorder::Impl {
 struct Work {bool frames;std::string text;unsigned count;};
 Limits limits;FILE *samples=nullptr,*frames=nullptr,*meta=nullptr;
 std::thread writer;std::mutex mutex;std::condition_variable changed;std::deque<Work>queue;
 uint64_t queued_bytes=0,accepted_bytes=0,frame_rows=0,origin=0,last=0,next=0,start_drop=0,start_id=0,end_drop=0,end_id=0,start_epoch=0,end_epoch=0;
 unsigned sample_rows=0;std::atomic<unsigned long long>written_frames{0},written_samples{0};uint64_t discarded_frames=0;bool frames_closed=false;std::atomic<bool>write_error{false};
 bool closing=false,stopped=false;std::string reason="session_end";double process=-1,main=-1;uint64_t elapsed_end=0;
 explicit Impl(const std::string &path,Limits lim):limits(lim){
  if(path.size()<7||path.substr(path.size()-6)!=".jsonl"||!limits.samples||limits.samples>3600||!limits.frames||limits.frames>1000000||limits.bytes<8192||limits.bytes>128ull*1024*1024||limits.queue_bytes<1024||limits.queue_bytes>4ull*1024*1024)throw std::invalid_argument("Invalid diagnostic recording limits");
  const auto stem=path.substr(0,path.size()-6);const std::string paths[]={path,stem+".frames.csv",stem+".metadata.json"};FILE **handles[]={&samples,&frames,&meta};unsigned opened=0;
  try{for(;opened<3;++opened){*handles[opened]=exclusive(paths[opened]);if(!*handles[opened])throw std::runtime_error("Diagnostic output already exists or cannot be created");}
   // Old rows and cumulative counters belong to a previous capture, not this session.
   nfs_frame_timing_capture(0);NfsFrameTiming old[128];for(unsigned i=0;i<8&&nfs_frame_timing_drain(old,128);++i){}
   const auto baseline=nfs_frame_timing_snapshot();start_drop=baseline.dropped_total;start_id=baseline.latest_frame_id;
   origin=SDL_GetTicksNS();last=origin;next=origin+1000000000ull;process=cpu_ns(false);main=cpu_ns(true);
   writer=std::thread([this]{write_loop();});
   put(false,diagnostic_header(0),0);put(true,"frame_id,timestamp_ns,interval_ns,window_epoch\n",0);
   nfs_set_engine_metrics_capture(1);nfs_frame_timing_capture(1);start_epoch=nfs_frame_timing_snapshot().window_epoch;
  }catch(...){if(writer.joinable()){{std::lock_guard<std::mutex>lock(mutex);closing=true;}changed.notify_one();writer.join();}for(unsigned i=0;i<opened;++i){if(*handles[i])std::fclose(*handles[i]);std::remove(paths[i].c_str());}throw;}
 }
 bool put(bool is_frames,std::string text,unsigned count){
  std::lock_guard<std::mutex>lock(mutex);
  if(write_error.load())return false;
  if(text.size()>limits.bytes-4096-accepted_bytes){reason="byte_limit";return false;}
  if(text.size()>limits.queue_bytes-queued_bytes){reason="writer_backlog";return false;}
  accepted_bytes+=text.size();queued_bytes+=text.size();queue.push_back({is_frames,std::move(text),count});changed.notify_one();return true;
 }
 void write_loop() noexcept {
  try {
  for(;;){Work work;{std::unique_lock<std::mutex>lock(mutex);changed.wait(lock,[this]{return closing||!queue.empty();});if(queue.empty()&&closing)break;work=std::move(queue.front());queue.pop_front();queued_bytes-=work.text.size();}
   if(!write_error.load()){FILE*f=work.frames?frames:samples;if(std::fwrite(work.text.data(),1,work.text.size(),f)!=work.text.size())write_error.store(true);else if(work.frames)written_frames.fetch_add(work.count);else written_samples.fetch_add(work.count);}
  }
  if(std::fflush(samples)||std::fflush(frames))write_error.store(true);
  const auto dropped=end_drop>=start_drop?end_drop-start_drop:0;
  const std::string summary="{\"schema\":\"nfsiise-desktop-recording-v1\",\"clock\":\"SDL_GetTicksNS\",\"clock_origin_ns\":"+std::to_string(origin)+",\"frame_scope\":\"swap submissions;not display photons or GPU duration\",\"fps_scope\":\"latest stamped rate window;null after2.5s\",\"game_cpu_scope\":\"original engine-thread 60-submission windows;sample_id identifies repeats;not presented-frame CPU\",\"unsupported_fields\":[\"ram_mb\",\"battery_percent\",\"battery_state\",\"brightness\",\"thermal_state\",\"low_power_mode\",\"app_state\",\"system_core_percent\"],\"end_reason\":\""+(write_error.load()?std::string("write_error"):reason)+"\",\"elapsed_ns\":"+std::to_string(elapsed_end)+",\"samples_enqueued\":"+std::to_string(sample_rows)+",\"samples_written\":"+std::to_string(written_samples.load())+",\"discarded_frames\":"+std::to_string(discarded_frames)+",\"frames_enqueued\":"+std::to_string(frame_rows)+",\"frames_written\":"+std::to_string(written_frames.load())+",\"start_window_epoch\":"+std::to_string(start_epoch)+",\"end_window_epoch\":"+std::to_string(end_epoch)+",\"start_frame_id\":"+std::to_string(start_id)+",\"end_frame_id\":"+std::to_string(end_id)+",\"capture_dropped\":"+std::to_string(dropped)+",\"writer_failed\":"+(write_error.load()?"true":"false")+"}\n";
  if(std::fwrite(summary.data(),1,summary.size(),meta)!=summary.size()||std::fflush(meta))write_error.store(true);
  }catch(...){write_error.store(true);}
 }
 bool drain(bool final=false){NfsFrameTiming rows[128];for(unsigned batch=0;batch<8;++batch){const auto count=nfs_frame_timing_drain(rows,128);if(!count)break;
   if(frames_closed){discarded_frames+=count;if(!final)return false;continue;}
   std::string text;unsigned accepted=0;
   for(unsigned i=0;i<count;++i){if(frame_rows+accepted>=limits.frames){discarded_frames+=count-i;reason="frame_limit";frames_closed=true;break;}const auto&r=rows[i];text+=std::to_string(r.frame_id)+","+std::to_string(r.timestamp_ns)+","+std::to_string(r.interval_ns)+","+std::to_string(r.window_epoch)+"\n";++accepted;}
   if(accepted&&!put(true,std::move(text),accepted)){discarded_frames+=accepted;frames_closed=true;}else frame_rows+=accepted;
   if(frames_closed&&!final)return false;
  }return !frames_closed;}
 void poll(){if(stopped)return;const auto now=SDL_GetTicksNS();if(write_error.load()){stop("write_error");return;}if(!drain()){stop(reason.c_str());return;}if(now<origin||now-origin>=3600000000000ull){stop("time_limit");return;}if(now<next)return;if(sample_rows>=limits.samples){stop("sample_limit");return;}
  DiagnosticSample s;s.elapsed=double(now-origin)/1e9;s.interval=double(now-last)/1e9;const auto snapshot=nfs_frame_timing_snapshot();
  if(snapshot.fps_timestamp_ns&&now>=snapshot.fps_timestamp_ns){s.fps_age=double(now-snapshot.fps_timestamp_ns)/1e9;if(s.fps_age<=2.5)s.fps=snapshot.fps;}
  s.game_cpu_ms=nfs_engine_cpu_sample(&s.game_cpu_id);const int cores=SDL_GetNumLogicalCPUCores();s.logical_cores=cores>0?unsigned(cores):0;s.width=nfs_render_width();s.height=nfs_render_height();s.frame_limit=nfs_frame_limit();const double pc=cpu_ns(false),mc=cpu_ns(true);
  if(pc>=process&&process>=0)s.process_ms=(pc-process)/1e6;if(mc>=main&&main>=0)s.main_ms=(mc-main)/1e6;
  if(s.process_ms>=0&&s.interval>0&&s.logical_cores)s.app_cpu_percent=s.process_ms/(s.interval*10*s.logical_cores);
  process=pc;main=mc;last=now;next=now+1000000000ull;
  std::string row=diagnostic_sample_json(s);
  for(const auto &field: {std::make_pair(std::string("\"low_power_mode\":false"),std::string("\"low_power_mode\":null")),{"\"thermal_state\":-1","\"thermal_state\":null"},{"\"battery_state\":0","\"battery_state\":null"},{"\"app_state\":0","\"app_state\":null"},{"\"system_core_percent\":[]","\"system_core_percent\":null"}}){const auto at=row.find(field.first);if(at!=std::string::npos)row.replace(at,field.first.size(),field.second);}
  if(!put(false,std::move(row),1)){stop(reason.c_str());return;}++sample_rows;
 }
 void stop(const char *why) noexcept {
  if(stopped)return;stopped=true;reason=safe_reason(why);if(reason=="writer_backlog"||reason=="byte_limit"||reason=="write_error")frames_closed=true;nfs_frame_timing_capture(0);nfs_set_engine_metrics_capture(0);
  try{drain(true);const auto final=nfs_frame_timing_snapshot();end_drop=final.dropped_total;end_id=final.latest_frame_id;end_epoch=final.window_epoch;const auto now=SDL_GetTicksNS();elapsed_end=now>=origin?now-origin:0;put(false,diagnostic_footer(reason,double(elapsed_end)/1e9,sample_rows),0);}catch(...){write_error.store(true);}
  {std::lock_guard<std::mutex>lock(mutex);closing=true;}changed.notify_one();if(writer.joinable())writer.join();
 }
 ~Impl(){stop("session_end");for(FILE*f:{samples,frames,meta})if(f)std::fclose(f);}
};
DesktopDiagnosticRecorder::DesktopDiagnosticRecorder(const std::string&p):DesktopDiagnosticRecorder(p,Limits{}){}
DesktopDiagnosticRecorder::DesktopDiagnosticRecorder(const std::string&p,Limits l):impl(new Impl(p,l)){}
DesktopDiagnosticRecorder::~DesktopDiagnosticRecorder()=default;
void DesktopDiagnosticRecorder::poll(){impl->poll();}
void DesktopDiagnosticRecorder::stop(const char*r)noexcept{impl->stop(r);}
bool DesktopDiagnosticRecorder::failed()const noexcept{return impl->write_error.load()||impl->reason=="writer_backlog";}
}
