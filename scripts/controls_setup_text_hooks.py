"""Add independent Controls setup text observations; preserve original instructions."""
# SPDX-License-Identifier: MIT
import re

MARKER = 'NFS_CONTROLS_SETUP_TEXT'
NAMES = ('Methods_01.cpp', 'Methods_03.cpp', 'Methods_09.cpp',
         'Methods_10.cpp', 'Methods_13.cpp', 'Methods_14.cpp')


def remove_controls_setup_text_hooks(text):
    result = re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+
                    MARKER+r'_END:\1\r?\n', '', text, flags=re.S)
    if MARKER in result:
        raise ValueError('Unmatched Controls setup text marker')
    return result


def _outside(text, at):
    prefix = text[:at]
    if prefix.count('// checked copy transaction begin') != prefix.count('// checked copy transaction end'):
        raise ValueError('Controls setup observation inside checked-copy lease')


def add_controls_setup_text_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Controls setup text hooks already present')
    original = text
    nl = '\r\n' if '\r\n' in text else '\n'

    def block(tag, code):
        return nl+'// '+MARKER+'_BEGIN:'+tag+nl+code+nl+'// '+MARKER+'_END:'+tag+nl

    def unique(before, after):
        nonlocal text
        if text.count(before) != 1:
            raise ValueError('Missing/duplicate Controls setup anchor: '+name+' '+before[:80])
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
            raise ValueError('Missing/duplicate Controls setup function: '+symbol)
        found = matches[0]
        text = text[:found.start()]+transform(found.group())+text[found.end():]

    def call(symbol):
        return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'

    def once(part, anchor):
        if part.count(anchor) != 1:
            raise ValueError('Missing/duplicate/changed Controls setup original anchor: '+anchor[:80])
        return part.index(anchor)

    def tails(part, symbol, start_label, finish_label):
        # Validate original source order, independent of all pre-existing marked
        # observations. Manual calls cannot introduce a C++ scope across gotos.
        start = once(part, start_label+':'+nl)
        finish = once(part, finish_label+':'+nl)
        reset, copy, draw, swap = [once(part, call(s)) for s in ('405B30', '405B60', '40FA90', '405C70')]
        if not start < reset < copy < draw < finish < swap:
            raise ValueError('Controls setup original reset/copy/text/finish order changed')
        if '\teax = esi; //mov'+nl not in part[copy:draw] and symbol != '403FC0':
            raise ValueError('Controls setup original form argument changed')
        if symbol == '4101C0':
            branch = once(part, 'loc_410269:'+nl)
            continuation = once(part, 'loc_41030D:'+nl)
            resource = once(part, '\teax = 0x137; //mov'+nl)
            append = once(part, call('4054E0'))
            if not draw < branch < resource < append < finish < continuation:
                raise ValueError('Controls setup appended resource137 branch changed')
            for form in ('off_4D349C', 'off_4D35E8'):
                once(part, '\tcmp(esi, static_cast<int32_t>(guest_address('+form+')));'+nl)
        elif symbol == '403970':
            resource = once(part, '\teax = 0x137; //mov'+nl)
            append = once(part, call('405580'))
            if not draw < finish < resource < append < swap:
                raise ValueError('Controls setup handler appended resource137 changed')
        else:
            once(part, '\tebp = 0x135; //mov'+nl)
            measured = once(part, '\teax = 0x120; //mov'+nl)
            append = once(part, call('405580'))
            once(part, 'loc_4042D1:'+nl)
            if not draw < measured < finish < append < swap:
                raise ValueError('Controls assignment prompt join changed')
            once(part, '\teax = to32i(esp+0x2C); //mov'+nl)
        _outside(part, draw)
        _outside(part, swap)
        # Insert backwards so both original offsets stay valid.
        part = part[:swap]+block('finish-'+symbol, ' nfs_controls_setup_text::finishTail(*this);')+part[swap:]
        return part[:draw]+block('begin-'+symbol, ' nfs_controls_setup_text::beginTail(*this);')+part[draw:]

    anchor = '#include "FunctionIds.h"'+nl
    unique(anchor, anchor+block('include', '#include "hd/ControlsSetupTextGameHooks.inc"'))
    if name == 'Methods_01.cpp':
        entry('4101C0', ' nfs_controls_setup_text::OwnerScope nfs_controls_setup_owner(*this,0x4101c0u);')
        entry('403970', ' nfs_controls_setup_text::OwnerScope nfs_controls_setup_owner(*this,0x403970u);')

        def prompt(part):
            anchor = '\tnfs::NativeControlBindingScope nfs_binding_scope;'+nl
            at = once(part, anchor)+len(anchor)
            _outside(part, at)
            return part[:at]+block('prompt-owner', ' nfs_controls_setup_text::PromptScope nfs_controls_setup_prompt(*this);')+part[at:]
        body('403FC0', prompt)
        for symbol, begin, end in (('4101C0', 'loc_41020E', 'loc_410293'),
                                   ('403970', 'loc_4039B5', 'loc_403A13'),
                                   ('403FC0', 'loc_40419F', 'loc_40421C')):
            body(symbol, lambda part, s=symbol, b=begin, e=end: tails(part, s, b, e))
        entry('4054E0', ' nfs_controls_setup_text::PolicyScope nfs_controls_setup_policy(*this,true);')
        entry('405580', ' nfs_controls_setup_text::PolicyScope nfs_controls_setup_policy(*this,false);')
        entry('405CF0', ' nfs_controls_setup_text::StringScope nfs_controls_setup_string(*this);')

        def policy(part):
            anchor = call('41B690')
            at = once(part, anchor)+len(anchor)
            _outside(part, at)
            return part[:at]+block('resource-alignment', ' nfs_controls_setup_text::resourceAlignment(*this);')+part[at:]
        body('405580', policy)
    elif name == 'Methods_09.cpp':
        entry('4824CC', ' nfs_controls_setup_text::SelectionScope nfs_controls_setup_selection(*this);')
    elif name == 'Methods_10.cpp':
        entry('482DA0', ' nfs_controls_setup_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    elif name == 'Methods_13.cpp':
        entry('4A5830', ' nfs_controls_setup_text::GlyphScope nfs_controls_setup_glyph(*this);')
    elif name == 'Methods_14.cpp':
        entry('4AC934', ' nfs_controls_setup_text::beforeOwnerRetire(*this,uint32_t(eax),true);')
        entry('4AC9C8', ' nfs_controls_setup_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    if name in ('Methods_13.cpp', 'Methods_14.cpp'):
        symbol, count = ('4A5830', 4) if name == 'Methods_13.cpp' else ('4B3EE0', 1)

        def stores(part):
            store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            if part.count(store) != count:
                raise ValueError('Controls setup original glyph store count changed')
            pieces = part.split(store)
            out = pieces[0]
            for i, piece in enumerate(pieces[1:]):
                _outside(out, len(out))
                out += block('word-open-'+str(i), ' { nfs_controls_setup_text::WordCapture nfs_controls_setup_word(*this,uint32_t(esi),uint16_t(bx));')
                out += store+block('word-close-'+str(i), ' }')+piece
            return out
        body(symbol, stores)
    for found in re.finditer(r'// '+MARKER+r'_BEGIN:', text):
        _outside(text, found.start())
    if remove_controls_setup_text_hooks(text) != original:
        raise ValueError('Controls setup text exact source reversal failed')
    return text
