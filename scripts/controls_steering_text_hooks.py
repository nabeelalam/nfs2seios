"""Add independent pre-diagram Controls steering text observations; preserve original instructions."""
# SPDX-License-Identifier: MIT
import re

MARKER = 'NFS_CONTROLS_STEERING_TEXT'
NAMES = ('Methods_01.cpp', 'Methods_03.cpp', 'Methods_09.cpp',
         'Methods_10.cpp', 'Methods_13.cpp', 'Methods_14.cpp')


def remove_controls_steering_text_hooks(text):
    result = re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+
                    MARKER+r'_END:\1\r?\n', '', text, flags=re.S)
    if MARKER in result:
        raise ValueError('Unmatched Controls steering text marker')
    return result


def _outside(text, at):
    prefix = text[:at]
    if prefix.count('// checked copy transaction begin') != prefix.count('// checked copy transaction end'):
        raise ValueError('Controls steering observation inside checked-copy lease')


def add_controls_steering_text_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Controls steering text hooks already present')
    original = text
    nl = '\r\n' if '\r\n' in text else '\n'

    def block(tag, code):
        return nl+'// '+MARKER+'_BEGIN:'+tag+nl+code+nl+'// '+MARKER+'_END:'+tag+nl

    def unique(before, after):
        nonlocal text
        if text.count(before) != 1:
            raise ValueError('Missing/duplicate Controls steering anchor: '+name+' '+before[:80])
        _outside(text, text.index(before))
        text = text.replace(before, after, 1)

    def entry(symbol, code):
        anchor = 'Fn(void) Game::_sub_'+symbol+'()'+nl+'{'+nl
        unique(anchor, anchor+block('entry-'+symbol, code))

    def body(symbol, transform):
        nonlocal text
        pattern = r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\r?\n\{.*?(?=\r?\nFn\(void\) Game::|\Z)'
        matches = list(re.finditer(pattern, text, re.S))
        if len(matches) != 1:
            raise ValueError('Missing/duplicate Controls steering function: '+symbol)
        found = matches[0]
        text = text[:found.start()]+transform(found.group())+text[found.end():]

    def call(symbol):
        return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'

    def once(part, anchor):
        if part.count(anchor) != 1:
            raise ValueError('Missing/duplicate/changed Controls steering original anchor: '+anchor[:80])
        return part.index(anchor)

    def tail(part):
        reset, copy, draw, chart, swap = [once(part, call(s)) for s in
                                        ('405B30', '405B60', '40FA90', '410330', '405C70')]
        branch = once(part, '\tcmp(esi, static_cast<int32_t>(guest_address(off_4D36A8)));'+nl)
        if not reset < copy < draw < branch < chart < swap:
            raise ValueError('Controls steering text/chart source order changed')
        if '\teax = esi; //mov'+nl not in part[copy:draw]:
            raise ValueError('Controls steering original form argument changed')
        _outside(part, draw)
        _outside(part, draw+len(call('40FA90')))
        # This packet proves text BEFORE the separately drawn steering diagram.
        # Final scene publication must independently prove that later bitmap.
        after = draw+len(call('40FA90'))
        part = part[:after]+block('finish-before-chart',
                    ' nfs_controls_steering_text::finishTail(*this);')+part[after:]
        return part[:draw]+block('begin-text',
                    ' nfs_controls_steering_text::beginTail(*this);')+part[draw:]

    anchor = '#include "FunctionIds.h"'+nl
    unique(anchor, anchor+block('include', '#include "hd/ControlsSteeringTextGameHooks.inc"'))
    if name == 'Methods_01.cpp':
        entry('4101C0', ' nfs_controls_steering_text::OwnerScope nfs_controls_steering_owner(*this,0x4101c0u);')
        body('4101C0', tail)
        entry('4054E0', ' nfs_controls_steering_text::PolicyScope nfs_controls_steering_policy(*this,true);')
        entry('405580', ' nfs_controls_steering_text::PolicyScope nfs_controls_steering_policy(*this,false);')
        entry('405CF0', ' nfs_controls_steering_text::StringScope nfs_controls_steering_string(*this);')

        def policy(part):
            anchor = call('41B690')
            at = once(part, anchor)+len(anchor)
            _outside(part, at)
            return part[:at]+block('resource-alignment', ' nfs_controls_steering_text::resourceAlignment(*this);')+part[at:]
        body('405580', policy)
    elif name == 'Methods_09.cpp':
        entry('4824CC', ' nfs_controls_steering_text::SelectionScope nfs_controls_steering_selection(*this);')
    elif name == 'Methods_10.cpp':
        entry('482DA0', ' nfs_controls_steering_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    elif name == 'Methods_13.cpp':
        entry('4A5830', ' nfs_controls_steering_text::GlyphScope nfs_controls_steering_glyph(*this);')
    elif name == 'Methods_14.cpp':
        entry('4AC934', ' nfs_controls_steering_text::beforeOwnerRetire(*this,uint32_t(eax),true);')
        entry('4AC9C8', ' nfs_controls_steering_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    if name in ('Methods_13.cpp', 'Methods_14.cpp'):
        symbol, count = ('4A5830', 4) if name == 'Methods_13.cpp' else ('4B3EE0', 1)

        def stores(part):
            store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            if part.count(store) != count:
                raise ValueError('Controls steering original glyph store count changed')
            pieces = part.split(store)
            out = pieces[0]
            for i, piece in enumerate(pieces[1:]):
                _outside(out, len(out))
                out += block('word-open-'+str(i), ' { nfs_controls_steering_text::WordCapture nfs_controls_steering_word(*this,uint32_t(esi),uint16_t(bx));')
                out += store+block('word-close-'+str(i), ' }')+piece
            return out
        body(symbol, stores)
    for found in re.finditer(r'// '+MARKER+r'_BEGIN:', text):
        _outside(text, found.start())
    if remove_controls_steering_text_hooks(text) != original:
        raise ValueError('Controls steering text exact source reversal failed')
    return text
