"""Private insertion-only Records integration; never compiles or starts a game."""
from pathlib import Path
import argparse,re,json,hashlib
O=Path(__file__).resolve().parent
H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--fallback-dir',type=Path);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 assert not a.output.exists();a.output.mkdir(parents=True);rows=[]
 for unit in [1,2,3,10,13]:
  name='Methods_%02d.cpp'%unit;source=a.source_dir/name
  if not source.exists()and a.fallback_dir:source=a.fallback_dir/name
  old=source.read_text();text=old;serial=0
  def block(code):
   nonlocal serial
   serial+=1;return '\n// NFS_PRIVATE_RECORDS_UI_BEGIN:'+str(serial)+'\n'+code+'\n// NFS_PRIVATE_RECORDS_UI_END:'+str(serial)+'\n'
  def section(symbol):
   m=re.search(r'Fn\([^)]*\) Game::_sub_'+symbol+r'\(\)\s*\{',text);assert m,symbol;z=text.find('\nFn(',m.end());return m.start(),len(text)if z<0 else z,m.end()
  def entry(symbol,code):
   nonlocal text
   lo,hi,at=section(symbol);text=text[:at]+block(code)+text[at:]
  def edit(symbol,fn):
   nonlocal text
   lo,hi,at=section(symbol);text=text[:lo]+fn(text[lo:hi])+text[hi:]
  anchor='#include "FunctionIds.h"';assert text.count(anchor)==1;text=text.replace(anchor,anchor+block('#include "'+('hd/ClassicScreens/RecordsQueueAdapter.inc'if unit in (2,3) else'hd/ClassicScreens/RecordsGameHooks.inc')+'"'),1)
  if unit==1:
   entry('405CF0',' nfs_records_ui_live::StringScope nfs_records_string(*this);')
   for symbol in ['405580','4054E0']:entry(symbol,' nfs_records_ui_live::ResourceScope nfs_records_resource(*this);')
   def measures(part):
    call='\tesp -= 4; _sub_482424(); esp += 4; //call';assert part.count(call)==2;pieces=part.split(call)
    return pieces[0]+''.join(call+block(' nfs_records_ui_live::measured(*this,uint32_t(ecx),int32_t(eax),'+str(2 if i==0 else 1)+');')+tail for i,tail in enumerate(pieces[1:]))
   edit('405580',measures)
  elif unit==2:
   entry('41C850',' nfs_records_ui_live::OwnerScope nfs_records_owner(*this);')
   def records(part):
    start=part.index('loc_41CA0F:\n');end=part.index('loc_41CA75:\n',start);call='\tesp -= 4; _sub_405B30(); esp += 4; //call';region=part[start:end];assert region.count(call)==2;at=start+region.rindex(call)+len(call);part=part[:at]+block(' nfs_records_ui_live::Adapter::begin(*this);')+part[at:]
    call='\tesp -= 4; _sub_40FA90(); esp += 4; //call';assert part.count(call)==1;part=part.replace(call,call+block(' nfs_records_ui_live::Adapter::end(*this);'))
    call='\tesp -= 4; _sub_426B70(); esp += 4; //call';tail='\tesp -= 4; _sub_4274F0(); esp += 4; //call';assert part.count(call)==1
    at=part.index(call);end=part.index(tail,at)+len(tail);region=part[at:end];assert region.count(tail)==1 and 'goto ' not in region
    region=region.replace(call,call+block(' nfs_records_native.stage();'),1)
    return part[:at]+block(' { nfs_records_ui_live::Presentation nfs_records_present(*this);\n nfs_records_ui_live::DrawScope nfs_records_native(*this);')+region+block(' }')+part[end:]
   edit('41C850',records)
  elif unit==3:pass # Records DrawScope now owns the complete handler presentation tail.
  elif unit==10:
   for symbol in ['482DA0','4848FC']:entry(symbol,' nfs_records_ui_live::retire(*this,uint32_t(eax));')
  elif unit==13:
   def stores(part):
    call='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov';assert part.count(call)==4
    return part.replace(call,block(' { nfs_records_ui_live::Word nfs_records_word(*this,uint32_t(esi),uint16_t(bx));')+call+block(' }'))
   edit('4A5830',stores)
  reverse=re.sub(r'\n// NFS_PRIVATE_RECORDS_UI_BEGIN:(\d+)\n.*?\n// NFS_PRIVATE_RECORDS_UI_END:\1\n','',text,flags=re.S);assert reverse==old
  dest=a.output/name;dest.write_text(text);rows.append(dict(unit=name,source=str(source),sourceSHA256=H(source),output=str(dest),outputSHA256=H(dest),insertions=serial,originalOperationsPreserved=True))
 manifest=dict(scope='Private complete Records text integration; no compile/game; original values/operations preserved',units=rows,feature='NFS_PRIVATE_RECORDS_HD=1',atlasFontLifetimeFixRequired=True,rendererHooksRequired=['created','retired','upload_begin','upload_done','select','triangle'],renderPixelOrDeviceAcceptance=False)
 (a.output/'RECORDS-SOURCE-INSERTIONS.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(dict(passed=True,units=[x['unit']for x in rows],output=str(a.output))))
if __name__=='__main__':main()
