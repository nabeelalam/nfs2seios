"""Asserted original-preserving Location preview producer hooks; run after scene hooks."""
import re
MARKER="NFS_LOCATION_PREVIEW"
NAMES=("Methods_01.cpp","Methods_13.cpp")
def add_location_preview_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Location preview hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Location preview anchor changed: "+name+" "+old[:64])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        anchor="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(anchor,anchor+block("entry-"+symbol,code))
    def body(symbol,mutate):
        match=re.search(r"Fn\(void\) Game::_sub_"+symbol+r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)",text,re.S)
        if not match:raise ValueError("Location preview body changed: "+symbol)
        unique(match.group(),mutate(match.group()))
    def wrap(part,anchor,callee,tag,scope):
        if part.count(anchor)!=1:raise ValueError("Location preview scoped anchor changed: "+tag)
        call="\tesp -= 4; _sub_"+callee+"(); esp += 4; //call";start=part.index(anchor);at=part.find(call,start+len(anchor))
        next_label=part.find("\nloc_",start+len(anchor))
        if at<0 or (next_label>=0 and at>=next_label):raise ValueError("Location preview original call changed: "+tag)
        region=part[start:next_label] if next_label>=0 else part[start:]
        if region.count(call)!=1:raise ValueError("Location preview original call count changed: "+tag)
        if callee=="40CD70":
            prefix=part[start:at]
            if not all(token in prefix for token in ("guest_address(aBlk4)","ebx = 0x13C;","edx = 0xD3;","eax = esi;")):raise ValueError("Location preview normal base arguments changed: "+tag)
        return part[:at]+block(tag+"-open"," { "+scope)+call+block(tag+"-close"," }")+part[at+len(call):]
    anchor='#include "FunctionIds.h"\n';unique(anchor,anchor+block("include",'#include "hd/LocationPreviewGameHooks.inc"'))
    if name=="Methods_01.cpp":
        entry("405B30"," nfs_location_preview_live::NameScope nfs_preview_name(*this);")
        entry("405B60"," nfs_location_preview_live::ResolvedScope nfs_preview_resolved(*this);")
        def location(part):
            part=wrap(part,"loc_40C3CC:\n","40CD70","normal-preview-initial","nfs_location_preview_live::Scope nfs_preview(*this,nfs_location_preview_live::Site::Initial);")
            return wrap(part,"loc_40C5C6:\n","40CD70","normal-preview-refresh","nfs_location_preview_live::Scope nfs_preview(*this,nfs_location_preview_live::Site::Refresh);")
        body("40C160",location)
        def preview(part):
            part=wrap(part,"loc_40CF46:\n","48266C","original-color","nfs_location_preview_live::ColorScope nfs_preview_color(*this);")
            return wrap(part,"loc_40CF46:\n","488620","original-fill","nfs_location_preview_live::FillScope nfs_preview_fill(*this);")
        body("40CD70",preview)
    else:
        def rows(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Location preview row dispatch count changed")
            return part.replace(call,block("row-open"," { nfs_location_preview_live::RowScope nfs_preview_row(*this);")+call+block("row-close"," }"))
        body("4A10BC",rows)
    return text
