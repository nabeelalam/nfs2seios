# SPDX-License-Identifier: MIT
"""Exact Location first-caption observer; run after Player/child initial hooks."""
MARKER="NFS_LOCATION_INITIAL"
def add_location_initial_sidebar_hooks(text,name):
 if name!="Methods_01.cpp":return text
 if MARKER in text:raise ValueError("Location initial hooks already present")
 def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
 def replace(old,new):
  nonlocal text
  if text.count(old)!=1:raise ValueError("Exact Location initial anchor changed: "+old[:60])
  text=text.replace(old,new,1)
 include='#include "FunctionIds.h"\n'
 replace(include,include+block("include",'#include "hd/LocationInitialSidebarGameHooks.inc"'))
 entry='Fn(void) Game::_sub_40C160()\n{\n'
 replace(entry,entry+block("owner"," nfs_location_initial_sidebar_live::OwnerScope nfs_location_initial_owner(*this);"))
 # The Player marker scopes exactly one first40C110 call before the original
 # slot0 upload and427280 swipe. No redraw/descriptor-only adoption.
 marker='// NFS_PLAYER_SCENE_END:initial-caption-open\n'
 replace(marker,marker+block("caller"," nfs_location_initial_sidebar_live::InitialCallerScope nfs_location_initial_caller(*this);"))
 return text
