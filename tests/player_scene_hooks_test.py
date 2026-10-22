"""Focused insertion/reversal guards; synthetic source, no game/assets required."""
import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "player_scene_hooks.py"
spec = importlib.util.spec_from_file_location("player_scene_hooks", SCRIPT)
hooks = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hooks)


def call(symbol):
    return "\tesp -= 4; _sub_" + symbol + "(); esp += 4; //call\n"


def fn(symbol, code=""):
    return "Fn(void) Game::_sub_" + symbol + "()\n{\n" + code + "}\n"


def units():
    initial_caption = "\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n\tebp = guest_transaction.load<int32_t>(dword_4D243C); //mov\n\t} // checked copy transaction end\n\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n\teax = guest_transaction.load<int32_t>(dword_4D2314+2); //mov\n\tsar(eax, (int32_t)0x10);\n\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov\n\t} // checked copy transaction end\n" + call("4825A8") + call("40C110") + call("4824BC") + "loc_40C25E:\n"
    initial = "loc_40C3CC:\n" + "dword_4D4D24\n" + call("4825A8") + "guest_address(aBlk4)\n" + call("405B30") + "eax = esi;\nedx = 0xD3;\nebx = 0x13C;\n" + call("40CD70") + "0x2800\n" + call("405B60") + "edx = 1;\n" + call("428250")
    refresh = initial.replace("loc_40C3CC:", "loc_40C5C6:").replace(call("428250"), call("40CAB0") + call("428250"))
    animation = "loc_40C792:\neax = to32i(esi+4);\n" + call("405B30") + "0x1000\n" + call("405B60")
    native = "loc_40C64C:\n\tpush32(0xFFFFFFFF);\n\tecx = 0x20; //mov\n\teax = 1; //mov\n\txor_(edx, edx);\nexisting_location_route;\n" + call("427B30")
    unused = "loc_unused:\n" + call("428250") * 8 + call("427B30") * 4
    lazy = " nfs_hd_scene_iteration.start();\n" + "loc_side:\nguest_address(aSide)\n" + call("405B30") + "loc_word:\noff_4D2474+ecx*4\n" + call("405B30") + call("405B30") * 3
    # Word is uniquely selected by the argument lookup, but its region must not
    # accidentally include following logo calls before a label. Real source
    # has loc40BF73 /40BFB5 /40C01F labels for those three calls.
    lazy = lazy.replace(call("405B30") * 4, call("405B30") + "loc_logos:\n" + call("405B30") * 3)
    lazy += " nfs_background_scene_live::beginGroup(*this);\n nfs_background_scene_live::postGroup(*this);\nloc_40C054:\n" + call("4825A8") + "0x2800\n nfs_logo_publisher::armCopy(*this);\n" + call("405B60") + " nfs_background_scene_live::copyEnd(*this);\nloc_40C0B6:\ndword_4D4D24\nedx = 1;\n" + call("428250") + "loc_slot0:\n" + call("428250")
    preview = "loc_40CDEB:\n" + call("48266C") + call("488620") + "loc_40CE28:\n" + call("405B30") * 5 + "loc_40CF46:\n" + call("48266C") + call("488620") + call("405B30") * 4 + "loc_40D026:\n" + call("48266C") + call("488620") + call("405B30")
    swatch = "".join(call(symbol) * count for symbol, count in (("458670", 1), ("457FA8", 1), ("4584E0", 1), ("48266C", 2), ("488620", 2)))
    swatch_call = "loc_40CAC8:\ndword_4D2F80\nedx = 0x16D;\nebx = 0x88;\n" + call("410090") + "loc_40CAE8:\ndword_4D2B68\nedx = 0x16D;\nebx = 0x88;\n" + call("410090")
    measure = "\n".join(" nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::" + kind + ");" for kind in ("right", "center")) + "\n"
    word = "guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n"
    prefix = '#include "FunctionIds.h"\n// Preserved preexisting transformation and UTF-8: Î©\n'
    return {
        "Methods_01.cpp": prefix + fn("40C160", initial_caption + initial + refresh + native + animation + unused) + fn("40C110") + fn("405CF0") + fn("40BBE0", lazy) + fn("405B30") + fn("405B60") + fn("405580", call("41B710") + "\tecx = eax; //mov\n" + measure) + fn("40CD70", preview) + fn("40CAB0", swatch_call) + fn("410090", swatch),
        "Methods_03.cpp": prefix + fn("428250", " nfs_background_scene_live::UploadEndScope nfs_hd_upload_end(*this);\n nfs_logo_publisher::UploadScope nfs_hd_upload(*this);\n" + call("483470") + "loc_4282FB:\n { nfs_logo_publisher::UploadTileScope nfs_hd_tile(*this);\n" + call("49755C") + " }\n" + call("482DA0")) + fn("4284B0") + fn("4287F0") + fn("4289F0"),
        "Methods_09.cpp": prefix + fn("4824CC"),
        "Methods_10.cpp": prefix + fn("482DA0") + fn("4848FC") + fn("482BD4") + fn("483070") + fn("483438"),
        "Methods_13.cpp": prefix + fn("4A5830", word * 4) + fn("4A14D4") + fn("4A10BC", "\tcall(to32i(esp+0x38));\n" * 4),
        "Methods_14.cpp": prefix + fn("4AC934") + fn("4AC9C8") + fn("4B3EE0", word),
    }


