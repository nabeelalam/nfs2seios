# SPDX-License-Identifier: MIT
"""Lexical Options bitmap entrance observer; original control flow preserved."""
MARKER='NFS_OPTIONS_BITMAP_PRELUDE'
def add_options_bitmap_prelude_hooks(text,name):
 if name not in ('Methods_01.cpp','Methods_03.cpp','Methods_10.cpp'):return text
 if MARKER in text:raise ValueError('Bitmap prelude observer already present')
 def block(tag,code):return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
 anchor='#include "FunctionIds.h"\n'
 if text.count(anchor)!=1:raise ValueError('Bitmap include anchor changed')
 text=text.replace(anchor,anchor+block('include','#include "hd/OptionsBitmapPreludeGameHooks.inc"'))
 if name=='Methods_03.cpp':
  anchor='Fn(void) Game::_sub_426950()\n{\n'
  if text.count(anchor)!=1:raise ValueError('Bitmap native source changed')
  return text.replace(anchor,anchor+block('native',' nfs_options_bitmap_live::NativeScope nfs_options_bitmap_native(*this);'))
 if name=='Methods_10.cpp':
  anchor='Fn(void) Game::_sub_486D10()\n{\n'
  if text.count(anchor)!=1:raise ValueError('Bitmap source copy changed')
  return text.replace(anchor,anchor+block('copy-original',' nfs_options_bitmap_live::originalCopy(*this);'))
 a=text.index('loc_40C792:\n');z=text.index('loc_40C7FB:',a);body=text[a:z]
 def replace(old,new):
  nonlocal body
  if body.count(old)!=1:raise ValueError('Bitmap prelude anchor changed: '+old[:50])
  body=body.replace(old,new)
 replace('loc_40C792:\n','loc_40C792:\n'+block('frame-open',' { nfs_options_bitmap_live::FrameScope nfs_options_bitmap_frame(*this);'))
 anchor='\tesp -= 4; _sub_40B6C4(); esp += 4; //call\n';replace(anchor,block('frame-start',' nfs_options_bitmap_frame.started();')+anchor)
 anchor='\tesp -= 4; _sub_405B60(); esp += 4; //call\n';replace(anchor,block('copy-open',' { nfs_options_bitmap_live::CopyScope nfs_options_bitmap_copy(*this);')+anchor+block('copy-close',' }'))
 anchor='\tesp -= 4; _sub_405C70(); esp += 4; //call\n';replace(anchor,anchor+block('frame-close',' }'))
 return text[:a]+body+text[z:]
