"""Asserted original-preserving Audio owner, actual callback and text hooks."""
import re
MARKER="NFS_AUDIO_LIVE"
NAMES=("Methods_01.cpp","Methods_09.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")
def add_audio_live_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Audio live hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(anchor,replacement):
        nonlocal text
        if text.count(anchor)!=1:raise ValueError("Audio live anchor changed: "+name+" "+anchor[:60])
        text=text.replace(anchor,replacement,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        m=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not m:raise ValueError("Audio live body changed: "+symbol)
        unique(m.group(),mutate(m.group()))
    def wrap(part,label,callee,tag,code,required=()):
        a=label+":\n"
        if part.count(a)!=1:raise ValueError("Audio live label changed: "+label)
        start=part.index(a);end=part.find("\nloc_",start+len(a));end=len(part)if end<0 else end
        call="\tesp -= 4; _sub_"+callee+"(); esp += 4; //call";region=part[start:end]
        if region.count(call)!=1:raise ValueError("Audio live normal call changed: "+tag)
        at=part.index(call,start)
        if any(v not in part[start:at]for v in required):raise ValueError("Audio live normal arguments changed: "+tag)
        return part[:at]+block(tag+"-open"," { "+code)+call+block(tag+"-close"," }")+part[at+len(call):]
    anchor='#include "FunctionIds.h"\n';unique(anchor,anchor+block("include",'#include "hd/AudioLiveGameHooks.inc"\n#include "hd/AudioSidebarCaptionGameHooks.inc"'))
    if name=="Methods_01.cpp":
        entry("40A570"," nfs_audio_text::OwnerScope nfs_audio_text_owner(*this);\n nfs_audio_sidebar_text::OwnerScope nfs_audio_sidebar_owner(*this);\n nfs_audio_live::OwnerScope nfs_audio_owner(*this);")
        entry("40C110"," nfs_audio_sidebar_text::TailScope nfs_audio_sidebar_tail(*this);")
        entry("405B30"," nfs_audio_live::NamedScope nfs_audio_named(*this);")
        entry("405B60"," nfs_audio_live::ResolvedScope nfs_audio_resolved(*this);")
        entry("405CF0"," nfs_audio_text::StringScope nfs_audio_string(*this);\n nfs_audio_sidebar_text::StringScope nfs_audio_sidebar_string(*this);\n nfs_audio_live::bindText(*this);")
        def normal(part):
            part=wrap(part,"loc_40A8C8","405B30","reset","nfs_audio_live::ResetScope nfs_audio_reset(*this);",("_sub_40EB20()","_sub_428A20()","eax = to32i(esi+4);","edx = 0x400;"))
            part=wrap(part,"loc_40A8C8","405B60","sidebar","nfs_audio_live::SidebarScope nfs_audio_sidebar(*this);",("dword_4D2314+2","dword_4D231C+eax*4","eax+0x20","edx = 0x1000;"))
            return wrap(part,"loc_40AB1D","40FA90","tail","nfs_audio_live::TailScope nfs_audio_tail(*this);\n nfs_audio_text::TailScope nfs_audio_text_tail(*this);",("eax = esi;",))
        body("40A570",normal)
    elif name=="Methods_09.cpp":entry("4824CC"," nfs_audio_text::SelectionScope nfs_audio_selection(*this);\n nfs_audio_sidebar_text::SelectionScope nfs_audio_sidebar_selection(*this);")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_audio_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_audio_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_audio_live::afterRetire(*this);")
        for symbol in ("482BD4","483438","486BC8"):entry(symbol," nfs_audio_live::RowScope nfs_audio_row(*this,uint32_t(game_id_sub_"+symbol+"));")
    elif name=="Methods_13.cpp":entry("4A5830"," nfs_audio_text::GlyphScope nfs_audio_glyph(*this);\n nfs_audio_sidebar_text::GlyphScope nfs_audio_sidebar_glyph(*this);")
    else:
        entry("4AC934"," nfs_audio_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_audio_sidebar_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_audio_live::afterRetire(*this);")
        entry("4AC9C8"," nfs_audio_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_audio_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_audio_live::afterRetire(*this);")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Audio original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_audio_text::WordCapture nfs_audio_word(*this,uint32_t(esi),uint16_t(bx));\n nfs_audio_sidebar_text::WordCapture nfs_audio_sidebar_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    return text
