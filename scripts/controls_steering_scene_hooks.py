"""Original-preserving Steering Controls-owned full640 native426950 queue hooks.

Compose after existing scene adapters. The original upload/select/triangle loop
and every existing wrapper remain untouched; Steering Controls uses shared arbitration.
"""
# SPDX-License-Identifier: MIT
import re
MARKER='NFS_CONTROLS_STEERING_SCENE'
NAMES=('Methods_03.cpp',)

def remove_controls_steering_scene_hooks(text):
    result=re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched Steering Controls scene marker')
    return result

def add_controls_steering_scene_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError('Steering Controls scene hooks already present')
    original=text;crlf='\r\n'in text
    if crlf and '\n'in text.replace('\r\n',''):raise ValueError('Mixed Steering Controls native source newlines')
    if crlf:text=text.replace('\r\n','\n')
    def block(tag,code):return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
    anchor='#include "FunctionIds.h"\n'
    if text.count(anchor)!=1:raise ValueError('Steering Controls scene include changed')
    text=text.replace(anchor,anchor+block('include','#include "hd/ControlsSteeringSceneQueueAdapter.inc"'),1)
    matches=list(re.finditer(r'Fn\(void\) Game::_sub_426950\(\)\n\{(?P<body>.*?)(?=\nFn\(void\) Game::|\Z)',text,re.S))
    if len(matches)!=1:raise ValueError('Missing/duplicate Steering Controls native426950')
    m=matches[0];body=m.group('body');last=body.rfind('}')
    if last<0 or body[last+1:].strip() or re.search(r'\b(return|co_return)\b',body):raise ValueError('Steering Controls native exits changed')
    # These exact original operations and loop edges are present in the measured
    # maintained426950 source. Metadata/counters never substitute their execution.
    required=('loc_42697D:\n','loc_4269A7:\n',
      '\tesp -= 4; ___CHP(); esp += 4; //call',
      '\tesp -= 4; _sub_4850B0(); esp += 4; //call',
      '\tesp -= 4; _sub_49755C(); esp += 4; //call',
      '\tesp -= 4; _sub_497738(); esp += 4; //call',
      '\tesp -= 4; _sub_497D6C(); esp += 4; //call',
      '\tadd(esi, (int32_t)0x20);','\tcmp(esi, (int32_t)0x1E0);',
      '\t\tgoto loc_4269A7;','\tadd(edi, (int32_t)0x20);',
      '\tcmp(edi, (int32_t)0x280);','\t\tgoto loc_42697D;',
      '\tpop32(ebx);')
    for item in required:
        if body.count(item)!=1:raise ValueError('Steering Controls native original operation/extent changed '+item)
    order=('loc_42697D:','loc_4269A7:','___CHP();','_sub_4850B0();','_sub_49755C();','_sub_497738();','_sub_497D6C();','cmp(esi, (int32_t)0x1E0);','cmp(edi, (int32_t)0x280);','pop32(ebx);')
    positions=[body.index(item)for item in order]
    if positions!=sorted(positions):raise ValueError('Steering Controls native original operation order changed')
    begin=block('begin',' nfs_controls_steering_scene_live::Intent nfs_controls_steering_scene_intent;\n try {\n nfs_controls_steering_scene_live::begin(*this,nfs_controls_steering_scene_intent);')
    end=block('finish',' nfs_controls_steering_scene_live::finish(nfs_controls_steering_scene_intent);\n } catch(...) {\n nfs_controls_steering_scene_live::cancel(nfs_controls_steering_scene_intent);\n throw;\n }')
    replacement='Fn(void) Game::_sub_426950()\n{'+begin+body[:last]+end+body[last:]
    text=text[:m.start()]+replacement+text[m.end():]
    # No added callback/include can execute inside an original checked-copy
    # lease. Preserve exact earlier scopes and fail closed on unbalanced leases.
    depth=0
    for event in re.finditer(r'// checked copy transaction (begin|end)|// '+MARKER+r'_BEGIN:',text):
        if event.group(1)=='begin':depth+=1
        elif event.group(1)=='end':
            depth-=1
            if depth<0:raise ValueError('Steering Controls native unmatched guest-copy end')
        elif depth:raise ValueError('Steering Controls native hook inside guest-copy lease')
    if depth:raise ValueError('Steering Controls native unmatched guest-copy begin')
    if crlf:text=text.replace('\n','\r\n')
    if remove_controls_steering_scene_hooks(text)!=original:raise ValueError('Steering Controls native byte reversal failed')
    return text
