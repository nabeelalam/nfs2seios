import re
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from location_caption_hooks import add_location_caption_hooks, ENTRIES, MARKER


def original(name):
    symbols=list(ENTRIES[name])
    if name == "Methods_14.cpp":symbols.append("4B3EE0")
    text='#include "FunctionIds.h"\n'
    for symbol in symbols:
        n=4 if symbol=="4A5830" else 1 if symbol=="4B3EE0" else 0
        text+='Fn(void) Game::_sub_'+symbol+'()\n{\n original_before();\n'
        text+=' guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'*n
        text+=' original_after();\n}\n'
    return text


class Hooks(unittest.TestCase):
    def test_exact_reversal_and_original_store_order(self):
        for name in ENTRIES:
            src=original(name);out=add_location_caption_hooks(src,name)
            pattern=r'\n// '+MARKER+r'_BEGIN:([^\n]+)\n.*?\n// '+MARKER+r'_END:\1\n'
            reverse,n=re.subn(pattern,'',out,flags=re.S)
            self.assertEqual(reverse,src);self.assertGreater(n,1)
            self.assertEqual(out.count('guest_store<int16_t>'),src.count('guest_store<int16_t>'))
            self.assertEqual(out.count('original_before();'),src.count('original_before();'))
            self.assertEqual(out.count('original_after();'),src.count('original_after();'))
            with self.assertRaises(ValueError):add_location_caption_hooks(out,name)

    def test_missing_entries_and_changed_stores_reject(self):
        for name in ENTRIES:
            src=original(name)
            with self.assertRaises(ValueError):add_location_caption_hooks(src.replace('FunctionIds.h','Other.h'),name)
            symbol=next(iter(ENTRIES[name]))
            with self.assertRaises(ValueError):add_location_caption_hooks(src.replace('_sub_'+symbol,'_sub_000000',1),name)
        for name in ('Methods_13.cpp','Methods_14.cpp'):
            with self.assertRaises(ValueError):add_location_caption_hooks(original(name).replace('static_cast<int16_t>(bx)','static_cast<int16_t>(ax)',1),name)

    def test_descriptor_release_and_selected_rebind_remain_distinct(self):
        a=add_location_caption_hooks(original("Methods_10.cpp"),"Methods_10.cpp")
        self.assertIn("beforeDescriptorRetire(*this,uint32_t(eax));",a)
        b=add_location_caption_hooks(original("Methods_14.cpp"),"Methods_14.cpp")
        self.assertEqual(b.count("beforeDescriptorRetire(*this,uint32_t(eax));"),1)
        self.assertEqual(b.count("beforeOwnerRetire(*this,uint32_t(eax),true);"),1)

    def test_unrelated_untouched(self):
        self.assertEqual(add_location_caption_hooks('anything','Methods_02.cpp'),'anything')

if __name__=='__main__':unittest.main()
