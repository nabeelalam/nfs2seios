"""Independent source-bound Opponent/Location physical photo transaction hooks."""
import re
MARKER="NFS_CHILD_PHOTO"
def add_child_photo_hooks(text,name):
 if name not in ("Methods_01.cpp","Methods_03.cpp","Methods_13.cpp"):return text
 if MARKER in text:raise ValueError("Child photo hooks already present")
 def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
 def entry(symbol,code):
  nonlocal text
  a="Fn(void) Game::_sub_"+symbol+"()\n{\n"
  if text.count(a)!=1:raise ValueError("Child photo entry changed: "+symbol)
  text=text.replace(a,a+block("entry-"+symbol,code),1)
 a='#include "FunctionIds.h"\n'
 if text.count(a)!=1:raise ValueError("Child photo include changed")
 text=text.replace(a,a+block("include",'#include "hd/ChildPhotoGameHooks.inc"'),1)
 if name=="Methods_01.cpp":entry("405B60"," nfs_child_photo_live::ResolvedScope nfs_child_photo_resolved(*this);")
 elif name=="Methods_03.cpp":
  entry("427ED0"," nfs_child_photo_live::BuildScope nfs_child_photo_build(*this);")
  entry("4279E0"," nfs_child_photo_live::DrawScope nfs_child_photo_draw(*this);")
  m=re.search(r"Fn\(void\) Game::_sub_427ED0\(\)\n\{\n",text)
  end=text.find("\nFn(",m.end());end=len(text) if end<0 else end
  old=text[m.start():end];part=old
  for symbol,scope,n in [("40CD70","PhotoScope",2),("4850B0","CropScope",2),("49A118","ConvertScope",1),("428440","ConvertScope",1),("49755C","UploadScope",2)]:
   call="\tesp -= 4; _sub_"+symbol+"(); esp += 4; //call"
   if part.count(call)!=n:raise ValueError("Child photo call changed: "+symbol)
   args=",true" if symbol=="49A118" else ",false" if symbol=="428440" else ""
   part=part.replace(call,block("open-"+symbol," { nfs_child_photo_live::"+scope+" nfs_child_photo_scope(*this"+args+");")+call+block("close-"+symbol," }"))
  text=text[:m.start()]+part+text[end:]
 if name=="Methods_13.cpp":
  m=re.search(r"Fn\(void\) Game::_sub_4A10BC\(\)\n\{\n",text)
  if not m:raise ValueError("Child photo row dispatcher changed")
  end=text.find("\nFn(",m.end());end=len(text) if end<0 else end
  old=text[m.start():end];call="\tcall(to32i(esp+0x38));"
  if old.count(call)!=4:raise ValueError("Child photo row dispatch count changed")
  part=old.replace(call,block("row-open"," { nfs_child_photo_live::RowScope nfs_child_photo_row(*this);")+call+block("row-close"," }"))
  text=text[:m.start()]+part+text[end:]
 return text
