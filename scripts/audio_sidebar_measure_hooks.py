"""Observe original Audio-owned sidebar right/center measurements unchanged."""
MARKER="NFS_AUDIO_SIDEBAR_MEASURE"
NAMES=("Methods_01.cpp",)
ANCHORS={
 "right":("loc_4055D3:\n\teax = ecx; //mov\n\tesp -= 4; _sub_482424(); esp += 4; //call\n","\tsub(esi, eax);\n"),
 "center":("loc_405646:\n\teax = ecx; //mov\n\tesp -= 4; _sub_482424(); esp += 4; //call\n","\tedx = eax; //mov\n")}
def add_audio_sidebar_measure_hooks(text,name):
 if name not in NAMES:return text
 if MARKER in text:raise ValueError("Audio sidebar measure hooks already present")
 start=text.find("Fn(void) Game::_sub_405580()\n{\n")
 if start<0:raise ValueError("Original405580 missing")
 end=text.find("\nFn(",start+1)
 if end<0:end=len(text)
 body=text[start:end]
 for kind,(prefix,suffix) in ANCHORS.items():
  anchor=prefix+suffix
  if body.count(anchor)!=1 or text.count(anchor)!=1:raise ValueError("Original "+kind+"482424 return changed")
  block=("\n// "+MARKER+"_BEGIN:measured-"+kind+"\n nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::"+kind+");\n// "+MARKER+"_END:measured-"+kind+"\n")
  body=body.replace(anchor,prefix+block+suffix,1)
 return text[:start]+body+text[end:]
