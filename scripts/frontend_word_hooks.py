"""Scoped main-menu hit areas for the optional authored condensed lettering.

Run after guest-memory and copy transformations. This inserts checked stores,
preserves the original trace/body, and never embeds original game source.
"""


from cached_menu_widget_hooks import add_cached_menu_widget_hooks

def add_frontend_word_hooks(text, name):
    if name == "Methods_02.cpp":
        # Read/warp scopes preserve every original operation. All checked guest
        # copies end before entering the shared host-metadata mutex.
        sample = """[this]() {
        std::array<int32_t,8> state;
        { nfs::GuestCopyTransaction copy;
          state = {copy.load<int32_t>(mousePositionX),copy.load<int32_t>(mousePositionY),
                   copy.load<int32_t>(dword_4E8938),copy.load<int32_t>(dword_4E893C),
                   copy.load<int32_t>(dword_4E8940),copy.load<int32_t>(dword_4E8944),
                   copy.load<int32_t>(dword_4D4C7C),copy.load<int32_t>(dword_4D4C80)}; }
        return state;
    }"""
        for function,kind in (("422060","Read"),("4222F0","Warp")):
            anchor = "Fn(void) Game::_sub_" + function + "()\n{\n"
            if text.count(anchor)!=1:raise ValueError("Original pointer source entry changed: "+function)
            args = ",uint32_t(edx),uint32_t(ebx),guest_address(dword_4D4C7C),guest_address(dword_4D4C80),uint32_t(eax)" if kind=="Read" else ""
            code = ("    /* NFS_MENU_POINTER_"+kind.upper()+"_BEGIN */\n"
                    "    auto nfs_pointer_source = "+sample+";\n"
                    "    nfs::MenuPointer"+kind+"Scope<decltype(nfs_pointer_source)> nfs_pointer_scope(nfs_pointer_source"+args+");\n"
                    "    /* NFS_MENU_POINTER_"+kind.upper()+"_END */\n")
            text=text.replace(anchor,anchor+code)
        return '#include "MenuResponsivePointerHooks.h"\n#include "GuestCopyTransaction.h"\n'+text
    if name == "Methods_01.cpp":
        entry = ('Fn(void) Game::_sub_405CF0()\n{\n'
                 '\tnfs_trace_guest_call(game_id_sub_405CF0,uint32_t(esp));')
        if text.count(entry) != 1 or 'nfs_front_text_scope' in text:
            raise ValueError("Original frontend text entry no longer matches")
        scope = '''
    nfs::FrontendTextScope nfs_front_text_scope(nfs::frontend_text_identity(
        uint32_t(eax), [](uint32_t address) { return guest_load<uint8_t>(address); }));'''
        text=text.replace(entry, entry+scope)
        form_entry='Fn(void) Game::_sub_40DC40()\n{\n'
        if text.count(form_entry)!=1:raise ValueError("Responsive Main source form entry changed")
        form_scope='    // One checked guest copy, released BEFORE the atomic host observation.\n    try {\n        const uint32_t nfs_descriptor_form=uint32_t(eax);\n        const bool nfs_main_form=nfs_descriptor_form==uint32_t(guest_address(off_4D3EC4));\n        const uint32_t nfs_cached_form=nfs_descriptor_form==uint32_t(guest_address(off_4D2790))?0x4D2790u:nfs_descriptor_form==uint32_t(guest_address(off_4D3A88))?0x4D3A88u:0u;\n        uint32_t nfs_mouse_x=0,nfs_mouse_y=0,nfs_opponent_texture=0,nfs_opponent_observed=0;\n        struct NfsMainTextureDescriptor { uint32_t words[8]; };\n        { nfs::GuestCopyTransaction nfs_main_observation;\n          nfs_mouse_x=nfs_main_observation.load<uint32_t>(dword_4D4C7C);\n          nfs_mouse_y=nfs_main_observation.load<uint32_t>(dword_4D4C80);\n          if(nfs_main_form||nfs_cached_form){\n            const uint32_t nfs_descriptor=nfs_main_observation.load<uint32_t>(dword_4EB5A4+5*4);\n            if(nfs_descriptor&&!(nfs_descriptor&3u)&&nfs_descriptor<=UINT32_MAX-31u){\n                const auto nfs_texture=nfs_main_observation.load<NfsMainTextureDescriptor>(nfs_descriptor);\n                nfs_opponent_texture=nfs_texture.words[5];\n                nfs_opponent_observed=!(nfs_opponent_texture&3u)&&nfs_opponent_texture<=0x200000u-32768u;\n            }\n          } }\n        if(nfs_cached_form)nfs_menu_cached_source_observation(nfs_cached_form,nfs_descriptor_form,nfs_mouse_x,nfs_mouse_y,nfs_opponent_texture,nfs_opponent_observed);\n        else nfs_menu_responsive_source_observation(nfs_main_form?0x401910u:0u,nfs_mouse_x,nfs_mouse_y,nfs_opponent_texture,nfs_opponent_observed);\n    } catch (...) { nfs_menu_responsive_source_observation(0,0,0,0,0); throw; }\n'
        text=text.replace(form_entry,form_entry+form_scope)
        return '#include "MenuResponsiveCached.h"\n#include "MenuResponsiveLayout.h"\n#include "MenuResponsiveDebug.h"\n#include "FrontendTextContext.h"\n'+text
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
    return add_cached_menu_widget_hooks('#include "FrontendHitBounds.h"\n#include "FrontendWordMode.h"\n' + text,name)
