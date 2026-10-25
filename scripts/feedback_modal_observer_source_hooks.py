"""Observe the exact original full640 native pass under a separate popup scope.

Apply after existing Controls scene hooks. This transformer adds no Joined
factory, renderer publication, artwork substitution, queue receipt or counter.
"""
# SPDX-License-Identifier: MIT
import re

MARKER = 'NFS_FEEDBACK_MODAL_OBSERVER_SOURCE'
NAMES = ('Methods_03.cpp',)
_INCLUDE = '#include "hd/FeedbackModalObserverQueueAdapter.inc"'
_SCOPE = ' nfs_feedback_modal_observer_queue::NativeScope nfs_feedback_modal_native(*this);'
_BLOCK = re.compile(
    r'\n// '+MARKER+r'_BEGIN:([a-z0-9-]+)\n(.*?)\n// '+MARKER+r'_END:\1\n',
    re.S)


def _lf(text):
    crlf = '\r\n' in text
    if crlf and '\n' in text.replace('\r\n', ''):
        raise ValueError('Mixed Feedback native observer source newlines')
    return (text.replace('\r\n', '\n') if crlf else text), crlf


def _code_only(text):
    # Preserve offsets/newlines while ignoring comments and string/character
    # literals for control-flow and brace checks. Required original spellings
    # are also checked verbatim separately below.
    pattern = r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group()), text, flags=re.S)


def remove_feedback_modal_observer_source_hooks(text):
    if MARKER not in text:
        return text
    normalized, crlf = _lf(text)
    blocks = list(_BLOCK.finditer(normalized))
    tags = [m.group(1) for m in blocks]
    if tags != ['include', 'native-scope']:
        raise ValueError('Missing, duplicate or reordered Feedback native observer marker')
    expected = {'include': _INCLUDE, 'native-scope': _SCOPE}
    if any(m.group(2) != expected[m.group(1)] for m in blocks):
        raise ValueError('Modified Feedback native observer block')
    result = _BLOCK.sub('', normalized)
    if MARKER in result:
        raise ValueError('Unmatched Feedback native observer marker')
    return result.replace('\n', '\r\n') if crlf else result


def add_feedback_modal_observer_source_hooks(text, name):
    if name not in NAMES:
        return text
    if MARKER in text:
        raise ValueError('Feedback native observer source hooks already present')
    original = text
    text, crlf = _lf(text)

    def block(tag, code):
        return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'

    anchor = '#include "FunctionIds.h"\n'
    if text.count(anchor) != 1:
        raise ValueError('Feedback native observer include changed')
    matches = list(re.finditer(
        r'Fn\(void\) Game::_sub_426950\(\)\n\{(?P<body>.*?)(?=\nFn\(void\) Game::|\Z)',
        text, re.S))
    if len(matches) != 1:
        raise ValueError('Missing/duplicate original native426950')
    match = matches[0]
    body = match.group('body')
    code = _code_only(body)
    last = body.rfind('}')
    if last < 0 or code[last+1:].strip():
        raise ValueError('Original native function extent changed')
    if re.search(r'\b(return|co_return|break|continue)\b', code):
        raise ValueError('Original native early exit changed')
    depth = 1
    for at, char in enumerate(code):
        if char == '{':
            depth += 1
        elif char == '}':
            depth -= 1
            if depth < 0 or (depth == 0 and code[at+1:].strip()):
                raise ValueError('Malformed original native scope extent')
    if depth:
        raise ValueError('Unclosed original native scope')

    for tag in ('begin', 'finish'):
        if body.count('// NFS_CONTROLS_SCENE_BEGIN:'+tag+'\n') != 1:
            raise ValueError('Feedback native observer requires existing Controls '+tag+' hook')
    for operation in ('nfs_controls_scene_live::begin(', 'nfs_controls_scene_live::finish('):
        if code.count(operation) != 1:
            raise ValueError('Existing Controls native wrapper changed')

    required = (
        '\tpush32(ebx);', '\txor_(edi, edi);', 'loc_42697D:\n',
        '\txor_(esi, esi);', 'loc_4269A7:\n',
        '\tesp -= 4; ___CHP(); esp += 4; //call',
        '\tesp -= 4; _sub_4850B0(); esp += 4; //call',
        '\tesp -= 4; _sub_49755C(); esp += 4; //call',
        '\tesp -= 4; _sub_497738(); esp += 4; //call',
        '\tadd(esi, (int32_t)0x20);',
        '\tesp -= 4; _sub_497D6C(); esp += 4; //call',
        '\tcmp(esi, (int32_t)0x1E0);', '\t\tgoto loc_4269A7;',
        '\tadd(edi, (int32_t)0x20);', '\tcmp(edi, (int32_t)0x280);',
        '\t\tgoto loc_42697D;', '\tpop32(ebx);')
    for operation in required:
        semantic = operation.split(' //call')[0]
        if body.count(operation) != 1 or code.count(semantic) != 1:
            raise ValueError('Original native operation/extent changed: '+operation.strip())
    order = tuple(operation.split(' //call')[0] for operation in required)
    positions = [code.index(operation) for operation in order]
    if positions != sorted(positions):
        raise ValueError('Original native upload/select/triangle/loop order changed')
    for index, bound, target in (('esi', '0x1E0', 'loc_4269A7'), ('edi', '0x280', 'loc_42697D')):
        branch = '\tcmp('+index+', (int32_t)'+bound+');\n\tif (jnz())\n\t\tgoto '+target+';'
        if code.count(branch) != 1:
            raise ValueError('Original native loop branch changed')
    if re.findall(r'\bgoto\s+([A-Za-z_][A-Za-z_0-9]*)\s*;', code) != ['loc_4269A7', 'loc_42697D']:
        raise ValueError('Original native control-flow edges changed')

    # The new RAII owner is deliberately outside every existing scene Intent.
    # Its destructor observes normal completion or cancels during unwinding.
    replacement = 'Fn(void) Game::_sub_426950()\n{'+block('native-scope', _SCOPE)+body
    text = text[:match.start()]+replacement+text[match.end():]
    text = text.replace(anchor, anchor+block('include', _INCLUDE), 1)

    depth = 0
    for event in re.finditer(r'// checked copy transaction (begin|end)|// '+MARKER+r'_BEGIN:', text):
        if event.group(1) == 'begin':
            depth += 1
        elif event.group(1) == 'end':
            depth -= 1
            if depth < 0:
                raise ValueError('Unmatched original guest-copy end')
        elif depth:
            raise ValueError('Feedback native observer hook inside guest-copy lease')
    if depth:
        raise ValueError('Unclosed original guest-copy lease')
    if crlf:
        text = text.replace('\n', '\r\n')
    if remove_feedback_modal_observer_source_hooks(text) != original:
        raise ValueError('Feedback native observer exact source reversal failed')
    return text
