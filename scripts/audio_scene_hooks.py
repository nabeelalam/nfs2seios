"""Original-preserving actual Audio full640 interleaved native pass hook."""
import re
MARKER="NFS_AUDIO_SCENE"
NAMES=("Methods_03.cpp",)
def add_audio_scene_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Audio scene hooks already present")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    anchor='#include "FunctionIds.h"\n'
    if text.count(anchor)!=1:raise ValueError("Audio scene include changed")
    text=text.replace(anchor,anchor+block("include",'#include "hd/AudioSceneQueueAdapter.inc"'),1)
    m=re.search(r"Fn\(void\) Game::_sub_426950\(\)\n\{(?P<body>.*?)(?=\nFn\(void\) Game::|\Z)",text,re.S)
    if not m:raise ValueError("Audio native entry missing")
    part=m.group();body=m.group('body');last=body.rfind('}')
    if last<0 or body[last+1:].strip() or re.search(r'\breturn\b',body):raise ValueError("Audio native exits changed")
    for symbol in ['49755C','497738','497D6C']:
        if body.count('_sub_'+symbol+'();')!=1:raise ValueError("Audio native original operation changed")
    if '0x1E0' not in body or '0x280' not in body:raise ValueError("Audio native extent changed")
    begin=block('begin',' nfs_audio_scene_live::Intent nfs_audio_scene_intent;\n try {\n nfs_audio_scene_live::begin(*this,nfs_audio_scene_intent);')
    end=block('finish',' nfs_audio_scene_live::finish(nfs_audio_scene_intent);\n } catch(...) {\n nfs_audio_scene_live::cancel(nfs_audio_scene_intent);\n throw;\n }')
    replacement='Fn(void) Game::_sub_426950()\n{'+begin+body[:last]+end+body[last:]
    return text[:m.start()]+replacement+text[m.end():]
