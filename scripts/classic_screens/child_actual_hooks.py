"""Append exact original art/selection geometry hooks after child-dialog staging."""
from pathlib import Path
import argparse,json,hashlib

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output-dir',type=Path,required=True);a=ap.parse_args();assert a.source_dir.is_dir()and not a.output_dir.exists();a.output_dir.mkdir(parents=True)
 routes={'428A20':'nfs_ls_child_live::DrawStartScope nfs_ls_child_draw_start(*this);','4976B8':'nfs_ls_child_live::BeforeNativeScope nfs_ls_child_preswap(*this);','497714':'nfs_ls_child_live::ClipScope nfs_ls_child_clip(*this);','4267C0':'nfs_ls_child_live::BeforeNativeScope nfs_ls_child_precursor(*this);'};found=set();rows=[]
 for f in sorted(a.source_dir.glob('Methods_*.cpp')):
  raw=f.read_bytes();t=raw.decode().replace('\r\n','\n');tags=[]
  for name,body in routes.items():
   key='Fn(void) Game::_sub_'+name+'()'
   if key not in t:continue
   assert name not in found;found.add(name);begin=t.index(key);at=t.index('{\n',begin)+2;t=t[:at]+'// NFS_LOADSAVE_CHILD_ACTUAL_BEGIN:'+name+'\n '+body+'\n// NFS_LOADSAVE_CHILD_ACTUAL_END:'+name+'\n'+t[at:];tags.append(name)
  if 'Fn(void) Game::_sub_405CF0()'in t:
   start=t.index('Fn(void) Game::_sub_405CF0()');end=t.find('Fn(void) Game::',start+1);end=len(t)if end<0 else end;part=t[start:end];needle='loc_426C27:\n';assert part.count(needle)==1;part=part.replace(needle,needle+'// NFS_LOADSAVE_CHILD_ACTUAL_BEGIN:hardware-end\n nfs_ls_child_live::hardwareEnd(*this,int32_t(edi));\n// NFS_LOADSAVE_CHILD_ACTUAL_END:hardware-end\n');t=t[:start]+part+t[end:];tags.append('hardware-end')
  if 'Fn(void) Game::_sub_401090()'in t:
   start=t.index('Fn(void) Game::_sub_401090()');end=t.find('Fn(void) Game::',start+1);end=len(t)if end<0 else end;part=t[start:end];needle='loc_401564:\n';assert part.count(needle)==1;part=part.replace(needle,needle+'// NFS_LOADSAVE_CHILD_ACTUAL_BEGIN:input-ready\n nfs_ls_child_live::inputReady(*this);\n// NFS_LOADSAVE_CHILD_ACTUAL_END:input-ready\n');mouse='\tesp -= 4; _sub_423B90(); esp += 4; //call\n';key='\tesp -= 4; _sub_405960(); esp += 4; //call\n';assert part.count(mouse)==1 and part.count(key)==1;part=part.replace(mouse,' nfs_ls_child_live::observeInput(*this,0);\n'+mouse+' nfs_ls_child_live::observeInput(*this,1);\n',1);part=part.replace(key,' nfs_ls_child_live::observeInput(*this,2);\n'+key+' nfs_ls_child_live::observeInput(*this,3);\n',1);escape=part.index('loc_4019CB:');audio=part.index('\tesp -= 4; _sub_4423F0(); esp += 4; //call\n',escape);tail=audio+len('\tesp -= 4; _sub_4423F0(); esp += 4; //call\n');part=part[:audio]+' nfs_ls_child_live::observeInput(*this,4);\n'+part[audio:tail]+' nfs_ls_child_live::observeInput(*this,5);\n'+part[tail:];t=t[:start]+part+t[end:];tags.append('input-ready')
  if not tags:continue
  if '#include "hd/ClassicScreens/ChildDialogGameHooks.inc"'not in t:
   key='#include "FunctionIds.h"';assert t.count(key)==1;t=t.replace(key,key+'\n#include "hd/ClassicScreens/ChildDialogGameHooks.inc"',1)
  out=a.output_dir/f.name;out.write_text(t);rows.append({'unit':f.name,'hooks':tags,'sourceSHA256':hashlib.sha256(raw).hexdigest(),'stagedSHA256':hashlib.sha256(out.read_bytes()).hexdigest()})
 assert found==set(routes),str(set(routes)-found)
 (a.output_dir/'LOADSAVE-CHILD-ACTUAL-INSERTIONS.json').write_text(json.dumps({'gameExecuted':False,'compilerExecuted':False,'units':rows},indent=2)+'\n');print(json.dumps({'preparedUnits':len(rows),'hooks':sorted(found)}))
if __name__=='__main__':main()
