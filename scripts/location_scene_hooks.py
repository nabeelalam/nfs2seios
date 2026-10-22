"""Original-preserving exact Location scene hooks; apply after background and caption hooks."""
import re
MARKER="NFS_LOCATION_SCENE"
NAMES=("Methods_01.cpp","Methods_03.cpp","Methods_10.cpp","Methods_13.cpp")
def add_location_scene_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Location scene hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Location scene anchor changed: "+name+" "+old[:64])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        anchor="Fn(void) Game::_sub_"+symbol+"()\n{\n"
        unique(anchor,anchor+block("entry-"+symbol,code))
    def after(old,tag,code):unique(old,old+block(tag,code))
    def body(symbol,mutate):
        nonlocal text
        match=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not match:raise ValueError("Location scene body changed: "+symbol)
        old=match.group();unique(old,mutate(old))
    def wrap(part,anchor,callee,tag,before,aftercode=" }"):
        if part.count(anchor)!=1:raise ValueError("Location scene call anchor changed: "+tag)
        at=part.find("\tesp -= 4; _sub_"+callee+"(); esp += 4; //call",part.index(anchor)+len(anchor))
        if at<0:raise ValueError("Location scene call missing: "+tag)
        call="\tesp -= 4; _sub_"+callee+"(); esp += 4; //call"
        return part[:at]+block(tag+"-open",before)+call+block(tag+"-close",aftercode)+part[at+len(call):]
    anchor='#include "FunctionIds.h"\n';unique(anchor,anchor+block("include",'#include "hd/LocationSceneQueueAdapter.inc"'))
    if name=="Methods_01.cpp":
        # Construct before caption owner/tail; destruct after its finalized packet.
        entry("40C160"," nfs_logo_publisher::LocationMenuOwnerScope nfs_location_menu(*this);\n nfs_location_scene_live::LocationScope nfs_location_scene(*this);")
        entry("40C110"," nfs_location_scene_live::TailScope nfs_location_scene_tail(*this);")
        entry("40BBE0"," nfs_location_scene_live::IterationScope nfs_location_iteration(*this);")
        entry("405B60"," nfs_location_scene_live::ResolvedScope nfs_location_resolved(*this);")
        after(" nfs_hd_scene_iteration.start();","lazy"," nfs_location_iteration.start();")
        for call,tag in (("beginGroup","group"),("postGroup","group-end"),("copyEnd","copy")):
            after(" nfs_background_scene_live::"+call+"(*this);",tag," nfs_location_scene_live::"+call+"(*this);")
        def redraw(part):
            part=wrap(part,"guest_address(aSide)","405B30","side"," { nfs_location_scene_live::RoleScope nfs_location_side(*this,nfs_location_scene_live::Phase::Side);")
            return wrap(part,"off_4D2474+ecx*4","405B30","word"," { nfs_location_scene_live::RoleScope nfs_location_word(*this,nfs_location_scene_live::Phase::Word);")
        body("40BBE0",redraw)
        def location(part):
            for label,tag in (("loc_40C3CC:\n","normal-base-initial"),("loc_40C5C6:\n","normal-base-refresh")):
                if part.count(label)!=1:raise ValueError("Location normal base label changed: "+tag)
                start=part.index(label);end=part.find("\tesp -= 4; _sub_405B30(); esp += 4; //call",start)
                selected=part[start:end] if end>=0 else ""
                if selected.count("guest_address(aBlk4)")!=1 or "dword_4D4D24" not in selected or "_sub_4825A8();" not in selected:
                    raise ValueError("Location normal base arguments changed: "+tag)
                part=wrap(part,label,"405B30",tag," { nfs_location_scene_live::RoleScope nfs_location_normal_base(*this,nfs_location_scene_live::Phase::Base,nfs_location_scene_live::BaseArgument::Blk4Direct);")
            part=wrap(part,"loc_40C792:\n","405B30","base"," { nfs_location_scene_live::RoleScope nfs_location_base(*this,nfs_location_scene_live::Phase::Base);")
            # Exact current full-menu draw, not the grey/animation sibling calls.
            anchor="\tpush32(0xFFFFFFFF);\n\tecx = 0x20; //mov\n\teax = 1; //mov\n\txor_(edx, edx);\n"
            begin=" {\n nfs_logo_publisher::NativePresentationScope nfs_location_route(*this);\n nfs_location_scene_live::Intent nfs_location_intent;\n try {\n nfs_location_scene_live::begin(*this,nfs_location_intent);"
            end=" nfs_location_scene_live::finish(nfs_location_intent);\n } catch(...) {\n nfs_location_scene_live::cancel(nfs_location_intent);\n throw;\n }\n }"
            return wrap(part,anchor,"427B30","native",begin,end)
        body("40C160",location)
    elif name=="Methods_03.cpp":
        entry("428250"," nfs_location_scene_live::UploadEndScope nfs_location_upload_end(*this);")
        for symbol in ("4284B0","4287F0","4289F0"):entry(symbol," nfs_location_scene_live::invalidateCache();")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_location_scene_live::retire(*this);")
        entry("482BD4"," nfs_location_scene_live::row(*this,uint32_t(game_id_sub_482BD4));")
        entry("483438"," nfs_location_scene_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_location_scene_live::CopyWrapperScope nfs_location_copy_wrapper(*this);")
    else:
        entry("4A14D4"," nfs_location_scene_live::byteCopyRow(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Location indexed dispatch changed")
            return part.replace(call,block("indexed-open"," { nfs_location_scene_live::IndexedSideCallScope nfs_location_indexed(*this,nfs_location_scene_live::indexedSideCallee(*this));")+call+block("indexed-close"," }"))
        body("4A10BC",dispatch)
    return text
