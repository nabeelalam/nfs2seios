"""Original-preserving Controls-owned retained343 capture. Apply after Controls text hooks.

Audio hooks remain independent and unchanged. Every splice is asserted, reversible,
and additive; source owner, lazy gate and original measurement supply authority.
"""
# SPDX-License-Identifier: MIT
import re
MARKER="NFS_CONTROLS_SIDEBAR"
NAMES=("Methods_01.cpp","Methods_09.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")

def _add_controls_sidebar_hooks_lf(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Controls sidebar hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Controls sidebar anchor changed: "+name+" "+old[:80])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        m=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not m:raise ValueError("Controls sidebar function changed: "+symbol)
        unique(m.group(),mutate(m.group()))
    a='#include "FunctionIds.h"\n';unique(a,a+block("include",'#include "hd/ControlsSidebarLiveGameHooks.inc"'))
    if name=="Methods_01.cpp":
        a=" nfs_controls_text::OwnerScope nfs_controls_owner(*this);"
        unique(a,block("caption-owner-before-parent"," nfs_controls_sidebar_text::OwnerScope nfs_controls_sidebar_owner(*this);")+a+block("owner-after-parent"," nfs_controls_sidebar_live::OwnerScope nfs_controls_cache_owner(*this);"))
        entry("40C110"," nfs_controls_sidebar_live::TailScope nfs_controls_cache_tail(*this);\n nfs_controls_sidebar_text::TailScope nfs_controls_sidebar_tail(*this);")
        entry("405CF0"," nfs_controls_sidebar_text::StringScope nfs_controls_sidebar_string(*this);\n nfs_controls_sidebar_live::bindCaption(*this);")
        entry("40BBE0"," nfs_controls_sidebar_live::IterationScope nfs_controls_cache_iteration(*this);")
        entry("405B60"," nfs_controls_sidebar_live::ResolvedScope nfs_controls_cache_resolved(*this);")
        def lazy(part):
            replacements=[(" nfs_hd_scene_iteration.start();"," nfs_controls_cache_iteration.start();","lazy-after-original-gate"),(" { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);"," nfs_controls_sidebar_live::RoleScope nfs_controls_cache_side(*this,nfs_controls_sidebar_live::Phase::side);","side"),(" { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);"," nfs_controls_sidebar_live::RoleScope nfs_controls_cache_word(*this,nfs_controls_sidebar_live::Phase::word);","word"),(" nfs_background_scene_live::beginGroup(*this);"," nfs_controls_sidebar_live::preGroup(*this);","group"),(" nfs_background_scene_live::postGroup(*this);"," nfs_controls_sidebar_live::postGroup(*this);","group-complete")]
            if part.count("loc_40BC20:")!=1 or part.index("loc_40BC20:")>part.index(" nfs_hd_scene_iteration.start();"):raise ValueError("Controls sidebar lazy sentinel changed")
            gate=part[:part.index("loc_40BC20:")]
            for original in ("test(dx, dx);","loc_40BC02:","dword_4D2440","dword_5637F4","goto loc_40BD11;"):
                if original not in gate:raise ValueError("Controls sidebar original lazy gate changed")
            for anchor,code,tag in replacements:
                if part.count(anchor)!=1:raise ValueError("Controls sidebar lazy source changed: "+tag)
                part=part.replace(anchor,anchor+block(tag,code),1)
            return part
        body("40BBE0",lazy)
        # Controls TEXT hooks pin both actual original482424 returns before
        # subtraction. Preserve those calls; add independent343 measurement.
        def measurements(part):
            for kind in ("right","center"):
                a=" nfs_controls_text::dialogMeasured(*this,uint32_t(ecx),int32_t(eax),int32_t(esi),"+("2"if kind=="right"else"1")+");"
                if part.count(a)!=1:raise ValueError("Controls sidebar actual measure changed: "+kind)
                part=part.replace(a,a+block("measured-"+kind," nfs_controls_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_controls_sidebar_text::SourceMeasureAlignment::"+kind+");"),1)
            return part
        body("405580",measurements)
    elif name=="Methods_09.cpp":entry("4824CC"," nfs_controls_sidebar_text::SelectionScope nfs_controls_sidebar_selection(*this);")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_controls_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_controls_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
        for symbol in ("482BD4","483288"):entry(symbol," nfs_controls_sidebar_live::row(*this,uint32_t(game_id_sub_"+symbol+"));")
        entry("483438"," nfs_controls_sidebar_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_controls_sidebar_live::CopyWrapperScope nfs_controls_cache_copy(*this);")
    elif name=="Methods_13.cpp":
        entry("4A14D4"," nfs_controls_sidebar_live::row(*this,uint32_t(game_id_sub_4A14D4),2);")
        entry("4A5830"," nfs_controls_sidebar_text::GlyphScope nfs_controls_sidebar_glyph(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Controls sidebar indexed dispatcher changed")
            pieces=part.split(call);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("indexed-open-"+str(k)," { nfs_controls_sidebar_live::IndexedSideCallScope nfs_controls_cache_indexed(*this,nfs_controls_sidebar_live::indexedSideCallee(*this));")+call+block("indexed-close-"+str(k)," }")+piece
            return out
        body("4A10BC",dispatch)
    else:
        entry("4AC934"," nfs_controls_sidebar_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_controls_sidebar_live::beforeOwnerRetire(*this,uint32_t(eax),true);")
        entry("4AC9C8"," nfs_controls_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_controls_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Controls sidebar original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_controls_sidebar_text::WordCapture nfs_controls_sidebar_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    return text


def remove_controls_sidebar_hooks(text):
    result=re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched Controls sidebar marker')
    return result

def add_controls_sidebar_hooks(text,name):
    if name not in NAMES:return text
    original=text;crlf='\r\n'in text
    # Preserve exact LF/CRLF bytes; mixed sources must be normalized by their
    # owner explicitly rather than silently rewriting pre-existing hooks here.
    if crlf and '\n'in text.replace('\r\n',''):raise ValueError('Mixed original line endings')
    prepared=_add_controls_sidebar_hooks_lf(text.replace('\r\n','\n')if crlf else text,name)
    if crlf:prepared=prepared.replace('\n','\r\n')
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:',prepared):
        prefix=prepared[:match.start()]
        if prefix.count('// checked copy transaction begin')!=prefix.count('// checked copy transaction end'):
            raise ValueError('Controls sidebar hook inside checked-copy lease')
    if remove_controls_sidebar_hooks(prepared)!=original:raise ValueError('Controls sidebar byte reversal failed')
    return prepared
