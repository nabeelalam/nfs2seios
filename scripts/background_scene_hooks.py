"""Asserted original-preserving hooks for optional owned whole-menu composition.

Run after checked guest-copy and frontend transformations. No original sources,
asset identities or files are embedded. A missing trusted catalogue is fallback.
"""
import re

MARKER = "NFS_HD_BACKGROUND"


def add_background_scene_hooks(text, name):
    if name not in ("Methods_01.cpp", "Methods_03.cpp", "Methods_10.cpp", "Methods_13.cpp"):
        return text
    if MARKER in text:
        raise ValueError("Background hooks already present")
    def block(tag, code):
        return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old, new):
        nonlocal text
        if text.count(old) != 1:
            raise ValueError("Original background anchor changed: "+name)
        text = text.replace(old, new, 1)
    def entry(symbol, code):
        match = re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{\n", text)
        if not match:
            raise ValueError("Original background entry missing: "+symbol)
        unique(match.group(), match.group()+block("entry-"+symbol, code))
    def body(symbol, mutate):
        nonlocal text
        match = re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{\n", text)
        if not match:
            raise ValueError("Original background body missing: "+symbol)
        end = text.find("\nFn(", match.end())
        if end < 0:
            end = len(text)
        old = text[match.start():end]
        new = mutate(old)
        unique(old, new)
    def checked_replace(part, old, new, count=1):
        if part.count(old) != count:
            raise ValueError("Original scoped background call changed: "+name)
        return part.replace(old, new)
    def wrap_next(part, anchor, callee, tag, before, after=" }"):
        if part.count(anchor) != 1:
            raise ValueError("Original background argument anchor changed: "+tag)
        start = part.index(anchor)+len(anchor)
        call = "\tesp -= 4; _sub_"+callee+"(); esp += 4; //call"
        at = part.find(call, start)
        if at < 0:
            raise ValueError("Original background call missing: "+tag)
        return part[:at]+block(tag+"-open",before)+call+block(tag+"-close",after)+part[at+len(call):]
    include = '#include "FunctionIds.h"\n'
    unique(include, include+block("include", '#include "hd/BackgroundSceneQueueAdapter.inc"'))
    if name == "Methods_01.cpp":
        entry("40D3D0", " nfs_logo_publisher::MenuOwnerScope nfs_hd_menu(*this);\n nfs_background_scene_live::MainScope nfs_hd_scene_menu(*this);")
        entry("40BBE0", " nfs_logo_publisher::OwnerScope nfs_hd_iteration(*this,1);\n nfs_background_scene_live::IterationScope nfs_hd_scene_iteration(*this);")
        entry("405B60", " nfs_logo_publisher::ResolvedScope nfs_hd_logo_resolved(*this);\n nfs_background_scene_live::ResolvedScope nfs_hd_scene_resolved(*this);")
        def redraw(part):
            part=checked_replace(part,"loc_40BC20:\n","loc_40BC20:\n"+block("lazy-start"," nfs_hd_iteration.start();\n nfs_hd_scene_iteration.start();"))
            part=wrap_next(part,"guest_address(aSide)","405B30","side"," { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);")
            part=wrap_next(part,"off_4D2474+ecx*4","405B30","word"," { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);")
            # Exact current-inset argument lookup, following all label draws.
            lookup = re.search(r"\teax = [^\n]*esp\+eax\*4[^\n]*; //mov",part)
            if not lookup or len(re.findall(r"\teax = [^\n]*esp\+eax\*4[^\n]*; //mov",part))!=1:
                raise ValueError("Original animated-logo argument lookup changed")
            part=wrap_next(part,lookup.group(),"405B30","group"," nfs_logo_publisher::preGroup(*this);\n nfs_background_scene_live::beginGroup(*this);", "")
            part=wrap_next(part,"guest_address(aLogo_0)","405B30","group-finish","", " nfs_logo_publisher::postGroup(*this);\n nfs_background_scene_live::postGroup(*this);")
            part=checked_replace(part,"loc_40C047:\n","loc_40C047:\n"+block("tail-begin"," nfs_background_scene_live::tailBegin(*this);"))
            part=checked_replace(part,"loc_40C054:\n","loc_40C054:\n"+block("tail-end"," nfs_background_scene_live::tailEnd(*this);"))
            part=wrap_next(part,"loc_40C054:\n","405B60","copy"," nfs_logo_publisher::armCopy(*this);", " nfs_background_scene_live::copyEnd(*this);")
            return part
        body("40BBE0",redraw)
        def main(part):
            part=wrap_next(part,"loc_40D63E:\n","405B30","base"," { nfs_background_scene_live::RoleScope nfs_hd_base(*this,nfs_background_scene_live::Phase::Base);")
            call="\tesp -= 4; _sub_427B30(); esp += 4; //call"
            begin=" {\n nfs_logo_publisher::NativePresentationScope nfs_hd_route(*this);\n nfs_background_scene_live::Intent nfs_hd_intent;\n try {\n nfs_background_scene_live::begin(*this,nfs_hd_intent);"
            end=" nfs_background_scene_live::finish(nfs_hd_intent);\n } catch(...) {\n nfs_background_scene_live::cancel(nfs_hd_intent);\n throw;\n }\n }"
            return checked_replace(part,call,block("native-begin",begin)+call+block("native-finish",end))
        body("40D3D0",main)
    elif name == "Methods_03.cpp":
        entry("428250"," nfs_background_scene_live::UploadEndScope nfs_hd_upload_end(*this);\n nfs_logo_publisher::UploadScope nfs_hd_upload(*this);")
        def upload(part):
            call="\tesp -= 4; _sub_49755C(); esp += 4; //call"
            return checked_replace(part,call,block("tile-open"," { nfs_logo_publisher::UploadTileScope nfs_hd_tile(*this);")+call+block("tile-close"," }"))
        body("428250",upload)
        for symbol in ("4284B0","4287F0","4289F0"):
            entry(symbol," nfs_background_scene_live::invalidateCache();\n nfs_logo_publisher::resetNativeCache();")
    elif name == "Methods_10.cpp":
        # Central free entry covers all callers before the original window retires.
        entry("482DA0"," nfs_background_scene_live::retire(*this);\n nfs_logo_publisher::retireNative(*this);")
        for symbol in ("482BD4","482ED4","482FD4","483070","483100","4831A8","483288"):
            code=" nfs_logo_publisher::row(*this,uint32_t(game_id_sub_"+symbol+"));"
            if symbol=="482BD4":
                code+="\n nfs_background_scene_live::row(*this,uint32_t(game_id_sub_482BD4));"
            entry(symbol,code)
        entry("483438"," nfs_background_scene_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_background_scene_live::CopyWrapperScope nfs_hd_copy_wrapper(*this);")
    else:
        entry("4A14D4"," nfs_background_scene_live::byteCopyRow(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            return checked_replace(part,call,block("indexed-open"," { nfs_background_scene_live::IndexedSideCallScope nfs_hd_indexed(*this,nfs_background_scene_live::indexedSideCallee(*this));")+call+block("indexed-close"," }"),4)
        body("4A10BC",dispatch)
    return text
