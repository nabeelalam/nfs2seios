"""Exact Loading47D5E0 insertion only; original operations remain unchanged."""
from pathlib import Path
import argparse,re
MARK='NFS_PRIVATE_LOADING_UI'
def add_loading_hooks(text,name):
 if name not in ('Methods_03.cpp','Methods_09.cpp'):return text
 if MARK in text:raise ValueError('Loading hooks already inserted')
 old=text;serial=0
 def block(code):
  nonlocal serial
  serial+=1;return '\n// '+MARK+'_BEGIN:'+str(serial)+'\n'+code+'\n// '+MARK+'_END:'+str(serial)+'\n'
 anchor='#include "FunctionIds.h"';assert text.count(anchor)==1;text=text.replace(anchor,anchor+block('#include "hd/ClassicScreens/'+('LoadingQueueAdapter.inc'if name=='Methods_03.cpp'else'LoadingGameHooks.inc')+'"'),1)
 symbol='426950'if name=='Methods_03.cpp'else'47D5E0';m=re.search(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\s*\{',text);assert m;end=text.find('\nFn(',m.end());end=len(text)if end<0 else end;part=text[m.start():end];at=part.index('{')+1
 if name=='Methods_03.cpp':part=part[:at]+block(' nfs_loading_ui_live::DrawScope nfs_loading_draw(*this);')+part[at:]
 else:
  part=part[:at]+block(' nfs_loading_ui_live::OwnerScope nfs_loading_owner(*this);')+part[at:]
  call='\tesp -= 4; _sub_483460(); esp += 4; //call'
  a=part.index('loc_47D8AA:\n');b=part.index('loc_47D8F2:\n',a);region=part[a:b];assert region.count(call)==1;region=region.replace(call,block(' { nfs_loading_ui_live::BackgroundScope nfs_loading_back(*this);')+call+block(' }'));part=part[:a]+region+part[b:]
  a=part.index('loc_47D8F2:\n');b=part.index('loc_47D91E:\n',a);region=part[a:b];assert region.count(call)==1;region=region.replace(call,block(' { nfs_loading_ui_live::WordScope nfs_loading_word(*this);')+call+block(' }'));part=part[:a]+region+part[b:]
  label='loc_47D91E:\n';a=part.index(label)+len(label);call='\tesp -= 4; _sub_428770(); esp += 4; //call';assert part[a:].count(call)==2;b=part.index(call,part.index(call,a)+len(call))+len(call);region=part[a:b];assert 'goto 'not in region;part=part[:a]+block(' { nfs_loading_ui_live::Presentation nfs_loading_present(*this);')+region+block(' }')+part[b:]
 text=text[:m.start()]+part+text[end:]
 assert re.sub(r'\n// '+MARK+r'_BEGIN:(\d+)\n.*?\n// '+MARK+r'_END:\1\n','',text,flags=re.S)==old
 return text

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
 for name in ('Methods_03.cpp','Methods_09.cpp'):(a.output/name).write_text(add_loading_hooks((a.source_dir/name).read_text(),name))
if __name__=='__main__':main()
