"""Dedicated insertion-only TrackInfo/Records native animated badge observers."""
from pathlib import Path
import argparse,re,json,hashlib
H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--fallback-dir',type=Path);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 assert not a.output.exists();a.output.mkdir(parents=True);rows=[]
 for unit in [1,2,3,10,12]:
  name='Methods_%02d.cpp'%unit;source=a.source_dir/name
  if not source.exists()and a.fallback_dir:source=a.fallback_dir/name
  old=source.read_text();assert 'NFS_PRIVATE_TRACK_BADGE_BEGIN:'not in old; text=old;serial=0
  def block(code):
   nonlocal serial
   serial+=1;return '\n// NFS_PRIVATE_TRACK_BADGE_BEGIN:'+str(serial)+'\n'+code+'\n// NFS_PRIVATE_TRACK_BADGE_END:'+str(serial)+'\n'
  def section(symbol):
   m=re.search(r'Fn\([^)]*\) Game::_sub_'+symbol+r'\(\)\s*\{',text);assert m,symbol;z=text.find('\nFn(',m.end());return m.start(),len(text)if z<0 else z,m.end()
  def entry(symbol,code):
   nonlocal text
   lo,hi,at=section(symbol);text=text[:at]+block(code)+text[at:]
  anchor='#include "FunctionIds.h"';assert text.count(anchor)==1;text=text.replace(anchor,anchor+block('#include "hd/TrackBadgeGameHooks.inc"'),1)
  if unit==1:entry('405B60',' nfs_track_badge_live::resolved(*this);')
  if unit==2:
   entry('419FD0',' nfs_track_badge_live::selected(*this);')
   for symbol,owner in [('41C1B0',1),('41B8D0',2),('41C850',3)]:entry(symbol,' nfs_track_badge_live::OwnerScope nfs_track_badge_owner(*this,'+str(owner)+');')
  if unit==3:
   entry('428140',' nfs_track_badge_live::BuildScope nfs_track_badge_build(*this);')
   entry('4283A0',' nfs_track_badge_live::ConvertScope nfs_track_badge_convert(*this);')
   entry('4274F0',' nfs_track_badge_live::DrawScope nfs_track_badge_draw(*this);')
  if unit==10:
   entry('486C60',' nfs_track_badge_live::StageScope nfs_track_badge_stage(*this);')
   entry('482DA0',' nfs_track_badge_live::retired(*this);')
  if unit==12:
   entry('49755C',' nfs_track_badge_live::UploadScope nfs_track_badge_upload(*this);')
   entry('497D6C',' nfs_track_badge_live::MeshScope nfs_track_badge_mesh(*this);')
   lo,hi,at=section('497D6C');part=text[lo:hi];call='\teax = grDrawTriangle(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8)); esp += 12;';assert part.count(call)==2
   part=part.replace(call,block(' nfs_track_badge_live::quad(*this);')+call,1);text=text[:lo]+part+text[hi:]
  reverse=re.sub(r'\n// NFS_PRIVATE_TRACK_BADGE_BEGIN:(\d+)\n.*?\n// NFS_PRIVATE_TRACK_BADGE_END:\1\n','',text,flags=re.S);assert reverse==old
  dest=a.output/name;dest.write_text(text);rows.append(dict(unit=name,source=str(source),sourceSHA256=H(source),output=str(dest),outputSHA256=H(dest),insertions=serial,originalOperationsPreserved=True))
 manifest=dict(scope='Dedicated rec6 native TrackInfo/Records badge source/staging/alpha200/upload and actual quad observers; rec7 catalogue only, no consumer admission',units=rows,rendererHooksRequired=['created','retired','upload_begin','upload_done','select','triangle'],renderPixelOrDeviceAcceptance=False,artworkPinMustRemainEmptyUntilPrivateValidation=True)
 (a.output/'TRACK_BADGE-SOURCE-INSERTIONS.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(dict(passed=True,units=[r['unit']for r in rows],output=str(a.output))))
if __name__=='__main__':main()
