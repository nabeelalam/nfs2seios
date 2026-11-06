"""Insertion-only slideshow owner412300 hooks; original operations remain exact."""
import re
MARK="NFS_PRIVATE_SLIDESHOW_UI"


def add_slideshow_owner_hooks(text,name):
    if name!="Methods_01.cpp":return text
    if MARK in text:raise ValueError("Slideshow hooks already inserted")
    old=text;serial=0
    def block(code):
        nonlocal serial
        serial+=1
        return "\n// "+MARK+"_BEGIN:"+str(serial)+"\n"+code+"\n// "+MARK+"_END:"+str(serial)+"\n"
    anchor='#include "Stubs.h"' if '#include "Stubs.h"' in text else '#include "FunctionIds.h"'
    if text.count(anchor)!=1:raise ValueError("Unexpected source includes")
    text=text.replace(anchor,anchor+block('#include "hd/ClassicScreens/SlideshowGameBridge.inc"'),1)
    match=re.search(r"Fn\(void\) Game::_sub_412300\(\)\s*\{",text)
    if not match:raise ValueError("Slideshow owner missing")
    end=text.find("\nFn(",match.end())
    if end<0:end=len(text)
    part=text[match.start():end]
    owner=block(' nfs_slideshow_live::configureOnEntry(*this);\n nfs_slideshow_live::Scope nfs_slideshow_owner(*this,nfs_slideshow_live::Event::Enter,nfs_slideshow_live::Event::Exit,uint32_t(eax),0,UINT32_MAX,UINT32_MAX);')
    at=part.index("{")+1;part=part[:at]+owner+part[at:]
    specs=[('410E80','ArchiveBefore','ArchiveAfter'),('412580','ArchiveBefore','ArchiveAfter'),
           ('405B30','BackgroundBefore','BackgroundAfter'),('40FA90','OverlayBefore','OverlayAfter'),
           ('405C70','PresentBefore','PresentAfter')]
    for symbol,before,after in specs:
        call='\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
        if part.count(call)!=1:raise ValueError("Unexpected slideshow call count: "+symbol)
        if before=='ArchiveBefore':
            pre=block(' nfs_slideshow_live::emit(*this,nfs_slideshow_live::Event::ArchiveBefore,uint32_t(edi),uint32_t(to32i(dword_4D40B8)),uint32_t(to32i(esp+8)),uint32_t(esi));')
            post=block(' nfs_slideshow_live::emit(*this,nfs_slideshow_live::Event::ArchiveAfter,uint32_t(edi),uint32_t(to32i(dword_4D40B8)),uint32_t(to32i(esp+8)),uint32_t(esi));')
            part=part.replace(call,pre+call+post,1)
        else:
            pre=block(' { nfs_slideshow_live::Scope nfs_slideshow_'+symbol+'(*this,nfs_slideshow_live::Event::'+before+',nfs_slideshow_live::Event::'+after+',uint32_t(edi),uint32_t(to32i(dword_4D40B8)),uint32_t(to32i(esp+8)),uint32_t(esi));')
            part=part.replace(call,pre+call+block(' }'),1)
    text=text[:match.start()]+part+text[end:]
    stripped=re.sub(r"\n// "+MARK+r"_BEGIN:(\d+)\n.*?\n// "+MARK+r"_END:\1\n","",text,flags=re.S)
    if stripped!=old:raise ValueError("Original source operations changed")
    return text


def add_slideshow_hooks(text,name):
    text=add_slideshow_owner_hooks(text,name)
    if name=='Methods_03.cpp':
        mark='NFS_PRIVATE_SLIDESHOW_QUEUE'
        if mark in text:raise ValueError('Slideshow queue hooks already inserted')
        old=text
        def insert(code):return '\n// '+mark+'_BEGIN:1\n'+code+'\n// '+mark+'_END:1\n'
        anchor='#include "FunctionIds.h"'
        if text.count(anchor)!=1:raise ValueError('Slideshow queue source includes changed')
        text=text.replace(anchor,anchor+insert('#include "hd/ClassicScreens/SlideshowQueueAdapter.inc"'),1)
        match=re.search(r'Fn\(void\) Game::_sub_426950\(\)\s*\{',text)
        if not match:raise ValueError('Slideshow presentation source missing')
        at=match.end();text=text[:at]+insert(' nfs_slideshow_live::DrawScope nfs_slideshow_draw(*this);')+text[at:]
        stripped=re.sub(r'\n// '+mark+r'_BEGIN:1\n.*?\n// '+mark+r'_END:1\n','',text,flags=re.S)
        if stripped!=old:raise ValueError('Original slideshow presentation changed')
        return text
    if name not in ('Methods_01.cpp','Methods_13.cpp'):return text
    mark='NFS_PRIVATE_SLIDESHOW_TEXT'
    if mark in text:raise ValueError('Slideshow text hooks already inserted')
    old=text;serial=0
    def block(code):
        nonlocal serial
        serial+=1
        return '\n// '+mark+'_BEGIN:'+str(serial)+'\n'+code+'\n// '+mark+'_END:'+str(serial)+'\n'
    if name=='Methods_13.cpp':
        anchor='#include "Stubs.h"' if '#include "Stubs.h"' in text else '#include "FunctionIds.h"'
        if text.count(anchor)!=1:raise ValueError('Unexpected text source includes')
        text=text.replace(anchor,anchor+block('#include "hd/ClassicScreens/SlideshowGameBridge.inc"'),1)
    def edit(symbol,fn):
        nonlocal text
        m=re.search(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\s*\{',text)
        if not m:raise ValueError('Text source routine missing: '+symbol)
        end=text.find('\nFn(',m.end())
        if end<0:end=len(text)
        text=text[:m.start()]+fn(text[m.start():end])+text[end:]
    def entry(symbol,code):
        def apply(part):
            at=part.index('{')+1
            return part[:at]+block(code)+part[at:]
        edit(symbol,apply)
    if name=='Methods_01.cpp':
        entry('405CF0',' nfs_slideshow_text::StringScope nfs_slideshow_string(*this);')
        for symbol in ('405580','4054E0'):
            entry(symbol,' nfs_slideshow_text::ResourceScope nfs_slideshow_resource(*this);')
        def measures(part):
            call='\tesp -= 4; _sub_482424(); esp += 4; //call'
            if part.count(call)!=2:raise ValueError('Text measurement source changed')
            pieces=part.split(call)
            return pieces[0]+''.join(call+block(' nfs_slideshow_text::measured(*this,uint32_t(ecx),int32_t(eax),'+str(2 if i==0 else 1)+');')+tail for i,tail in enumerate(pieces[1:]))
        edit('405580',measures)
    else:
        def stores(part):
            choices=['guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov','to16i(esi) = bx; //mov']
            matches=[call for call in choices if part.count(call)==4]
            if len(matches)!=1:raise ValueError('Unexpected glyph store source')
            call=matches[0]
            return part.replace(call,block(' { nfs_slideshow_text::Word nfs_slideshow_word(*this,uint32_t(esi),uint16_t(bx));')+call+block(' }'))
        edit('4A5830',stores)
    stripped=re.sub(r'\n// '+mark+r'_BEGIN:(\d+)\n.*?\n// '+mark+r'_END:\1\n','',text,flags=re.S)
    if stripped!=old:raise ValueError('Original text operations changed')
    return text
