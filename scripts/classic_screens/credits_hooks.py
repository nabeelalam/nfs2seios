"""Prepare exact Credits hardware-text hooks. No compiler, game or assets."""
from pathlib import Path
import argparse,json,hashlib
H=lambda b:hashlib.sha256(b).hexdigest()
def span(text,name):
 a=text.index('Fn(void) Game::_sub_'+name+'()');z=text.find('Fn(void) Game::',a+1);return a,len(text)if z<0 else z
def entry(text,name,body,tag):
 a,z=span(text,name);f=text[a:z];at=f.index('{\n')+2;f=f[:at]+'// NFS_CREDITS_UI_BEGIN:'+tag+'\n'+body+'\n// NFS_CREDITS_UI_END:'+tag+'\n'+f[at:];return text[:a]+f+text[z:]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output-dir',type=Path,required=True);a=ap.parse_args();assert a.source_dir.is_dir()and not a.output_dir.exists();a.output_dir.mkdir(parents=True)
 routes={'40A0B0':('nfs_credits_ui_live::OwnerScope nfs_credits_owner(*this);','owner'),'427B30':('nfs_credits_ui_live::BackdropScope nfs_credits_backdrop(*this);','backdrop'),'4976B8':('nfs_credits_ui_live::SwapScope nfs_credits_swap(*this);','swap'),'497714':('nfs_credits_ui_live::ClipScope nfs_credits_clip(*this);','clip'),'405CF0':('nfs_credits_ui_live::StringScope nfs_credits_string(*this);','string')};found=set();rows=[]
 for source in sorted(a.source_dir.glob('Methods_*.cpp')):
  raw=source.read_bytes();assert len(raw)<16*1024*1024;text=raw.decode().replace(chr(13)+chr(10),chr(10));tags=[]
  for name,(body,tag)in routes.items():
   if 'Fn(void) Game::_sub_'+name+'()'in text:assert name not in found;found.add(name);text=entry(text,name,body,tag);tags.append(tag)
  if 'string'in tags:
   start,end=span(text,'405CF0');f=text[start:end];needle='loc_426C27:\n';assert f.count(needle)==1;f=f.replace(needle,needle+'// NFS_CREDITS_UI_BEGIN:hardware-end\n nfs_credits_ui_live::hardwareEnd(*this,int32_t(edi));\n// NFS_CREDITS_UI_END:hardware-end\n');text=text[:start]+f+text[end:];tags.append('hardware-end')
  if not tags:continue
  needle='#include "FunctionIds.h"';assert text.count(needle)==1;text=text.replace(needle,needle+'\n// NFS_CREDITS_UI_BEGIN:include\n#include "hd/ClassicScreens/CreditsGameHooks.inc"\n// NFS_CREDITS_UI_END:include',1);out=a.output_dir/source.name;out.write_text(text);rows.append({'unit':source.name,'sourceSHA256':H(raw),'outputSHA256':H(out.read_bytes()),'insertions':tags,'lineEndingsNormalized':True})
 assert found==set(routes),str(set(routes)-found);(a.output_dir/'CREDITS-SOURCE-INSERTIONS.json').write_text(json.dumps({'scope':'Dedicated Credits owner and original hardware text only; original calls/content/timing retained','compilerExecuted':False,'gameExecuted':False,'units':rows},indent=2)+'\n');print(json.dumps({'preparedUnits':len(rows),'functions':sorted(found)}))
if __name__=='__main__':main()
