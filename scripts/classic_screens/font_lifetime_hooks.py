"""Preserve the persistent original font context across temporary atlas loading."""

def add_font_lifetime_hooks(text, unit):
    if unit != 'Methods_03.cpp': return text
    if 'atlasFontLifetime' in text:
        raise ValueError('Font lifetime hooks already present')
    include = '#include "hd/ClassicScreens/AtlasFontLifetime.inc"\n'
    dependency = '#include \"FunctionIds.h\"\n'
    if text.count(dependency) != 1:
        raise ValueError('Missing font lifetime source dependencies')
    at = text.index(dependency)+len(dependency)
    text = text[:at]+include+text[at:]
    entry = 'Fn(void) Game::_sub_426F10()\n{\n'
    if text.count(entry) != 1: raise ValueError('Font loader entry changed')
    start = text.index(entry)+len(entry)
    end = text.index('\nFn(', start)
    tail = text[start:end]
    anchor = '\treturn;\nloc_4270F7:'
    if tail.count(anchor) != 1: raise ValueError('Font loader return changed')
    tail = tail.replace(anchor, ' atlasFontLifetime.finish();\n'+anchor)
    return text[:start]+' nfs_atlas_lifetime::Scope atlasFontLifetime(*this);\n'+tail+text[end:]
