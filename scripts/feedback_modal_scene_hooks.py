"""Maintained original-preserving Feedback parent/scene and lazy-copy scopes.

Apply after Controls children, Feedback text and backdrop transforms. The native
426950 scope remains owned by feedback_modal_observer_source_hooks. These hooks
never invent packets, source counters or publication authority.
"""
# SPDX-License-Identifier: MIT
import re
MARKER = 'NFS_FEEDBACK_MODAL_SCENE'
NAMES = ('Methods_01.cpp', 'Methods_10.cpp', 'Methods_13.cpp')
HELPERS = (
    'FeedbackModalObserverQueueAdapter.inc', 'FeedbackModalSceneGameHooks.inc',
    'ControlsRetainedParentGameHooks.inc', 'ControlsLazyParentCopyGameHooks.inc',
    'ControlsLazyParentCopyRecords.h', 'ControlsLazyParentCopyReplay.h',
    'ControlsLazyParentKeyedTransform.h', 'ControlsSidebarRecipeEquivalent.h',
    'ControlsLazyParentTextProtection.h', 'ControlsLazyParentRecomposition.h',
    'FeedbackModalSceneRuntimeAbi.h', 'FeedbackModalBackdropArtAbi.h',
)
_BLOCK = re.compile(r'\n// '+MARKER+r'_BEGIN:([a-z0-9-]+)\n.*?\n// '+MARKER+r'_END:\1\n', re.S)

def _lf(text):
    crlf = '\r\n' in text
    if crlf and '\n' in text.replace('\r\n', ''):
        raise ValueError('Mixed Feedback scene source newlines')
    return (text.replace('\r\n', '\n') if crlf else text), crlf

def remove_feedback_modal_scene_hooks(text):
    text, crlf = _lf(text)
    text = _BLOCK.sub('', text)
    if MARKER in text:
        raise ValueError('Unmatched Feedback scene marker')
    return text.replace('\n', '\r\n') if crlf else text

