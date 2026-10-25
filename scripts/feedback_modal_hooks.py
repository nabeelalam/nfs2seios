"""Inactive, original-preserving Feedback popup collection hooks.

Preparation must opt in explicitly until live backdrop/parent/native joins exist.
"""
# SPDX-License-Identifier: MIT
import re
MARKER = 'NFS_FEEDBACK_MODAL_TEXT'
NAMES = ('Methods_01.cpp', 'Methods_09.cpp', 'Methods_10.cpp', 'Methods_13.cpp', 'Methods_14.cpp')

def remove_feedback_modal_hooks(text):
    result = re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n', '', text, flags=re.S)
    if MARKER in result:
        raise ValueError('Unmatched Feedback modal marker')
    return result

def add_feedback_modal_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Feedback modal hooks already present')
    original = text
    crlf = '\r\n' in text
    if crlf and '\n' in text.replace('\r\n', ''):
        raise ValueError('Mixed Feedback source newlines')
    if crlf:
        text = text.replace('\r\n', '\n')
    def block(tag, code):
        return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
    def edit(symbol, transform):
        nonlocal text
        pattern = r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)'
        matches = list(re.finditer(pattern, text, re.S))
        if len(matches) != 1:
            raise ValueError('Missing/duplicate Feedback source function '+symbol)
        m = matches[0]
        text = text[:m.start()]+transform(m.group())+text[m.end():]
    def entry(symbol, tag, code):
        edit(symbol, lambda part: part.replace('{', '{'+block(tag, code), 1))
    def call(symbol):
        return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
    include = '#include "FunctionIds.h"\n'
    if text.count(include) != 1:
        raise ValueError('Feedback source include changed')
    text = text.replace(include, include+block('include', '#include "hd/FeedbackModalTextGameHooks.inc"'), 1)
    if name == 'Methods_01.cpp':
        def arm(part):
            start, end = part.find('loc_4109D5:\n'), part.find('loc_410A1E:\n')
            if start < 0 or end <= start:
                raise ValueError('Feedback unavailable route changed')
            region = part[start:end]
            first, swap = call('402E80'), call('405C70')
            if region.count(first) != 1 or region.count(swap) != 1 or region.index(first) >= region.index(swap):
                raise ValueError('Feedback modal/presentation order changed')
            # An intervening scope close could destroy our Arm before swap.
            # This finite original route has two adjacent calls; fail closed
            # instead of silently composing through another observer's braces.
            if region[region.index(first)+len(first):region.index(swap)].strip():
                raise ValueError('Feedback modal/presentation calls are no longer adjacent')
            if re.search(r'\b(goto|return|co_return)\b', region[region.index(first):region.index(swap)]):
                raise ValueError('Feedback arm contains a control-flow exit')
            changed = region.replace(first, block('arm-open', ' { nfs_feedback_modal_live::ArmScope nfs_feedback_invocation(*this);\n nfs_feedback_modal_text_live::ArmScope nfs_feedback_text_arm(*this);')+first, 1)
            changed = changed.replace(swap, swap+block('arm-close', ' nfs_feedback_modal_text_live::presented(*this);\n }'), 1)
            return part[:start]+changed+part[end:]
        edit('410414', arm)
        entry('402E80', 'modal', ' nfs_feedback_modal_text_live::ModalScope nfs_feedback_text_modal(*this);')
        entry('4054E0', 'policy', ' nfs_feedback_modal_text_live::PolicyScope nfs_feedback_text_policy(*this);')
        entry('405CF0', 'string', ' nfs_feedback_modal_text_live::StringScope nfs_feedback_text_string(*this);')
        def font(part):
            anchor = call('4056F0')
            if part.count(anchor) != 1:
                raise ValueError('Feedback selected font call changed')
            return part.replace(anchor, anchor+block('font-selected', ' nfs_feedback_modal_text_live::selectedFont(*this);'), 1)
        edit('4054E0', font)
    elif name == 'Methods_09.cpp':
        entry('482424', 'measure', ' nfs_feedback_modal_text_live::MeasureScope nfs_feedback_text_measure(*this);')
    elif name == 'Methods_10.cpp':
        entry('482DA0', 'descriptor-retire', ' nfs_feedback_modal_text_live::beforeDescriptorRetire(*this,uint32_t(eax));')
    elif name == 'Methods_13.cpp':
        entry('4A5830', 'glyph', ' nfs_feedback_modal_text_live::GlyphScope nfs_feedback_text_glyph(*this);')
    else:
        entry('4AC934', 'owner-retire', ' nfs_feedback_modal_text_live::beforeOwnerRetire(*this,uint32_t(eax));')
        entry('4AC9C8', 'descriptor-retire', ' nfs_feedback_modal_text_live::beforeDescriptorRetire(*this,uint32_t(eax));')
    if name in ('Methods_13.cpp', 'Methods_14.cpp'):
        symbol, count = ('4A5830', 4) if name == 'Methods_13.cpp' else ('4B3EE0', 1)
        def stores(part):
            store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            if part.count(store) != count:
                raise ValueError('Feedback original glyph store count changed')
            pieces = part.split(store)
            result = pieces[0]
            for i, tail in enumerate(pieces[1:]):
                result += block('word-open-'+str(i), ' { nfs_feedback_modal_text_live::WordCapture nfs_feedback_text_word(*this,uint32_t(esi),uint16_t(bx));')+store+block('word-close-'+str(i), ' }')+tail
            return result
        edit(symbol, stores)
    depth = 0
    for event in re.finditer(r'// checked copy transaction (begin|end)|// '+MARKER+r'_BEGIN:', text):
        if event.group(1) == 'begin':
            depth += 1
        elif event.group(1) == 'end':
            depth -= 1
            if depth < 0:
                raise ValueError('Unmatched original guest lease')
        elif depth:
            raise ValueError('Feedback observation nested in original guest lease')
    if depth:
        raise ValueError('Unclosed original guest lease')
    if crlf:
        text = text.replace('\n', '\r\n')
    if remove_feedback_modal_hooks(text) != original:
        raise ValueError('Feedback exact source reversal failed')
    return text
