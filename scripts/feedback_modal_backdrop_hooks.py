"""Opt-in source-preserving original Feedback backdrop observations.

Compose after feedback_modal_hooks; it owns the sole outer Invocation arm.
This module never registers or enables a native renderer.
"""
# SPDX-License-Identifier: MIT
import re
MARKER = 'NFS_FEEDBACK_MODAL_BACKDROP'
NAMES = ('Methods_01.cpp', 'Methods_09.cpp', 'Methods_10.cpp', 'Methods_13.cpp')

def remove_feedback_backdrop_hooks(text):
    result = re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n', '', text, flags=re.S)
    if MARKER in result:
        raise ValueError('Unmatched Feedback backdrop marker')
    return result

def add_feedback_backdrop_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Feedback backdrop hooks already present')
    original = text
    crlf = '\r\n' in text
    if crlf and '\n' in text.replace('\r\n', ''):
        raise ValueError('Mixed backdrop source newlines')
    if crlf:
        text = text.replace('\r\n', '\n')
    def block(tag, code):
        return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'
    def edit(symbol, transform):
        nonlocal text
        matches = list(re.finditer(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)', text, re.S))
        if len(matches) != 1:
            raise ValueError('Missing/duplicate backdrop function '+symbol)
        m = matches[0]
        text = text[:m.start()]+transform(m.group())+text[m.end():]
    def entry(symbol, code):
        edit(symbol, lambda part: part.replace('{', '{'+block('entry-'+symbol, code), 1))
    include = '#include "FunctionIds.h"\n'
    if text.count(include) != 1:
        raise ValueError('Backdrop source include changed')
    text = text.replace(include, include+block('include', '#include "hd/FeedbackModalBackdropGameHooks.inc"'), 1)
    ns = 'nfs_feedback_modal_backdrop_live::'
    if name == 'Methods_01.cpp':
        if 'NFS_FEEDBACK_MODAL_TEXT_BEGIN:arm-open' not in text:
            raise ValueError('Backdrop requires the original shared invocation arm')
        entry('402E80', ' '+ns+'ModalScope nfs_feedback_backdrop_modal(*this);')
        def modal(part):
            anchor = 'loc_40301B:\n'
            if part.count(anchor) != 1:
                raise ValueError('Backdrop original pretext label changed')
            part = part.replace(anchor, anchor+block('pretext', ' '+ns+'pretext(*this);'), 1)
            call = '\tesp -= 4; _sub_428A20(); esp += 4; //call'
            if part.count(call) != 2:
                raise ValueError('Backdrop original main-restore call count changed')
            pieces = part.split(call)
            result = pieces[0]
            for i, tail in enumerate(pieces[1:]):
                result += block('main-open-'+str(i), ' { '+ns+'EventScope nfs_feedback_main_select(*this,'+ns+'Event::mainSelect);')+call+block('main-close-'+str(i), ' }')+tail
            return result
        edit('402E80', modal)
    elif name == 'Methods_09.cpp':
        entry('482424', ' '+ns+'WidthScope nfs_feedback_backdrop_width(*this);')
        for symbol, kind in (('482474','allocate'),('4825A8','select'),('48266C','fillColor'),('482740','fill')):
            entry(symbol, ' '+ns+'EventScope nfs_feedback_backdrop_event(*this,'+ns+'Event::'+kind+');')
    elif name == 'Methods_10.cpp':
        for symbol, kind in (('482C78','body'),('482DA0','retire'),('482E00','clip'),('482EA8','named'),('48330C','draw'),('4833A0','hotspot')):
            entry(symbol, ' '+ns+'EventScope nfs_feedback_backdrop_event(*this,'+ns+'Event::'+kind+');')
        for symbol in ('482938','4829E4','482BD4','482FD4','483070','483288'):
            def stores(part, symbol=symbol):
                store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
                if part.count(store) != 1:
                    raise ValueError('Backdrop original known store changed '+symbol)
                return part.replace(store, block('store-open-'+symbol, ' { '+ns+'StoreScope nfs_feedback_backdrop_store(*this,uint32_t(esi),uint16_t(bx));')+store+block('store-close-'+symbol, ' }'), 1)
            edit(symbol, stores)
    else:
        def rows(part):
            call = '\tcall(to32i(esp+0x38));'
            if part.count(call) != 4:
                raise ValueError('Backdrop original row-dispatch count changed')
            pieces = part.split(call)
            result = pieces[0]
            for i, tail in enumerate(pieces[1:]):
                result += block('row-open-'+str(i), ' { '+ns+'RowScope nfs_feedback_backdrop_row(*this);')+call+block('row-close-'+str(i), ' }')+tail
            return result
        edit('4A10BC', rows)
    depth = 0
    for event in re.finditer(r'// checked copy transaction (begin|end)|// '+MARKER+r'_BEGIN:', text):
        if event.group(1) == 'begin':
            depth += 1
        elif event.group(1) == 'end':
            depth -= 1
            if depth < 0:
                raise ValueError('Unmatched original backdrop guest lease')
        elif depth:
            raise ValueError('Backdrop observation nested in original guest lease')
    if depth:
        raise ValueError('Unclosed original backdrop guest lease')
    if crlf:
        text = text.replace('\n', '\r\n')
    if remove_feedback_backdrop_hooks(text) != original:
        raise ValueError('Backdrop exact source reversal failed')
    return text
