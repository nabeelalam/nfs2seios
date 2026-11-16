"""Asserted original-preserving hooks for the exact optional Opponent caption.

No original source, asset pixels, content keys or private paths are embedded.
The producer remains disabled without configured artwork and an owned font.
"""
import re

MARKER = "NFS_OPPONENT_CAPTION"
ENTRIES = {
    "Methods_01.cpp": {"40C160": "OwnerScope", "40C110": "TailScope", "405CF0": "StringScope", "405580": "ResourceScope"},
    "Methods_09.cpp": {"4824CC": "SelectionScope"},
    "Methods_10.cpp": {"482DA0": "retire-descriptor"},
    "Methods_13.cpp": {"4A5830": "GlyphScope"},
    "Methods_14.cpp": {"4AC934": "retire-selected", "4AC9C8": "retire-descriptor"},
}


def add_opponent_caption_hooks(text, name):
    if name not in ENTRIES:
        return text
    if MARKER in text:
        raise ValueError("Opponent caption hooks already present")

    def block(tag, code):
        return "\n// " + MARKER + "_BEGIN:" + tag + "\n" + code + "\n// " + MARKER + "_END:" + tag + "\n"

    anchor = '#include "FunctionIds.h"\n'
    if text.count(anchor) != 1:
        raise ValueError("Opponent caption include anchor changed: " + name)
    text = text.replace(anchor, anchor + block("include", '#include "hd/OpponentCaptionGameHooks.inc"\n#include "hd/OpponentInitialSidebarGameHooks.inc"'), 1)
    for symbol, role in ENTRIES[name].items():
        anchor = "Fn(void) Game::_sub_" + symbol + "()\n{\n"
        if text.count(anchor) != 1:
            raise ValueError("Opponent caption entry changed: " + symbol)
        if role == "retire-descriptor":
            code = " nfs_opponent_caption::beforeDescriptorRetire(*this,uint32_t(eax));"
        elif role == "retire" or role == "retire-selected":
            code = " nfs_opponent_caption::beforeOwnerRetire(*this,uint32_t(eax)" + (",true" if role == "retire-selected" else "") + ");"
        else:
            code = " nfs_opponent_caption::" + role + " nfs_opponent_" + symbol.lower() + "(*this);"
            if symbol=="40C160":code=" nfs_opponent_initial_sidebar_live::OwnerScope nfs_opponent_initial_owner(*this);\n"+code
        text = text.replace(anchor, anchor + block("entry-" + symbol, code), 1)
    if name=="Methods_01.cpp":
        pattern=r"Fn\(void\) Game::_sub_405580\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)"
        match=re.search(pattern,text,re.S)
        if not match:raise ValueError("Opponent resource body missing")
        part=match.group();source="\tesp -= 4; _sub_41B710(); esp += 4; //call"
        if part.count(source)!=1:raise ValueError("Opponent resource resolution changed")
        part=part.replace(source,source+block("resource-resolved"," nfs_opponent_caption::resolvedResource(*this,uint32_t(eax));"),1)
        for kind in ("right","center"):
            anchor=" nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::"+kind+");"
            if part.count(anchor)!=1:raise ValueError("Opponent measurement changed: "+kind)
            part=part.replace(anchor,anchor+block("measured-"+kind," nfs_opponent_caption::measured(*this,uint32_t(ecx),int32_t(eax),nfs_opponent_caption::SourceMeasureAlignment::"+kind+");"),1)
        text=text[:match.start()]+part+text[match.end():]
        anchor='// NFS_PLAYER_SCENE_END:initial-caption-open\n'
        if text.count(anchor)!=1:raise ValueError("Opponent exact initial caption owner missing")
        text=text.replace(anchor,anchor+block("initial-caller"," nfs_opponent_caption::InitialCallerScope nfs_opponent_initial(*this,nfs_player_main_origin);"),1)
        seal=' nfs_player_scene_live::completeInitialCaption(*this);'
        if text.count(seal)!=1:raise ValueError("Opponent initial seal missing")
        text=text.replace(seal,seal+block("initial-seal"," nfs_opponent_initial_sidebar_live::completeInitialCaption(*this);"),1)
    if name in ("Methods_13.cpp", "Methods_14.cpp"):
        symbol, count = ("4A5830", 4) if name == "Methods_13.cpp" else ("4B3EE0", 1)
        pattern = r"Fn\(void\) Game::_sub_" + symbol + r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)"
        match = re.search(pattern, text, re.S)
        if not match:
            raise ValueError("Opponent glyph body changed: " + symbol)
        old = match.group()
        store = "guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
        if old.count(store) != count:
            raise ValueError("Opponent original glyph stores changed: " + symbol)
        parts = old.split(store)
        new = parts[0]
        for k, part in enumerate(parts[1:]):
            new += block("word-open-" + str(k), " { nfs_opponent_caption::WordCapture nfs_opponent_word(*this,uint32_t(esi),uint16_t(bx));")
            new += store
            new += block("word-close-" + str(k), " }") + part
        text = text[:match.start()] + new + text[match.end():]
    return text