class PlayerHooksTests(unittest.TestCase):
    def test_every_unit_exact_reversal(self):
        for name, before in units().items():
            with self.subTest(name=name):
                after = hooks.add_player_scene_hooks(before, name)
                self.assertEqual(hooks.reverse_player_scene_hooks(after).encode(), before.encode())
                self.assertGreater(after.count(hooks.MARKER), 0)
                self.assertIn("Preserved preexisting transformation", after)
                with self.assertRaises(ValueError):
                    hooks.add_player_scene_hooks(after, name)

    def test_lexical_sites_and_scope_order(self):
        after = hooks.add_player_scene_hooks(units()["Methods_01.cpp"], "Methods_01.cpp")
        owner = [after.index(token) for token in ("PlayerMenuOwnerScope", "nfs_player1_caption::OwnerScope", "nfs_player2_caption::OwnerScope", "PlayerScope", "SourceLifetimeScope")]
        self.assertEqual(owner, sorted(owner))
        self.assertEqual(after.count("nfs_player_copy_live::Scope"), 3)
        self.assertEqual(after.count("CachedUploadSiteScope"), 3)
        self.assertEqual(after.count("nfs_player_preview_live::Scope"), 2)
        self.assertEqual(after.count("PresentationScope"), 1)
        self.assertNotIn("nfs_logo_publisher::NativePresentationScope", after)
        self.assertEqual(after.count("nfs_player_swatch_live::Scope"), 2)
        self.assertIn("NFS_PLAYER_SCENE_BEGIN:swatch-dword-4d2f80-open", after)
        self.assertIn("NFS_PLAYER_SCENE_BEGIN:swatch-dword-4d2b68-open", after)
        self.assertEqual(after.count("nfs_player_swatch_live::ColorScope"), 2)
        self.assertEqual(after.count("nfs_player_swatch_live::FillScope"), 2)
        animated = after[after.index("loc_40C792:"):after.index("loc_unused:")]
        self.assertNotIn("nfs_player_copy_live::Scope", animated)
        self.assertNotIn("CachedUploadSiteScope", animated)
        self.assertIn("existing_location_route;", after)

    def test_exact_resource_measure_occurrence(self):
        after = hooks.add_player_scene_hooks(units()["Methods_01.cpp"], "Methods_01.cpp")
        section = after[after.index("Fn(void) Game::_sub_405580()"):after.index("Fn(void) Game::_sub_40CD70()")]
        tokens = ("nfs_player1_caption::ResourceScope", "nfs_player2_caption::ResourceScope", "_sub_41B710();", "nfs_player1_caption::resolvedResource", "nfs_player2_caption::resolvedResource", "ecx = eax; //mov", "nfs_player1_caption::measured")
        at = [section.index(token) for token in tokens]
        self.assertEqual(at, sorted(at))
        self.assertEqual(section.count(call("41B710")), 1)
        self.assertEqual(section.count("ResourceScope"), 2)
        self.assertEqual(after.count("InitialCallerScope"), 2)

    def test_initial_caller_after_selection_before_restore(self):
        after = hooks.add_player_scene_hooks(units()["Methods_01.cpp"], "Methods_01.cpp")
        section = after[after.index("\tebp = guest_transaction.load<int32_t>(dword_4D243C)"):after.index("loc_40C25E:")]
        tokens = ("guest_transaction.load<int32_t>(dword_4D2314+2)", "guest_transaction.load<int32_t>(dword_4D231C+eax*4)", "_sub_4825A8();", "acquireForInitialCaption(*this)", "nfs_player1_caption::InitialCallerScope", "nfs_player2_caption::InitialCallerScope", "_sub_40C110();", "NFS_PLAYER_SCENE_END:initial-caption-close", "_sub_4824BC();")
        positions = [section.index(token) for token in tokens]
        self.assertEqual(positions, sorted(positions))
        selection_end = section.index("checked copy transaction end", section.index("guest_transaction.load<int32_t>(dword_4D231C+eax*4)"))
        self.assertLess(selection_end, section.index("_sub_4825A8();"))
        self.assertEqual(after.count("acquireForInitialCaption(*this)"), 1)
        tail = after[after.index("Fn(void) Game::_sub_40C110()"):after.index("Fn(void) Game::_sub_405CF0()")]
        self.assertNotIn("InitialCallerScope", tail)
        self.assertIn("nfs_player1_caption::TailScope", tail)
        self.assertIn("nfs_player2_caption::TailScope", tail)
        for name, token in (("Methods_10.cpp", "beforeAllocationFree"), ("Methods_10.cpp", "beforeDescriptorRetire"), ("Methods_14.cpp", "beforeOwnerRetire"), ("Methods_14.cpp", "beforeDescriptorRetire")):
            self.assertIn("nfs_main_sidebar_origin_live::" + token, hooks.add_player_scene_hooks(units()[name], name))

    def test_upload_construction_and_original_tile(self):
        before = units()["Methods_03.cpp"]
        after = hooks.add_player_scene_hooks(before, "Methods_03.cpp")
        tokens = ("nfs_player_scene_live::UploadEndScope", "nfs_logo_publisher::UploadScope", "nfs_player_native_live::UploadScope", "nfs_player_initial_native_live::UploadScope", "nfs_player_initial_native_live::stageBegin", "nfs_logo_publisher::UploadTileScope", "nfs_player_native_live::UploadTileScope", "nfs_player_initial_native_live::UploadTileScope", "_sub_49755C();", "nfs_player_initial_native_live::stageFinish", "_sub_482DA0();")
        at = [after.index(token) for token in tokens]
        self.assertEqual(at, sorted(at))
        self.assertEqual(after.count(call("49755C")), 1)

    def test_initial_upload_stage_lifetime(self):
        after = hooks.add_player_scene_hooks(units()["Methods_03.cpp"], "Methods_03.cpp")
        self.assertLess(after.index("_sub_483470();"), after.index("nfs_player_initial_native_live::stageBegin"))
        self.assertLess(after.index("nfs_player_initial_native_live::stageBegin"), after.index("loc_4282FB:"))
        self.assertLess(after.index("_sub_49755C();"), after.index("nfs_player_initial_native_live::stageFinish"))
        self.assertLess(after.index("nfs_player_initial_native_live::stageFinish"), after.index("_sub_482DA0();"))
        self.assertEqual(after.count("nfs_player_initial_native_live::stageBegin"), 1)
        self.assertEqual(after.count("nfs_player_initial_native_live::stageFinish"), 1)
        allocation = hooks.add_player_scene_hooks(units()["Methods_10.cpp"], "Methods_10.cpp")
        self.assertIn("nfs_player_initial_native_live::retireAllocation(uint32_t(eax))", allocation)
        owner = hooks.add_player_scene_hooks(units()["Methods_01.cpp"], "Methods_01.cpp")
        self.assertIn("nfs_player_initial_native_live::SourceLifetimeScope", owner)

    def test_all_real_row_dispatches_and_both_captions(self):
        after = hooks.add_player_scene_hooks(units()["Methods_13.cpp"], "Methods_13.cpp")
        for token in ("IndexedSideCallScope", "nfs_player_preview_live::RowScope", "nfs_player_swatch_live::RowScope", "nfs_player1_caption::WordCapture", "nfs_player2_caption::WordCapture"):
            self.assertEqual(after.count(token), 4)
        self.assertEqual(after.count("\tcall(to32i(esp+0x38));"), 4)

    def test_changed_missing_extra_anchors_rejected(self):
        cases = [
            ("Methods_01.cpp", "loc_40C25E:", "loc_changed:"),
            ("Methods_01.cpp", "\tebp = guest_transaction.load<int32_t>(dword_4D243C); //mov\n", "\tebp = 0;\n"),
            ("Methods_01.cpp", call("40C110"), call("40C110") * 2),
            ("Methods_01.cpp", "\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov\n", "\teax = 0;\n"),
            ("Methods_01.cpp", "\teax = guest_transaction.load<int32_t>(dword_4D2314+2); //mov\n", "\teax = to32i(dword_4D2314+2); //mov\n"),
            ("Methods_01.cpp", call("4824BC"), ""),
            ("Methods_01.cpp", "\t} // checked copy transaction end\n" + call("4825A8"), call("4825A8")),
            ("Methods_01.cpp", "loc_40C3CC:", "loc_wrong:"),
            ("Methods_01.cpp", "loc_40CDEB:", "loc_wrong:"),
            ("Methods_01.cpp", "ebx = 0x13C;", "ebx = 0;"),
            ("Methods_01.cpp", "edx = 0x16D;", "edx = 0;"),
            ("Methods_01.cpp", "dword_4D2B68", "dword_4D309C"),
            ("Methods_01.cpp", " nfs_hd_scene_iteration.start();", ""),
            ("Methods_01.cpp", call("40CAB0"), call("40CAB0") * 2),
            ("Methods_01.cpp", call("48266C"), ""),
            ("Methods_01.cpp", call("41B710"), ""),
            ("Methods_01.cpp", call("41B710"), call("41B710") * 2),
            ("Methods_03.cpp", call("49755C"), call("49755C") * 2),
            ("Methods_03.cpp", "loc_4282FB:", "loc_changed:"),
            ("Methods_03.cpp", call("483470"), ""),
            ("Methods_03.cpp", call("482DA0"), call("482DA0") * 2),
            ("Methods_03.cpp", "nfs_hd_upload(*this)", "changed(*this)"),
            ("Methods_13.cpp", "\tcall(to32i(esp+0x38));", ""),
            ("Methods_14.cpp", "guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov", ""),
        ]
        for name, old, new in cases:
            with self.subTest(name=name, old=old):
                before = units()[name]
                self.assertIn(old, before)
                with self.assertRaises(ValueError):
                    hooks.add_player_scene_hooks(before.replace(old, new, 1), name)
        before = units()["Methods_09.cpp"]
        with self.assertRaises(ValueError):
            hooks.add_player_scene_hooks(before + fn("4824CC"), "Methods_09.cpp")
        with self.assertRaises(ValueError):
            hooks.add_player_scene_hooks(before.replace('#include "FunctionIds.h"\n', ''), "Methods_09.cpp")

    def test_reversal_rejects_malformed_or_duplicate_tags(self):
        for text in ("// NFS_PLAYER_SCENE_BEGIN:x\n", "\n// NFS_PLAYER_SCENE_BEGIN:x\na\n// NFS_PLAYER_SCENE_END:y\n"):
            with self.assertRaises(ValueError):
                hooks.reverse_player_scene_hooks(text)
        block = "\n// NFS_PLAYER_SCENE_BEGIN:x\na\n// NFS_PLAYER_SCENE_END:x\n"
        with self.assertRaises(ValueError):
            hooks.reverse_player_scene_hooks(block + block)

    def test_public_helper_is_inert_on_other_units(self):
        text = "unrelated original text Î©\n"
        self.assertEqual(hooks.add_player_scene_hooks(text, "Methods_06.cpp"), text)
        self.assertEqual(hooks.reverse_player_scene_hooks(text), text)
        public = SCRIPT.read_text(encoding="utf-8")
        self.assertNotIn("nfs_player_original_observer", public)
        self.assertNotIn("prepared-v3", public)
        self.assertNotRegex(public, r"(?i)(?:(?<![a-z0-9_])[a-z]:[\\/]|/mnt/[a-z]/|/home/)")


    def test_initial_caption_seal_follows_closed_caller_before_target_restore(self):
        text = hooks.add_player_scene_hooks(units()["Methods_01.cpp"], "Methods_01.cpp")
        closed = text.index("// NFS_PLAYER_SCENE_END:initial-caption-close")
        sealed = text.index(" nfs_player_scene_live::completeInitialCaption(*this);")
        restore = text.index(call("4824BC"), closed)
        self.assertLess(closed, sealed)
        self.assertLess(sealed, restore)
        self.assertEqual(text.count("completeInitialCaption(*this)"), 1)
        self.assertEqual(hooks.reverse_player_scene_hooks(text), units()["Methods_01.cpp"])


if __name__ == "__main__":
    unittest.main()
