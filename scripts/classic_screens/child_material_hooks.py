"""Append exact original art/selection geometry hooks after child-dialog staging."""
from pathlib import Path
import argparse,json,hashlib

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output-dir',type=Path,required=True);a=ap.parse_args();assert a.source_dir.is_dir()and not a.output_dir.exists();a.output_dir.mkdir(parents=True)
 routes={'402E80':'nfs_ls_child_live::ModalScope nfs_ls_child_modal(*this);','4054E0':'nfs_ls_child_live::PolicyScope nfs_ls_child_policy(*this);','482EA8':'nfs_ls_child_live::NamedScope nfs_ls_child_named(*this);','482EB4':'nfs_ls_child_live::NamedScope nfs_ls_child_named(*this);','48330C':'nfs_ls_child_live::ArtScope nfs_ls_child_art(*this,false);','482C78':'nfs_ls_child_live::ArtScope nfs_ls_child_art(*this,true);'};found=set();rows=[]
 for f in sorted(a.source_dir.glob('Methods_*.cpp')):
  raw=f.read_bytes();t=raw.decode().replace('\r\n','\n');tags=[]
  for name,body in routes.items():
   key='Fn(void) Game::_sub_'+name+'()'
   if key not in t:continue
   assert name not in found;found.add(name);begin=t.index(key);at=t.index('{\n',begin)+2;t=t[:at]+'// NFS_LOADSAVE_CHILD_MATERIAL_BEGIN:'+name+'\n '+body+'\n// NFS_LOADSAVE_CHILD_MATERIAL_END:'+name+'\n'+t[at:];tags.append(name)
  if not tags:continue
  if '#include "hd/ClassicScreens/ChildDialogGameHooks.inc"'not in t:
   key='#include "FunctionIds.h"';assert t.count(key)==1;t=t.replace(key,key+'\n#include "hd/ClassicScreens/ChildDialogGameHooks.inc"',1)
  out=a.output_dir/f.name;out.write_text(t);rows.append({'unit':f.name,'hooks':tags,'sourceSHA256':hashlib.sha256(raw).hexdigest(),'stagedSHA256':hashlib.sha256(out.read_bytes()).hexdigest()})
 assert found==set(routes),str(set(routes)-found)
 (a.output_dir/'LOADSAVE-CHILD-MATERIAL-INSERTIONS.json').write_text(json.dumps({'gameExecuted':False,'compilerExecuted':False,'units':rows},indent=2)+'\n');print(json.dumps({'preparedUnits':len(rows),'hooks':sorted(found)}))
if __name__=='__main__':main()
