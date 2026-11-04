"""Exact insertion-only title splash integration; no original calls are removed."""
from pathlib import Path
import argparse,re,json,hashlib
MARK='NFS_PRIVATE_INTRO_UI'
def add_intro_hooks(text,name):
 if name not in ('Methods_01.cpp','Methods_03.cpp','Methods_10.cpp'):return text
 if MARK in text:raise ValueError('Intro hooks already inserted')
 old=text;serial=0
 def block(code):
  nonlocal serial
  serial+=1;return '\n// '+MARK+'_BEGIN:'+str(serial)+'\n'+code+'\n// '+MARK+'_END:'+str(serial)+'\n'
 def entry(symbol,code):
  nonlocal text
  m=re.search(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\s*\{',text);assert m,symbol;text=text[:m.end()]+block(code)+text[m.end():]
 anchor='#include "FunctionIds.h"';assert text.count(anchor)==1;text=text.replace(anchor,anchor+block('#include "hd/ClassicScreens/'+('IntroQueueAdapter.inc'if name=='Methods_03.cpp'else'IntroGameHooks.inc')+'"'),1)
 if name=='Methods_01.cpp':
  entry('408558',' nfs_intro_ui_live::OwnerScope nfs_intro_owner(*this);')
  m=re.search(r'Fn\(void\) Game::_sub_408558\(\)\s*\{',text);assert m;end=text.find('\nFn(',m.end());end=len(text)if end<0 else end;part=text[m.start():end]
  for symbol,scope,count in [('483470','BackgroundScope',1),('486D10','CopyrightScope',2),('428770','Presentation',2)]:
   call='\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call';assert part.count(call)==count,(symbol,part.count(call));part=part.replace(call,block(' { nfs_intro_ui_live::'+scope+' nfs_intro_'+symbol+'(*this);')+call+block(' }'))
  text=text[:m.start()]+part+text[end:]
 elif name=='Methods_03.cpp':entry('426950',' nfs_intro_ui_live::DrawScope nfs_intro_native(*this);')
 else:
  for symbol in ['482DA0','4848FC']:entry(symbol,' nfs_intro_ui_live::retire(*this,uint32_t(eax));')
 assert re.sub(r'\n// '+MARK+r'_BEGIN:(\d+)\n.*?\n// '+MARK+r'_END:\1\n','',text,flags=re.S)==old
 return text

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False);rows=[]
 for name in ('Methods_01.cpp','Methods_03.cpp','Methods_10.cpp'):
  old=(a.source_dir/name).read_text();new=add_intro_hooks(old,name);(a.output/name).write_text(new);rows.append({'unit':name,'originalOperationsPreserved':True,'sourceSha256':hashlib.sha256(old.encode()).hexdigest(),'outputSha256':hashlib.sha256(new.encode()).hexdigest()})
 (a.output/'INTRO-SOURCE-INSERTIONS.json').write_text(json.dumps({'scope':'Exact original splash408558; optional immutable artist provider; source generation only','units':rows,'gameExecuted':False,'compilerExecuted':False},indent=2)+'\n')
if __name__=='__main__':main()
