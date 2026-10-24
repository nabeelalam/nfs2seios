"""Reversible independent steering-chart observations after the steering text hooks.

Dependency: ControlsSteeringChartGameHooks.inc owns source/chart observations;
these insertions grant no native renderer authority or normal-pipeline activation.
"""
# SPDX-License-Identifier: MIT
import re
MARKER='NFS_CONTROLS_STEERING_CHART'
NAMES=('Methods_01.cpp','Methods_10.cpp')
TEXT='NFS_CONTROLS_STEERING_TEXT'
STORE='\tguest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'
CODES={
 'include':'#include "hd/ControlsSteeringChartGameHooks.inc"',
 'entry-410330':' nfs_controls_steering_chart_live::ChartScope nfs_controls_steering_chart(*this);',
 'entry-405B60':' nfs_controls_steering_chart_live::ResolvedScope nfs_controls_steering_chart_resolved(*this);',
 'entry-483288':' nfs_controls_steering_chart_live::RowScope nfs_controls_steering_chart_row(*this,uint32_t(game_id_sub_483288));',
 'entry-483070':' nfs_controls_steering_chart_live::UnsupportedRowScope nfs_controls_steering_chart_unsupported(*this);',
 'word-open':' { nfs_controls_steering_chart_live::WordCapture nfs_controls_steering_chart_word(*this,uint32_t(esi),uint16_t(bx));',
 'word-close':' }',
}
SETS=({'include','entry-410330','entry-405B60'},
      {'include','entry-483288','entry-483070','word-open','word-close'})

def _normalized(text):
    crlf='\r\n' in text
    if '\r' in text.replace('\r\n','') or (crlf and '\n' in text.replace('\r\n','')):
        raise ValueError('Mixed steering chart source newlines')
    return (text.replace('\r\n','\n') if crlf else text),crlf

def _block(tag,code):
    return '\n// '+MARKER+'_BEGIN:'+tag+'\n'+code+'\n// '+MARKER+'_END:'+tag+'\n'

def remove_controls_steering_chart_hooks(text):
    source,crlf=_normalized(text)
    pattern=r'\n// '+MARKER+r'_BEGIN:([^\n]+)\n(.*?)\n// '+MARKER+r'_END:\1\n'
    matches=list(re.finditer(pattern,source,re.S))
    if not matches:
        if MARKER in source:raise ValueError('Malformed steering chart markers')
        return text
    tags=[m.group(1) for m in matches]
    if len(tags)!=len(set(tags)) or set(tags) not in SETS:
        raise ValueError('Partial, repeated or unknown steering chart hooks')
    for m in matches:
        if m.group(1) not in CODES or m.group(2)!=CODES[m.group(1)]:
            raise ValueError('Changed or nested steering chart block')
    if 'word-open' in tags:
        opened=next(m for m in matches if m.group(1)=='word-open')
        closed=next(m for m in matches if m.group(1)=='word-close')
        if opened.end()>closed.start() or source[opened.end():closed.start()]!=STORE:
            raise ValueError('Changed steering chart protected store')
        if not source[:opened.start()].endswith('loc_4832FD:\n') or not source[closed.end():].startswith('loc_483300:\n'):
            raise ValueError('Changed steering chart protected labels')
    result=re.sub(pattern,'',source,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched steering chart marker')
    return result.replace('\n','\r\n') if crlf else result

reverse_controls_steering_chart_hooks=remove_controls_steering_chart_hooks

def _outside(text,at):
    prefix=text[:at]
    if prefix.count('// checked copy transaction begin')!=prefix.count('// checked copy transaction end'):
        raise ValueError('Steering chart hook inside checked-copy transaction')

def _clean(text):
    # Preserve offsets while ignoring braces in C++ comments and quoted strings.
    pattern=r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern,lambda m:' '*len(m.group()),text,flags=re.S)

def _span(text,symbol):
    pattern=r'Fn\(void\) Game::_sub_'+re.escape(symbol)+r'\(\)\n\{\n'
    matches=list(re.finditer(pattern,text))
    if len(matches)!=1:raise ValueError('Missing/ambiguous steering chart function '+symbol)
    m=matches[0];start=text.index('{',m.start());clean=_clean(text);depth=1;at=start+1
    while at<len(clean) and depth:
        depth+=(clean[at]=='{')-(clean[at]=='}');at+=1
    if depth or re.search(r'\bFn\s*\(',clean[start+1:at-1]):
        raise ValueError('Malformed/nested steering chart function '+symbol)
    return m.start(),start+2,at

