"""Exact original-preserving Player scene hooks; apply after Setup hooks."""
import re
MARKER = "NFS_PLAYER_SCENE"
NAMES = ("Methods_01.cpp", "Methods_03.cpp", "Methods_09.cpp", "Methods_10.cpp", "Methods_13.cpp", "Methods_14.cpp")
_BLOCK = re.compile(r"\n// NFS_PLAYER_SCENE_BEGIN:([a-z0-9-]+)\n.*?\n// NFS_PLAYER_SCENE_END:\1\n", re.S)

def reverse_player_scene_hooks(text):
    """Remove only this transform's complete blocks; reject malformed markers."""
    tags = _BLOCK.findall(text)
    if len(tags) != len(set(tags)):
        raise ValueError("Duplicate Player hook tag")
    result = _BLOCK.sub("", text)
    if MARKER in result:
        raise ValueError("Malformed Player hook block")
    return result

def add_player_scene_hooks(text, name):
    if name not in NAMES:
        return text
    original = text
    if MARKER in text:
        raise ValueError("Player hooks already present")
    tags = set()
    def block(tag, code):
        if tag in tags:
            raise ValueError("Duplicate Player hook tag: " + tag)
        tags.add(tag)
        return "\n// " + MARKER + "_BEGIN:" + tag + "\n" + code + "\n// " + MARKER + "_END:" + tag + "\n"
    def replace(part, old, new, count=1):
        if part.count(old) != count:
            raise ValueError("Player anchor/count changed: " + name + " " + old[:72])
        return part.replace(old, new)
    def unique(old, new):
        nonlocal text
        text = replace(text, old, new)
    def entry(symbol, code):
        anchor = "Fn(void) Game::_sub_" + symbol + "()\n{\n"
        unique(anchor, anchor + block("entry-" + symbol.lower(), code))
    def body(symbol, mutate):
        nonlocal text
        pattern = r"Fn\(void\) Game::_sub_" + symbol + r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)"
        matches = list(re.finditer(pattern, text, re.S))
        if len(matches) != 1:
            raise ValueError("Player function changed: " + symbol)
        match = matches[0]
        text = text[:match.start()] + mutate(match.group()) + text[match.end():]
    def call(symbol):
        return "\tesp -= 4; _sub_" + symbol + "(); esp += 4; //call"
    def counts(part, expected):
        for symbol, count in expected.items():
            if part.count(call(symbol)) != count:
                raise ValueError("Player original call count changed: " + symbol)
    def wrap(part, anchor, symbol, tag, code, require=()):
        if part.count(anchor) != 1:
            raise ValueError("Player lexical anchor changed: " + tag)
        start = part.index(anchor) + len(anchor)
        end = part.find("\nloc_", start)
        if end < 0:
            end = len(part)
        region = part[start:end]
        target = call(symbol)
        if region.count(target) != 1 or any(token not in region[:region.index(target)] for token in require):
            raise ValueError("Player lexical call/arguments changed: " + tag)
        at = start + region.index(target)
        return part[:at] + block(tag + "-open", " { " + code) + target + block(tag + "-close", " }") + part[at+len(target):]
    def wrap_all(part, symbol, tag, scope, count):
        target = call(symbol)
        if part.count(target) != count:
            raise ValueError("Player scoped call count changed: " + tag)
        pieces = part.split(target)
        out = pieces[0]
        for k, piece in enumerate(pieces[1:]):
            out += block(tag + "-" + str(k) + "-open", " { " + scope) + target + block(tag + "-" + str(k) + "-close", " }") + piece
        return out
    def captions(kind, args="*this"):
        return "\n".join(" nfs_player" + str(p) + "_caption::" + kind + "(" + args + ");" for p in (1, 2))
    include = '#include "FunctionIds.h"\n'
    unique(include, include + block("include", '#include "hd/PlayerSceneQueueAdapter.inc"\n#include "hd/PlayerSwatchGameHooks.inc"\n#include "hd/PlayerInitialNativeUploadGameHooks.inc"'))
    if name == "Methods_01.cpp":
        entry("40C160", " nfs_logo_publisher::PlayerMenuOwnerScope nfs_player_menu(*this);\n nfs_player1_caption::OwnerScope nfs_player1_owner(*this);\n nfs_player2_caption::OwnerScope nfs_player2_owner(*this);\n nfs_player_scene_live::PlayerScope nfs_player_scene(*this);\n nfs_player_scene_live::SourceLifetimeScope nfs_player_source_lifetime(*this);\n nfs_player_initial_native_live::SourceLifetimeScope nfs_player_initial_source_lifetime(*this);")
        entry("40C110", " nfs_player_scene_live::TailScope nfs_player_tail(*this);\n nfs_player1_caption::TailScope nfs_player1_tail(*this);\n nfs_player2_caption::TailScope nfs_player2_tail(*this);")
        entry("405CF0", " nfs_player1_caption::StringScope nfs_player1_string(*this);\n nfs_player2_caption::StringScope nfs_player2_string(*this);")
        entry("405580", " nfs_player1_caption::ResourceScope nfs_player1_resource(*this);\n nfs_player2_caption::ResourceScope nfs_player2_resource(*this);")
        entry("40BBE0", " nfs_player_scene_live::IterationScope nfs_player_iteration(*this);")
        entry("405B30", " nfs_player_preview_live::NameScope nfs_player_preview_name(*this);")
        entry("405B60", " nfs_player_scene_live::ResolvedScope nfs_player_resolved(*this);\n nfs_player_preview_live::ResolvedScope nfs_player_preview_resolved(*this);")
        anchor = " nfs_hd_scene_iteration.start();"
        unique(anchor, anchor + block("lazy-start", " nfs_player_iteration.start();"))
        for fn in ("beginGroup", "postGroup", "copyEnd"):
            anchor = " nfs_background_scene_live::" + fn + "(*this);"
            unique(anchor, anchor + block("scene-" + fn.lower(), " nfs_player_scene_live::" + fn + "(*this);"))
        def lazy(part):
            counts(part, {"405B30": 5, "405B60": 1, "428250": 2})
            part = wrap(part, "guest_address(aSide)", "405B30", "side", "nfs_player_scene_live::RoleScope nfs_player_side(*this,nfs_player_scene_live::Phase::Side);")
            part = wrap(part, "off_4D2474+ecx*4", "405B30", "word", "nfs_player_scene_live::RoleScope nfs_player_word(*this,nfs_player_scene_live::Phase::Word);")
            part = wrap(part, "loc_40C054:\n", "405B60", "copy-lazy", "nfs_player_copy_live::Scope nfs_player_copy(*this,nfs_player_copy_live::Site::Lazy);", ("_sub_4825A8();", "0x2800"))
            return wrap(part, "loc_40C0B6:\n", "428250", "upload-lazy", "nfs_player_scene_live::CachedUploadSiteScope nfs_player_upload_site(*this,nfs_player_scene_live::CachedUploadSite::Lazy);", ("dword_4D4D24", "edx = 1;"))
        body("40BBE0", lazy)
        def measurements(part):
            part = replace(part, call("41B710"), call("41B710") + block("resource-resolved", captions("resolvedResource", "*this,uint32_t(eax)")))
            for alignment in ("right", "center"):
                anchor = " nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::" + alignment + ");"
                code = "\n".join(" nfs_player" + str(p) + "_caption::measured(*this,uint32_t(ecx),int32_t(eax),nfs_player" + str(p) + "_caption::SourceMeasureAlignment::" + alignment + ");" for p in (1, 2))
                part = replace(part, anchor, anchor + block("measure-" + alignment, code))
            return part
        body("405580", measurements)
        def menu(part):
            counts(part, {"405B30": 3, "405B60": 3, "40CD70": 2, "40CAB0": 1, "40C110": 1, "428250": 10, "427B30": 5})
            # Exact pre-map caller: selected343 has just been selected, ESI is
            # current Player and EBP is the previous descriptor. Acquisition is
            # sealed Main history; neither policy can grant itself an origin.
            initial = "\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n\teax = guest_transaction.load<int32_t>(dword_4D2314+2); //mov\n\tsar(eax, (int32_t)0x10);\n\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov\n\t} // checked copy transaction end\n" + call("4825A8") + "\n" + call("40C110") + "\n" + call("4824BC")
            if part.count(initial) != 1 or part.index(initial) > part.index("loc_40C25E:\n") or part[:part.index(initial)].count("\tebp = guest_transaction.load<int32_t>(dword_4D243C); //mov\n") != 1:
                raise ValueError("Player initial caption caller changed")
            initial_open = block("initial-caption-open", " { auto nfs_player_main_origin = nfs_main_sidebar_origin_live::acquireForInitialCaption(*this);\n nfs_player1_caption::InitialCallerScope nfs_player1_initial(*this,nfs_player_main_origin);\n nfs_player2_caption::InitialCallerScope nfs_player2_initial(*this,nfs_player_main_origin);")
            part = replace(part, initial, initial.replace(call("40C110"), initial_open + call("40C110") + block("initial-caption-close", " }") + block("initial-caption-seal", " nfs_player_scene_live::completeInitialCaption(*this);"), 1))
            for label, site in (("loc_40C3CC:\n", "Initial"), ("loc_40C5C6:\n", "Refresh")):
                tag = site.lower()
                part = wrap(part, label, "405B30", "base-" + tag, "nfs_player_scene_live::RoleScope nfs_player_base(*this,nfs_player_scene_live::Phase::Base,nfs_player_scene_live::BaseArgument::Blk4Direct);", ("guest_address(aBlk4)", "dword_4D4D24", "_sub_4825A8();"))
                part = wrap(part, label, "40CD70", "preview-" + tag, "nfs_player_preview_live::Scope nfs_player_preview(*this,nfs_player_preview_live::Site::" + site + ");", ("eax = esi;", "edx = 0xD3;", "ebx = 0x13C;"))
                part = wrap(part, label, "405B60", "copy-" + tag, "nfs_player_copy_live::Scope nfs_player_copy(*this,nfs_player_copy_live::Site::" + site + ");", ("0x2800",))
                part = wrap(part, label, "428250", "upload-" + tag, "nfs_player_scene_live::CachedUploadSiteScope nfs_player_upload_site(*this,nfs_player_scene_live::CachedUploadSite::" + site + ");", ("dword_4D4D24", "edx = 1;"))
            part = wrap(part, "loc_40C792:\n", "405B30", "base-animation", "nfs_player_scene_live::RoleScope nfs_player_base(*this,nfs_player_scene_live::Phase::Base);", ("eax = to32i(esi+4)",))
            anchor = "\tpush32(0xFFFFFFFF);\n\tecx = 0x20; //mov\n\teax = 1; //mov\n\txor_(edx, edx);\n"
            return wrap(part, anchor, "427B30", "presentation", "nfs_player_scene_live::PresentationScope nfs_player_presentation(*this);")
        body("40C160", menu)
        def preview(part):
            counts(part, {"405B30": 10, "48266C": 3, "488620": 3})
            part = wrap(part, "loc_40CDEB:\n", "48266C", "preview-color", "nfs_player_preview_live::ColorScope nfs_player_preview_color(*this);")
            return wrap(part, "loc_40CDEB:\n", "488620", "preview-fill", "nfs_player_preview_live::FillScope nfs_player_preview_fill(*this);")
        body("40CD70", preview)
        def swatch_call(part):
            counts(part, {"410090": 2})
            for label, row in (("loc_40CAC8:\n", "dword_4D2F80"), ("loc_40CAE8:\n", "dword_4D2B68")):
                part = wrap(part, label, "410090", "swatch-" + row.lower().replace("_", "-"), "nfs_player_swatch_live::Scope nfs_player_swatch(*this,nfs_player_swatch_live::Site::Refresh);", (row, "edx = 0x16D;", "ebx = 0x88;"))
            return part
        body("40CAB0", swatch_call)
        def swatch(part):
            for symbol, scope, count in (("458670", "Hsv", 1), ("457FA8", "Rgb", 1), ("4584E0", "Shade", 1), ("48266C", "Color", 2), ("488620", "Fill", 2)):
                part = wrap_all(part, symbol, "swatch-" + scope.lower(), "nfs_player_swatch_live::" + scope + "Scope nfs_player_swatch_" + scope.lower() + "(*this);", count)
            return part
        body("410090", swatch)
    elif name == "Methods_03.cpp":
        anchor = " nfs_logo_publisher::UploadScope nfs_hd_upload(*this);"
        unique(anchor, block("upload-end", " nfs_player_scene_live::UploadEndScope nfs_player_upload_end(*this);") + anchor + block("native-upload", " nfs_player_native_live::UploadScope nfs_player_native_upload(*this);") + block("initial-native-upload", " nfs_player_initial_native_live::UploadScope nfs_player_initial_native_upload(*this);"))
        def tiles(part):
            counts(part, {"483470": 1, "49755C": 1, "482DA0": 1})
            # Before the label: initial fallthrough only. Original outer-loop
            # jumps target the following label and cannot repeat stageBegin.
            label = "loc_4282FB:\n"
            part = replace(part, label, block("initial-stage-begin", " nfs_player_initial_native_live::stageBegin(*this);") + label)
            part = replace(part, call("482DA0"), block("initial-stage-finish", " nfs_player_initial_native_live::stageFinish(*this);") + call("482DA0"))
            return wrap_all(part, "49755C", "native-tile", "nfs_player_native_live::UploadTileScope nfs_player_native_tile(*this);\n nfs_player_initial_native_live::UploadTileScope nfs_player_initial_native_tile(*this);", 1)
        body("428250", tiles)
        for symbol in ("4284B0", "4287F0", "4289F0"):
            entry(symbol, " nfs_player_scene_live::invalidateCache();\n nfs_player_scene_live::resetSources();")
    elif name == "Methods_09.cpp":
        entry("4824CC", " nfs_player1_caption::SelectionScope nfs_player1_selection(*this);\n nfs_player2_caption::SelectionScope nfs_player2_selection(*this);")
    elif name == "Methods_10.cpp":
        entry("4848FC", " nfs_main_sidebar_origin_live::beforeAllocationFree(*this,uint32_t(eax));\n nfs_player_initial_native_live::retireAllocation(uint32_t(eax));")
        entry("482DA0", " nfs_main_sidebar_origin_live::beforeDescriptorRetire(*this,uint32_t(eax));\n nfs_player_scene_live::retireSource(*this);\n nfs_player_scene_live::retire(*this);\n" + captions("beforeDescriptorRetire", "*this,uint32_t(eax)"))
        entry("482BD4", " nfs_player_scene_live::row(*this,uint32_t(game_id_sub_482BD4));")
        entry("483070", " nfs_player_copy_live::row(*this,uint32_t(game_id_sub_483070));")
        entry("483438", " nfs_player_scene_live::row(*this,uint32_t(game_id_sub_483438));\n nfs_player_scene_live::CopyWrapperScope nfs_player_copy_wrapper(*this);")
    elif name == "Methods_13.cpp":
        entry("4A5830", " nfs_player1_caption::GlyphScope nfs_player1_glyph(*this);\n nfs_player2_caption::GlyphScope nfs_player2_glyph(*this);")
        entry("4A14D4", " nfs_player_scene_live::byteCopyRow(*this);")
        def dispatch(part):
            anchor = "\tcall(to32i(esp+0x38));"
            if part.count(anchor) != 4:
                raise ValueError("Player original dispatch count changed")
            pieces = part.split(anchor)
            result = pieces[0]
            for k, piece in enumerate(pieces[1:]):
                result += block("row-" + str(k) + "-open", " { nfs_player_scene_live::IndexedSideCallScope nfs_player_indexed(*this,nfs_player_scene_live::indexedSideCallee(*this));\n nfs_player_preview_live::RowScope nfs_player_preview_row(*this);\n nfs_player_swatch_live::RowScope nfs_player_swatch_row(*this);") + anchor + block("row-" + str(k) + "-close", " }") + piece
            return result
        body("4A10BC", dispatch)
    elif name == "Methods_14.cpp":
        entry("4AC934", " nfs_main_sidebar_origin_live::beforeOwnerRetire(*this,uint32_t(eax),true);\n" + captions("beforeOwnerRetire", "*this,uint32_t(eax),true"))
        entry("4AC9C8", " nfs_main_sidebar_origin_live::beforeDescriptorRetire(*this,uint32_t(eax));\n" + captions("beforeDescriptorRetire", "*this,uint32_t(eax)"))
    if name in ("Methods_13.cpp", "Methods_14.cpp"):
        symbol, count = ("4A5830", 4) if name == "Methods_13.cpp" else ("4B3EE0", 1)
        def words(part):
            anchor = "guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
            if part.count(anchor) != count:
                raise ValueError("Player original glyph store count changed")
            pieces = part.split(anchor)
            out = pieces[0]
            for k, piece in enumerate(pieces[1:]):
                out += block("word-" + str(k) + "-open", " { nfs_player1_caption::WordCapture nfs_player1_word(*this,uint32_t(esi),uint16_t(bx));\n nfs_player2_caption::WordCapture nfs_player2_word(*this,uint32_t(esi),uint16_t(bx));") + anchor + block("word-" + str(k) + "-close", " }") + piece
            return out
        body(symbol, words)
    if reverse_player_scene_hooks(text) != original:
        raise ValueError("Player insertion-only reversal failed")
    return text
