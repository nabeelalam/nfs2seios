"""Reversible Feedback whole-scene renderer integration after native observation."""
# SPDX-License-Identifier: MIT
import re
from feedback_modal_observer_hooks import _bounds
MARKER='NFS_FEEDBACK_MODAL_SCENE_RENDERER'
HELPERS=('FeedbackModalSceneRenderer.inc','FeedbackModalSceneRuntimeAbi.h',
 'FeedbackModalSceneLayerAbi.h','FeedbackModalSceneLayers.h','FeedbackModalSceneLayers.c.inc',
 'FeedbackModalWordGpu.h','FeedbackModalWordGpu.c.inc','FeedbackModalWordRuntimeAbi.h',
 'FeedbackModalBackdropGpu.h','FeedbackModalBackdropGpu.c.inc','FeedbackModalBackdropArtAbi.h',
 'FeedbackModalBackdropArtProvider.h')
_BLOCK=re.compile(r'\r?\n/\* NFS_FEEDBACK_MODAL_SCENE_RENDERER_BEGIN:([a-z0-9-]+) \*/\r?\n.*?\r?\n/\* NFS_FEEDBACK_MODAL_SCENE_RENDERER_END:\1 \*/\r?\n',re.S)
def reverse_feedback_modal_scene_renderer_hooks(text):
 tags=_BLOCK.findall(text)
 if len(tags)!=len(set(tags)):raise ValueError('Duplicate Feedback scene renderer tag')
 result=_BLOCK.sub('',text)
 if MARKER in result:raise ValueError('Malformed Feedback scene renderer hook')
 return result
remove_feedback_modal_scene_renderer_hooks=reverse_feedback_modal_scene_renderer_hooks
def add_feedback_modal_scene_renderer_hooks(text):
 original=text
 if MARKER in text:raise ValueError('Feedback scene renderer hooks already present')
 nl='\r\n'if'\r\n'in text else'\n'
 def insert(anchor,code,tag,function=None,after=False):
  nonlocal text
  start,end=_bounds(text,function)if function else(0,len(text));part=text[start:end]
  if part.count(anchor)!=1:raise ValueError('Missing/duplicate Feedback scene renderer anchor: '+tag)
  at=start+part.index(anchor)+(len(anchor)if after else 0)
  before=text[:at]
  if before.count('// checked copy transaction begin')!=before.count('// checked copy transaction end'):raise ValueError('Feedback scene hook inside guest-copy lease')
  block=nl+'/* '+MARKER+'_BEGIN:'+tag+' */'+nl+code.replace('\n',nl)+nl+'/* '+MARKER+'_END:'+tag+' */'+nl
  text=text[:at]+block+text[at:]
 insert('} TextureInfo;','static void nfs_feedback_scene_retired(void);\nstatic void nfs_feedback_scene_forget(void);','forward',after=True)
 insert(' nfs_feedback_observer_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);',' nfs_feedback_scene_retired();','retired','destroyContext')
 insert(' nfs_feedback_observer_retired(0);',' nfs_feedback_scene_forget();','abandoned','nfs_glide_abandon_unbound')
 observer='#include "hd/FeedbackModalNativeObserver.inc"';controls='#include "hd/ControlsSceneRenderer.inc"'
 if text.count(observer)!=1 or text.count(controls)!=1 or text.index(observer)>text.index(controls):raise ValueError('Feedback observer/Controls implementation order changed')
 insert(controls,'#include "hd/FeedbackModalSceneRenderer.inc"','implementation',after=True)
 if reverse_feedback_modal_scene_renderer_hooks(text)!=original:raise ValueError('Feedback scene renderer reversal failed')
 return text
