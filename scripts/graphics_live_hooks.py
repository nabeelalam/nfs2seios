"""Asserted original-preserving Graphics complete-frame hooks.
"""
import re
MARKER="NFS_GRAPHICS_LIVE"
NAMES=("Methods_01.cpp","Methods_09.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")
def add_graphics_live_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Graphics live already applied")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(a,b):
        nonlocal text
        if text.count(a)!=1:raise ValueError("Graphics source anchor changed: "+name+" "+a[:100])
        text=text.replace(a,b,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        m=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not m:raise ValueError("Graphics live function changed: "+symbol)
        unique(m.group(),mutate(m.group()))
    def wrap(part,label,callee,tag,code,required=()):
        a=label+":\n"
        if part.count(a)!=1:raise ValueError("Graphics label changed: "+label)
        start=part.index(a);end=part.find("\nloc_",start+len(a));end=len(part)if end<0 else end
        call="\tesp -= 4; _sub_"+callee+"(); esp += 4; //call";region=part[start:end]
        if region.count(call)!=1 or any(v not in region[:region.index(call)]for v in required):raise ValueError("Graphics normal call/args changed: "+tag)
        at=part.index(call,start)
        return part[:at]+block(tag+"-open"," { "+code)+call+block(tag+"-close"," }")+part[at+len(call):]
    a='#include "FunctionIds.h"\n';unique(a,a+block("include",'#include "hd/GraphicsLiveGameHooks.inc"'))
    if name=="Methods_01.cpp":
        entry("4101C0"," nfs_graphics_text::OwnerScope nfs_graphics_text_owner(*this);\n nfs_graphics_live::OwnerScope nfs_graphics_owner(*this);")
        entry("405B60"," nfs_graphics_live::ResolvedScope nfs_graphics_resolved(*this);")
        entry("405580"," nfs_graphics_text::PolicyScope nfs_graphics_policy(*this,false);")
        entry("4054E0"," nfs_graphics_text::PolicyScope nfs_graphics_policy(*this,true);")
        def policy(part):
            a="\tesp -= 4; _sub_41B690(); esp += 4; //call"
            if part.count(a)!=1:raise ValueError("Graphics alignment helper changed")
            return part.replace(a,a+block("resource-alignment"," nfs_graphics_text::resourceAlignment(*this);"),1)
        body("405580",policy)
        entry("405CF0"," nfs_graphics_text::StringScope nfs_graphics_string(*this);\n nfs_graphics_live::bindText(*this);")
        def normal(part):
            part=wrap(part,"loc_41020E","405B30","reset","nfs_graphics_live::ResetScope nfs_graphics_reset(*this);",("_sub_40EB20()","_sub_428A20()","eax = to32i(esi+4);","edx = 0x400;"))
            part=wrap(part,"loc_41020E","405B60","sidebar-copy","nfs_graphics_live::SidebarScope nfs_graphics_sidebar(*this);",("dword_4D2314+2","dword_4D231C+eax*4","eax+0x20","edx = 0x1000;"))
            return wrap(part,"loc_41020E","40FA90","full-text","nfs_graphics_live::TailScope nfs_graphics_tail(*this);\n nfs_graphics_text::TailScope nfs_graphics_text_tail(*this);",("eax = esi;",))
        body("4101C0",normal)
    elif name=="Methods_09.cpp":entry("4824CC"," nfs_graphics_text::SelectionScope nfs_graphics_selection(*this);")
    elif name=="Methods_10.cpp":entry("482DA0"," nfs_graphics_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_graphics_live::afterRetire(*this);")
    elif name=="Methods_13.cpp":entry("4A5830"," nfs_graphics_text::GlyphScope nfs_graphics_glyph(*this);")
    else:
        entry("4AC934"," nfs_graphics_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_graphics_live::afterRetire(*this);")
        entry("4AC9C8"," nfs_graphics_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_graphics_live::afterRetire(*this);")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Graphics original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_graphics_text::WordCapture nfs_graphics_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    # Removal of observer blocks must reproduce the exact original source bytes.
    restored=re.sub(r"\n// "+MARKER+r"_BEGIN:[^\n]*\n.*?\n// "+MARKER+r"_END:[^\n]*\n","",text,flags=re.S)
    return text
