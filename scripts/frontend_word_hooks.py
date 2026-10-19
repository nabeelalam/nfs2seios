"""Scoped main-menu hit areas for the optional authored condensed lettering.

Run after guest-memory and copy transformations. This inserts checked stores,
preserves the original trace/body, and never embeds original game source.
"""


def add_frontend_word_hooks(text, name):
    if name != "Methods_03.cpp":
        return text
    entry = ('Fn(void) Game::_sub_423CA0()\n{\n'
             '\tnfs_trace_guest_call(game_id_sub_423CA0,uint32_t(esp));')
    if text.count(entry) != 1 or 'NfsFrontHitScope' in text:
        raise ValueError("Original main-menu hit entry no longer matches")
    scope = '''
    const int nfs_front_active = nfs_front_word_hit_active();
    // Do not inspect guest menu tables when the renderer has no matching art.
    const unsigned nfs_count = nfs_front_active
        ? static_cast<unsigned short>(to16i(word_4E8C06)) : 0;
    NfsFrontHitRect nfs_rects[8] = {};
    if (nfs_count == 8) {
        for (unsigned i = 0; i < 8; ++i) {
            nfs_rects[i] = {
                static_cast<int16_t>(to16i(dword_4E89B6 + 2 + 2*i)),
                static_cast<int16_t>(to16i(dword_4E8A56 + 2 + 2*i)),
                static_cast<int16_t>(to16i(dword_4E8AA6 + 2 + 2*i)),
                static_cast<int16_t>(to16i(dword_4E8A06 + 2 + 2*i)),
                static_cast<int16_t>(to16i(dword_4E8964 + 4 + 2*i))};
        }
    }
    auto nfs_set_left = [this](unsigned i, int16_t value) {
        guest_store<int16_t>(dword_4E8A56 + 2 + 2*i, value);
    };
    NfsFrontHitScope<decltype(nfs_set_left)> nfs_hit_scope(
        nfs_front_active, nfs_count == 8 ? static_cast<uint32_t>(to32i(eax)) : 0,
        nfs_count, nfs_rects, nfs_set_left);
'''
    prefix, trace = entry.rsplit('\n', 1)
    text = text.replace(entry, prefix + '\n' + scope + '\n' + trace)
    return '#include "FrontendHitBounds.h"\n#include "FrontendWordMode.h"\n' + text
