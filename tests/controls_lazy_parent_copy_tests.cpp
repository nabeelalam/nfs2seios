// SPDX-License-Identifier: MIT
// Synthetic source-only tests. Not wired into CMake or executed by author.
#include "hd/ControlsLazyParentCopyReplay.h"
#include <cassert>
#include <type_traits>
using namespace nfs_controls_lazy_parent_copy;
static_assert(!std::is_constructible<VerifiedUpdate,const Record&>::value,"No raw update constructor");
static nfs_logo_capture::Target target(uint32_t width,uint32_t owner,uint32_t bitmap,uint32_t y,uint32_t x){
 nfs_logo_capture::Target t;t.owner=owner;t.w[1]=width;t.w[2]=480;t.w[5]=width;t.w[6]=480;t.w[7]=0x7810;t.w[8]=bitmap;t.w[10]=width*2;t.w[11]=y;t.w[12]=x;t.header[0]=0x78;t.header[1]=width|(480u<<16);
 for(unsigned i=0;i<width;++i)t.x[i]=2*i;for(unsigned i=0;i<480;++i)t.y[i]=16+i*width*2;t.ownerWords=t.w;return t;
}
struct Fixture {
 Current c;Record r;
 Fixture(int placement=0,uint32_t clipLeft=0){
  c.binding={1,2,3,4,5,6,7,8,9,0x1000,0x100000,0x2000};
  c.sidebar=target(343,0x1000,0x100000,0x400000,0x410000);c.main=target(640,0x2000,0x200000,0x420000,0x410000);
  c.sidebar.ownerState={0,0x1000,0x3000,0x2000,0};c.main.ownerState=c.sidebar.ownerState;
  c.sidebar.header[3]=uint32_t(placement)&4095;c.main.w[3]=clipLeft;
  c.sidebarBytes.resize(SourceBytes);for(unsigned j=0;j<4;++j)for(unsigned k=0;k<4;++k)c.sidebarBytes[j*4+k]=uint8_t(c.sidebar.header[j]>>(8*k));
  for(unsigned i=16;i<SourceBytes;++i)c.sidebarBytes[i]=uint8_t(i*17);
  r.bindingBefore=r.bindingAfter=c.binding;r.complete=1;r.site=0x40bbe0;r.mode=0x2800;r.originalArgs={0x100000,0x2800,0,0};r.packetAccepted=1;r.copyBegin=2;r.resolveBegin=3;
  r.sourceBefore=r.sourceAfter=c.sidebar;r.mainBefore=r.mainAfter=c.main;r.sidebarBefore=r.sidebarAfter=c.sidebarBytes;r.mainPlaneBefore.resize(PlaneBytes,55);r.mainPlaneAfter=r.mainPlaneBefore;
  r.resolved={placement,0,std::max(placement,int(clipLeft)),0,std::min(placement+343,640),480};
  uint64_t seq=4;const unsigned n=unsigned(r.resolved.right-r.resolved.left),bytes=2*n;
  for(unsigned y=0;y<480;++y){Row row;row.dispatchBegin=seq++;row.copyBegin=seq++;row.copyEnd=seq++;row.dispatchEnd=seq++;row.dispatch=DispatchId;row.callback=CopyId;
   const unsigned so=16+(y*343+unsigned(r.resolved.left-placement))*2,po=(y*640+unsigned(r.resolved.left))*2;
   row.dispatchArgs={c.sidebar.w[8]+so,c.main.w[8]+16+po,n,123};row.copyArgs={row.dispatchArgs[0],row.dispatchArgs[1],bytes,123};row.copyReturn={row.copyArgs[1]+bytes,row.copyArgs[0]+bytes,0,123};row.dispatchReturn=row.copyReturn;
   row.sourceBefore.assign(c.sidebarBytes.begin()+so,c.sidebarBytes.begin()+so+bytes);row.sourceAfter=row.sourceBefore;row.destinationBefore.assign(bytes,55);row.destinationAfter=row.sourceBefore;
   std::copy(row.sourceBefore.begin(),row.sourceBefore.end(),r.mainPlaneAfter.begin()+po);r.rows.push_back(row);
  }r.resolveEnd=seq++;r.copyEnd=seq;
 }
 bool ok()const{return ReplayVerifier::verify(r,c).failure==Failure::None;}
};
static void word(std::vector<uint8_t>&v,size_t at,uint16_t x){v[at]=uint8_t(x);v[at+1]=uint8_t(x>>8);}
static void dword(std::vector<uint8_t>&v,size_t at,uint32_t x){for(unsigned k=0;k<4;++k)v[at+k]=uint8_t(x>>(k*8));}
struct KeyedFixture:Fixture {
 explicit KeyedFixture(int placement=0,uint32_t clipLeft=0,uint32_t key=0x2110):Fixture(placement,clipLeft){
  r.rowKind=RowKind::Keyed;r.keyBefore=r.keyAfter=key;r.mainPlaneAfter=r.mainPlaneBefore;
  uint64_t seq=r.resolveBegin+1;
  for(size_t i=0;i<r.rows.size();++i){const auto&old=r.rows[i];KeyedRow v;
   v.rowBegin=seq++;v.transformBegin=seq++;v.transformEnd=seq++;v.forwardBegin=seq++;v.forwardEnd=seq++;v.rowEnd=seq++;
   v.wrapper=nfs_controls_lazy_parent_keyed_transform::WrapperId;v.transform=nfs_controls_lazy_parent_keyed_transform::TransformId;v.forward=nfs_controls_lazy_parent_keyed_transform::ForwardId;v.keyBefore=v.keyAfter=key;
   v.entry=old.dispatchArgs;v.temporaryAddress=v.allocationBase=0x800000;v.allocationBytes=0x1400;v.wrapperEsp=v.temporaryAddress+0x1408;
   const uint32_t n=v.entry[2];v.transformArgs={v.entry[0],v.temporaryAddress,n,n};v.forwardArgs={v.temporaryAddress,v.entry[1],n,n};
   v.sourceBefore=v.sourceAfter=old.sourceBefore;v.destinationBefore=old.destinationBefore;v.destinationAfter=v.destinationBefore;v.temporaryBeforeForward.resize(n*4);
   for(unsigned x=0;x<n;++x){const uint32_t pixel=uint32_t(v.sourceBefore[x*2])|(uint32_t(v.sourceBefore[x*2+1])<<8);
    const uint32_t red=pixel/2048,green=(pixel/32)%64,blue=pixel%32;
    dword(v.temporaryBeforeForward,x*4,pixel==key?0:0xff000000u+red*0x80000u+green*0x400u+blue*8u);
    if(pixel!=key)word(v.destinationAfter,x*2,uint16_t(pixel));
   }
   v.temporaryAfterForward=v.temporaryBeforeForward;
   const size_t po=v.entry[1]-c.main.w[8]-16;
   std::copy(v.destinationAfter.begin(),v.destinationAfter.end(),r.mainPlaneAfter.begin()+po);r.keyedRows.push_back(v);
  }
  r.rows.clear();r.resolveEnd=seq++;r.copyEnd=seq;
 }
};
int main(){
 {Fixture f;assert(f.ok());auto result=ReplayVerifier::verify(f.r,f.c);auto expected=result.update->afterPlane()[0];f.r.mainPlaneAfter[0]^=1;assert(result.update->afterPlane()[0]==expected);}
 {Fixture f(-167);assert(f.ok());assert(f.r.resolved.right==176);}
 {Fixture f(-167,13);assert(f.ok());assert(f.r.rows[0].copyArgs[0]==0x100000+16+(167+13)*2);}
 // Most copied pixels already agree: changed bounds are only 175 columns,
 // while all 343 columns are genuinely written and must be recorded.
 {Fixture f;for(unsigned y=0;y<480;++y){auto&row=f.r.rows[y];for(unsigned x=175*2;x<343*2;++x){unsigned at=y*1280+x;f.r.mainPlaneBefore[at]=f.r.mainPlaneAfter[at];row.destinationBefore[x]=row.destinationAfter[x];}}assert(f.ok());f.r.rows[0].dispatchArgs[2]=175;assert(!f.ok());}
 {Fixture f;f.r.mainPlaneAfter.back()^=1;assert(!f.ok());}
 {Fixture f;f.r.sidebarAfter[20]^=1;assert(!f.ok());}
 {Fixture f;f.r.rows.pop_back();assert(!f.ok());}
 {Fixture f;std::swap(f.r.rows[0],f.r.rows[1]);assert(!f.ok());}
 {Fixture f;f.r.rows[0].copyArgs[2]/=2;assert(!f.ok());}
 {Fixture f;f.r.rows[0].callback++;assert(!f.ok());}
 {Fixture f;f.r.rows[0].dispatch++;assert(!f.ok());}
 {Fixture f;f.r.rows[0].copyReturn[0]--;assert(!f.ok());}
 {Fixture f;f.r.rows[0].copyReturn[3]++;assert(!f.ok());}
 {Fixture f;f.r.rows[0].sourceAfter[0]^=1;assert(!f.ok());}
 {Fixture f;f.r.rows[0].destinationBefore[0]^=1;assert(!f.ok());}
 {Fixture f;f.r.rows[0].copyBegin=f.r.rows[0].dispatchBegin;assert(!f.ok());}
 {Fixture f;f.r.bindingAfter.cacheRevision++;assert(!f.ok());}
 {Fixture f;f.c.binding.packetSerial++;assert(!f.ok());}
 {Fixture f;f.r.mainAfter.w[5]--;assert(!f.ok());}
 {Fixture f;f.r.resolved.x=1;assert(!f.ok());}
 {Fixture f;f.r.originalArgs[1]=0x1000;assert(!f.ok());}
 {Fixture f;f.r.complete=0;assert(!f.ok());}
 {Fixture f;f.r.error=1;assert(!f.ok());}
 // Aliasing descriptor/LUT tails must fail despite otherwise consistent metadata.
 {Fixture f;f.c.main.w[12]=f.c.sidebar.w[8]+SourceBytes-4;f.c.main.ownerWords=f.c.main.w;f.r.mainBefore=f.r.mainAfter=f.c.main;assert(!f.ok());}
 {Fixture f;f.c.sidebar.owner=f.c.main.w[8]+100;f.c.binding.sourceOwner=f.c.sidebar.owner;f.c.sidebar.ownerState[1]=f.c.sidebar.owner;f.r.bindingBefore=f.r.bindingAfter=f.c.binding;f.r.sourceBefore=f.r.sourceAfter=f.c.sidebar;assert(!f.ok());}
 // Distinct original transform/forward route, including preserved transparent pixels.
 {KeyedFixture f;assert(f.ok());assert(f.r.mainPlaneAfter[0]==55&&f.r.mainPlaneAfter[1]==55);auto v=ReplayVerifier::verify(f.r,f.c);assert(v.update->rowCount()==480);}
 {KeyedFixture f(-167,13);assert(f.ok());assert(f.r.keyedRows[0].entry[0]==0x100000+16+(167+13)*2);}
 {KeyedFixture f(17,20,0);assert(f.ok());}
 {KeyedFixture f(0,0,0x10000);assert(f.ok());}
 {KeyedFixture f;f.r.rowKind=RowKind::Direct;assert(!f.ok());}
 {KeyedFixture f;f.r.rowKind=static_cast<RowKind>(99);assert(!f.ok());}
 {Fixture f;f.r.keyedRows.emplace_back();assert(!f.ok());}
 {KeyedFixture f;f.r.rows.emplace_back();assert(!f.ok());}
 {KeyedFixture f;f.r.keyAfter++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].keyBefore++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].keyAfter++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].wrapper++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].transform++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].forward++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].entry[0]++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].entry[1]++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].entry[2]--;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].transformArgs[3]--;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].forwardArgs[0]++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].forwardArgs[1]++;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].allocationBytes--;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].wrapperEsp--;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].wrapperEsp=0x1407;assert(!f.ok());}
 // Preserve a self-consistent temp address but put its allocation across metadata.
 {KeyedFixture f;auto&v=f.r.keyedRows[0];v.temporaryAddress=v.allocationBase=f.c.main.w[12]-4;v.wrapperEsp=v.temporaryAddress+0x1408;v.transformArgs[1]=v.forwardArgs[0]=v.temporaryAddress;assert(!f.ok());}
 {KeyedFixture f;auto&v=f.r.keyedRows[0];v.temporaryAddress=v.allocationBase=f.c.main.w[8];v.wrapperEsp=v.temporaryAddress+0x1408;v.transformArgs[1]=v.forwardArgs[0]=v.temporaryAddress;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].temporaryBeforeForward.back()^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].temporaryBeforeForward.pop_back();assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].temporaryAfterForward.back()^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].sourceBefore.back()^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].sourceAfter.back()^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].destinationBefore[0]^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].destinationAfter[0]^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].transformBegin=f.r.keyedRows[0].rowBegin;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].transformEnd=f.r.keyedRows[0].forwardBegin;assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows[0].forwardEnd=f.r.keyedRows[0].rowEnd;assert(!f.ok());}
 {KeyedFixture f;std::swap(f.r.keyedRows[0],f.r.keyedRows[1]);assert(!f.ok());}
 {KeyedFixture f;f.r.keyedRows.pop_back();assert(!f.ok());}
 {KeyedFixture f;f.r.mainPlaneAfter.back()^=1;assert(!f.ok());}
 {KeyedFixture f;f.r.sidebarAfter.back()^=1;assert(!f.ok());}

}
