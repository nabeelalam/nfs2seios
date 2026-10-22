// SPDX-License-Identifier: MIT
#pragma once
#include "AudioBackgroundProof.h"
namespace nfs_audio_frame {
class BackgroundReplayVerifier {
 static bool draw(const BackgroundDraw&d,bool sidebar,const Binding&binding,std::vector<uint16_t>&replay)noexcept{
  if(d.complete!=1||d.error||!targetValid(d.before,binding)||!nfs_logo_capture::same(d.before,d.after)||d.sourceBefore!=d.sourceAfter||!d.args[0]||d.args[1]!=(sidebar?0x1000u:0x400u)||d.args[2]||d.args[3]||d.sourceBefore.size()<16||d.sourceBefore.size()>614416||uint64_t(d.args[0])+d.sourceBefore.size()>0x100000000ULL||nfs_small_logo_live::overlap(d.args[0],d.sourceBefore.size(),binding.surface.geometry[8],614416))return false;
  const auto&source=d.sourceBefore;const unsigned width=sidebar?343:640;if(nfs_small_logo_live::le32(source.data())!=0x78||nfs_small_logo_live::le32(source.data()+4)!=(width|(480u<<16))||nfs_small_logo_live::le32(source.data()+8)||nfs_small_logo_live::le32(source.data()+12)||source.size()!=16+width*480*2)return false;
  const uint32_t callback=sidebar?0xf00019cc:0xf0001878;if(d.tableBefore!=d.tableAfter||d.tableBefore[3]!=callback||d.tableBefore[6]||d.tableBefore[8]||(!sidebar&&d.tableBefore[7]!=2))return false;
  // Mode0x1000 consumes raw header/pixels, not a source-window LUT. The
  // actual cache table/handle/bitmap tuple binds that source independently.
  if(sidebar){int32_t selected=int32_t(d.sidebarWord)>>16;if(selected<0||selected>1||!d.sidebarHandle||d.sidebarTableEntry!=d.sidebarHandle||d.sidebarBitmap!=d.args[0])return false;}
  const bool flat=!sidebar&&d.rows.size()==1;if(!flat&&d.rows.size()!=480)return false;
  for(unsigned y=0;y<(flat?1u:480u);++y){const auto&r=d.rows[y];unsigned n=flat?Pixels:width;uint32_t src=d.args[0]+16+y*width*2,dst=binding.surface.geometry[8]+16+y*1280;if(r.callback!=callback||r.args[0]!=src||r.args[1]!=dst||r.args[2]!=n||r.source.size()!=n*2||r.before.size()!=n||r.after.size()!=n||std::memcmp(r.source.data(),source.data()+16+y*width*2,n*2))return false;for(unsigned x=0;x<n;++x){size_t at=flat?x:y*640+x;uint16_t value=nfs_small_logo_live::le16(r.source.data()+x*2),expected=sidebar&&!value?replay[at]:value;if(r.before[x]!=replay[at]||r.after[x]!=expected)return false;replay[at]=expected;}}
  return true;
 }
public:
 static std::shared_ptr<const VerifiedPreBars>verify(const BackgroundRecord&r,const Binding&current,const std::shared_ptr<const BackgroundCatalog>&catalog)noexcept{
  if(!current.valid()||!(r.binding==current)||!catalog||r.complete!=1||r.error||r.preReset.size()!=Pixels||r.postReset.size()!=Pixels||r.preBars.size()!=Pixels||r.reset.sourceBefore!=catalog->source())return {};
  try{auto replay=r.preReset;if(!draw(r.reset,false,current,replay)||replay!=r.postReset||!draw(r.sidebar,true,current,replay)||replay!=r.preBars)return {};auto p=std::shared_ptr<VerifiedPreBars>(new VerifiedPreBars);p->binding_=current;p->catalog_=catalog;p->pixels_=std::move(replay);return p;}catch(...){return {};}
 }
};
}
