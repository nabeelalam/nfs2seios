"""Independent steering-child343 capture; apply after steering text and chart hooks.

Audio hooks remain independent and unchanged. Every splice is asserted, reversible,
and additive; source owner, lazy gate and original measurement supply authority.
"""
# SPDX-License-Identifier: MIT
import re
from controls_steering_chart_hooks import _span as _function_span
MARKER="NFS_CONTROLS_STEERING_SIDEBAR"
NAMES=("Methods_01.cpp","Methods_09.cpp","Methods_10.cpp","Methods_13.cpp","Methods_14.cpp")

def _add_controls_steering_sidebar_hooks_lf(text,name):
    if name not in NAMES:return text
    if MARKER in text:raise ValueError("Controls steering sidebar hooks already present")
    from controls_steering_text_hooks import remove_controls_steering_text_hooks
    from controls_steering_chart_hooks import remove_controls_steering_chart_hooks
    if text.count('// NFS_CONTROLS_STEERING_TEXT_BEGIN:include\n#include \"hd/ControlsSteeringTextGameHooks.inc\"\n// NFS_CONTROLS_STEERING_TEXT_END:include')!=1 or remove_controls_steering_text_hooks(text)==text:
        raise ValueError("Complete independent steering text hooks required")
    required={'Methods_01.cpp':'entry-4101C0','Methods_09.cpp':'entry-4824CC','Methods_10.cpp':'entry-482DA0','Methods_13.cpp':'entry-4A5830','Methods_14.cpp':'entry-4AC934'}[name]
    if text.count('// NFS_CONTROLS_STEERING_TEXT_BEGIN:'+required+'\n')!=1 or text.count('// NFS_CONTROLS_STEERING_TEXT_END:'+required+'\n')!=1:
        raise ValueError("Steering text owner/retirement context required")
    priorCode={
        'Methods_01.cpp':' nfs_controls_steering_text::OwnerScope nfs_controls_steering_owner(*this,0x4101c0u);',
        'Methods_09.cpp':' nfs_controls_steering_text::SelectionScope nfs_controls_steering_selection(*this);',
        'Methods_10.cpp':' nfs_controls_steering_text::beforeDescriptorRetire(*this,uint32_t(eax));',
        'Methods_13.cpp':' nfs_controls_steering_text::GlyphScope nfs_controls_steering_glyph(*this);',
        'Methods_14.cpp':' nfs_controls_steering_text::beforeOwnerRetire(*this,uint32_t(eax),true);',
    }[name]
    expected='// NFS_CONTROLS_STEERING_TEXT_BEGIN:'+required+'\n'+priorCode+'\n// NFS_CONTROLS_STEERING_TEXT_END:'+required
    if text.count(expected)!=1:raise ValueError("Changed steering owner/retirement observation")
    if name in ('Methods_01.cpp','Methods_10.cpp'):
        if 'NFS_CONTROLS_STEERING_CHART_BEGIN:include' not in text or remove_controls_steering_chart_hooks(text)==text:
            raise ValueError("Complete independent steering chart hooks required")
    def block(tag,code):return "\n// "+MARKER+"_BEGIN:"+tag+"\n"+code+"\n// "+MARKER+"_END:"+tag+"\n"
    def unique(old,new):
        nonlocal text
        if text.count(old)!=1:raise ValueError("Controls steering sidebar anchor changed: "+name+" "+old[:80])
        text=text.replace(old,new,1)
    def entry(symbol,code):
        a="Fn(void) Game::_sub_"+symbol+"()\n{\n";unique(a,a+block("entry-"+symbol,code))
    def body(symbol,mutate):
        start,_,end=_function_span(text,symbol)
        part=text[start:end]
        unique(part,mutate(part))
    a='#include "FunctionIds.h"\n';unique(a,a+block("include",'#include "hd/ControlsSteeringSidebarCaptionGameHooks.inc"\n#include "hd/ControlsSteeringSidebarLiveGameHooks.inc"'))
    if name=="Methods_01.cpp":
        for symbol,handler in (("4101C0","4101c0"),):
            def owners(part,handler=handler):
                a=" nfs_controls_steering_text::OwnerScope nfs_controls_steering_owner(*this,0x"+handler+"u);"
                if part.count(a)!=1:raise ValueError("Controls steering sidebar lexical owner changed: "+handler)
                return part.replace(a,a+block("caption-owner-"+handler," nfs_controls_steering_sidebar_text::OwnerScope nfs_controls_steering_sidebar_owner(*this);")+block("cache-owner-"+handler," nfs_controls_steering_sidebar_live::OwnerScope nfs_controls_steering_cache_owner(*this);"),1)
            body(symbol,owners)
        entry("40C110"," nfs_controls_steering_sidebar_live::TailScope nfs_controls_steering_cache_tail(*this);\n nfs_controls_steering_sidebar_text::TailScope nfs_controls_steering_sidebar_tail(*this);")
        entry("405CF0"," nfs_controls_steering_sidebar_text::StringScope nfs_controls_steering_sidebar_string(*this);\n nfs_controls_steering_sidebar_live::bindCaption(*this);")
        entry("40BBE0"," nfs_controls_steering_sidebar_live::IterationScope nfs_controls_steering_cache_iteration(*this);")
        entry("405B60"," nfs_controls_steering_sidebar_live::ResolvedScope nfs_controls_steering_cache_resolved(*this);")
        def lazy(part):
            replacements=[(" nfs_hd_scene_iteration.start();"," nfs_controls_steering_cache_iteration.start();","lazy-after-original-gate"),(" { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);"," nfs_controls_steering_sidebar_live::RoleScope nfs_controls_steering_cache_side(*this,nfs_controls_steering_sidebar_live::Phase::side);","side"),(" { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);"," nfs_controls_steering_sidebar_live::RoleScope nfs_controls_steering_cache_word(*this,nfs_controls_steering_sidebar_live::Phase::word);","word"),(" nfs_background_scene_live::beginGroup(*this);"," nfs_controls_steering_sidebar_live::preGroup(*this);","group"),(" nfs_background_scene_live::postGroup(*this);"," nfs_controls_steering_sidebar_live::postGroup(*this);","group-complete")]
            if part.count("loc_40BC20:")!=1 or part.index("loc_40BC20:")>part.index(" nfs_hd_scene_iteration.start();"):raise ValueError("Controls steering sidebar lazy sentinel changed")
            gate=part[:part.index("loc_40BC20:")]
            for original in ("test(dx, dx);","loc_40BC02:","dword_4D2440","dword_5637F4","goto loc_40BD11;"):
                if original not in gate:raise ValueError("Controls steering sidebar original lazy gate changed")
            for anchor,code,tag in replacements:
                if part.count(anchor)!=1:raise ValueError("Controls steering sidebar lazy source changed: "+tag)
                part=part.replace(anchor,anchor+block(tag,code),1)
            return part
        body("40BBE0",lazy)
        # Controls TEXT hooks pin both actual original482424 returns before
        # subtraction. Preserve those calls; add independent343 measurement.
        def measurements(part):
            # Exact original measurement returns, before right/center arithmetic.
            # Parent observations may sit between these instructions but neither
            # their metadata nor their current state grants child authority.
            call="\tesp -= 4; _sub_482424(); esp += 4; //call"
            centre="\tedx = eax; //mov\n\tsar(edx, (int32_t)0x1F);\n\tsub(eax, edx);\n\tsar(eax, (int32_t)1);\n\tedx = si; //movsx\n\tsub(edx, eax);\n\tesi = edx; //mov"
            for label,kind,suffix in (("loc_4055D3","right","\tsub(esi, eax);"),("loc_405646","center",centre)):
                head=label+":\n"
                if part.count(head)!=1:raise ValueError("Controls steering sidebar measurement label changed")
                start=part.index(head);end=part.find("\nloc_",start+len(head));end=len(part)if end<0 else end
                region=part[start:end];prefix=head+"\teax = ecx; //mov\n"+call
                if region.count(prefix)!=1 or region.count(call)!=1 or region.count(suffix)!=1 or region.index(suffix)<region.index(call):raise ValueError("Controls steering sidebar original measurement changed: "+kind)
                at=start+region.index(call)+len(call)
                part=part[:at]+block("measured-"+kind," nfs_controls_steering_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_controls_steering_sidebar_text::SourceMeasureAlignment::"+kind+");")+part[at:]
            return part
        body("405580",measurements)
    elif name=="Methods_09.cpp":entry("4824CC"," nfs_controls_steering_sidebar_text::SelectionScope nfs_controls_steering_sidebar_selection(*this);")
    elif name=="Methods_10.cpp":
        entry("482DA0"," nfs_controls_steering_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_controls_steering_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
        for symbol in ("482BD4","483288"):entry(symbol," nfs_controls_steering_sidebar_live::row(*this,uint32_t(game_id_sub_"+symbol+"));")
        entry("483438"," nfs_controls_steering_sidebar_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_controls_steering_sidebar_live::CopyWrapperScope nfs_controls_steering_cache_copy(*this);")
    elif name=="Methods_13.cpp":
        entry("4A14D4"," nfs_controls_steering_sidebar_live::row(*this,uint32_t(game_id_sub_4A14D4),2);")
        entry("4A5830"," nfs_controls_steering_sidebar_text::GlyphScope nfs_controls_steering_sidebar_glyph(*this);")
        def dispatch(part):
            call="\tcall(to32i(esp+0x38));"
            if part.count(call)!=4:raise ValueError("Controls steering sidebar indexed dispatcher changed")
            pieces=part.split(call);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("indexed-open-"+str(k)," { nfs_controls_steering_sidebar_live::IndexedSideCallScope nfs_controls_steering_cache_indexed(*this,nfs_controls_steering_sidebar_live::indexedSideCallee(*this));")+call+block("indexed-close-"+str(k)," }")+piece
            return out
        body("4A10BC",dispatch)
    else:
        entry("4AC934"," nfs_controls_steering_sidebar_text::beforeOwnerRetire(*this,uint32_t(eax),true);\n nfs_controls_steering_sidebar_live::beforeOwnerRetire(*this,uint32_t(eax),true);")
        entry("4AC9C8"," nfs_controls_steering_sidebar_text::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_controls_steering_sidebar_live::beforeDescriptorRetire(*this,uint32_t(eax));")
    if name in ("Methods_13.cpp","Methods_14.cpp"):
        symbol,count=("4A5830",4)if name=="Methods_13.cpp"else("4B3EE0",1)
        def words(part):
            store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(store)!=count:raise ValueError("Controls steering sidebar original word stores changed: "+symbol)
            pieces=part.split(store);out=pieces[0]
            for k,piece in enumerate(pieces[1:]):out+=block("word-open-"+str(k)," { nfs_controls_steering_sidebar_text::WordCapture nfs_controls_steering_sidebar_word(*this,uint32_t(esi),uint16_t(bx));")+store+block("word-close-"+str(k)," }")+piece
            return out
        body(symbol,words)
    return text


def _codes():
    live='nfs_controls_steering_sidebar_live';caption='nfs_controls_steering_sidebar_text'
    result={
        'include':'#include "hd/ControlsSteeringSidebarCaptionGameHooks.inc"\n#include "hd/ControlsSteeringSidebarLiveGameHooks.inc"',
        'caption-owner-4101c0':' '+caption+'::OwnerScope nfs_controls_steering_sidebar_owner(*this);',
        'cache-owner-4101c0':' '+live+'::OwnerScope nfs_controls_steering_cache_owner(*this);',
        'entry-40C110':' '+live+'::TailScope nfs_controls_steering_cache_tail(*this);\n '+caption+'::TailScope nfs_controls_steering_sidebar_tail(*this);',
        'entry-405CF0':' '+caption+'::StringScope nfs_controls_steering_sidebar_string(*this);\n '+live+'::bindCaption(*this);',
        'entry-40BBE0':' '+live+'::IterationScope nfs_controls_steering_cache_iteration(*this);',
        'entry-405B60':' '+live+'::ResolvedScope nfs_controls_steering_cache_resolved(*this);',
        'lazy-after-original-gate':' nfs_controls_steering_cache_iteration.start();',
        'side':' '+live+'::RoleScope nfs_controls_steering_cache_side(*this,'+live+'::Phase::side);',
        'word':' '+live+'::RoleScope nfs_controls_steering_cache_word(*this,'+live+'::Phase::word);',
        'group':' '+live+'::preGroup(*this);','group-complete':' '+live+'::postGroup(*this);',
        'entry-4824CC':' '+caption+'::SelectionScope nfs_controls_steering_sidebar_selection(*this);',
        'entry-482DA0':' '+caption+'::beforeDescriptorRetire(*this,uint32_t(eax));\n '+live+'::beforeDescriptorRetire(*this,uint32_t(eax));',
        'entry-483438':' '+live+'::row(*this,uint32_t(game_id_sub_483438));\n '+live+'::CopyWrapperScope nfs_controls_steering_cache_copy(*this);',
        'entry-4A14D4':' '+live+'::row(*this,uint32_t(game_id_sub_4A14D4),2);',
        'entry-4A5830':' '+caption+'::GlyphScope nfs_controls_steering_sidebar_glyph(*this);',
        'entry-4AC934':' '+caption+'::beforeOwnerRetire(*this,uint32_t(eax),true);\n '+live+'::beforeOwnerRetire(*this,uint32_t(eax),true);',
        'entry-4AC9C8':' '+caption+'::beforeDescriptorRetire(*this,uint32_t(eax));\n '+live+'::beforeDescriptorRetire(*this,uint32_t(eax));',
    }
    for symbol in ('482BD4','483288'):
        result['entry-'+symbol]=' '+live+'::row(*this,uint32_t(game_id_sub_'+symbol+'));'
    for kind in ('right','center'):
        result['measured-'+kind]=' '+caption+'::measured(*this,uint32_t(ecx),int32_t(eax),'+caption+'::SourceMeasureAlignment::'+kind+');'
    for i in range(4):
        result['indexed-open-'+str(i)]=' { '+live+'::IndexedSideCallScope nfs_controls_steering_cache_indexed(*this,'+live+'::indexedSideCallee(*this));'
        result['indexed-close-'+str(i)]=' }'
        result['word-open-'+str(i)]=' { '+caption+'::WordCapture nfs_controls_steering_sidebar_word(*this,uint32_t(esi),uint16_t(bx));'
        result['word-close-'+str(i)]=' }'
    return result
CODES=_codes()
SETS=(
 {'include','caption-owner-4101c0','cache-owner-4101c0','entry-40C110','entry-405CF0','entry-40BBE0','entry-405B60','lazy-after-original-gate','side','word','group','group-complete','measured-right','measured-center'},
 {'include','entry-4824CC'},
 {'include','entry-482DA0','entry-482BD4','entry-483288','entry-483438'},
 {'include','entry-4A14D4','entry-4A5830'}|{'indexed-'+part+'-'+str(i)for part in ('open','close')for i in range(4)}|{'word-'+part+'-'+str(i)for part in ('open','close')for i in range(4)},
 {'include','entry-4AC934','entry-4AC9C8','word-open-0','word-close-0'},
)
def remove_controls_steering_sidebar_hooks(text):
    crlf='\r\n' in text
    if '\r' in text.replace('\r\n','') or (crlf and '\n' in text.replace('\r\n','')):
        raise ValueError('Mixed steering sidebar source newlines')
    source=text.replace('\r\n','\n') if crlf else text
    pattern=r'\n// '+MARKER+r'_BEGIN:([^\n]+)\n(.*?)\n// '+MARKER+r'_END:\1\n'
    matches=list(re.finditer(pattern,source,re.S))
    if not matches:
        if MARKER in source:raise ValueError('Malformed steering sidebar markers')
        return text
    tags=[m.group(1)for m in matches]
    # Function-level removals are supported for focused source comparisons;
    # whole-unit additions independently require their exact complete tag set.
    if len(tags)!=len(set(tags)) or any(tag not in CODES for tag in tags):
        raise ValueError('Repeated or unknown steering sidebar blocks')
    if 'include' in tags and set(tags) not in SETS:raise ValueError('Partial steering sidebar unit')
    for match in matches:
        if match.group(2)!=CODES[match.group(1)]:raise ValueError('Changed steering sidebar observation')
    for prefix,original in (('word','guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'),('indexed','\tcall(to32i(esp+0x38));')):
        for i in range(4):
            opened=next((m for m in matches if m.group(1)==prefix+'-open-'+str(i)),None)
            closed=next((m for m in matches if m.group(1)==prefix+'-close-'+str(i)),None)
            if bool(opened)!=bool(closed):raise ValueError('Partial steering sidebar store/call bracket')
            if opened and (opened.end()>closed.start() or source[opened.end():closed.start()]!=original):
                raise ValueError('Changed steering sidebar protected original store/call')
    result=re.sub(pattern,'',source,flags=re.S)
    if MARKER in result:raise ValueError('Unmatched steering sidebar marker')
    return result.replace('\n','\r\n') if crlf else result


def add_controls_steering_sidebar_hooks(text,name):
    if name not in NAMES:return text
    original=text;crlf='\r\n'in text
    # Preserve exact LF/CRLF bytes; mixed sources must be normalized by their
    # owner explicitly rather than silently rewriting pre-existing hooks here.
    if '\r' in text.replace('\r\n','') or (crlf and '\n'in text.replace('\r\n','')):raise ValueError('Mixed original line endings')
    prepared=_add_controls_steering_sidebar_hooks_lf(text.replace('\r\n','\n')if crlf else text,name)
    if crlf:prepared=prepared.replace('\n','\r\n')
    for match in re.finditer(r'// '+MARKER+r'_BEGIN:',prepared):
        prefix=prepared[:match.start()]
        if prefix.count('// checked copy transaction begin')!=prefix.count('// checked copy transaction end'):
            raise ValueError('Controls steering sidebar hook inside checked-copy lease')
    tags=re.findall(r'// '+MARKER+r'_BEGIN:([^\r\n]+)',prepared)
    if len(tags)!=len(set(tags)) or set(tags)!=SETS[NAMES.index(name)]:raise ValueError('Incomplete steering sidebar observations')
    if remove_controls_steering_sidebar_hooks(prepared)!=original:raise ValueError('Controls steering sidebar byte reversal failed')
    return prepared
