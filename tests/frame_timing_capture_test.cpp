// SPDX-License-Identifier: MIT
#include "FrameTimingCapture.h"
#include <cassert>
int main(){
 nfs::FrameTimingCapture c;NfsFrameTiming rows[1024]{};
 assert(!c.snapshot().enabled);c.presented(10);assert(c.drain(rows,1024)==0);
 c.published(100,59.5);auto s=c.snapshot();assert(s.fps_timestamp_ns==100&&s.fps==59.5);
 c.capture(true);c.presented(200);c.presented(220);
 assert(c.drain(nullptr,100)==0&&c.drain(rows,0)==0);
 assert(c.drain(rows,1)==1&&rows[0].frame_id==1&&rows[0].interval_ns==0);
 c.reset();s=c.snapshot();assert(s.fps_timestamp_ns==0&&s.fps==0);
 c.presented(250);assert(c.drain(rows,1024)==2);
 assert(rows[0].frame_id==2&&rows[0].interval_ns==20);
 assert(rows[1].frame_id==3&&rows[1].interval_ns==0&&rows[1].window_epoch>rows[0].window_epoch);
 c.presented(260);c.capture(false);c.presented(270);s=c.snapshot();assert(!s.enabled&&s.latest_frame_id==4&&s.latest_timestamp_ns==260);
 c.capture(true);c.presented(280);assert(c.drain(rows,1024)==2);
 assert(rows[0].frame_id==4&&rows[1].frame_id==5&&rows[1].interval_ns==0&&rows[1].window_epoch>rows[0].window_epoch);
 for(unsigned i=0;i<1030;++i)c.presented(300+i*10);
 s=c.snapshot();assert(s.dropped_total==6&&s.latest_frame_id==1035);
 assert(c.drain(rows,1024)==1024);assert(rows[0].frame_id==12&&rows[1023].frame_id==1035);
 for(unsigned i=1;i<1024;++i)assert(rows[i].frame_id==rows[i-1].frame_id+1&&rows[i].timestamp_ns>rows[i-1].timestamp_ns&&rows[i].interval_ns==10);
 assert(c.drain(rows,1024)==0);c.presented(1);assert(c.drain(rows,1)==1&&rows[0].interval_ns==0);
 c.capture(false);c.reset();s=c.snapshot();assert(!s.enabled&&s.fps==0&&s.fps_timestamp_ns==0&&s.dropped_total==6);
}
