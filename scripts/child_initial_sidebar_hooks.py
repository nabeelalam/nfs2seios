# SPDX-License-Identifier: MIT
"""Exact Setup/Options initial caption join, after existing Player scene hooks."""
MARKER="NFS_CHILD_INITIAL"
def add_child_initial_sidebar_hooks(text,name):
 if name not in ("Methods_01.cpp","Methods_10.cpp"):return text
 if MARKER in text:raise ValueError("Child initial hooks already present")
 def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
 def replace(old,new):
  nonlocal text
  if text.count(old)!=1:raise ValueError("Exact child initial anchor changed: "+old[:60])
  text=text.replace(old,new,1)
 include='#include "FunctionIds.h"\n'
 replace(include,include+block("include",'#include "hd/ChildInitialSidebarGameHooks.inc"\n#include "hd/SetupInitialFullGameHooks.inc"\n#include "hd/LocationInitialFullGameHooks.inc"\n#include "hd/OpponentInitialFullGameHooks.inc"'))
 if name=="Methods_10.cpp":
  anchor='Fn(void) Game::_sub_483070()\n{\n'
  replace(anchor,anchor+block("full-copy-row"," nfs_setup_initial_full_live::row(*this);\n nfs_location_initial_full_live::row(*this);\n nfs_opponent_initial_full_live::row(*this);"))
  return text
 entry='Fn(void) Game::_sub_40C160()\n{\n'
 replace(entry,entry+block("owner"," nfs_child_initial_sidebar_live::OwnerScope nfs_child_initial_owner(*this);"))
 # Existing Player scope owns this exact first caption call, not later redraws.
 openmark='// NFS_PLAYER_SCENE_END:initial-caption-open\n'
 replace(openmark,openmark+block("caption-open"," nfs_setup_caption::ChildInitialCallerScope nfs_setup_initial(*this,nfs_player_main_origin);\n nfs_options_caption::ChildInitialCallerScope nfs_options_initial(*this,nfs_player_main_origin);"))
 copyopen='// NFS_PLAYER_SCENE_END:copy-initial-open\n'
 replace(copyopen,copyopen+block("full-copy-open"," { nfs_setup_initial_full_live::CopyScope nfs_setup_initial_full(*this);\n nfs_location_initial_full_live::CopyScope nfs_location_initial_full(*this);\n nfs_opponent_initial_full_live::CopyScope nfs_opponent_initial_full(*this);"))
 copyclose='// NFS_PLAYER_SCENE_BEGIN:copy-initial-close\n'
 replace(copyclose,block("full-copy-close"," }")+copyclose)
 # Original40C5C6 may replace the first full cache before ANY native draw.
 # Re-observe that actual copy with a fresh base/preview binding, not a lease
 # borrowed from the earlier initial copy. All producer predicates stay exact.
 copyopen='// NFS_PLAYER_SCENE_END:copy-refresh-open\n'
 replace(copyopen,copyopen+block("full-refresh-copy-open"," { nfs_setup_initial_full_live::CopyScope nfs_setup_refresh_full(*this);\n nfs_location_initial_full_live::CopyScope nfs_location_refresh_full(*this);\n nfs_opponent_initial_full_live::CopyScope nfs_opponent_refresh_full(*this,true);"))
 copyclose='// NFS_PLAYER_SCENE_BEGIN:copy-refresh-close\n'
 replace(copyclose,block("full-refresh-copy-close"," }")+copyclose)
 seal=' nfs_player_scene_live::completeInitialCaption(*this);'
 replace(seal,seal+block("caption-seal"," nfs_child_initial_sidebar_live::completeInitialCaption(*this);"))
 return text
