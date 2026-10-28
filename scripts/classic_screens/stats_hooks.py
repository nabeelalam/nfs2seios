"""Prepare Stats UI source insertions only; no compiler, game or assets."""
from pathlib import Path
import argparse,re,json,hashlib
H=lambda b:hashlib.sha256(b).hexdigest()

def function_span(text,name):
 start=text.index('Fn(void) Game::_sub_'+name+'()');end=text.find('Fn(void) Game::',start+1)
 return start,len(text)if end<0 else end

def entry(text,name,body,tag):
 start,end=function_span(text,name);f=text[start:end];needle='{\n';at=f.index(needle)+len(needle);f=f[:at]+'// NFS_STATS_UI_BEGIN:'+tag+'\n'+body+'\n// NFS_STATS_UI_END:'+tag+'\n'+f[at:];return text[:start]+f+text[end:]

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output-dir',type=Path,required=True);a=ap.parse_args();assert a.source_dir.is_dir()and not a.output_dir.exists();a.output_dir.mkdir(parents=True)
 routes={'426050':('nfs_stats_ui_live::OwnerScope nfs_stats_owner(*this);','owner'),'425130':('nfs_stats_ui_live::GraphScope nfs_stats_graph(*this);','graph'),'4967E0':('nfs_stats_ui_live::line(*this);','line'),'488620':('nfs_stats_ui_live::rectangle(*this);','rectangle'),'405B30':('nfs_stats_ui_live::DotScope nfs_stats_dot(*this);','dot'),'405CF0':('nfs_stats_ui_live::StringScope nfs_stats_string(*this);','text')}
 found=set();rows=[]
 for source in sorted(a.source_dir.glob('Methods_*.cpp')):
  raw=source.read_bytes();assert len(raw)<16*1024*1024;text=raw.decode().replace(chr(13)+chr(10),chr(10));original=text;tags=[]
  for name,(body,tag)in routes.items():
   if 'Fn(void) Game::_sub_'+name+'()'in text:assert name not in found;found.add(name);text=entry(text,name,body,tag);tags.append(tag)
  if '426050'in found and 'Fn(void) Game::_sub_426050()'in text:
   start,end=function_span(text,'426050');f=text[start:end]
   begin='\tesp -= 4; _sub_405B30(); esp += 4; //call\n\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n\tdl = guest_transaction.load<int8_t>(byte_512DFB);'
   assert f.count(begin)==1;f=f.replace(begin,begin.replace('\n\t{','\n// NFS_STATS_UI_BEGIN:frame-begin\n nfs_stats_ui_live::beginFrame(*this);\n// NFS_STATS_UI_END:frame-begin\n\t{',1));call='\tesp -= 4; _sub_426B70(); esp += 4; //call';assert f.count(call)==1;tail='\tesp -= 4; _sub_4274F0(); esp += 4; //call';at=f.index(call);stop=f.index(tail,at)+len(tail);region=f[at:stop];assert region.count(tail)==1 and 'goto ' not in region
   region=region.replace(call,call+'\n// NFS_STATS_UI_BEGIN:frame-stage\n nfs_stats_draw.stage();\n// NFS_STATS_UI_END:frame-stage',1)
   f=f[:at]+'// NFS_STATS_UI_BEGIN:frame-draw-open\n { nfs_stats_ui_live::DrawScope nfs_stats_draw(*this);\n// NFS_STATS_UI_END:frame-draw-open\n'+region+'\n// NFS_STATS_UI_BEGIN:frame-draw-close\n }\n// NFS_STATS_UI_END:frame-draw-close'+f[stop:];text=text[:start]+f+text[end:];tags+=['frame-begin','frame-draw-open','frame-draw-close']
  if not tags:continue
  needle='#include "FunctionIds.h"';assert text.count(needle)==1;include='hd/ClassicScreens/StatsQueueAdapter.inc'if 'owner'in tags else'hd/ClassicScreens/StatsGameHooks.inc';text=text.replace(needle,needle+'\n// NFS_STATS_UI_BEGIN:include\n#include "'+include+'"\n// NFS_STATS_UI_END:include',1)
  output=a.output_dir/source.name;output.write_text(text);rows.append({'unit':source.name,'sourceSha256':H(raw),'outputSha256':H(output.read_bytes()),'insertions':tags,'newlineNormalized':True})
 assert found==set(routes),str(set(routes)-found)
 (a.output_dir/'STATS-SOURCE-INSERTIONS.json').write_text(json.dumps({'scope':'Stats display hooks only; original calls/operands retained','compilerExecuted':False,'gameExecuted':False,'units':rows},indent=2)+'\n');print(json.dumps({'preparedUnits':len(rows),'functions':sorted(found)}))
if __name__=='__main__':main()
