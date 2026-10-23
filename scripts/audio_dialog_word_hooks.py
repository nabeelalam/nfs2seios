"""Additive genuine original Audio full-dialog finite body-word observations."""
# SPDX-License-Identifier: MIT
import re
MARKER="NFS_AUDIO_DIALOG_WORD"
NAMES=("Methods_01.cpp",)
def add_audio_dialog_word_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Audio dialog words already present")
    nl="\r\n" if "\r\n" in text else "\n"
    def block(tag,code):return nl+"// "+MARKER+"_BEGIN:"+tag+nl+code+nl+"// "+MARKER+"_END:"+tag+nl
    def one(source,code,tag):
        nonlocal text
        if text.count(source)!=1:raise ValueError("Audio dialog source anchor changed: "+tag)
        text=text.replace(source,source+block(tag,code),1)
    one('#include "FunctionIds.h"'+nl,'#include "hd/AudioTextGameHooks.inc"','include')
    head='Fn(void) Game::_sub_405580()'+nl+'{'+nl
    one(head,' nfs_audio_text::DialogPolicyScope nfs_audio_dialog_policy(*this);','policy')
    start=text.index(head);end=text.find(nl+'Fn(void) Game::',start+len(head));end=len(text)if end<0 else end
    body=text[start:end]
    call='\tesp -= 4; _sub_41B710(); esp += 4; //call'
    if body.count(call)!=1:raise ValueError("Audio dialog original41B710 changed")
    def outside_lease(at):
        prefix=body[:at]
        if prefix.count("// checked copy transaction begin")!=prefix.count("// checked copy transaction end"):raise ValueError("Audio dialog observation inside guest lease")
    outside_lease(body.index(call))
    body=body.replace(call,call+block('resolved',' nfs_audio_text::dialogResolvedResource(*this,uint32_t(eax));'),1)
    anchor='// NFS_AUDIO_SIDEBAR_MEASURE_END:measured-right'+nl
    if body.count(anchor)!=1:raise ValueError("Audio original right-measure hook absent")
    outside_lease(body.index(anchor))
    body=body.replace(anchor,anchor+block('measured',' nfs_audio_text::dialogMeasured(*this,uint32_t(ecx),int32_t(eax),int32_t(esi));'),1)
    # Original register subtraction must remain after the observation and outside
    # every checked-copy block. Existing Audio/Setup/Options/Player hooks remain.
    if body.index('nfs_audio_text::dialogMeasured')>body.index('\tsub(esi, eax);'):raise ValueError("Audio measurement no longer precedes alignment")
    return text[:start]+body+text[end:]
def remove_audio_dialog_word_hooks(text):
    return re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
