# SPDX-License-Identifier: MIT
"""Independent exact original40D1B0 preview-change loop (21 frames), not Escape."""
MARKER="NFS_CHILD_EXIT"
def add_child_exit_hooks(text,name):
 if name not in ("Methods_01.cpp","Methods_12.cpp"):return text
 if MARKER in text:raise ValueError("Child exit hooks already present")
 def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
 def one(s,a,b):
  if s.count(a)!=1:raise ValueError("Exact ChildExit anchor changed: "+a[:80])
  return s.replace(a,b,1)
 text=one(text,'#include "FunctionIds.h"\n','#include "FunctionIds.h"\n'+block('include','#include "hd/ChildExitPreludeQueueAdapter.inc"\n#include "hd/ClassicScreens/RecordsCursorQueueAdapter.inc"'))
 def call(symbol):return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
 if name=="Methods_12.cpp":
  tex='\teax = grTexSource(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8), to32i(esp + 12)); esp += 16;'
  text=one(text,tex,block('actual-texture',' nfs_child_exit_live::selected(*this); nfs_records_ui_live::cursorSelected(*this);')+tex)
  start=text.index('Fn(void) Game::_sub_497D6C()\n');end=text.find('\nFn(',start+1);end=len(text)if end<0 else end;part=text[start:end]
  tri='\teax = grDrawTriangle(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8)); esp += 12;'
  if part.count(tri)!=2:raise ValueError('Exact ChildExit actual quad changed')
  part=part.replace(tri,block('actual-native-quad',' nfs_child_exit_live::nativeQuad(*this); nfs_records_ui_live::cursorQuad(*this);')+tri,1)
  return text[:start]+part+text[end:]
 menu='Fn(void) Game::_sub_40C160()\n{\n';text=one(text,menu,menu+block('static-menu-scope',' nfs_child_static_scene::MenuScope nfs_child_static_menu(reinterpret_cast<uintptr_t>(this),nfs_logo_publisher::threadToken(),uint32_t(eax));'))
 resource='Fn(void) Game::_sub_405580()\n{\n';text=one(text,resource,resource+block('word-resource',' nfs_child_exit_live::ResourceScope nfs_child_exit_resource(*this);'))
 entry='Fn(void) Game::_sub_40D1B0()\n{\n';text=one(text,entry,entry+block('session',' nfs_child_exit_live::SessionScope nfs_child_exit_session(*this);\n std::unique_ptr<nfs_child_exit_live::FrameScope> nfs_child_exit_frame;'))
 a=text.index(entry);z=text.index('loc_40D237:\n',a);part=text[a:z]
 part=one(part,call('427ED0'),call('427ED0')+block('incoming-player-photo',' nfs_child_exit_session.primePlayerPhoto();'))
 text=text[:a]+part+text[z:]
 a=text.index('loc_40D237:\n');z=text.index('loc_40D2DE:\n',a);part=text[a:z]
 if part.count('loc_40D24B:\n')!=1 or part.count(call('427B30'))!=1 or part.count(call('4976B8'))!=1 or part.count(call('40FA90'))!=1 or part.count(call('40CB00'))!=1 or part.count(call('4276F0'))!=1 or part.count(call('4267C0'))!=1:raise ValueError('Exact first ChildExit path changed')
 part=one(part,'loc_40D237:\n','loc_40D237:\n'+block('arm',' nfs_child_exit_session.arm();'))
 part=one(part,'loc_40D24B:\n','loc_40D24B:\n'+block('frame',' nfs_child_exit_frame.reset(new nfs_child_exit_live::FrameScope(*this,nfs_child_exit_session));'))
 part=one(part,call('49769C'),call('49769C')+block('cleared',' if(nfs_child_exit_frame)nfs_child_exit_frame->cleared();'))
 part=one(part,call('427B30'),call('427B30')+block('full',' if(nfs_child_exit_frame)nfs_child_exit_frame->full();'))
 part=one(part,call('40FA90'),block('widgets-first-begin',' if(nfs_child_exit_frame)nfs_child_exit_frame->section(NFS_CHILD_EXIT_WIDGETS_BEGIN);')+call('40FA90')+block('widgets-first-end',' if(nfs_child_exit_frame)nfs_child_exit_frame->sectionEnd(NFS_CHILD_EXIT_WIDGETS_END);'))
 part=one(part,call('40CB00'),block('words-first-begin',' if(nfs_child_exit_frame)nfs_child_exit_frame->section(NFS_CHILD_EXIT_WORDS_BEGIN);')+call('40CB00'))
 part=one(part,call('40CB00'),call('40CB00')+block('words-first-end',' if(nfs_child_exit_frame)nfs_child_exit_frame->sectionEnd(NFS_CHILD_EXIT_WORDS_END);'))
 for symbol,tag,start,finish in [('4276F0','branding','BRANDING_BEGIN','BRANDING_END'),('4267C0','cursor','CURSOR_BEGIN','CURSOR_END')]:
  part=one(part,call(symbol),block(tag+'-begin',' if(nfs_child_exit_frame)nfs_child_exit_frame->section(NFS_CHILD_EXIT_'+start+');')+call(symbol)+block(tag+'-end',' if(nfs_child_exit_frame)nfs_child_exit_frame->sectionEnd(NFS_CHILD_EXIT_'+finish+');'))
 part=one(part,call('4976B8'),block('ready',' if(nfs_child_exit_frame)nfs_child_exit_frame->ready();')+call('4976B8')+block('end',' if(nfs_child_exit_frame)nfs_child_exit_frame->end();\n nfs_child_exit_frame.reset();'))
 text=text[:a]+part+text[z:]
 a=text.index('loc_40D38F:\n');z=text.index('\n}',a);part=text[a:z]
 if part.count(call('4279E0'))!=1 or part.count(call('427B30'))!=1 or part.count(call('40CB00'))!=1 or 'goto loc_40D2C3;'not in part:raise ValueError('Exact moving ChildExit path changed')
 part=one(part,call('4279E0'),block('photo-begin',' if(nfs_child_exit_frame)nfs_child_exit_frame->photoBegin();')+call('4279E0')+block('photo-end',' if(nfs_child_exit_frame)nfs_child_exit_frame->photoEnd();'))
 part=one(part,call('427B30'),call('427B30')+block('sidebar',' if(nfs_child_exit_frame)nfs_child_exit_frame->sidebar();'))
 part=one(part,call('40CB00'),block('words-moving-begin',' if(nfs_child_exit_frame)nfs_child_exit_frame->section(NFS_CHILD_EXIT_WORDS_BEGIN);')+call('40CB00')+block('words-moving-end',' if(nfs_child_exit_frame)nfs_child_exit_frame->sectionEnd(NFS_CHILD_EXIT_WORDS_END);'))
 return text[:a]+part+text[z:]
