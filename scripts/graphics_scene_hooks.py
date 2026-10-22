"""Original-preserving actual Graphics full640 interleaved native pass hook."""
import re
MARKER="NFS_GRAPHICS_SCENE"
NAMES=("Methods_03.cpp",)
def add_graphics_scene_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Graphics scene hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    anchor='#include "FunctionIds.h"\n'
    if text.count(anchor)!=1:raise ValueError("Graphics scene include changed")
    text=text.replace(anchor,anchor+block("include",'#include "hd/GraphicsSceneQueueAdapter.inc"'),1)
    m=re.search(r"Fn\(void\) Game::_sub_426950\(\)\n\{(?P<body>.*?)(?=\nFn\(void\) Game::|\Z)",text,re.S)
    if not m:raise ValueError("Graphics native entry missing")
    part=m.group();body=m.group('body');last=body.rfind('}')
    if last<0 or body[last+1:].strip() or re.search(r'\breturn\b',body):raise ValueError("Graphics native exits changed")
    for symbol in ['49755C','497738','497D6C']:
        if body.count('_sub_'+symbol+'();')!=1:raise ValueError("Graphics native original operation changed")
    if '0x1E0' not in body or '0x280' not in body:raise ValueError("Graphics native extent changed")
    begin=block('begin',' nfs_graphics_scene_live::Intent nfs_graphics_scene_intent;\n try {\n nfs_graphics_scene_live::begin(*this,nfs_graphics_scene_intent);')
    end=block('finish',' nfs_graphics_scene_live::finish(nfs_graphics_scene_intent);\n } catch(...) {\n nfs_graphics_scene_live::cancel(nfs_graphics_scene_intent);\n throw;\n }')
    replacement='Fn(void) Game::_sub_426950()\n{'+begin+body[:last]+end+body[last:]
    return text[:m.start()]+replacement+text[m.end():]
