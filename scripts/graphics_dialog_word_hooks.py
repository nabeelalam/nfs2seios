"""Additive genuine original Graphics full-dialog finite body-word observations."""
# SPDX-License-Identifier: MIT
import re
MARKER="NFS_GRAPHICS_DIALOG_WORD"
NAMES=("Methods_01.cpp",)
def add_graphics_dialog_word_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Graphics dialog words already present")
    nl="\r\n" if "\r\n" in text else "\n"
    def block(tag,code):return nl+"// "+MARKER+"_BEGIN:"+tag+nl+code+nl+"// "+MARKER+"_END:"+tag+nl
    def one(source,code,tag):
        nonlocal text
        if text.count(source)!=1:raise ValueError("Graphics dialog source anchor changed: "+tag)
        text=text.replace(source,source+block(tag,code),1)
    one('#include "FunctionIds.h"'+nl,'#include "hd/GraphicsTextGameHooks.inc"','include')
    head='Fn(void) Game::_sub_405580()'+nl+'{'+nl
    one(head,' nfs_graphics_text::DialogPolicyScope nfs_graphics_dialog_policy(*this);','policy')
    start=text.index(head);end=text.find(nl+'Fn(void) Game::',start+len(head));end=len(text)if end<0 else end
    body=text[start:end]
    call='\tesp -= 4; _sub_41B710(); esp += 4; //call'
    if body.count(call)!=1:raise ValueError("Graphics dialog original41B710 changed")
    def outside_lease(at):
        prefix=body[:at]
        if prefix.count("// checked copy transaction begin")!=prefix.count("// checked copy transaction end"):raise ValueError("Graphics dialog observation inside guest lease")
    outside_lease(body.index(call))
    body=body.replace(call,call+block('resolved',' nfs_graphics_text::dialogResolvedResource(*this,uint32_t(eax));'),1)
    for label,kind in (("right",2),("center",1)):
        anchor='// NFS_AUDIO_SIDEBAR_MEASURE_END:measured-'+label+nl
        if body.count(anchor)!=1:raise ValueError("Graphics original measure hook absent: "+label)
        outside_lease(body.index(anchor))
        body=body.replace(anchor,anchor+block('measured-'+label,' nfs_graphics_text::dialogMeasured(*this,uint32_t(ecx),int32_t(eax),int32_t(esi),'+str(kind)+');'),1)
    anchor='// NFS_GRAPHICS_LIVE_END:resource-alignment'+nl
    if body.count(anchor)!=1:raise ValueError("Graphics original alignment hook absent")
    outside_lease(body.index(anchor))
    body=body.replace(anchor,anchor+block('resource-alignment',' nfs_graphics_text::dialogResourceAlignment(*this);'),1)
    # Original register subtraction must remain after the observation and outside
    # every checked-copy block. Existing Graphics/Setup/Options/Player hooks remain.
    if body.index('nfs_graphics_text::dialogMeasured')>body.index('\tsub(esi, eax);'):raise ValueError("Graphics measurement no longer precedes alignment")
    return text[:start]+body+text[end:]
def remove_graphics_dialog_word_hooks(text):
    return re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
