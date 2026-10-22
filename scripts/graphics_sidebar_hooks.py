"""Original-preserving Graphics-owned retained343 capture. Apply after Graphics live hooks.

Audio hooks remain independent and unchanged. Every splice is asserted, reversible,
and additive; source owner, lazy gate and original measurement supply authority.
"""
import re
MARKER="NFS_GRAPHICS_SIDEBAR"
NAMES=("Methods_01.cpp","Methods_09.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")

def add_graphics_sidebar_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Graphics sidebar hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Graphics sidebar anchor changed: "+name+" "+old[:80])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        m=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not m:raise ValueError("Graphics sidebar function changed: "+symbol)
        unique(m.group(),mutate(m.group()))
    a='#include "FunctionIds.h"\n';unique(a,a+block("include",'#include "hd/GraphicsSidebarLiveGameHooks.inc"'))
    if name=="Methods_01.cpp":
        a=" nfs_graphics_live::OwnerScope nfs_graphics_owner(*this);"
        unique(a,block("caption-owner-before-parent"," nfs_graphics_sidebar_text::OwnerScope nfs_graphics_sidebar_owner(*this);")+a+block("owner-after-parent"," nfs_graphics_sidebar_live::OwnerScope nfs_graphics_cache_owner(*this);"))
        entry("40C110"," nfs_graphics_sidebar_live::TailScope nfs_graphics_cache_tail(*this);\n nfs_graphics_sidebar_text::TailScope nfs_graphics_sidebar_tail(*this);")
        entry("405CF0"," nfs_graphics_sidebar_text::StringScope nfs_graphics_sidebar_string(*this);\n nfs_graphics_sidebar_live::bindCaption(*this);")
        entry("40BBE0"," nfs_graphics_sidebar_live::IterationScope nfs_graphics_cache_iteration(*this);")
        entry("405B60"," nfs_graphics_sidebar_live::ResolvedScope nfs_graphics_cache_resolved(*this);")
        def lazy(part):
            replacements=[(" nfs_hd_scene_iteration.start();"," nfs_graphics_cache_iteration.start();","lazy-after-original-gate"),(" { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);"," nfs_graphics_sidebar_live::RoleScope nfs_graphics_cache_side(*this,nfs_graphics_sidebar_live::Phase::side);","side"),(" { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);"," nfs_graphics_sidebar_live::RoleScope nfs_graphics_cache_word(*this,nfs_graphics_sidebar_live::Phase::word);","word"),(" nfs_background_scene_live::beginGroup(*this);"," nfs_graphics_sidebar_live::preGroup(*this);","group"),(" nfs_background_scene_live::postGroup(*this);"," nfs_graphics_sidebar_live::postGroup(*this);","group-complete")]
            if part.count("loc_40BC20:")!=1 or part.index("loc_40BC20:")>part.index(" nfs_hd_scene_iteration.start();"):raise ValueError("Graphics sidebar lazy sentinel changed")
            for anchor,code,tag in replacements:
                if part.count(anchor)!=1:raise ValueError("Graphics sidebar lazy source changed: "+tag)
                part=part.replace(anchor,anchor+block(tag,code),1)
            return part
        body("40BBE0",lazy)
        # Existing Audio observer is itself attached to the exact original482424
        # returns, before subtraction/color changes. Keep that witness unchanged.
        def measurements(part):
            for kind in ("right","center"):
                a=" nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::"+kind+");"
                if part.count(a)!=1:raise ValueError("Graphics sidebar actual measure changed: "+kind)
                part=part.replace(a,a+block("measured-"+kind," nfs_graphics_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_graphics_sidebar_text::SourceMeasureAlignment::"+kind+");"),1)
            return part
        body("405580",measurements)
    elif name=="Methods_09.cpp":entry("4824CC"," nfs_graphics_sidebar_text::SelectionScope nfs_graphics_sidebar_selection(*this);")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_graphics_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_graphics_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
        for symbol in ("482BD4","483288"):entry(symbol," nfs_graphics_sidebar_live::row(*this,uint32_t(game_id_sub_"+symbol+"));")
        entry("483438"," nfs_graphics_sidebar_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_graphics_sidebar_live::CopyWrapperScope nfs_graphics_cache_copy(*this);")
    elif name=="Methods_13.cpp":
        entry("4A14D4"," nfs_graphics_sidebar_live::row(*this,uint32_t(game_id_sub_4A14D4),2);")
        entry("4A5830"," nfs_graphics_sidebar_text::GlyphScope nfs_graphics_sidebar_glyph(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Graphics sidebar indexed dispatcher changed")
            pieces=part.split(call);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("indexed-open-"+str(k)," { nfs_graphics_sidebar_live::IndexedSideCallScope nfs_graphics_cache_indexed(*this,nfs_graphics_sidebar_live::indexedSideCallee(*this));")+call+block("indexed-close-"+str(k)," }")+piece
            return out
        body("4A10BC",dispatch)
    else:
        entry("4AC934"," nfs_graphics_sidebar_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_graphics_sidebar_live::beforeOwnerRetire(*this,uint32_t(eax),true);")
        entry("4AC9C8"," nfs_graphics_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_graphics_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Graphics sidebar original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_graphics_sidebar_text::WordCapture nfs_graphics_sidebar_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    return text
