"""Original-preserving exact Setup scene hooks; apply last, after Options hooks."""
import re
MARKER="NFS_SETUP_SCENE"
NAMES=("Methods_01.cpp","Methods_03.cpp","Methods_10.cpp","Methods_13.cpp","Methods_09.cpp","Methods_14.cpp")
def add_setup_scene_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Setup scene hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Setup scene anchor changed: "+name+" "+old[:64])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        anchor="Fn(void) Game::_sub_"+symbol+"()\n{\n"
        unique(anchor,anchor+block("entry-"+symbol,code))
    def after(old,tag,code):unique(old,old+block(tag,code))
    def body(symbol,mutate):
        nonlocal text
        match=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not match:raise ValueError("Setup scene body changed: "+symbol)
        old=match.group();unique(old,mutate(old))
    def wrap(part,anchor,callee,tag,before,aftercode=" }"):
        if part.count(anchor)!=1:raise ValueError("Setup scene call anchor changed: "+tag)
        at=part.find("\tesp -= 4; _sub_"+callee+"(); esp += 4; //call",part.index(anchor)+len(anchor))
        if at<0:raise ValueError("Setup scene call missing: "+tag)
        call="\tesp -= 4; _sub_"+callee+"(); esp += 4; //call"
        return part[:at]+block(tag+"-open",before)+call+block(tag+"-close",aftercode)+part[at+len(call):]
    anchor='#include "FunctionIds.h"\n';unique(anchor,anchor+block("include",'#include "hd/SetupSceneQueueAdapter.inc"'))
    if name=="Methods_01.cpp":
        # Construct before caption owner/tail; destruct after its finalized packet.
        entry("40C160"," nfs_logo_publisher::SetupMenuOwnerScope nfs_setup_menu(*this);\n nfs_setup_caption::OwnerScope nfs_setup_caption_owner(*this);\n nfs_setup_scene_live::SetupScope nfs_setup_scene(*this);")
        entry("40C110"," nfs_setup_scene_live::TailScope nfs_setup_scene_tail(*this);\n nfs_setup_caption::TailScope nfs_setup_caption_tail(*this);")
        entry("405CF0"," nfs_setup_caption::StringScope nfs_setup_caption_string(*this);")
        entry("40BBE0"," nfs_setup_scene_live::IterationScope nfs_setup_iteration(*this);")
        entry("405B60"," nfs_setup_scene_live::ResolvedScope nfs_setup_resolved(*this);")
        after(" nfs_hd_scene_iteration.start();","lazy"," nfs_setup_iteration.start();")
        for call,tag in (("beginGroup","group"),("postGroup","group-end"),("copyEnd","copy")):
            after(" nfs_background_scene_live::"+call+"(*this);",tag," nfs_setup_scene_live::"+call+"(*this);")
        def redraw(part):
            part=wrap(part,"guest_address(aSide)","405B30","side"," { nfs_setup_scene_live::RoleScope nfs_setup_side(*this,nfs_setup_scene_live::Phase::Side);")
            return wrap(part,"off_4D2474+ecx*4","405B30","word"," { nfs_setup_scene_live::RoleScope nfs_setup_word(*this,nfs_setup_scene_live::Phase::Word);")
        body("40BBE0",redraw)
        def measurements(part):
            for kind in ("right","center"):
                a=" nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::"+kind+");"
                if part.count(a)!=1:raise ValueError("Setup original measurement changed: "+kind)
                part=part.replace(a,a+block("measured-"+kind," nfs_setup_caption::measured(*this,uint32_t(ecx),int32_t(eax),nfs_setup_caption::SourceMeasureAlignment::"+kind+");"),1)
            return part
        body("405580",measurements)
        def setup(part):
            for label,tag in (("loc_40C3CC:\n","normal-base-initial"),("loc_40C5C6:\n","normal-base-refresh")):
                if part.count(label)!=1:raise ValueError("Setup normal base label changed: "+tag)
                start=part.index(label);end=part.find("\tesp -= 4; _sub_405B30(); esp += 4; //call",start)
                selected=part[start:end] if end>=0 else ""
                if selected.count("guest_address(aBlk4)")!=1 or "dword_4D4D24" not in selected or "_sub_4825A8();" not in selected:
                    raise ValueError("Setup normal base arguments changed: "+tag)
                part=wrap(part,label,"405B30",tag," { nfs_setup_scene_live::RoleScope nfs_setup_normal_base(*this,nfs_setup_scene_live::Phase::Base,nfs_setup_scene_live::BaseArgument::Blk4Direct);")
            part=wrap(part,"loc_40C792:\n","405B30","base"," { nfs_setup_scene_live::RoleScope nfs_setup_base(*this,nfs_setup_scene_live::Phase::Base);")
            # Exact current full-menu draw, not the grey/animation sibling calls.
            anchor="\tpush32(0xFFFFFFFF);\n\tecx = 0x20; //mov\n\teax = 1; //mov\n\txor_(edx, edx);\n"
            begin=" {\n nfs_setup_scene_live::Intent nfs_setup_intent;\n try {\n nfs_setup_scene_live::begin(*this,nfs_setup_intent);"
            end=" nfs_setup_scene_live::finish(nfs_setup_intent);\n } catch(...) {\n nfs_setup_scene_live::cancel(nfs_setup_intent);\n throw;\n }\n }"
            return wrap(part,anchor,"427B30","native",begin,end)
        body("40C160",setup)
    elif name=="Methods_03.cpp":
        entry("428250"," nfs_setup_scene_live::UploadEndScope nfs_setup_upload_end(*this);")
        for symbol in ("4284B0","4287F0","4289F0"):entry(symbol," nfs_setup_scene_live::invalidateCache();")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_setup_caption::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_setup_scene_live::retire(*this);")
        entry("482BD4"," nfs_setup_scene_live::row(*this,uint32_t(game_id_sub_482BD4));")
        entry("483438"," nfs_setup_scene_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_setup_scene_live::CopyWrapperScope nfs_setup_copy_wrapper(*this);")
    elif name=="Methods_13.cpp":
        entry("4A5830"," nfs_setup_caption::GlyphScope nfs_setup_caption_glyph(*this);")
        entry("4A14D4"," nfs_setup_scene_live::byteCopyRow(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Setup indexed dispatch changed")
            return part.replace(call,block("indexed-open"," { nfs_setup_scene_live::IndexedSideCallScope nfs_setup_indexed(*this,nfs_setup_scene_live::indexedSideCallee(*this));")+call+block("indexed-close"," }"))
        body("4A10BC",dispatch)

    elif name=="Methods_09.cpp":entry("4824CC"," nfs_setup_caption::SelectionScope nfs_setup_caption_selection(*this);")
    elif name=="Methods_14.cpp":
        entry("4AC934"," nfs_setup_caption::beforeOwnerRetire(*this,uint32_t(eax),true);")
        entry("4AC9C8"," nfs_setup_caption::beforeDescriptorRetire(*this,uint32_t(eax));")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Setup original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_setup_caption::WordCapture nfs_setup_caption_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    return text
