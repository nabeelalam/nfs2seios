"""Observe complete original child Controls frames after setup text/sidebar hooks.

Manual tail boundaries include appended footer/prompt text without introducing
C++ scopes across original gotos. No renderer admission or input changes.
"""
# SPDX-License-Identifier: MIT
import re

MARKER = 'NFS_CONTROLS_SETUP_LIVE'
NAMES = ('Methods_01.cpp', 'Methods_09.cpp', 'Methods_10.cpp',
         'Methods_13.cpp', 'Methods_14.cpp')


def remove_controls_setup_live_hooks(text):
    result = re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+
                    MARKER+r'_END:\1\r?\n', '', text, flags=re.S)
    if MARKER in result:
        raise ValueError('Unmatched Controls setup live marker')
    return result


def add_controls_setup_live_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Controls setup live hooks already present')
    original = text
    crlf = '\r\n' in text
    if crlf and '\n' in text.replace('\r\n', ''):
        raise ValueError('Mixed Controls setup source newlines')
    if crlf:
        text = text.replace('\r\n', '\n')
    for dependency in ('ControlsSetupTextGameHooks.inc',
                       'ControlsSetupSidebarLiveGameHooks.inc'):
        if text.count('#include "hd/'+dependency+'"\n') != 1:
            raise ValueError('Controls setup live prerequisite changed '+dependency)

    def block(tag, code):
        return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'

    def unique(anchor, replacement):
        nonlocal text
        if text.count(anchor) != 1:
            raise ValueError('Missing/duplicate Controls setup live anchor '+anchor[:100])
        text = text.replace(anchor, replacement, 1)

    def body(symbol, transform):
        nonlocal text
        matches = list(re.finditer(r'Fn\(void\) Game::_sub_'+symbol+
                                   r'\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)', text, re.S))
        if len(matches) != 1:
            raise ValueError('Missing/duplicate Controls setup live function '+symbol)
        found = matches[0]
        text = text[:found.start()]+transform(found.group())+text[found.end():]

    def once(part, anchor):
        if part.count(anchor) != 1:
            raise ValueError('Controls setup live original anchor changed '+anchor[:100])
        return part.index(anchor)

    def call(symbol):
        return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'

    def wrap(part, label, callee, tag, code, required):
        start = once(part, label+':\n')
        end = part.find('\nloc_', start+len(label)+2)
        end = len(part) if end < 0 else end
        region = part[start:end]
        at = once(region, call(callee))
        prefix = region[:at]
        if any(needle not in prefix for needle in required):
            raise ValueError('Controls setup original source arguments changed '+tag)
        at += start
        return (part[:at]+block(tag+'-open', ' { '+code)+call(callee)+
                block(tag+'-close', ' }')+part[at+len(call(callee)):])

    anchor = '#include "FunctionIds.h"\n'
    unique(anchor, anchor+block('include', '#include "hd/ControlsSetupLiveGameHooks.inc"'))
    if name == 'Methods_01.cpp':
        for symbol in ('4101C0', '403970'):
            def owner(part, symbol=symbol):
                anchor = (' nfs_controls_setup_text::OwnerScope '
                          'nfs_controls_setup_owner(*this,0x'+symbol.lower()+'u);')
                at = once(part, anchor)+len(anchor)
                return (part[:at]+block('owner-'+symbol,
                        ' nfs_controls_setup_live::OwnerScope nfs_controls_setup_live_owner(*this);')+part[at:])
            body(symbol, owner)
        anchor = 'Fn(void) Game::_sub_405B60()\n{\n'
        unique(anchor, anchor+block('resolved',
               ' nfs_controls_setup_live::ResolvedScope nfs_controls_setup_live_resolved(*this);'))
        anchor = ' nfs_controls_setup_text::StringScope nfs_controls_setup_string(*this);'
        unique(anchor, anchor+block('bind-text', ' nfs_controls_setup_live::bindText(*this);'))
        for symbol, label in (('4101C0', 'loc_41020E'),
                              ('403970', 'loc_4039B5'),
                              ('403FC0', 'loc_40419F')):
            def tail(part, symbol=symbol, label=label):
                reset, copy, draw, swap = [once(part, call(s)) for s in ('405B30', '405B60', '40FA90', '405C70')]
                begin = once(part, ' nfs_controls_setup_text::beginTail(*this);')
                finish = once(part, ' nfs_controls_setup_text::finishTail(*this);')
                if not once(part, label+':\n') < reset < copy < begin < draw < finish < swap:
                    raise ValueError('Controls setup complete frame ordering changed')
                required = ('_sub_428A20()', '\tedx = 0x400; //mov\n')
                if symbol == '403FC0':
                    required += ('(esp+0x2C)', '(eax+4)', '\tecx = edi; //mov\n', '\tebx = edi; //mov\n')
                else:
                    required += ('_sub_40EB20()', '\teax = to32i(esi+4); //mov\n',
                                 '\txor_(ecx, ecx);\n', '\txor_(ebx, ebx);\n')
                part = wrap(part, label, '405B30', 'reset-'+symbol,
                            'nfs_controls_setup_live::ResetScope nfs_controls_setup_live_reset(*this);', required)
                zeros = ('\tecx = edi; //mov\n', '\tebx = edi; //mov\n') if symbol == '403FC0' else ('\txor_(ecx, ecx);\n', '\txor_(ebx, ebx);\n')
                part = wrap(part, label, '405B60', 'sidebar-'+symbol,
                            'nfs_controls_setup_live::SidebarScope nfs_controls_setup_live_sidebar(*this);',
                            ('dword_4D2314+2', '\tsar(eax, (int32_t)0x10);\n', 'dword_4D231C+eax*4',
                             'eax+0x20', '\tedx = 0x1000; //mov\n')+zeros)
                anchor = ' nfs_controls_setup_text::beginTail(*this);'
                at = once(part, anchor)
                part = part[:at]+block('begin-'+symbol, ' nfs_controls_setup_live::beginTail(*this);')+part[at:]
                anchor = ' nfs_controls_setup_text::finishTail(*this);'
                at = once(part, anchor)+len(anchor)
                return part[:at]+block('finish-'+symbol, ' nfs_controls_setup_live::finishTail(*this);')+part[at:]
            body(symbol, tail)
    elif name == 'Methods_10.cpp':
        anchor = ' nfs_controls_setup_text::beforeDescriptorRetire(*this,uint32_t(eax));'
        unique(anchor, anchor+block('retired-descriptor', ' nfs_controls_setup_live::afterRetire(*this);'))
    elif name == 'Methods_14.cpp':
        for tag, anchor in (('retired-owner', ' nfs_controls_setup_text::beforeOwnerRetire(*this,uint32_t(eax),true);'),
                            ('retired-descriptor', ' nfs_controls_setup_text::beforeDescriptorRetire(*this,uint32_t(eax));')):
            unique(anchor, anchor+block(tag, ' nfs_controls_setup_live::afterRetire(*this);'))
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:', text):
        prefix = text[:match.start()]
        if prefix.count('// checked copy transaction begin') != prefix.count('// checked copy transaction end'):
            raise ValueError('Controls setup live hook inside checked-copy lease')
    if crlf:
        text = text.replace('\n', '\r\n')
    if remove_controls_setup_live_hooks(text) != original:
        raise ValueError('Controls setup live byte reversal failed')
    return text