def add_feedback_modal_scene_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Feedback scene hooks already present')
    original = text
    text, crlf = _lf(text)
    def block(tag, code):
        return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
    def edit(symbol, transform):
        nonlocal text
        pattern = r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)'
        found = list(re.finditer(pattern, text, re.S))
        if len(found) != 1:
            raise ValueError('Missing/duplicate Feedback scene original '+symbol)
        m = found[0]
        text = text[:m.start()]+transform(m.group())+text[m.end():]
    def once(part, anchor, replacement, label):
        if part.count(anchor) != 1:
            raise ValueError('Feedback scene original changed: '+label)
        return part.replace(anchor, replacement, 1)
    def call(symbol):
        return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
    def entry(symbol, tag, scope, required):
        def transform(part):
            for anchor in required:
                if part.count(anchor) != 1:
                    raise ValueError('Feedback scene wrapper changed: '+symbol+' '+anchor)
            return part.replace('{', '{'+block(tag, scope), 1)
        edit(symbol, transform)
    include = '#include "FunctionIds.h"\n'
    text = once(text, include, include+block('include', '#include "hd/ControlsLazyParentCopyGameHooks.inc"\n#include "hd/FeedbackModalObserverQueueAdapter.inc"'), 'include')
    if name == 'Methods_01.cpp':
        def parent(part):
            # 410414 contains other legitimate popup/presentation branches.
            # Bind only the already authenticated unavailable-Feedback route,
            # exactly as the text Invocation transform does.
            start_label, end_label = 'loc_4109D5:\n', 'loc_410A1E:\n'
            for label in (start_label, end_label):
                if len(re.findall(r'^'+re.escape(label), part, re.M)) != 1:
                    raise ValueError('Feedback scene unavailable-route label changed')
            start, end = part.index(start_label), part.index(end_label)
            if end <= start:
                raise ValueError('Feedback scene unavailable-route extent changed')
            region = part[start:end]
            arm = ' { nfs_feedback_modal_live::ArmScope nfs_feedback_invocation(*this);'
            region = once(region, arm, arm+block('parent-acquire', ' auto nfs_feedback_parent=nfs_controls_retained_parent::acquireForFeedback(*this);'), 'invocation arm')
            first, present = call('402E80'), call('405C70')
            if region.count(first) != 1 or region.count(present) != 1 or region.index(first) >= region.index(present):
                raise ValueError('Feedback scene popup/presentation order changed')
            if region[region.index(first)+len(first):region.index(present)].strip():
                raise ValueError('Feedback scene popup/presentation calls no longer adjacent')
            code = (' auto nfs_feedback_joined=nfs_feedback_modal_scene_live::acquire(*this,nfs_feedback_parent);\n'
                    ' nfs_feedback_modal_observer_queue::Intent nfs_feedback_observer_intent;\n'
                    ' nfs_feedback_modal_observer_queue::Scope nfs_feedback_observer_scope(*this,nfs_feedback_joined,nfs_feedback_observer_intent);')
            region = region.replace(first, first+block('source-join', code), 1)
            return part[:start]+region+part[end:]
        edit('410414', parent)
        def copy(part):
            label = 'loc_40C054:\n'
            # Count the semantic label token too: a malformed concatenated
            # duplicate must not hide behind the one remaining trailing newline.
            if part.count('loc_40C054:') != 1 or len(re.findall(r'^loc_40C054:\n', part, re.M)) != 1:
                raise ValueError('Feedback lazy-copy original label changed')
            at = part.index(label)
            tail = part[at:]
            draw = call('405B60')
            if tail.count(draw) != 1:
                raise ValueError('Feedback lazy-copy original draw extent changed')
            # Later loc40C0B6/40C0E5 upload branches legitimately read these
            # bitmap/table fields again. Bind the setup ending at this one copy,
            # not every subsequent use in the enclosing 40BBE0 function.
            setup = tail[:tail.index(draw)+len(draw)]
            required = (call('4825A8'), '\tedx = 0x2800; //mov',
                        '\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov',
                        '\txor_(ebx, ebx);', '\txor_(ecx, ecx);',
                        '\teax = guest_transaction.load<int32_t>(eax+0x20); //mov', call('405B60'))
            for anchor in required:
                if setup.count(anchor) != 1:
                    raise ValueError('Feedback lazy-copy original tail changed: '+anchor)
            order = [setup.index(x) for x in required]
            if order != sorted(order):
                raise ValueError('Feedback lazy-copy original tail order changed')
            draw = call('405B60')
            tail = tail.replace(draw, block('copy-open', ' { nfs_controls_lazy_parent_live::CopyScope nfs_feedback_lazy_copy(*this);')+draw+block('copy-close', ' }'), 1)
            return part[:at]+tail
        edit('40BBE0', copy)
    elif name == 'Methods_10.cpp':
        entry('48330C', 'resolve', ' nfs_controls_lazy_parent_live::ResolveScope nfs_feedback_lazy_resolve(*this);', (call('4A10BC'),))
        entry('483438', 'direct-dispatch', ' nfs_controls_lazy_parent_live::DispatchScope nfs_feedback_lazy_dispatch(*this);', ('\tadd(ebx, ebx);', '\t_sub_4A14D4(); return; //jmp'))
    else:
        def transform(part):
            first, second = '\tcall(to32i(dword_571264));', '\tcall(to32i(dword_571260));'
            if part.count(first) != 1 or part.count(second) != 1 or part.index(first) >= part.index(second):
                raise ValueError('Feedback keyed transform/forward calls changed')
            for anchor in ('\tsub(esp, (int32_t)0x1400);', '\teax = esp; //mov', '\tebx = ecx; //mov', '\tedx = esi; //mov'):
                if part.count(anchor) != 1:
                    raise ValueError('Feedback keyed transform ABI changed: '+anchor)
            part = part.replace('{', '{'+block('transform-scope', ' nfs_controls_lazy_parent_live::TransformScope nfs_feedback_lazy_transform(*this);'), 1)
            part = part.replace(first, block('transform-before', ' nfs_feedback_lazy_transform.beforeTransform();')+first+block('transform-after', ' nfs_feedback_lazy_transform.afterTransform();'), 1)
            return part.replace(second, block('forward-before', ' nfs_feedback_lazy_transform.beforeForward();')+second+block('forward-after', ' nfs_feedback_lazy_transform.afterForward();'), 1)
        edit('4A1090', transform)
        def indirect(part):
            anchor = '\tcall(to32i(esp+0x38));'
            if part.count(anchor) != 4:
                raise ValueError('Feedback original indirect row branch count changed')
            pieces = part.split(anchor)
            out = pieces[0]
            for index, suffix in enumerate(pieces[1:]):
                out += block('indirect-'+str(index), ' nfs_controls_lazy_parent_live::observeIndirect(*this);')+anchor+suffix
            return out
        edit('4A10BC', indirect)
        entry('4A14D4', 'direct-bytes', ' nfs_controls_lazy_parent_live::ByteScope nfs_feedback_lazy_bytes(*this);', ('\tswap(eax, edx);',))
    depth = 0
    for event in re.finditer(r'// checked copy transaction (begin|end)|// '+MARKER+r'_BEGIN:', text):
        if event.group(1) == 'begin':
            depth += 1
        elif event.group(1) == 'end':
            depth -= 1
            if depth < 0:
                raise ValueError('Feedback scene unmatched guest-copy end')
        elif depth:
            raise ValueError('Feedback scene hook inside guest-copy lease')
    if depth:
        raise ValueError('Feedback scene unclosed guest-copy lease')
    if crlf:
        text = text.replace('\n', '\r\n')
    if remove_feedback_modal_scene_hooks(text) != original:
        raise ValueError('Feedback scene exact original reversal failed')
    return text
