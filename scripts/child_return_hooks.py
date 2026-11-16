# SPDX-License-Identifier: MIT
"""Exact actual Escape routes: 40CA3D/427320 fade and40C970 reverse."""
MARKER="NFS_CHILD_RETURN"
def add_child_return_hooks(text,name):
 if name not in ("Methods_01.cpp","Methods_03.cpp","Methods_12.cpp"):return text
 if MARKER in text:raise ValueError("ChildReturn already enrolled")
 def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
 def one(s,a,b):
  if s.count(a)!=1:raise ValueError("Exact ChildReturn anchor changed: "+a[:100])
  return s.replace(a,b,1)
 def call(s):return "\tesp -= 4; _sub_"+s+"(); esp += 4; //call"
 def stage(k):return " if(nfs_child_return_frame)nfs_child_return_frame->stage(NFS_CHILD_RETURN_"+k+");"
 def section(k):return " if(nfs_child_return_frame)nfs_child_return_frame->section(NFS_CHILD_RETURN_"+k+"_BEGIN);"
 def end(k):return " if(nfs_child_return_frame)nfs_child_return_frame->sectionEnd(NFS_CHILD_RETURN_"+k+"_END);"
 text=one(text,'#include "FunctionIds.h"\n','#include "FunctionIds.h"\n'+block('include','#include "hd/ChildReturnGameHooks.inc"'))
 if name=="Methods_12.cpp":
  a='\teax = grTexSource(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8), to32i(esp + 12)); esp += 16;'
  text=one(text,a,block('texture',' nfs_child_return_live::selected(*this);')+a)
  a=text.index('Fn(void) Game::_sub_497D6C()\n');z=text.find('\nFn(',a+1);z=len(text)if z<0 else z;part=text[a:z]
  tri='\teax = grDrawTriangle(to32i(esp + 0), to32i(esp + 4), to32i(esp + 8)); esp += 12;'
  if part.count(tri)!=2:raise ValueError('Return quad shape changed')
  part=part.replace(tri,block('quad',' nfs_child_return_live::nativeQuad(*this);')+tri,1)
  return text[:a]+part+text[z:]
 if name=="Methods_01.cpp":
  actual=text[text.index("loc_40C970:\n"):text.index("loc_40CA53:\n")]
  for proof in ["loc_40CA3D:\n\tecx = 1; //mov\n\tedx = 0xE6; //mov\n\teax = 0x8C; //mov\n\txor_(ebx, ebx);\n"+call("427320"),"\tcmp(eax, (int32_t)0xFFFFFEE8);","\tsub(esi, (int32_t)0xF);","\tpush32(0xFF8C8C8C);"]:
   if actual.count(proof)!=1:raise ValueError("Actual Return source parameters changed")
  a='Fn(void) Game::_sub_40C160()\n{\n'
  text=one(text,a,a+block('session',' nfs_child_return_live::Session nfs_child_return_session(*this);\n std::unique_ptr<nfs_child_return_live::Frame> nfs_child_return_frame;'))
  text=one(text,'loc_40C970:\n','loc_40C970:\n'+block('reverse-arm',' nfs_child_return_session.arm(NFS_CHILD_RETURN_REVERSE);'))
  text=one(text,'loc_40C9B3:\n','loc_40C9B3:\n'+block('reverse-seal',' nfs_child_return_session.uploaded();'))
  a=text.index('loc_40C9B3:\n');z=text.index('loc_40CA30:\n',a);part=text[a:z]
  anchor='\t\tgoto loc_40CA75;'
  part=one(part,anchor,anchor+block('reverse-frame',' nfs_child_return_frame.reset(new nfs_child_return_live::Frame(*this,nfs_child_return_session,NFS_CHILD_RETURN_REVERSE));'))
  part=one(part,call('49769C'),call('49769C')+block('reverse-clear',stage('CLEAR')))
  # Both original427B30 calls have distinct source roles and remain ordered.
  c=call('427B30')
  if part.count(c)!=2:raise ValueError('Return reverse two planes changed')
  part=part.replace(c,block('reverse-base-begin',stage('BASE_BEGIN'))+c+block('reverse-base-end',stage('BASE_END')),1)
  pos=part.rfind(c);part=part[:pos]+block('reverse-side-begin',stage('SIDEBAR_BEGIN'))+part[pos:].replace(c,c+block('reverse-side-end',stage('SIDEBAR_END')),1)
  for symbol,tag in [('4276F0','BRANDING'),('4279E0','PHOTO')]:part=one(part,call(symbol),block('reverse-'+tag+'-begin',section(tag))+call(symbol)+block('reverse-'+tag+'-end',end(tag)))
  part=one(part,call('4976B8'),block('reverse-ready',' if(nfs_child_return_frame)nfs_child_return_frame->ready();')+call('4976B8')+block('reverse-end',' if(nfs_child_return_frame)nfs_child_return_frame->end();\n nfs_child_return_frame.reset();'))
  text=text[:a]+part+text[z:]
  text=one(text,'loc_40CA3D:\n','loc_40CA3D:\n'+block('fade-arm',' nfs_child_return_session.arm(NFS_CHILD_RETURN_FADE);'))
  return text
 # Original upload/call body is source-authenticated independently of MT admission.
 a='Fn(void) Game::_sub_428250()\n{\n';text=one(text,a,a+block('sidebar-upload',' nfs_child_return_live::SidebarUploadScope nfs_child_return_upload(*this);'))
 a=text.index('Fn(void) Game::_sub_428250()\n');z=text.find('\nFn(',a+1);part=text[a:z];c=call('49755C')
 if part.count(c)!=1:raise ValueError('Return actual300 tile anchor changed')
 part=one(part,c,block('side-tile-open',' { nfs_child_return_live::SidebarTileScope nfs_child_return_tile(*this);')+c+block('side-tile-close',' }'));text=text[:a]+part+text[z:]
 a=text.index('Fn(void) Game::_sub_427320()\n');z=text.find('\nFn(',a+1);part=text[a:z]
 part=one(part,'{\n','{\n'+block('fade-frame-storage',' std::unique_ptr<nfs_child_return_live::Frame> nfs_child_return_frame;'))
 part=one(part,'loc_42733A:\n','loc_42733A:\n'+block('fade-frame',' if(nfs_child_return_live::session())nfs_child_return_frame.reset(new nfs_child_return_live::Frame(*this,*nfs_child_return_live::session(),NFS_CHILD_RETURN_FADE));'))
 part=one(part,call('49769C'),call('49769C')+block('fade-clear',stage('CLEAR')))
 for symbol,tag in [('427C70','BASE'),('427B30','SIDEBAR')]:part=one(part,call(symbol),block('fade-'+tag+'-begin',stage(tag+'_BEGIN'))+call(symbol)+block('fade-'+tag+'-end',stage(tag+'_END')))
 part=one(part,call('4276F0'),block('fade-brand-begin',section('BRANDING'))+call('4276F0')+block('fade-brand-end',end('BRANDING')))
 part=one(part,call('4976B8'),block('fade-ready',' if(nfs_child_return_frame)nfs_child_return_frame->ready();')+call('4976B8')+block('fade-end',' if(nfs_child_return_frame)nfs_child_return_frame->end();\n nfs_child_return_frame.reset();'))
 return text[:a]+part+text[z:]
