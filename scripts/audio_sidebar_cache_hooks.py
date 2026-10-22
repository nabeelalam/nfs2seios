"""Original-preserving Audio-owned lazy343 cache hooks. Apply after Audio live hooks."""
import re
MARKER="NFS_AUDIO_SIDEBAR_CACHE"
NAMES=("Methods_01.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")
def add_audio_sidebar_cache_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Audio sidebar cache hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Audio sidebar anchor changed: "+name+" "+old[:70])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        m=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not m:raise ValueError("Audio sidebar function changed: "+symbol)
        unique(m.group(),mutate(m.group()))
    unique('#include "FunctionIds.h"\n','#include "FunctionIds.h"\n'+block("include",'#include "hd/AudioSidebarLiveGameHooks.inc"'))
    if name=="Methods_01.cpp":
        a=" nfs_audio_live::OwnerScope nfs_audio_owner(*this);";unique(a,a+block("owner-after-parent"," nfs_audio_sidebar_live::OwnerScope nfs_audio_cache_owner(*this);"))
        a=" nfs_audio_sidebar_text::TailScope nfs_audio_sidebar_tail(*this);";unique(a,block("caption-tail-before-text"," nfs_audio_sidebar_live::TailScope nfs_audio_cache_tail(*this);")+a)
        a=" nfs_audio_live::bindText(*this);";unique(a,a+block("first-caption"," nfs_audio_sidebar_live::bindCaption(*this);"))
        entry("40BBE0"," nfs_audio_sidebar_live::IterationScope nfs_audio_cache_iteration(*this);")
        entry("405B60"," nfs_audio_sidebar_live::ResolvedScope nfs_audio_cache_resolved(*this);")
        def lazy(part):
            replacements=[(" nfs_hd_scene_iteration.start();"," nfs_audio_cache_iteration.start();","lazy-after-original-gate"),(" { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);"," nfs_audio_sidebar_live::RoleScope nfs_audio_cache_side(*this,nfs_audio_sidebar_live::Phase::side);","side"),(" { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);"," nfs_audio_sidebar_live::RoleScope nfs_audio_cache_word(*this,nfs_audio_sidebar_live::Phase::word);","word"),(" nfs_background_scene_live::beginGroup(*this);"," nfs_audio_sidebar_live::preGroup(*this);","group"),(" nfs_background_scene_live::postGroup(*this);"," nfs_audio_sidebar_live::postGroup(*this);","group-complete")]
            if part.count("loc_40BC20:")!=1 or part.index("loc_40BC20:")>part.index(" nfs_hd_scene_iteration.start();"):raise ValueError("Audio sidebar lazy sentinel changed")
            for anchor,code,tag in replacements:
                if part.count(anchor)!=1:raise ValueError("Audio sidebar lazy source changed: "+tag)
                part=part.replace(anchor,anchor+block(tag,code),1)
            return part
        body("40BBE0",lazy)
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_audio_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
        for symbol in ("482BD4","483288"):
            entry(symbol," nfs_audio_sidebar_live::row(*this,uint32_t(game_id_sub_"+symbol+"));")
        entry("483438"," nfs_audio_sidebar_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_audio_sidebar_live::CopyWrapperScope nfs_audio_cache_copy(*this);")
    elif name=="Methods_13.cpp":
        entry("4A14D4"," nfs_audio_sidebar_live::row(*this,uint32_t(game_id_sub_4A14D4),2);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Audio sidebar indexed dispatcher changed")
            pieces=part.split(call);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("indexed-open-"+str(k)," { nfs_audio_sidebar_live::IndexedSideCallScope nfs_audio_cache_indexed(*this,nfs_audio_sidebar_live::indexedSideCallee(*this));")+call+block("indexed-close-"+str(k)," }")+piece
            return out
        body("4A10BC",dispatch)
    else:
        entry("4AC934"," nfs_audio_sidebar_live::beforeOwnerRetire(*this,uint32_t(eax),true);")
        entry("4AC9C8"," nfs_audio_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
    return text
