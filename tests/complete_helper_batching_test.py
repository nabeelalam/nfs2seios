"""Generator selection/shape gates; original-method semantics are separate."""
import hashlib
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import batch_complete_helpers as helpers
import prepare_full_mapped as full


def fixture(method):
    entry = f"Fn(void) Game::{method}()\n{{\n"
    if method == "___CHP":
        return (entry + "\tpush32(eax);\n" + helpers.BEGIN +
                "\tguest_transaction.load<int16_t>(esp);\n" + helpers.END +
                "\tpush32(to32i(esp));\n\tguest_store<int8_t>(esp+1, 31);\n"
                "\tfldcw(to16i(esp));\n\tfrndint();\n\tfldcw(to16i(esp+4));\n}")
    return (entry + "".join(f"\tpush32({r});\n" for r in ("ecx","esi","edi","ebp")) +
            helpers.BEGIN + "\teax = guest_transaction.load<int32_t>(esi);\n"
            "\timul32(edx);\n" + helpers.END +
            "".join(f"\tpop32({r});\n" for r in ("ebp","edi","esi","ecx")) + "}")


def pinned_fixture(unit):
    method, copies, _, _ = helpers.TARGETS[unit]
    before = fixture(method);after = helpers.rewrite_helper(before, method)
    digest = lambda text:hashlib.sha256(text.encode()).hexdigest()
    return before,after,(method,copies,digest(before),digest(after))


class CompleteHelperTests(unittest.TestCase):
    def test_exact_selection_and_surrounding_text_preserved(self):
        for unit in helpers.TARGETS:
            before,after,pin = pinned_fixture(unit)
            outside = "Fn(void) Game::_sub_000000()\n{\n\tcallback();\n}\n"
            with patch.dict(helpers.TARGETS,{unit:pin}):
                result,manifest = helpers.batch_complete_helpers(outside+before+"\n"+outside,unit)
            self.assertEqual(result,outside+after+"\n"+outside)
            self.assertEqual(manifest[0]['maximumCopiesPerScope'],pin[1])

    def test_unreviewed_whole_method_rejected(self):
        for unit in helpers.TARGETS:
            with self.assertRaises(ValueError):helpers.batch_complete_helpers(fixture(helpers.TARGETS[unit][0]),unit)

    def test_changed_input_and_reapplication_rejected(self):
        for unit in helpers.TARGETS:
            before,after,pin = pinned_fixture(unit)
            with patch.dict(helpers.TARGETS,{unit:pin}):
                for bad in (before.replace("push32","push16",1),before.replace("{\n","{\n\tcallback();\n",1),after):
                    with self.assertRaises(ValueError):helpers.batch_complete_helpers(bad,unit)

    def test_missing_duplicate_and_wrong_entry_rejected(self):
        unit="Methods_13.cpp";before,_,pin=pinned_fixture(unit)
        with patch.dict(helpers.TARGETS,{unit:pin}):
            for bad in ("",before+"\n"+before,before.replace(pin[0],"_sub_000000")):
                with self.assertRaises(ValueError):helpers.batch_complete_helpers(bad,unit)

    def test_hidden_comment_or_string_entry_rejected(self):
        unit="Methods_10.cpp";before,_,pin=pinned_fixture(unit)
        with patch.dict(helpers.TARGETS,{unit:pin}):
            for bad in ("/*\n"+before+"\n*/",'"\n'+before+'\n"',"'\n"+before+"\n'"):
                with self.assertRaises(ValueError):helpers.batch_complete_helpers(bad,unit)

    def test_unrelated_comments_and_includes_preserved(self):
        unit="Methods_10.cpp";before,after,pin=pinned_fixture(unit)
        prefix='#include "Game.h"\n/* an unrelated helper\'s comment */\n'
        with patch.dict(helpers.TARGETS,{unit:pin}):
            self.assertEqual(helpers.batch_complete_helpers(prefix+before,unit)[0],prefix+after)

    def test_changed_output_pin_rejected(self):
        unit="Methods_10.cpp";before,_,pin=pinned_fixture(unit)
        with patch.dict(helpers.TARGETS,{unit:pin[:3]+('0'*64,)}):
            with self.assertRaises(ValueError):helpers.batch_complete_helpers(before,unit)

    def test_other_units_untouched(self):
        source=fixture("___CHP")
        self.assertEqual(helpers.batch_complete_helpers(source,"Methods_09.cpp"),(source,[]))

    def test_later_push_arguments_remain_after_preceding_stores(self):
        result=helpers.rewrite_helper(fixture("_sub_49EAC8"),"_sub_49EAC8")
        self.assertLess(result.index('store<int32_t>(esp,saved_ecx)'),result.index('saved_esi=esi'))
        self.assertLess(result.index('store<int32_t>(esp,saved_esi)'),result.index('saved_edi=edi'))
        self.assertLess(result.index('esp -= 4;'),result.index('nfs::GuestCopyTransaction'))

    def test_option_requires_fresh_output_before_preparation(self):
        with patch.object(full,'prepare_mapped') as prepare:
            with self.assertRaises(ValueError):full.prepare_full(checked_copy_bundle=True)
            prepare.assert_not_called()
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);existing=root/'.build'/'existing';existing.mkdir(parents=True)
            with patch.object(full,'ROOT',root),patch.object(full,'prepare_mapped') as prepare:
                with self.assertRaises(FileExistsError):full.prepare_full(output_dir=existing,checked_copy_bundle=True)
                with self.assertRaises(ValueError):full.prepare_full(output_dir=root/'outside',checked_copy_bundle=True)
                prepare.assert_not_called()


if __name__ == '__main__':unittest.main()
