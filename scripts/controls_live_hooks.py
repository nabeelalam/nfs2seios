"""Observe only the genuine original Controls complete640 source pass.

Compose after Controls TEXT and retained343 sidebar hooks. No renderer admission,
input changes, source replacement or private observation outputs are installed.
"""
# SPDX-License-Identifier: MIT
import re
MARKER='NFS_CONTROLS_LIVE'
NAMES=('Methods_01.cpp','Methods_09.cpp','Methods_10.cpp','Methods_13.cpp','Methods_14.cpp')

def remove_controls_live_hooks(text):
    result=re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched Controls live marker')
    return result

def add_controls_live_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError('Controls live hooks already present')
    original=text;crlf='\r\n'in text
    if crlf and '\n'in text.replace('\r\n',''):raise ValueError('Mixed Controls original source newlines')
    if crlf:text=text.replace('\r\n','\n')
    for dependency in ('ControlsTextGameHooks.inc','ControlsSidebarLiveGameHooks.inc'):
        if text.count('#include \"hd/'+dependency+'\"\n')!=1:raise ValueError('Controls live prerequisite changed '+dependency)
    def block(tag,code):return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
    def unique(anchor,replacement):
        nonlocal text
        if text.count(anchor)!=1:raise ValueError('Missing/duplicate Controls live anchor '+anchor[:100])
        text=text.replace(anchor,replacement,1)
    def entry(symbol,code):
        anchor='Fn(void) Game::_sub_'+symbol+'()\n{\n';unique(anchor,anchor+block('entry-'+symbol,code))
    def body(symbol,transform):
        nonlocal text
        matches=list(re.finditer(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)',text,re.S))
        if len(matches)!=1:raise ValueError('Missing/duplicate Controls live function '+symbol)
        match=matches[0];text=text[:match.start()]+transform(match.group())+text[match.end():]
    def wrap(part,label,callee,tag,code,required):
        head=label+':\n'
        if part.count(head)!=1:raise ValueError('Controls normal source label changed')
        start=part.index(head);end=part.find('\nloc_',start+len(head));end=len(part)if end<0 else end
        region=part[start:end];call='\tesp -= 4; _sub_'+callee+'(); esp += 4; //call'
        if region.count(call)!=1 or any(needle not in region[:region.index(call)]for needle in required):raise ValueError('Controls original source call/args changed '+tag)
        at=start+region.index(call)
        return part[:at]+block(tag+'-open',' { '+code)+call+block(tag+'-close',' }')+part[at+len(call):]
    anchor='#include "FunctionIds.h"\n';unique(anchor,anchor+block('include','#include "hd/ControlsLiveGameHooks.inc"'))
    if name=='Methods_01.cpp':
        # The existing text owner establishes the lexical ownerEpoch first.
        anchor=' nfs_controls_text::OwnerScope nfs_controls_owner(*this);'
        unique(anchor,anchor+block('owner-after-text',' nfs_controls_live::OwnerScope nfs_controls_live_owner(*this);'))
        entry('405B60',' nfs_controls_live::ResolvedScope nfs_controls_live_resolved(*this);')
        anchor=' nfs_controls_text::StringScope nfs_controls_string(*this);'
        unique(anchor,anchor+block('bind-after-string',' nfs_controls_live::bindText(*this);'))
        def normal(part):
            part=wrap(part,'loc_41020E','405B30','reset','nfs_controls_live::ResetScope nfs_controls_live_reset(*this);',('_sub_40EB20()','_sub_428A20()','eax = to32i(esi+4);','edx = 0x400;'))
            part=wrap(part,'loc_41020E','405B60','sidebar-copy','nfs_controls_live::SidebarScope nfs_controls_live_sidebar(*this);',('dword_4D2314+2','dword_4D231C+eax*4','eax+0x20','edx = 0x1000;'))
            anchor=' { nfs_controls_text::TailScope nfs_controls_tail(*this);'
            close='// NFS_CONTROLS_TEXT_END:tail-close\n'
            if part.count(anchor)!=1 or part.count(close)!=1 or part.index(close)<part.index(anchor):raise ValueError('Controls text tail nesting changed')
            between=part[part.index(anchor):part.index(close)]
            if between.count('\tesp -= 4; _sub_40FA90(); esp += 4; //call')!=1:raise ValueError('Controls text original tail call changed')
            # Outer scope must finish AFTER the existing text collector publishes.
            part=part.replace(anchor,block('tail-open',' { nfs_controls_live::TailScope nfs_controls_live_tail(*this);')+anchor,1)
            return part.replace(close,close+block('tail-close',' }'),1)
        body('4101C0',normal)
    elif name=='Methods_10.cpp':
        anchor=' nfs_controls_text::beforeDescriptorRetire(*this,uint32_t(eax));'
        unique(anchor,anchor+block('retired-descriptor',' nfs_controls_live::afterRetire(*this);'))
    elif name=='Methods_14.cpp':
        for tag,anchor in (('retired-owner',' nfs_controls_text::beforeOwnerRetire(*this,uint32_t(eax),true);'),('retired-descriptor',' nfs_controls_text::beforeDescriptorRetire(*this,uint32_t(eax));')):
            unique(anchor,anchor+block(tag,' nfs_controls_live::afterRetire(*this);'))
    # Every new callback executes outside checked-copy transactions, including
    # hooks nested inside pre-existing text/sidebar marker blocks.
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:',text):
        prefix=text[:match.start()]
        if prefix.count('// checked copy transaction begin')!=prefix.count('// checked copy transaction end'):raise ValueError('Controls live hook inside guest-copy lease')
    if crlf:text=text.replace('\n','\r\n')
    if remove_controls_live_hooks(text)!=original:raise ValueError('Controls live byte reversal failed')
    return text
