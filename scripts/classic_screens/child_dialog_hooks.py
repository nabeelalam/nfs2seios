"""Stage dedicated LoadSave child hooks; does not execute game/compiler."""
from pathlib import Path
import argparse,hashlib,json

def span(t,n):
 a=t.index('Fn(void) Game::_sub_'+n+'()');z=t.find('Fn(void) Game::',a+1);return a,len(t)if z<0 else z

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output-dir',type=Path,required=True);a=ap.parse_args();assert a.source_dir.is_dir()and not a.output_dir.exists();a.output_dir.mkdir(parents=True)
 routes={'40938C':'nfs_ls_child_live::OwnerScope nfs_ls_child_owner(*this);','401090':'nfs_ls_child_live::ChildScope nfs_ls_child_scope(*this,0x401090);','401AE0':'nfs_ls_child_live::ChildScope nfs_ls_child_scope(*this,0x401ae0);','402460':'nfs_ls_child_live::ChildScope nfs_ls_child_scope(*this,0x402460);','402CD0':'nfs_ls_child_live::ChildScope nfs_ls_child_scope(*this,0x402cd0);','405CF0':'nfs_ls_child_live::StringScope nfs_ls_child_string(*this);','426950':'nfs_ls_child_live::PresentScope nfs_ls_child_present(*this);'}
 found=set();rows=[]
 for f in sorted(a.source_dir.glob('Methods_*.cpp')):
  original=f.read_bytes();t=original.decode().replace('\r\n','\n');tags=[]
  for name,body in routes.items():
   if 'Fn(void) Game::_sub_'+name+'()'not in t:continue
   assert name not in found;found.add(name);s,z=span(t,name);part=t[s:z];at=part.index('{\n')+2;part=part[:at]+'// NFS_LOADSAVE_CHILD_UI_BEGIN:'+name+'\n '+body+'\n// NFS_LOADSAVE_CHILD_UI_END:'+name+'\n'+part[at:];t=t[:s]+part+t[z:];tags.append(name)
  if not tags:continue
  needle='#include "FunctionIds.h"';assert t.count(needle)==1;t=t.replace(needle,needle+'\n#include "hd/ClassicScreens/ChildDialogGameHooks.inc"',1);out=a.output_dir/f.name;out.write_text(t);rows.append({'unit':f.name,'hooks':tags,'sourceSHA256':hashlib.sha256(original).hexdigest(),'stagedSHA256':hashlib.sha256(out.read_bytes()).hexdigest()})
 assert found==set(routes),str(set(routes)-found)
 (a.output_dir/'LOADSAVE-CHILD-SOURCE-INSERTIONS.json').write_text(json.dumps({'gameExecuted':False,'compilerExecuted':False,'units':rows},indent=2)+'\n');print(json.dumps({'preparedUnits':len(rows),'hooks':sorted(found)}))
if __name__=='__main__':main()