def add_controls_steering_chart_hooks(text,name):
    if name not in NAMES:return text
    original=text;text,crlf=_normalized(text)
    if MARKER in text:raise ValueError('Steering chart hooks already present or malformed')
    prior='\n// '+TEXT+'_BEGIN:include\n#include "hd/ControlsSteeringTextGameHooks.inc"\n// '+TEXT+'_END:include\n'
    if text.count(prior)!=1:raise ValueError('Complete prior steering text include required')
    prior_tag='entry-4101C0' if name=='Methods_01.cpp' else 'entry-482DA0'
    if text.count('// '+TEXT+'_BEGIN:'+prior_tag+'\n')!=1 or text.count('// '+TEXT+'_END:'+prior_tag+'\n')!=1:
        raise ValueError('Prior steering text owner/lifetime hook required')
    expected_prior={
        'include':'#include "hd/ControlsSteeringTextGameHooks.inc"',
        'entry-4101C0':' nfs_controls_steering_text::OwnerScope nfs_controls_steering_owner(*this,0x4101c0u);',
        'begin-text':' nfs_controls_steering_text::beginTail(*this);',
        'finish-before-chart':' nfs_controls_steering_text::finishTail(*this);',
        'entry-4054E0':' nfs_controls_steering_text::PolicyScope nfs_controls_steering_policy(*this,true);',
        'entry-405580':' nfs_controls_steering_text::PolicyScope nfs_controls_steering_policy(*this,false);',
        'entry-405CF0':' nfs_controls_steering_text::StringScope nfs_controls_steering_string(*this);',
        'resource-alignment':' nfs_controls_steering_text::resourceAlignment(*this);',
    } if name=='Methods_01.cpp' else {
        'include':'#include "hd/ControlsSteeringTextGameHooks.inc"',
        'entry-482DA0':' nfs_controls_steering_text::beforeDescriptorRetire(*this,uint32_t(eax));',
    }
    prior_blocks=re.findall(r'\n// '+TEXT+r'_BEGIN:([^\n]+)\n(.*?)\n// '+TEXT+r'_END:\1\n',text,re.S)
    if len(prior_blocks)!=len(expected_prior) or dict(prior_blocks)!=expected_prior:
        raise ValueError('Complete unchanged steering text hooks required')
    # The shared pre-chart text collector must remain intact and removable.
    from controls_steering_text_hooks import remove_controls_steering_text_hooks
    remove_controls_steering_text_hooks(text)
    at=text.index(prior)+len(prior);_outside(text,at)
    text=text[:at]+_block('include',CODES['include'])+text[at:]
    def entry(symbol):
        nonlocal text
        _,at,_=_span(text,symbol);_outside(text,at)
        text=text[:at]+_block('entry-'+symbol,CODES['entry-'+symbol])+text[at:]
    if name=='Methods_01.cpp':
        entry('410330');entry('405B60')
    else:
        # Guard the actual indexed row structure before adding observations.
        start,_,end=_span(text,'483288');part=text[start:end]
        anchors=('loc_483292:\n','\tebx = guest_transaction.load<int32_t>(dword_5636EC); //mov\n',
                 'loc_4832FD:\n'+STORE+'loc_483300:\n',
                 '\tesi = static_cast<int32_t>(guest_address(esi+2)); //lea\n',
                 '\tdec(edi);\n','\t\tgoto loc_483292;\n')
        if any(part.count(a)!=1 for a in anchors) or part.count(STORE)!=1 or part.count('loc_4832FD:')!=1 or part.count('loc_483300:')!=1:
            raise ValueError('Indexed steering chart original row/store labels changed')
        start0,_,end0=_span(text,'483070');unsupported=text[start0:end0]
        if unsupported.count('loc_48307A:\n')!=1 or unsupported.count('\tebx = to32i(edi); //mov\n')!=1:
            raise ValueError('Unsupported steering chart row structure changed')
        entry('483288');entry('483070')
        start,_,end=_span(text,'483288');part=text[start:end]
        label='loc_4832FD:\n';at=part.index(label)+len(label);_outside(text,start+at)
        replacement=label+_block('word-open',CODES['word-open'])+STORE+_block('word-close',CODES['word-close'])+'loc_483300:\n'
        needle=label+STORE+'loc_483300:\n'
        part=part.replace(needle,replacement,1);text=text[:start]+part+text[end:]
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:',text):_outside(text,match.start())
    result=text.replace('\n','\r\n') if crlf else text
    if remove_controls_steering_chart_hooks(result)!=original:
        raise ValueError('Steering chart exact source reversal failed')
    return result
