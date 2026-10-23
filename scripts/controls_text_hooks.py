"""Add only the original Controls text collector; preserve every original operation."""
# SPDX-License-Identifier: MIT
import re
MARKER='NFS_CONTROLS_TEXT'
NAMES=('Methods_01.cpp','Methods_09.cpp','Methods_10.cpp','Methods_13.cpp','Methods_14.cpp')

def remove_controls_text_hooks(text):
    result=re.sub(r'\r?\n// '+MARKER+r'_BEGIN:([^\r\n]+)\r?\n.*?// '+MARKER+r'_END:\1\r?\n','',text,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched Controls text hook marker')
    return result

def _outside(text,at):
    prefix=text[:at]
    if prefix.count('// checked copy transaction begin')!=prefix.count('// checked copy transaction end'):
        raise ValueError('Controls text observation inside checked-copy lease')

def add_controls_text_hooks(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError('Controls text hooks already present')
    original=text;nl='\r\n' if '\r\n' in text else '\n'
    def block(tag,code):return nl+'// '+MARKER+'_BEGIN:'+tag+nl+code+nl+'// '+MARKER+'_END:'+tag+nl
    def unique(before,after):
        nonlocal text
        if text.count(before)!=1:raise ValueError('Missing/duplicate Controls source anchor: '+name+' '+before[:90])
        _outside(text,text.index(before));text=text.replace(before,after,1)
    def entry(symbol,code):
        anchor='Fn(void) Game::_sub_'+symbol+'()'+nl+'{'+nl
        unique(anchor,anchor+block('entry-'+symbol,code))
    def body(symbol,transform):
        nonlocal text
        pattern=r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\r?\n\{.*?(?=\r?\nFn\(void\) Game::|\Z)'
        found=list(re.finditer(pattern,text,re.S))
        if len(found)!=1:raise ValueError('Missing/duplicate Controls function: '+symbol)
        match=found[0];part=match.group();changed=transform(part)
        # Check each injected block in complete source, not just a sliced body.
        text=text[:match.start()]+changed+text[match.end():]
    def call(symbol):return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call'
    def after_call(part,symbol,tag,code):
        anchor=call(symbol)
        if part.count(anchor)!=1:raise ValueError('Missing/duplicate Controls original call: '+symbol)
        _outside(part,part.index(anchor));return part.replace(anchor,anchor+block(tag,code),1)
    anchor='#include "FunctionIds.h"'+nl
    unique(anchor,anchor+block('include','#include "hd/ControlsTextGameHooks.inc"'))
    if name=='Methods_01.cpp':
        entry('4101C0',' nfs_controls_text::OwnerScope nfs_controls_owner(*this);')
        entry('4054E0',' nfs_controls_text::PolicyScope nfs_controls_policy(*this,true);')
        entry('405580',' nfs_controls_text::PolicyScope nfs_controls_policy(*this,false);'+nl+' nfs_controls_text::DialogPolicyScope nfs_controls_dialog_policy(*this);')
        entry('405CF0',' nfs_controls_text::StringScope nfs_controls_string(*this);')
        def policy(part):
            part=after_call(part,'41B690','resource-alignment',' nfs_controls_text::resourceAlignment(*this);'+nl+' nfs_controls_text::dialogResourceAlignment(*this);')
            part=after_call(part,'41B710','resolved-resource',' nfs_controls_text::dialogResolvedResource(*this,uint32_t(eax));')
            for label,kind,suffix in (
                ('loc_4055D3',2,'\tsub(esi, eax);'),
                ('loc_405646',1,'\tedx = eax; //mov'+nl+'\tsar(edx, (int32_t)0x1F);'+nl+'\tsub(eax, edx);'+nl+'\tsar(eax, (int32_t)1);'+nl+'\tedx = si; //movsx'+nl+'\tsub(edx, eax);'+nl+'\tesi = edx; //mov')):
                head=label+':'+nl
                if part.count(head)!=1:raise ValueError('Original Controls measurement label changed')
                start=part.index(head);end=part.find(nl+'loc_',start+len(head));end=len(part)if end<0 else end
                region=part[start:end];prefix=head+'\teax = ecx; //mov'+nl+call('482424')
                if region.count(prefix)!=1 or region.count(call('482424'))!=1 or region.count(suffix)!=1 or not(suffix+nl in region or region.endswith(suffix)) or region.index(suffix)<region.index(call('482424')):
                    raise ValueError('Original Controls measurement registers/order changed')
                at=start+region.index(call('482424'));_outside(part,at)
                at+=len(call('482424'))
                part=part[:at]+block('measure-'+str(kind),' nfs_controls_text::dialogMeasured(*this,uint32_t(ecx),int32_t(eax),int32_t(esi),'+str(kind)+');')+part[at:]
            return part
        body('405580',policy)
        def normal(part):
            head='loc_41020E:'+nl
            if part.count(head)!=1:raise ValueError('Controls normal owner label changed')
            start=part.index(head);end=part.find(nl+'loc_',start+len(head));end=len(part)if end<0 else end
            region=part[start:end]
            originals=[call(s)for s in ('405B30','405B60','40FA90')]
            if any(region.count(c)!=1 for c in originals) or not region.index(originals[0])<region.index(originals[1])<region.index(originals[2]):
                raise ValueError('Original normal reset/sidebar/full-text sequence changed')
            if 'eax = esi; //mov' not in region[region.index(originals[1]):region.index(originals[2])]:raise ValueError('Original normal full-text form argument changed')
            at=start+region.index(originals[2]);_outside(part,at)
            return part[:at]+block('tail-open',' { nfs_controls_text::TailScope nfs_controls_tail(*this);')+originals[2]+block('tail-close',' }')+part[at+len(originals[2]):]
        body('4101C0',normal)
    elif name=='Methods_09.cpp':
        entry('4824CC',' nfs_controls_text::SelectionScope nfs_controls_selection(*this);')
    elif name=='Methods_10.cpp':
        entry('482DA0',' nfs_controls_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    elif name=='Methods_13.cpp':
        entry('4A5830',' nfs_controls_text::GlyphScope nfs_controls_glyph(*this);')
    else:
        entry('4AC934',' nfs_controls_text::beforeOwnerRetire(*this,uint32_t(eax),true);')
        entry('4AC9C8',' nfs_controls_text::beforeDescriptorRetire(*this,uint32_t(eax));')
    if name in ('Methods_13.cpp','Methods_14.cpp'):
        symbol,count=('4A5830',4)if name=='Methods_13.cpp'else('4B3EE0',1)
        def words(part):
            store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            if part.count(store)!=count:raise ValueError('Original Controls glyph store count changed')
            pieces=part.split(store);out=pieces[0]
            for index,piece in enumerate(pieces[1:]):
                _outside(out,len(out))
                out+=block('word-open-'+str(index),' { nfs_controls_text::WordCapture nfs_controls_word(*this,uint32_t(esi),uint16_t(bx));')+store+block('word-close-'+str(index),' }')+piece
            return out
        body(symbol,words)
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:',text):_outside(text,match.start())
    if remove_controls_text_hooks(text)!=original:raise ValueError('Controls text exact source reversal failed')
    return text
