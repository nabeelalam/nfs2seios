"""Independent complete Steering background/text/chart source observations.
Apply after steering text, chart and optional sidebar observations. Never activates
normal preparation or native presentation by itself.
"""
# SPDX-License-Identifier: MIT
import re
MARKER='NFS_CONTROLS_STEERING_LIVE'
NAMES=('Methods_01.cpp','Methods_10.cpp','Methods_14.cpp')
CODES={
 'include':'#include "hd/ControlsSteeringLiveGameHooks.inc"',
 'owner':' nfs_controls_steering_live::OwnerScope nfs_controls_steering_live_owner(*this);',
 'resolved':' nfs_controls_steering_live::ResolvedScope nfs_controls_steering_live_resolved(*this);',
 'bind-text':' nfs_controls_steering_live::bindText(*this);',
 'reset-open':' { nfs_controls_steering_live::ResetScope nfs_controls_steering_live_reset(*this);',
 'reset-close':' }',
 'sidebar-open':' { nfs_controls_steering_live::SidebarScope nfs_controls_steering_live_sidebar(*this);',
 'sidebar-close':' }',
 'begin-text':' nfs_controls_steering_live::beginTail(*this);',
 'finish-text':' nfs_controls_steering_live::finishTail(*this);',
 'finish-chart':' nfs_controls_steering_live::finishChart(*this);',
 'retire-descriptor':' nfs_controls_steering_live::afterRetire(*this);',
 'retire-owner':' nfs_controls_steering_live::afterRetire(*this);',
}
SETS=({'include','owner','resolved','bind-text','reset-open','reset-close','sidebar-open','sidebar-close','begin-text','finish-text','finish-chart'}, {'include','retire-descriptor'}, {'include','retire-owner','retire-descriptor'})
def _normalize(text):
 crlf='\r\n'in text
 if '\r'in text.replace('\r\n','')or(crlf and '\n'in text.replace('\r\n','')):raise ValueError('Mixed steering live newlines')
 return text.replace('\r\n','\n')if crlf else text,crlf
def block(tag):return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+CODES[tag]+'\n// '+MARKER+'_END:'+tag+'\n'
def outside(text,at):
 s=text[:at]
 if s.count('// checked copy transaction begin')!=s.count('// checked copy transaction end'):raise ValueError('Steering live hook inside checked-copy lease')
def call(symbol):return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
def remove_controls_steering_live_hooks(text):
 s,crlf=_normalize(text);pat=r'\n// '+MARKER+r'_BEGIN:([^\n]+)\n(.*?)\n// '+MARKER+r'_END:\1\n';ms=list(re.finditer(pat,s,re.S))
 if not ms:
  if MARKER in s:raise ValueError('Malformed steering live markers')
  return text
 tags=[m[1]for m in ms]
 if len(tags)!=len(set(tags))or set(tags)not in SETS:raise ValueError('Partial/duplicate steering live hooks')
 for m in ms:
  if m[1]not in CODES or m[2]!=CODES[m[1]]:raise ValueError('Changed steering live block')
 for tag,symbol in [('reset','405B30'),('sidebar','405B60')]:
  if tag+'-open'in tags:
   a=next(m for m in ms if m[1]==tag+'-open');b=next(m for m in ms if m[1]==tag+'-close')
   if a.end()>b.start()or s[a.end():b.start()]!=call(symbol):raise ValueError('Changed protected steering live call')
 out=re.sub(pat,'',s,flags=re.S)
 if MARKER in out:raise ValueError('Unmatched steering live block')
 return out.replace('\n','\r\n')if crlf else out

def add_controls_steering_live_hooks(text,name):
 if name not in NAMES:return text
 original=text;s,crlf=_normalize(text)
 if MARKER in s:raise ValueError('Steering live already present or malformed')
 if s.count('#include "hd/ControlsSteeringTextGameHooks.inc"\n')!=1:raise ValueError('Prior steering text collector required')
 if name in ('Methods_01.cpp','Methods_10.cpp')and s.count('#include "hd/ControlsSteeringChartGameHooks.inc"\n')!=1:raise ValueError('Prior steering chart collector required')
 def insert(anchor,tag,before=False):
  nonlocal s
  if s.count(anchor)!=1:raise ValueError('Missing/duplicate steering live anchor '+tag)
  at=s.index(anchor)+(0 if before else len(anchor));outside(s,at);s=s[:at]+block(tag)+s[at:]
 insert('#include "FunctionIds.h"\n','include')
 if name=='Methods_01.cpp':
  insert('// NFS_CONTROLS_STEERING_TEXT_END:entry-4101C0\n','owner')
  insert('Fn(void) Game::_sub_405B60()\n{\n','resolved')
  insert('// NFS_CONTROLS_STEERING_TEXT_END:entry-405CF0\n','bind-text')
  pattern=r'Fn\(void\) Game::_sub_4101C0\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)';parts=list(re.finditer(pattern,s,re.S))
  if len(parts)!=1:raise ValueError('Missing/duplicate steering original handler')
  match=parts[0];part=match.group();positions=[]
  for sym in ['405B30','405B60','40FA90','410330','405C70']:
   if part.count(call(sym))!=1:raise ValueError('Changed steering full-frame call '+sym)
   positions.append(part.index(call(sym)))
  if positions!=sorted(positions)or part.count('loc_41020E:\n')!=1:raise ValueError('Changed steering source order')
  resetPrefix=part[part.index('loc_41020E:\n'):positions[0]];copyPrefix=part[positions[0]:positions[1]]
  if any(x not in resetPrefix for x in ('_sub_428A20()','_sub_40EB20()','\tedx = 0x400; //mov\n','\teax = to32i(esi+4); //mov\n','\txor_(ecx, ecx);\n','\txor_(ebx, ebx);\n')):raise ValueError('Changed steering original reset arguments')
  if any(x not in copyPrefix for x in ('dword_4D2314+2','\tsar(eax, (int32_t)0x10);\n','dword_4D231C+eax*4','eax+0x20','\tedx = 0x1000; //mov\n','\txor_(ecx, ecx);\n','\txor_(ebx, ebx);\n')):raise ValueError('Changed steering original sidebar arguments')
  for sym,tag in [('405B30','reset'),('405B60','sidebar')]:
   anchor=call(sym);at=part.index(anchor);outside(part,at);part=part.replace(anchor,block(tag+'-open')+anchor+block(tag+'-close'),1)
  before='\n// NFS_CONTROLS_STEERING_TEXT_BEGIN:begin-text\n';after='// NFS_CONTROLS_STEERING_TEXT_END:finish-before-chart\n'
  if part.count(before)!=1 or part.count(after)!=1:raise ValueError('Original steering text boundary required')
  part=part.replace(before,block('begin-text')+before,1);part=part.replace(after,after+block('finish-text'),1);part=part.replace(call('410330'),call('410330')+block('finish-chart'),1)
  s=s[:match.start()]+part+s[match.end():]
 elif name=='Methods_10.cpp':insert('// NFS_CONTROLS_STEERING_TEXT_END:entry-482DA0\n','retire-descriptor')
 else:
  insert('// NFS_CONTROLS_STEERING_TEXT_END:entry-4AC934\n','retire-owner');insert('// NFS_CONTROLS_STEERING_TEXT_END:entry-4AC9C8\n','retire-descriptor')
 for m in re.finditer('// '+MARKER+'_BEGIN:',s):outside(s,m.start())
 result=s.replace('\n','\r\n')if crlf else s
 if remove_controls_steering_live_hooks(result)!=original:raise ValueError('Steering live exact reversal failed')
 return result
