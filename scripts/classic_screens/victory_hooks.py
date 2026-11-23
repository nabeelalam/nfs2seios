"""Private insertion-only Victory integration; never compiles or starts a game."""
from pathlib import Path
import argparse,re,json,hashlib
O=Path(__file__).resolve().parent
H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--fallback-dir',type=Path);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 assert not a.output.exists();a.output.mkdir(parents=True);rows=[]
 for unit in [1,2,3,10,12,13]:
  name='Methods_%02d.cpp'%unit;source=a.source_dir/name
  if not source.exists()and a.fallback_dir:source=a.fallback_dir/name
  old=source.read_text();text=old;serial=0
  def block(code):
   nonlocal serial
   serial+=1;return '\n// NFS_PRIVATE_VICTORY_UI_BEGIN:'+str(serial)+'\n'+code+'\n// NFS_PRIVATE_VICTORY_UI_END:'+str(serial)+'\n'
  def section(symbol):
   m=re.search(r'Fn\([^)]*\) Game::_sub_'+symbol+r'\(\)\s*\{',text);assert m,symbol;z=text.find('\nFn(',m.end());return m.start(),len(text)if z<0 else z,m.end()
  def entry(symbol,code):
   nonlocal text
   lo,hi,at=section(symbol);text=text[:at]+block(code)+text[at:]
  def edit(symbol,fn):
   nonlocal text
   lo,hi,at=section(symbol);text=text[:lo]+fn(text[lo:hi])+text[hi:]
  anchor='#include "FunctionIds.h"';assert text.count(anchor)==1;text=text.replace(anchor,anchor+block('#include "'+('hd/ClassicScreens/VictoryQueueAdapter.inc'if unit==3 else'hd/ClassicScreens/VictoryGameHooks.inc')+'"'),1)
  if unit==1:
   entry('405CF0',' nfs_victory_ui_live::StringScope nfs_victory_string(*this);')
   entry('405B30',' nfs_victory_ui_live::BitmapScope nfs_victory_bitmap(*this);')
   entry('405B60',' nfs_victory_ui_live::bitmapResolved(*this);')
   for symbol in ['405580','4054E0']:entry(symbol,' nfs_victory_ui_live::ResourceScope nfs_victory_resource(*this);')
   def measures(part):
    call='\tesp -= 4; _sub_482424(); esp += 4; //call';assert part.count(call)==2;pieces=part.split(call)
    return pieces[0]+''.join(call+block(' nfs_victory_ui_live::measured(*this,uint32_t(ecx),int32_t(eax),'+str(2 if i==0 else 1)+');')+tail for i,tail in enumerate(pieces[1:]))
   edit('405580',measures)
  elif unit==2:
   entry('4154C0',' nfs_victory_ui_live::OwnerScope nfs_victory_owner(*this);')
   def victory(part):
    call='\tesp -= 4; _sub_410E80(); esp += 4; //call';assert part.count(call)==1;part=part.replace(call,call+block(' nfs_victory_ui_live::archiveReady(*this);'))
    call='\tesp -= 4; _sub_415CC0(); esp += 4; //call';assert part.count(call)==1;part=part.replace(call,block(' nfs_victory_ui_live::Adapter::begin(*this);')+call+block(' nfs_victory_ui_live::Adapter::end(*this);'))
    call='\tesp -= 4; _sub_426B70(); esp += 4; //call';assert part.count(call)==1;part=part.replace(call,block(' { nfs_victory_ui_live::Presentation nfs_victory_present(*this);')+call+block(' }'))
    call='\tesp -= 4; _sub_4274F0(); esp += 4; //call';anchor='loc_4156FE:\n'+call;assert part.count(anchor)==1;return part.replace(anchor,'loc_4156FE:\n'+block(' { nfs_victory_ui_live::LogoTailScope nfs_victory_logo_tail(*this);')+call+block(' }'))
   edit('4154C0',victory)
   def selected(part):
    at=part.index('loc_415B6A:');head,tail=part[:at],part[at:];anchor='ecx = ax; //movsx';assert tail.count(anchor)==1
    return head+tail.replace(anchor,anchor+block(' nfs_victory_ui_live::logoSelected(*this);'),1)
   edit('4159E0',selected)
  elif unit==3:
   entry('426950',' nfs_victory_ui_live::DrawScope nfs_victory_native(*this);')
   entry('4280B0',' nfs_victory_ui_live::LogoBuildScope nfs_victory_logo_build(*this);')
  elif unit==10:
   for symbol in ['482DA0','4848FC']:entry(symbol,' nfs_victory_ui_live::retire(*this,uint32_t(eax));')
  elif unit==12:
   entry('49755C',' nfs_victory_ui_live::LogoUploadScope nfs_victory_logo_upload(*this);')
   entry('497D6C',' nfs_victory_ui_live::LogoMeshScope nfs_victory_logo_mesh(*this);')
   def quad(part):
    call='\teax = grDrawTriangle(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8)); esp += 12;';assert part.count(call)==2
    return part.replace(call,block(' nfs_victory_ui_live::logoQuad(*this);')+call,1)
   edit('497D6C',quad)
  elif unit==13:
   def stores(part):
    call='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov';assert part.count(call)==4
    return part.replace(call,block(' { nfs_victory_ui_live::Word nfs_victory_word(*this,uint32_t(esi),uint16_t(bx));')+call+block(' }'))
   edit('4A5830',stores)
  reverse=re.sub(r'\n// NFS_PRIVATE_VICTORY_UI_BEGIN:(\d+)\n.*?\n// NFS_PRIVATE_VICTORY_UI_END:\1\n','',text,flags=re.S);assert reverse==old
  dest=a.output/name;dest.write_text(text);rows.append(dict(unit=name,source=str(source),sourceSHA256=H(source),output=str(dest),outputSHA256=H(dest),insertions=serial,originalOperationsPreserved=True))
 manifest=dict(scope='Victory text/artwork and exact animated car-logo tail insertions; no compile/game; original operations preserved',units=rows,feature='NFS_PRIVATE_VICTORY_HD=1',atlasFontLifetimeFixRequired=True,rendererHooksRequired=['created','retired','upload_begin','upload_done','select','triangle','logo_batch_begin','logo_batch_end','completed_parent','pending_tail_presentation_guard'],standardLogoCarCount=12,bonusOrOmegaVerified=False,renderPixelOrDeviceAcceptance=False)
 (a.output/'VICTORY-SOURCE-INSERTIONS.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(dict(passed=True,units=[x['unit']for x in rows],output=str(a.output))))
if __name__=='__main__':main()
